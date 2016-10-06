#include <ESP8266WiFi.h>
#include <DNSServer.h> //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include "FS.h"
#include "light.h"

ESP8266WebServer server(80);
const char* www_username = "user";
const char* www_password = "pass";

bool authRequired = false;

const int numLights = 5;
Light* lights[numLights];
Light *r, *g, *b;

int interruptPin = 4;
bool door_control = false;
void setLightState() {
  if (door_control)
    lights[1]->set(digitalRead(interruptPin) == LOW);
}

void setup(void){
  analogWriteRange(255);
  analogWriteFreq(59);
  
  lights[0] = new Light(2, true, true); // built-in led
  lights[1] = new Light(15);              // Relay light
  lights[2] = r = new Light(14, true);   // LED strip red channel
  lights[3] = g = new Light(12, true);   // LED strip green channel
  lights[4] = b = new Light(13, true);   // LED strip blue channel
  
  SPIFFS.begin();
  Serial.begin(115200);
  WiFiManager wifiManager;
  wifiManager.setSTAStaticIPConfig(IPAddress(192,168,1,5), IPAddress(192,168,1,1), IPAddress(255,255,255,0));
  wifiManager.autoConnect("ESP_CONFIG"); // Will set up as AP if necessary

  // Serve control page
  server.on("/", [](){
    File file = SPIFFS.open("/index.html", "r");
    server.streamFile(file, "text/html");
    file.close();
  });

  server.on("/light/set", [](){
    if(!server.authenticate(www_username, www_password) && authRequired)
      return server.requestAuthentication();
    for (int i = 0; i < numLights; i++)
      if (server.hasArg(String(i)))
        lights[i]->set(server.arg(String(i)).toInt());
    server.send(200, "text/plain", "Changes applied");
  });

  server.on("/light/status", [](){
    server.send(200, "text/plain", String(lights[server.arg("id").toInt()]->get()));
  });

  server.on("/settings/doorsensor/enable", [](){
    if(!server.authenticate(www_username, www_password) && authRequired)
      return server.requestAuthentication();
    door_control = true;
    server.send(200, "text/plain", "enabled");
  });

  server.on("/settings/doorsensor/disable", [](){
    if(!server.authenticate(www_username, www_password) && authRequired)
      return server.requestAuthentication();
    door_control = false;
    server.send(200, "text/plain", "disabled");
  });  
  
  server.on("/sensor/brightness", [](){
    server.send(200, "text/plain", (String)analogRead(A0));
  });

  server.begin();
  Serial.println("HTTP server started");
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), setLightState, CHANGE);
}

void loop(void){
  server.handleClient();
}
