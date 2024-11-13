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
  updateValues(float a, float b, float c, float d);

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
  // Set LED GPIO
  const int ledPin = 2;
  // Timer variables
  unsigned long previousMillis;
  const long interval = 10000;  // interval to wait for Wi-Fi connection (milliseconds)
  bool restart;
  // List of networks
  std::vector<String> m_network_list;
  // Test values
  float temperature;
  float humidity;
  float pressure;
  float gasResistance;

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
    if(var == "TEMPERATURE"){
      return String(temperature);
    }
    else if(var == "HUMIDITY"){
      return String(humidity);
    }
    else if(var == "PRESSURE"){
      return String(pressure);
    }
    else if(var == "GAS"){
      return String(gasResistance);
    }

    return String();
  }

  //! Wifi form callback function
  void 
  wifiFormCallback(AsyncWebServerRequest *request);

  // Creates list of networks dropdown
  String 
  wifiFormProcessor(const String& var)
  {
    if(var == "NETWORK_LIST") 
    {
      String options;
      for (int i = 0; i < m_network_list.size(); ++i)
        options += "<option value=\"" + m_network_list[i] + "\">" + m_network_list[i] + "</option>\n";

      return options;
    }

    return String();
  }

  //! Initialize Serial and LittleFS
  void
  initialize();

  void
  scanNetworks()
  {
    m_network_list.clear();
    int n = WiFi.scanNetworks();
    debug(String(n) + " networks found:\n");
    for (int i = 0; i < n; ++i)
    {
      m_network_list.push_back(WiFi.SSID(i));
      debug(m_network_list[i] + "\n");
    }
  }

  void
  debug(const String& str)
  {
#ifdef SERVER_MANAGER_DEBUG
      Serial.print(str);
#endif    
  }
};

#endif