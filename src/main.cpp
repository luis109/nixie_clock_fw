#include <Arduino.h>
#include "ServerManager.hpp"


// // Create AsyncWebServer object on port 80
// AsyncWebServer server(80);

ServerManager g_ws_manager;

// Initialize LittleFS
void initFS() {
  if (!LittleFS.begin()) {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  else{
    Serial.println("LittleFS mounted successfully");
  }
}

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);

  initFS();

  g_ws_manager.run();
}

void loop() {
  if (g_ws_manager.dev_restart()){
    delay(5000);
    ESP.restart();
  }
  // delay(5000);
}