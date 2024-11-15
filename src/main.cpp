#include <Arduino.h>
#include "ServerManager.hpp"
#include "time.h"

ServerManager g_ws_manager;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;

unsigned long lastTime = 0;  
unsigned long timerDelay = 500;  // send readings timer

String 
getLocalTimeString(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return String();
  }
  
  char time_str[64];
  strftime(time_str, 64, "%A, %B %d %Y %H:%M:%S", &timeinfo);

  return String(time_str);
}

void setup() 
{
  g_ws_manager.begin();

  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  // while (getLocalTimeString() == "")
  //   delay(5000);

  // Init web server
  g_ws_manager.updateTimeString(getLocalTimeString());
}

void loop() 
{
  if (g_ws_manager.dev_restart())
  {
    delay(5000);
    ESP.restart();
  }

  if ((millis() - lastTime) > timerDelay) 
  {
    g_ws_manager.updateTimeString(getLocalTimeString());
    g_ws_manager.run();
    
    lastTime = millis();
  }
}