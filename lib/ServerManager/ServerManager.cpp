#include "ServerManager.hpp"

ServerManager::ServerManager():
  server(new AsyncWebServer(80)),
  events(new AsyncEventSource("/events")),
  subnet(IPAddress(255, 255, 0, 0)),
  dns(IPAddress(8, 8, 8, 8)),
  restart(false),
  m_time_str(String())
{ }

ServerManager::~ServerManager()
{
  delete events;
  delete server;
}

void 
ServerManager::begin()
{
  initialize();

  if(initWiFi())
  {
    // Route for root / web page
    server->on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
      request->send(LittleFS, "/index.html", "text/html", false, std::bind(&ServerManager::mainPageProcessor, this, std::placeholders::_1));
    });
    server->serveStatic("/", LittleFS, "/");
    
    // Handle Web Server Events
    events->onConnect([this](AsyncEventSourceClient *client){
      if(client->lastId()){
        debug("Client reconnected! Last message ID that it got is:" + String(client->lastId())+ "\n");
      }
      // send event with message "hello!", id current millis
      // and set reconnect delay to 1 second
      client->send("hello!", NULL, millis(), 10000);
    });
    
    server->addHandler(events);
    server->begin();
    debug("Server started\n"); 
  }
  else 
  {
    // Connect to Wi-Fi network with SSID and password
    debug("Setting AP (Access Point)\n");
    WiFi.softAP("ESP-WIFI-MANAGER", NULL);

    IPAddress IP = WiFi.softAPIP();
    debug("AP IP address: " + IP.toString() + "\n");
    debug("**Start Network Scan**\n");
    WiFi.scanNetworks(true);

    // Web Server Root URL
    server->on("/", HTTP_GET, [this](AsyncWebServerRequest *request){
      request->send(LittleFS, "/wifimanager.html", "text/html");
    });
    server->serveStatic("/", LittleFS, "/");
    server->on("/scan", HTTP_GET, std::bind(&ServerManager::wifiFormScanNetworks, this, std::placeholders::_1));
    server->on("/", HTTP_POST, std::bind(&ServerManager::wifiFormCallback, this, std::placeholders::_1));
    server->begin();
  }
}

void 
ServerManager::run()
{
  // Send Events to the Web Server with the Sensor Readings
  events->send("ping", NULL, millis());
  events->send(m_time_str.c_str(), "curr_time", millis());
}

void
ServerManager::initialize()
{
  // Serial port for debugging purposes
#ifdef SERVER_MANAGER_DEBUG
  Serial.begin(115200);
#endif

  // Initialize LittleFS
  if (!LittleFS.begin())
    debug("An error has occurred while mounting LittleFS\n");
  else
    debug("LittleFS mounted successfully\n");

  // Get timezone codes
  getTimezones();
}

String 
ServerManager::readFile(fs::FS &fs, const char * path, bool singleLine)
{
  debug("Reading file: " + String(path) + "\r\n");
  File file = fs.open(path, "r");
  if(!file || file.isDirectory()){
    debug("- failed to open file for reading");
    return String();
  }

  String fileContent;
  while(file.available()){
    if (singleLine)
      fileContent = file.readStringUntil('\n');
    else
      fileContent = file.readString();
    break;
  }
  file.close();
  return fileContent;
}

void 
ServerManager::writeFile(fs::FS &fs, const char * path, const char * message)
{
  debug("Writing file: " + String(path) + "\r\n");

  File file = fs.open(path, "w");
  if(!file)
  {
    debug("- failed to open file for writing");
    return;
  }
  if(file.print(message))
    debug("- file written");
  else
    debug("- frite failed");
  file.close();
}

