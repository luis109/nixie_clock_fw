#ifndef SERVER_MANAGER_HPP
#define SERVER_MANAGER_HPP

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

class ServerManager
{
  public:
    ServerManager();

  private:
};

#endif