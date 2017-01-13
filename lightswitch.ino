#include <ESP8266WiFi.h>
#include <DNSServer.h> // Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>
//#include <ESP8266HTTPClient.h> // TODO Verify this works without
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

const int numLights = 5;
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
  if (lights[1]->get() > 0)
    lights[1]->set(0);
  else if (lights[2] > 0 || lights[3] > 0 || lights[4] > 0) {
    lights[2]->set(0);
    lights[3]->set(0);
    lights[4]->set(0);
  } else {
    lights[1]->set(1);
  }
}

void setup(void) {
  // 3.3 Reference voltage for Logic Level Converter
  pinMode(12, OUTPUT);
  digitalWrite(12, HIGH);

  // Set up PWM
  analogWriteRange(255);
  analogWriteFreq(200);

  lights[0] = new Light(2, "Built-in Status LED", true, true);       // built-in led
  lights[1] = new Light(15, "Main Light");                           // Relay light
  lights[2] = new Light(14, "RGB LED Strip: Red Channel", true);     // LED strip red channel
  lights[3] = new Light(13, "RGB LED Strip: Green Channel", true);   // LED strip green channel
  lights[4] = new Light(16, "RGB LED Strip: Blue Channel", true);    // LED strip blue channel

  // Start Filesystem
  SPIFFS.begin();

  // Initialize Serial Connection for Debug Logging
  Serial.begin(115200);

  // Start Wifi Autoconfig
  WiFiManager wifiManager;
  wifiManager.setSTAStaticIPConfig(IPAddress(192, 168, 1, 5), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));
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
    server.send(200, "text/plain", response);
  }); 

  server.on("/light/set", []() {
    if (!server.authenticate(www_username, www_password) && authRequired)
      return server.requestAuthentication();
    for (int i = 0; i < numLights; i++)
      if (server.hasArg(String(i)))
        lights[i]->set(server.arg(String(i)).toInt());
    server.send(200, "text/plain", "Changes applied");
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
  //Serial.println("HTTP server started");

  //  // Door handler
  //  pinMode(door_pin, INPUT_PULLUP);
  //  attachInterrupt(digitalPinToInterrupt(door_pin), setLightStateOnDoorAction, CHANGE);

  //  // Manual button handler
  //  pinMode(button_pin, INPUT_PULLUP);
  //  attachInterrupt(digitalPinToInterrupt(button_pin), toggleLightOnButtonPress, RISING);

  //  // Online disable switch
  //  pinMode(enable_switch_pin, INPUT_PULLUP);
}

void loop(void) {  
  server.handleClient();
}
