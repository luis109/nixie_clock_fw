#include <Arduino.h>
#include "ServerManager.hpp"

ServerManager g_ws_manager;

unsigned long lastTime = 0;  
unsigned long timerDelay = 500;  // send readings timer

float var = 0;

void setup() 
{

  g_ws_manager.begin();
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
    var++;
    g_ws_manager.updateValues(var, var, var, var);
    g_ws_manager.run();
    
    lastTime = millis();
  }
}