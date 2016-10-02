#include <ESP8266WiFi.h>
#include <DNSServer.h> //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include "FS.h"
//#include "led_light.h"
#include "overhead_light.h"

ESP8266WebServer server(80);

const char* www_username = "user";
const char* www_password = "pass";

OverheadLight light;

void setup(void){  
  SPIFFS.begin();
  Serial.begin(115200);
  WiFiManager wifiManager;
  wifiManager.setSTAStaticIPConfig(IPAddress(192,168,1,5), IPAddress(192,168,1,1), IPAddress(255,255,255,0));
  wifiManager.autoConnect("ESP_CONFIG"); // Will set up as AP if necessary

  server.on("/", [](){
      File file = SPIFFS.open("/index.html", "r");
      server.streamFile(file, "text/html");
      file.close();
    });

  server.on("/on", [](){
    if(!server.authenticate(www_username, www_password))
      return server.requestAuthentication();
    light.set(true);
    server.send(200, "text/plain", "on");
  });
  
  server.on("/off", [](){
    if(!server.authenticate(www_username, www_password))
      return server.requestAuthentication();
    light.set(false);
    server.send(200, "text/plain", "off");
  });

  server.on("/status", [](){
    server.send(200, "text/plain", light.get() ? "on" : "off");
  });

  server.on("/brightness", [](){
    server.send(200, "text/plain", (String)analogRead(A0));
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void){
  server.handleClient();
}
