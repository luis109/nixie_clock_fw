#include <Arduino.h>
#include "ServerManager.hpp"


// // Create AsyncWebServer object on port 80
// AsyncWebServer server(80);

ServerManager g_ws_manager;

void setup() {
  g_ws_manager.run();
}

void loop() {
  if (g_ws_manager.dev_restart()){
    delay(5000);
    ESP.restart();
  }
  // delay(5000);
}