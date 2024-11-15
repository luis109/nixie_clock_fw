#ifndef SERVER_MANAGER_HPP
#define SERVER_MANAGER_HPP

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
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
    return m_restart;
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
  
  //! HTTP POST request parameters
  //! Wifi manager
  const char* WIFI_FORM_SSID_PARAM = "ssid";
  const char* WIFI_FORM_PASS_PARAM = "pass";
  const char* WIFI_FORM_IP_PARAM = "ip";
  const char* WIFI_FORM_GATEWAY_PARAM = "gateway";
  const char* WIFI_FORM_SUBNET_PARAM = "subnet";
  const char* WIFI_FORM_DNS_PARAM = "dns";
  //! Settings
  const char* TIMEZONE_FORM_TIMEZONE_PARAM = "timezone_setting";

  //! File paths to save input values permanently
  //! Wifi manager form
  const char* ssidPath = "/ssid.txt";
  const char* passPath = "/pass.txt";
  const char* ipPath = "/ip.txt";
  const char* gatewayPath = "/gateway.txt";
  const char* subnetPath = "/subnet.txt";
  const char* dnsPath = "/dns.txt";
  //! Time settings
  const char* timezonesPath = "/zones.json";
  const char* timezonePath = "/timezone.txt";
  
  // Timer variables
  const long interval = 10000;  // interval to wait for Wi-Fi connection (milliseconds)
  //! Restart ESP flag
  bool m_restart;
  //! Map of timezones
  JsonDocument m_timezones;
  String m_curr_timezone;
  String m_default_timezone = "Europe/Lisbon";
  // Time string
  String m_time_str;

  //! Read File from LittleFS
  String 
  readFile(fs::FS &fs, const char * path, bool singleLine = true);

  //! Write file to LittleFS
  void 
  writeFile(fs::FS &fs, const char * path, const char * message);

  //! Initialize WiFi
  bool 
  initWiFi();

  //! Replaces placeholder with LED state value
  String 
  mainPageProcessor(const String& var);

  //! Timezone form callback, to set timezone settings
  void
  timezoneFormCallback(AsyncWebServerRequest *request);
  
  //! Wifi form scan networks function
  void 
  wifiFormScanNetworks(AsyncWebServerRequest *request);

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

  void
  getTimezones();
};

#endif