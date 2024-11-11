#include "ServerManager.hpp"

ServerManager::ServerManager():
  server(new AsyncWebServer(80)),
  subnet(IPAddress(255, 255, 0, 0)),
  restart(false),
  previousMillis(0)
{ }

ServerManager::~ServerManager()
{
  delete server;
}

void 
ServerManager::run()
{
  // Set GPIO 2 as an OUTPUT
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  initialize();

  if(initWiFi())
  {
    // Route for root / web page
    server->on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
      request->send(LittleFS, "/index.html", "text/html", false, std::bind(&ServerManager::processor, this, std::placeholders::_1));
    });
    server->serveStatic("/", LittleFS, "/");
    
    // Route to set GPIO state to HIGH
    server->on("/on", HTTP_GET, [this](AsyncWebServerRequest *request) {
      digitalWrite(ledPin, LOW);
      request->send(LittleFS, "/index.html", "text/html", false, std::bind(&ServerManager::processor, this, std::placeholders::_1));
    });

    // Route to set GPIO state to LOW
    server->on("/off", HTTP_GET, [this](AsyncWebServerRequest *request) {
      digitalWrite(ledPin, HIGH);
      request->send(LittleFS, "/index.html", "text/html", false, std::bind(&ServerManager::processor, this, std::placeholders::_1));
    });
    server->begin();
    debug("Server started"); 
  }
  else 
  {
    // Connect to Wi-Fi network with SSID and password
    debug("Setting AP (Access Point)");
    // NULL sets an open Access Point
    WiFi.softAP("ESP-WIFI-MANAGER", NULL);

    IPAddress IP = WiFi.softAPIP();
    debug("AP IP address: " + IP.toString() + "\n");

    // Web Server Root URL
    server->on("/", HTTP_GET, [this](AsyncWebServerRequest *request){
      request->send(LittleFS, "/wifimanager.html", "text/html");
    });
    
    server->serveStatic("/", LittleFS, "/");

    server->on("/", HTTP_POST, std::bind(&ServerManager::wifiFormCallback, this, std::placeholders::_1));
    server->begin();
  }
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
}

String 
ServerManager::readFile(fs::FS &fs, const char * path)
{
  debug("Reading file: " + String(path) + "\r\n");
  File file = fs.open(path, "r");
  if(!file || file.isDirectory()){
    debug("- failed to open file for reading");
    return String();
  }

  String fileContent;
  while(file.available()){
    fileContent = file.readStringUntil('\n');
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

  if (!WiFi.config(localIP, localGateway, subnet)){
    debug("STA Failed to configure");
    return false;
  }
  WiFi.begin(ssid.c_str(), pass.c_str());

  debug("Connecting to WiFi...");
  delay(20000);
  if(WiFi.status() != WL_CONNECTED) {
    debug("Failed to connect.");
    return false;
  }

  debug(WiFi.localIP().toString());
  return true;
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