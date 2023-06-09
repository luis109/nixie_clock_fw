#ifndef SERVER_MANAGER_HPP
#define SERVER_MANAGER_HPP

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include "ServerManagerDefinitions.hpp"

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

//Variables to save values from HTML form
String ssid;
String pass;
String ip;
String gateway;

// Set the device's IP address
IPAddress localIP;

// Set your Gateway IP address
IPAddress localGateway;
IPAddress subnet(255, 255, 0, 0);
IPAddress dns(8, 8, 8, 8);

// Stores LED state
String ledState;

// Restart ESP flag
boolean restart = false;

// Read File from LittleFS
String 
readFile(fs::FS &fs, const char * path)
{
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path, "r");
  if(!file || file.isDirectory()){
    Serial.println("- failed to open file for reading");
    return String();
  }

  String fileContent;
  while(file.available()){
    fileContent = file.readStringUntil('\n');
    break;
  }
  file.close();
  return fileContent;
}

// Write file to LittleFS
void 
writeFile(fs::FS &fs, const char * path, const char * message)
{
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, "w");
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- frite failed");
  }
  file.close();
}

// Initialize WiFi
bool 
initWiFi() 
{
  if(ssid=="" || ip==""){
    Serial.println("Undefined SSID or IP address.");
    return false;
  }

  WiFi.mode(WIFI_STA);
  localIP.fromString(ip.c_str());
  localGateway.fromString(gateway.c_str());

  if (!WiFi.config(localIP, localGateway, subnet, dns)){
    Serial.println("STA Failed to configure");
    return false;
  }
  WiFi.begin(ssid.c_str(), pass.c_str());

  Serial.println("Connecting to WiFi...");
  delay(SM_WIFI_TIMEOUT);
  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("Failed to connect.");
    return false;
  }

  Serial.println(WiFi.localIP());
  return true;
}

// Replaces placeholder with LED state value
String 
processor(const String& var)
{
  if(var == "STATE") {
    if(!digitalRead(SM_LED_PIN)) {
      ledState = "ON";
    }
    else {
      ledState = "OFF";
    }
    return ledState;
  }
  return String();
}

void
webStateMachine()
{
  // Set GPIO 2 as an OUTPUT
  pinMode(SM_LED_PIN, OUTPUT);
  digitalWrite(SM_LED_PIN, LOW);

  // Load values saved in LittleFS
  ssid = readFile(LittleFS, SM_PATH_SSID);
  pass = readFile(LittleFS, SM_PATH_PASS);
  ip = readFile(LittleFS, SM_PATH_IP);
  gateway = readFile (LittleFS, SM_PATH_GATEWAY);

#ifdef SERVER_MANAGER_DEBUG
  Serial.println(ssid);
  Serial.println(pass);
  Serial.println(ip);
  Serial.println(gateway);
#endif


  if(initWiFi()) {
    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) 
    {
      request->send(LittleFS, "/index.html", "text/html", false, processor);
    });
    
    server.serveStatic("/", LittleFS, "/");
    
    // Route to set GPIO state to HIGH
    server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request) 
    {
      digitalWrite(SM_LED_PIN, LOW);
      request->send(LittleFS, "/index.html", "text/html", false, processor);
    });

    // Route to set GPIO state to LOW
    server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request) 
    {
      digitalWrite(SM_LED_PIN, HIGH);
      request->send(LittleFS, "/index.html", "text/html", false, processor);
    });
    server.begin();
  }
  else 
  {
    // Connect to Wi-Fi network with SSID and password
#ifdef SERVER_MANAGER_DEBUG
    Serial.println("Setting AP (Access Point)");
#endif
    // NULL sets an open Access Point
    WiFi.softAP("ESP-WIFI-MANAGER", NULL);

    IPAddress IP = WiFi.softAPIP();
#ifdef SERVER_MANAGER_DEBUG
    Serial.print("AP IP address: ");
    Serial.println(IP);
#endif

    // Web Server Root URL
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(LittleFS, "/wifimanager.html", "text/html");
    });
    
    server.serveStatic("/", LittleFS, "/");
    
    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
      int params = request->params();
      for(int i=0;i<params;i++){
        AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
          // HTTP POST ssid value
          if (p->name() == SM_PARAM_SSID) {
            ssid = p->value().c_str();
            // Write file to save value
            writeFile(LittleFS, SM_PATH_SSID, ssid.c_str());
          }
          // HTTP POST pass value
          if (p->name() == SM_PARAM_PASS) {
            pass = p->value().c_str();
            // Write file to save value
            writeFile(LittleFS, SM_PATH_PASS, pass.c_str());
          }
          // HTTP POST ip value
          if (p->name() == SM_PARAM_IP) {
            ip = p->value().c_str();
            // Write file to save value
            writeFile(LittleFS, SM_PATH_IP, ip.c_str());
          }
          // HTTP POST gateway value
          if (p->name() == SM_PARAM_GATEWAY) {
            gateway = p->value().c_str();
            // Write file to save value
            writeFile(LittleFS, SM_PATH_GATEWAY, gateway.c_str());
          }
#ifdef SERVER_MANAGER_DEBUG
          Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
#endif
        }
      }
      restart = true;
      request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
    });
    server.begin();
  }
}

#endif