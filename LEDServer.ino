#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "FS.h"
//#include "led_light.h"
#include "overhead_light.h"

const char* ssid = "SWIFT";
const char* password = "tempswift";
IPAddress ip(192,168,1,5);  //Node static IP
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

ESP8266WebServer server(80);

const char* www_username = "user";
const char* www_password = "pass";

OverheadLight light;

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup(void){  
  SPIFFS.begin();
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  WiFi.config(ip, gateway, subnet);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

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

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void){
  server.handleClient();
}
