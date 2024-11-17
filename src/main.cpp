#include <Arduino.h>
#include "ServerManager.hpp"
#include "DisplayDriver.hpp"

ServerManager g_ws_manager;
DisplayDriver g_ddriver;

unsigned long lastTime = 0;  
unsigned long timerDelay = 500;  // send readings timer

// String 
// getLocalTimeString(){
//   struct tm timeinfo;
//   if (!g_ws_manager.getInternetTime(&timeinfo))
//     return String();
  
//   char time_str[64];
//   strftime(time_str, 64, "%A, %B %d %Y %H:%M:%S", &timeinfo);
//   return String(time_str);
// }

void setup() 
{
  // Init web server
  g_ws_manager.begin();
  g_ws_manager.updateTimeString(g_ws_manager.getInternetTimeStr());

  g_ddriver.begin();
  g_ddriver.setDisplay(12, 34, 56);
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
    g_ws_manager.updateTimeString(g_ws_manager.getInternetTimeStr());
    g_ws_manager.run();
    
    lastTime = millis();
  }

  g_ddriver.run();
}