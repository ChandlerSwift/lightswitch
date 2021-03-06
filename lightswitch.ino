// Enables serial printing of debug logs. Comment to disable:
#define DEBUG 1

#ifdef DEBUG
 #define DEBUG_PRINT(x)  Serial.println (x)
#else
 #define DEBUG_PRINT(x)
#endif

#include <ESP8266WiFi.h>
#include <DNSServer.h> // Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include "FS.h"
#include "light.h"

/* 
 *  GPIO Assignments:
 *  0:  Enable/Disable web control switch
 *  2:  Built-in status LED
 *  4:  Door switch sensor
 *  5:  Manual control button
 *  12: Always-on voltage reference
 *  13: LED green channel
 *  14: LED red channel
 *  15: Main light
 *  16: LED blue channel
 */

ESP8266WebServer server(80);
const char* www_username = "user";
const char* www_password = "pass";

bool authRequired = false;

const int numLights = 4;
Light* lights[numLights];

// int enable_switch_pin = 0;

int door_pin = 4;
bool door_control = false;
void setLightStateOnDoorAction() {
  if (door_control)
    lights[1]->set(digitalRead(door_pin) == LOW);
}

int button_pin = 5;

void toggleLightOnButtonPress() {
  /*
   * If the main light is on, turns off
   * If the main light is not on but the LEDs are, turns off LEDs
   * If no lights are on, turns on main light
   */
  if (lights[0]->get() > 0)
    lights[0]->set(0);
  else if (lights[1] > 0 || lights[2] > 0 || lights[3] > 0) {
    lights[1]->set(0);
    lights[2]->set(0);
    lights[3]->set(0);
  } else {
    lights[0]->set(1);
  }
}

int status_led_pin = 2;
int enable_switch_pin = 0;
void set_enable_light() { // Called by interrupt on pin 0
  digitalWrite(status_led_pin, !(digitalRead(enable_switch_pin) == LOW)); // Set status light to switch state, inverted because the LED is active LOW
}

bool enable_switch_is_off() {
  return digitalRead(enable_switch_pin) == HIGH;
}

void setup(void) {
  // 3.3 Reference voltage for Logic Level Converter
  pinMode(12, OUTPUT);
  digitalWrite(12, HIGH);

  // Indicator Light for switch
  pinMode(status_led_pin, OUTPUT);
  digitalWrite(status_led_pin, digitalRead(enable_switch_pin)); // set initial value on startup

  // Set up PWM
  analogWriteRange(255);
  analogWriteFreq(200);

  lights[0] = new Light(15, "Main Light");                           // Relay light
  lights[1] = new Light(14, "RGB LED Strip: Red Channel", true);     // LED strip red channel
  lights[2] = new Light(13, "RGB LED Strip: Green Channel", true);   // LED strip green channel
  lights[3] = new Light(16, "RGB LED Strip: Blue Channel", true);    // LED strip blue channel

  // Start Filesystem
  SPIFFS.begin();

  // Initialize Serial Connection for Debug Logging
  Serial.begin(115200);

  // Start Wifi Autoconfig
  WiFiManager wifiManager;
  wifiManager.setSTAStaticIPConfig(IPAddress(192, 168, 1, 10), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));
  wifiManager.autoConnect("ESP_CONFIG"); // Will set up as AP if necessary

  ///////////////////////////////////////
  // Page handlers
  ///////////////////////////////////////

  // Serve control page
  server.on("/", []() {
    File file = SPIFFS.open("/index.html", "r");
    server.streamFile(file, "text/html");
    file.close();
  });

  // Serve API Reference page
  server.on("/api.html", []() {
    File file = SPIFFS.open("/api.html", "r");
    server.streamFile(file, "text/html");
    file.close();
  });
  
  // Serve JS Implementation Page
  server.on("/implementation.html", []() {
    File file = SPIFFS.open("/implementation.html", "r");
    server.streamFile(file, "text/html");
    file.close();
  });

  // Build and return JSON string with information about each light
  server.on("/lights", []() {
    String response = "[";
    for (int i = 0; i < numLights; i++)
      response += String(i ? "," : "") + // comma if not first entry
                  "{\"status\":" +
                  lights[i]->get() +
                  ",\"dimmable\":" +
                  (lights[i]->isDimmable() ? "1" : "0") +
                  ",\"description\":\"" +
                  lights[i]->getDescription() + "\"}"; // comma all but last time
    response += "]";
    server.send(200, "application/json", response);
  }); 

  server.on("/light/set", []() {
    if (enable_switch_is_off()) {
      server.send(503, "text/plain", "The controller has been switched offline.");
    } else {
      if (!server.authenticate(www_username, www_password) && authRequired)
        return server.requestAuthentication();
      for (int i = 0; i < numLights; i++)
        if (server.hasArg(String(i)))
          lights[i]->set(server.arg(String(i)).toInt());
      server.send(200, "text/plain", "Changes applied");
    }
  });

  server.on("/light/status", []() {
    server.send(200, "text/plain", String(lights[server.arg("id").toInt()]->get()));
  });

  server.on("/settings/doorsensor/enable", []() {
    if (!server.authenticate(www_username, www_password) && authRequired)
      return server.requestAuthentication();
    door_control = true;
    server.send(200, "text/plain", (String)door_control);
  });

  server.on("/settings/doorsensor/disable", []() {
    if (!server.authenticate(www_username, www_password) && authRequired)
      return server.requestAuthentication();
    door_control = false;
    server.send(200, "text/plain", (String)door_control);
  });

  // Left in for compatibility with old scripts, remove in v2?
  server.on("/settings/doorsensor", []() {
    server.send(200, "text/plain", (String)door_control);
  });

  server.on("/sensor/door", []() {
    server.send(200, "text/plain", (String)door_control);
  });

  server.on("/sensor/brightness", []() {
    server.send(200, "text/plain", (String)analogRead(A0));
  });

  server.on("/sensor/door", []() {
    server.send(200, "text/plain", (String)digitalRead(door_pin));
  });

  // Set up server
  server.begin();
  DEBUG_PRINT("HTTP server started");

  //  // Door handler
  //  pinMode(door_pin, INPUT_PULLUP);
  //  attachInterrupt(digitalPinToInterrupt(door_pin), setLightStateOnDoorAction, CHANGE);

  // Manual button handler
  pinMode(button_pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(button_pin), toggleLightOnButtonPress, RISING);

  // Web Control Disable Switch
  pinMode(enable_switch_pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(enable_switch_pin), set_enable_light, CHANGE);

}

void loop(void) {  
  server.handleClient();
}
