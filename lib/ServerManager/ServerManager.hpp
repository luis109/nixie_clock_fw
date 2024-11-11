#ifndef SERVER_MANAGER_HPP
#define SERVER_MANAGER_HPP

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <functional>

// Create AsyncWebServer object on port 80
// AsyncWebServer server(80);
class ServerManager
{
public:
    ServerManager();
    ~ServerManager();

  // Run webserver (webserver main)
  void 
  run();

  bool
  dev_restart()
  {
    return restart;
  }

private:
  AsyncWebServer* server;
  
  // Search for parameter in HTTP POST request
  const char* PARAM_INPUT_1 = "ssid";
  const char* PARAM_INPUT_2 = "pass";
  const char* PARAM_INPUT_3 = "ip";
  const char* PARAM_INPUT_4 = "gateway";

  //Variables to save values from HTML form
  String ssid;
  String pass;
  String ip;
  String gateway;

  // File paths to save input values permanently
  const char* ssidPath = "/ssid.txt";
  const char* passPath = "/pass.txt";
  const char* ipPath = "/ip.txt";
  const char* gatewayPath = "/gateway.txt";
  IPAddress localIP;
  // Set your Gateway IP address
  IPAddress localGateway;
  IPAddress subnet;
  // Set LED GPIO
  const int ledPin = 2;
  // Timer variables
  unsigned long previousMillis;
  const long interval = 10000;  // interval to wait for Wi-Fi connection (milliseconds)
  bool restart;

  // Read File from LittleFS
  String 
  readFile(fs::FS &fs, const char * path);

  // Write file to LittleFS
  void 
  writeFile(fs::FS &fs, const char * path, const char * message);

  // Initialize WiFi
  bool 
  initWiFi();

  // Replaces placeholder with LED state value
  String 
  processor(const String& var)
  {
    if(var == "STATE") {
      String ledState;
        if(!digitalRead(ledPin)) {
          ledState = "ON";
        }
        else {
          ledState = "OFF";
        }
        return ledState;
      }
      return String();
  }

  //! Wifi form callback function
  void 
  wifiFormCallback(AsyncWebServerRequest *request);
};

#endif