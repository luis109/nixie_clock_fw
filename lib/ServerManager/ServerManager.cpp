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
  
  // Load values saved in LittleFS
  ssid = readFile(LittleFS, ssidPath);
  pass = readFile(LittleFS, passPath);
  ip = readFile(LittleFS, ipPath);
  gateway = readFile (LittleFS, gatewayPath);
  Serial.println(ssid);
  Serial.println(pass);
  Serial.println(ip);
  Serial.println(gateway);

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
    Serial.println("Server started"); 
  }
  else 
  {
    // Connect to Wi-Fi network with SSID and password
    Serial.println("Setting AP (Access Point)");
    // NULL sets an open Access Point
    WiFi.softAP("ESP-WIFI-MANAGER", NULL);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP); 

    // Web Server Root URL
    server->on("/", HTTP_GET, [this](AsyncWebServerRequest *request){
      request->send(LittleFS, "/wifimanager.html", "text/html");
    });
    
    server->serveStatic("/", LittleFS, "/");

    server->on("/", HTTP_POST, std::bind(&ServerManager::wifiFormCallback, this, std::placeholders::_1));
    server->begin();
  }
}

String 
ServerManager::readFile(fs::FS &fs, const char * path)
{
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path, "r");
  if(!file || file.isDirectory()){
    Serial.println("- failed to open file for reading");
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
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, "w");
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- frite failed");
  }
  file.close();
}

bool 
ServerManager::initWiFi()
{
  if(ssid=="" || ip==""){
    Serial.println("Undefined SSID or IP address.");
    return false;
  }

  WiFi.mode(WIFI_STA);
  localIP.fromString(ip.c_str());
  localGateway.fromString(gateway.c_str());

  if (!WiFi.config(localIP, localGateway, subnet)){
    Serial.println("STA Failed to configure");
    return false;
  }
  WiFi.begin(ssid.c_str(), pass.c_str());

  Serial.println("Connecting to WiFi...");
  delay(20000);
  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("Failed to connect.");
    return false;
  }

  Serial.println(WiFi.localIP());
  return true;
}

 
void 
ServerManager::wifiFormCallback(AsyncWebServerRequest *request)
{
  int params = request->params();
  for(int i=0;i<params;i++){
    AsyncWebParameter* p = request->getParam(i);
    if(p->isPost()){
      // HTTP POST ssid value
      if (p->name() == PARAM_INPUT_1) {
        ssid = p->value().c_str();
        Serial.print("SSID set to: ");
        Serial.println(ssid);
        // Write file to save value
        // writeFile(LittleFS, ssidPath, ssid.c_str());
      }
      // HTTP POST pass value
      if (p->name() == PARAM_INPUT_2) {
        pass = p->value().c_str();
        Serial.print("Password set to: ");
        Serial.println(pass);
        // Write file to save value
        // writeFile(LittleFS, passPath, pass.c_str());
      }
      // HTTP POST ip value
      if (p->name() == PARAM_INPUT_3) {
        ip = p->value().c_str();
        Serial.print("IP Address set to: ");
        Serial.println(ip);
        // Write file to save value
        // writeFile(LittleFS, ipPath, ip.c_str());
      }
      // HTTP POST gateway value
      if (p->name() == PARAM_INPUT_4) {
        gateway = p->value().c_str();
        Serial.print("Gateway set to: ");
        Serial.println(gateway);
        // Write file to save value
        // writeFile(LittleFS, gatewayPath, gateway.c_str());
      }
      //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
    }
  }
  restart = true;
  request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
}