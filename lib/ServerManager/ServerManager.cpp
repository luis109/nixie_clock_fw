#include "ServerManager.hpp"

ServerManager::ServerManager():
  server(new AsyncWebServer(80)),
  events(new AsyncEventSource("/events")),
  m_restart(false),
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

    server->on("/", HTTP_POST, std::bind(&ServerManager::timezoneFormCallback, this, std::placeholders::_1));
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
  // Set timezone
  m_curr_timezone = readFile(LittleFS, timezonePath);
  if (m_curr_timezone.isEmpty())
    m_curr_timezone = m_default_timezone;
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
  String gateway = readFile(LittleFS, gatewayPath);
  String subnet = readFile(LittleFS, subnetPath);
  String dns = readFile(LittleFS, dnsPath);

  debug("SSID: " + ssid + "\n");
  debug("Password: " + pass + "\n");
  debug("IP: " + ip + "\n");
  debug("Gateway: " + gateway + "\n");
  debug("Subnet mask: " + subnet + "\n");
  debug("DNS: " + dns + "\n");

  if(ssid=="" || ip==""){
    debug("Undefined SSID or IP address.");
    return false;
  }

  WiFi.mode(WIFI_STA);
  IPAddress localIP;
  localIP.fromString(ip.c_str());
  IPAddress localGateway;
  localGateway.fromString(gateway.c_str());
  IPAddress localSubnet; 
  localSubnet.fromString(subnet);
  IPAddress localDNS;
  localDNS.fromString(dns);

  if (!WiFi.config(localIP, localGateway, localSubnet, localDNS)){
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

  if (var == "TIMEZONES")
  {
    String options;
    options.clear();

    if (!m_curr_timezone.isEmpty())
      options += "<option value=\"" + m_curr_timezone + "\">" + m_curr_timezone + "</option>\n";
    for (JsonPair kv : m_timezones.as<JsonObject>())
    {
      if (String(kv.key().c_str()) == m_curr_timezone)
        continue;
      options += "<option value=\"" + String(kv.key().c_str()) + "\">" + String(kv.key().c_str()) + "</option>\n";
    }
    return options;
  }

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

  String ip;
  for(int i=0;i<params;i++){
    AsyncWebParameter* p = request->getParam(i);
    if(p->isPost()){
      // HTTP POST ssid value
      if (p->name() == WIFI_FORM_SSID_PARAM) 
      {
        writeFile(LittleFS, ssidPath, p->value().c_str());
        debug("SSID set to: " + String(p->value().c_str()) + "\n");
      }
      // HTTP POST pass value
      if (p->name() == WIFI_FORM_PASS_PARAM) 
      {
        writeFile(LittleFS, passPath, p->value().c_str());
        debug("Password set to: " + String(p->value().c_str()) + "\n");
      }
      // HTTP POST ip value
      if (p->name() == WIFI_FORM_IP_PARAM) 
      {
        ip = p->value().c_str();
        writeFile(LittleFS, ipPath, p->value().c_str());
        debug("IP Address set to: " + String(p->value().c_str()) + "\n");
      }
      // HTTP POST gateway value
      if (p->name() == WIFI_FORM_GATEWAY_PARAM) 
      {
        writeFile(LittleFS, gatewayPath, p->value().c_str());
        debug("Gateway set to: " + String(p->value().c_str()) + "\n");
      }
      // HTTP POST subnet value
      if (p->name() == WIFI_FORM_SUBNET_PARAM) 
      {
        writeFile(LittleFS, subnetPath, p->value().c_str());
        debug("Subnet set to: " + String(p->value().c_str()) + "\n");
      }
      // HTTP POST subnet value
      if (p->name() == WIFI_FORM_DNS_PARAM) 
      {
        writeFile(LittleFS, dnsPath, p->value().c_str());
        debug("DNS set to: " + String(p->value().c_str()) + "\n");
      }
      debug("POST[" + p->name() + "]: " + p->value() + "\n");
    }
  }
  m_restart = true;
  request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
}

void 
ServerManager::timezoneFormCallback(AsyncWebServerRequest *request)
{
  int params = request->params();

  for(int i=0;i<params;i++)
  {
    AsyncWebParameter* p = request->getParam(i);
    if(p->isPost()){
      // HTTP POST ssid value
      if (p->name() == TIMEZONE_FORM_TIMEZONE_PARAM) 
      {
        String key = p->value().c_str();
        m_curr_timezone = key;
        debug("Zone: " + key + "\n");
        debug("Code: " + m_timezones[key].as<String>() + "\n");
      }
      debug("POST[" + p->name() + "]: " + p->value() + "\n");
    }
  }
  request->send(LittleFS, "/index.html", "text/html", false, std::bind(&ServerManager::mainPageProcessor, this, std::placeholders::_1));
}


void 
ServerManager::getTimezones()
{
  String timezones_json = readFile(LittleFS, timezonesPath, false);
  DeserializationError error = deserializeJson(m_timezones, timezones_json);

  if (error)
    debug("Error deserializing timezone json: " + String(error.c_str()) + "\n");

  // for (JsonPair kv : m_timezones.as<JsonObject>())
  //   debug("[" + String(kv.key().c_str()) + "] = " + kv.value().as<String>() + "\n");
}