#include <Arduino.h>
#include "ServerManager.hpp"
#include "DisplayDriver.hpp"

ServerManager g_ws_manager;
DisplayDriver g_ddriver;

void setup() 
{
  // Init web server
  g_ws_manager.begin();


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

  g_ws_manager.run();
  g_ddriver.run();
}