bool 
ServerManager::initWiFi()
{
  // Load values saved in LittleFS
  String ssid = readFile(LittleFS, ssidPath);
  String pass = readFile(LittleFS, passPath);
  String ip = readFile(LittleFS, ipPath);
  String gateway = readFile (LittleFS, gatewayPath);

  debug("SSID: " + ssid + "\n");
  debug("Password: " + pass + "\n");
  debug("IP: " + ip + "\n");
  debug("Gateway: " + gateway + "\n");

  if(ssid=="" || ip==""){
    debug("Undefined SSID or IP address.");
    return false;
  }

  WiFi.mode(WIFI_STA);
  localIP.fromString(ip.c_str());
  localGateway.fromString(gateway.c_str());

  if (!WiFi.config(localIP, localGateway, subnet, dns)){
    debug("STA Failed to configure");
    return false;
  }
  WiFi.begin(ssid.c_str(), pass.c_str());

  debug("Connecting to WiFi...");
  delay(interval);
  if(WiFi.status() != WL_CONNECTED) {
    debug("Failed to connect.");
    return false;
  }

  debug(WiFi.localIP().toString());
  return true;
}

String 
ServerManager::mainPageProcessor(const String& var)
{
  if(var == "CURR_TIME")
    return m_time_str;

  return String();
}

void 
ServerManager::wifiFormScanNetworks(AsyncWebServerRequest *request)
{
  String json = "[";
  int n = WiFi.scanComplete();
  if(n == -2){
    WiFi.scanNetworks(true);
  } else if(n){
    for (int i = 0; i < n; ++i){
      if(i) json += ",";
      json += "{";
      json += "\"rssi\":"+String(WiFi.RSSI(i));
      json += ",\"ssid\":\""+WiFi.SSID(i)+"\"";
      json += ",\"bssid\":\""+WiFi.BSSIDstr(i)+"\"";
      json += ",\"channel\":"+String(WiFi.channel(i));
      json += ",\"secure\":"+String(WiFi.encryptionType(i));
      json += "}";
    }
    WiFi.scanDelete();
    if(WiFi.scanComplete() == -2){
      WiFi.scanNetworks(true);
    }
  }
  json += "]";
  request->send(200, "application/json", json);
  json = String();
}

void 
ServerManager::wifiFormCallback(AsyncWebServerRequest *request)
{
  int params = request->params();
  String ssid;
  String pass;
  String ip;
  String gateway;

  for(int i=0;i<params;i++){
    AsyncWebParameter* p = request->getParam(i);
    if(p->isPost()){
      // HTTP POST ssid value
      if (p->name() == WIFI_FORM_SSID_PARAM) 
      {
        ssid = p->value().c_str();
        // Write file to save value
        writeFile(LittleFS, ssidPath, ssid.c_str());
        debug("SSID set to: " + ssid + "\n");
      }
      // HTTP POST pass value
      if (p->name() == WIFI_FORM_PASS_PARAM) 
      {
        pass = p->value().c_str();
        // Write file to save value
        writeFile(LittleFS, passPath, pass.c_str());
        debug("Password set to: " + pass + "\n");
      }
      // HTTP POST ip value
      if (p->name() == WIFI_FORM_IP_PARAM) 
      {
        ip = p->value().c_str();
        // Write file to save value
        writeFile(LittleFS, ipPath, ip.c_str());
        debug("IP Address set to: " + ip + "\n");
      }
      // HTTP POST gateway value
      if (p->name() == WIFI_FORM_GATEWAY_PARAM) 
      {
        gateway = p->value().c_str();
        // Write file to save value
        writeFile(LittleFS, gatewayPath, gateway.c_str());
        debug("Gateway set to: " + gateway + "\n");
      }
      debug("POST[" + p->name() + "]: " + p->value() + "\n");
    }
  }
  restart = true;
  request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
}


void 
ServerManager::getTimezones()
{
  String timezones_json = readFile(LittleFS, timezonePath, false);
  DeserializationError error = deserializeJson(m_timezones, timezones_json);

  if (error)
    debug("Error deserializing timezone json: " + String(error.c_str()) + "\n");

  // for (JsonPair kv : m_timezones.as<JsonObject>())
  //   debug("[" + String(kv.key().c_str()) + "] = " + kv.value().as<String>() + "\n");
}