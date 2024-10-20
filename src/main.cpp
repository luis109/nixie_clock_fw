#include <Arduino.h>
#include "DisplayDriver.hpp"
// #include "ServerManager.hpp"

DisplayDriver g_ddriver;


// Initialize LittleFS
// void initFS() {
//   if (!LittleFS.begin()) {
//     Serial.println("An error has occurred while mounting LittleFS");
//   }
//   else{
//     Serial.println("LittleFS mounted successfully");
//   }
// }

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);

  // initFS();

  // webStateMachine();

  // Init and get the time
  // configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // Init display driver
  g_ddriver.begin();
}

void loop() {
  // if (restart){
  //   delay(5000);
  //   ESP.restart();
  // }
}