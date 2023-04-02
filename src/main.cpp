#include <Arduino.h>
#include "ServerManager.hpp"

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;

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

  // Set GPIO 2 as an OUTPUT
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  webStateMachine();

  // Init and get the time
  // configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {
  if (restart){
    delay(5000);
    ESP.restart();
  }
}