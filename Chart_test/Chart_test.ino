

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "FS.h"
#include "DHT.h"
#define DHTPIN 2
#define DHTTYPE DHT11
int result=0;
/* Set these to your desired credentials. */
const char *ssid = "CaptoBox";
const char *password = "1234567890";

ESP8266WebServer server(80);


void setup() {
  delay(1000);
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, HIGH);
  Serial.begin(115200);
  Serial.println();
  Serial.print("Configuration en point d'acces...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(ssid);

  SPIFFS.begin();
  Dir dir = SPIFFS.openDir("/");

  serveur();
  server.begin();

}


void loop() {
  server.handleClient();
}

void serveur() {

  server.on("/", HTTP_GET, []() {
    if (!handleFileRead("/index.htm")) server.send(404, "text/plain", "FileNotFound");
  });

  server.on("/yolo", HTTP_GET, []() {
   // DHT dht(DHTPIN, DHTTYPE);
   // dht.begin();
    result = analogRead(A0);
    Serial.println(result);
  });

  server.on("/all", HTTP_GET, []() {
    digitalWrite(BUILTIN_LED, LOW);
    String json = "{";
    json += "\"heap\":" + String(ESP.getFreeHeap());
    json += ", \"analog\":" + String(analogRead(A0));
    json += ", \"gpio\":" + String((uint32_t)(((GPI | GPO) & 0xFFFF) | ((GP16I & 0x01) << 16)));
    json += "}";
    server.send(200, "text/json", json);
    json = String();
    digitalWrite(BUILTIN_LED, HIGH);
  });

  server.onNotFound([]() {
    if (!handleFileRead(server.uri()))
      server.send(404, "text/plain", "FileNotFound");
  });
}



String getContentType(String filename) {
  if (server.hasArg("download")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path) {
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.htm";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}
