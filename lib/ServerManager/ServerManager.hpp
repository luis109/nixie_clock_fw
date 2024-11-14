#ifndef SERVER_MANAGER_HPP
#define SERVER_MANAGER_HPP

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <functional>

#define SERVER_MANAGER_DEBUG
class ServerManager
{
public:
    ServerManager();
    ~ServerManager();

  // Begin webserver
  void 
  begin();

  // Handle events
  void 
  run();

  bool
  dev_restart()
  {
    return restart;
  }

  // Update variable values
  void
  updateTimeString(const String& time_str)
  {
    m_time_str = time_str;
  }

private:
  AsyncWebServer* server;
  AsyncEventSource* events;
  
  // Search for parameter in HTTP POST request
  const char* WIFI_FORM_SSID_PARAM = "ssid";
  const char* WIFI_FORM_PASS_PARAM = "pass";
  const char* WIFI_FORM_IP_PARAM = "ip";
  const char* WIFI_FORM_GATEWAY_PARAM = "gateway";

  // File paths to save input values permanently
  const char* ssidPath = "/ssid.txt";
  const char* passPath = "/pass.txt";
  const char* ipPath = "/ip.txt";
  const char* gatewayPath = "/gateway.txt";
  IPAddress localIP;
  // Set your Gateway IP address
  IPAddress localGateway;
  IPAddress subnet;
  IPAddress dns;
  // Timer variables
  const long interval = 10000;  // interval to wait for Wi-Fi connection (milliseconds)
  bool restart;
  // List of networks
  std::vector<String> m_network_list;
  // Time string
  String m_time_str;

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
    if(var == "CURR_TIME")
      return m_time_str;

    return String();
  }

  //! Wifi form callback function
  void 
  wifiFormCallback(AsyncWebServerRequest *request);

  //! Initialize Serial and LittleFS
  void
  initialize();

  void
  debug(const String& str)
  {
#ifdef SERVER_MANAGER_DEBUG
      Serial.print(str);
#endif    
  }
};

#endif