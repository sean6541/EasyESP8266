/*
  IMPORTANT! THIS IS NOT A NORMAL LIBRARY! SEE README.md
  Board(s): ESP8266, ESP32 (Untested)
  Depends:
    - Reactduino: https://github.com/Reactduino/Reactduino
    - ESPAsyncTCP: https://github.com/me-no-dev/ESPAsyncTCP
    - ESPAsyncWebServer: https://github.com/me-no-dev/ESPAsyncWebServer
    - ArduinoJson: https://github.com/bblanchon/ArduinoJson
    - rBase64: https://github.com/boseji/rBASE64
    - EEPROMJson: https://github.com/sean6541/EEPROMJson
*/
#include <Arduino.h>
#include <Reactduino.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <rBase64.h>
#include <FS.h>
#include <EEPROMJson.h>

extern void set();
String system_username = "";
String system_password = "";
String new_ssid = "";
String new_psk = "";

EEPROMJson eejs(4096);
AsyncWebServer server(80);
AsyncStaticWebHandler& static_srv(server.serveStatic("/", SPIFFS, "/www/"));

void wifiSTA() {
  WiFi.softAPdisconnect();
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  if (new_psk == "") {
    WiFi.begin(new_ssid.c_str());
  } else {
    WiFi.begin(new_ssid.c_str(), new_psk.c_str());
  }
  new_ssid = "";
  new_psk = "";
}

void wifiSTA(String ssid, String pskk = "") {
  new_ssid = ssid;
  new_psk = "";
  if (pskk != "") {
    new_psk = pskk;
  }
  wifiSTA();
}

void wifiAP() {
  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  IPAddress ip(10, 3, 141, 1);
  IPAddress gateway(10, 3, 141, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.softAPConfig(ip, gateway, subnet);
  WiFi.softAP(AP_SSID);
}

void wifiReset() {
  WiFi.disconnect();
  ESP.restart();
}

void wifiCheck() {
  if (WiFi.status() != WL_CONNECTED) {
    wifiAP();
  }
}

bool checkAuth(AsyncWebServerRequest * request) {
  if (system_username == "" || system_password == "") {
    return true;
  }
  String auth_header = request->header("Authorization");
  String auth_type = auth_header.substring(0, auth_header.indexOf(" "));
  String auth_b64 = auth_header.substring(auth_header.indexOf(" ") + 1);
  rbase64.decode(auth_b64);
  String auth = rbase64.result();
  Serial.println(auth);
  String auth_user = auth.substring(0, auth.indexOf(":"));
  String auth_pass = auth.substring(auth.indexOf(":") + 1);
  if (auth_type == "Basic" && auth_user == system_username && auth_pass == system_password) {
    return true;
  } else {
    return false;
  }
}

Reactduino app([] () {
  SPIFFS.begin();
  JsonObject& eejson = eejs.getJson();
  JsonVariant _system_username = eejson["system"]["username"];
  JsonVariant _system_password = eejson["system"]["password"];
  if (_system_username.success() && _system_password.success()) {
    system_username = _system_username.as<String>();
    system_password = _system_password.as<String>();
  }
  WiFi.softAPdisconnect();
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin();
  app.delay(6000, wifiCheck);
  server.on("/setup", HTTP_POST, [] (AsyncWebServerRequest * request) {}, NULL, [] (AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    DynamicJsonBuffer requJB;
    JsonObject& requJ = requJB.parseObject(data);
    JsonVariant _system_username = requJ["system"]["username"];
    JsonVariant _system_password = requJ["system"]["password"];
    if (_system_username.success() && _system_password.success()) {
      system_username = _system_username.as<String>();
      system_password = _system_password.as<String>();
      JsonObject& eejson = eejs.getJson();
      eejson["system"]["username"] = system_username;
      eejson["system"]["password"] = system_password;
      eejs.setJson(eejson);
      if (system_username != "" && system_password != "") {
        static_srv.setAuthentication(system_username.c_str(), system_password.c_str());
      }
    }
    String ssid = requJ["wifi"]["ssid"];
    JsonVariant pskk = requJ["wifi"]["psk"];
    new_ssid = ssid;
    new_psk = "";
    if (pskk.success()) {
      new_psk = pskk.as<String>();
    }
    request->send(200);
    app.delay(2000, wifiSTA);
  }).setFilter(ON_AP_FILTER);
  server.on("/setauth", HTTP_POST, [] (AsyncWebServerRequest * request) {}, NULL, [] (AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    if (!checkAuth(request)) {
      request->send(401);
      return;
    }
    DynamicJsonBuffer requJB;
    JsonObject& requJ = requJB.parseObject(data);
    JsonVariant _system_username = requJ["username"];
    JsonVariant _system_password = requJ["password"];
    if (_system_username.success() && _system_password.success()) {
      system_username = _system_username.as<String>();
      system_password = _system_password.as<String>();
      JsonObject& eejson = eejs.getJson();
      eejson["system"]["username"] = system_username;
      eejson["system"]["password"] = system_password;
      eejs.setJson(eejson);
      if (system_username != "" && system_password != "") {
        static_srv.setAuthentication(system_username.c_str(), system_password.c_str());
      }
    }
    request->send(200);
  });
  server.on("/connect", HTTP_POST, [] (AsyncWebServerRequest * request) {}, NULL, [] (AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    if (!checkAuth(request)) {
      request->send(401);
      return;
    }
    DynamicJsonBuffer requJB;
    JsonObject& requJ = requJB.parseObject(data);
    String ssid = requJ["ssid"];
    JsonVariant pskk = requJ["psk"];
    new_ssid = ssid;
    new_psk = "";
    if (pskk.success()) {
      new_psk = pskk.as<String>();
    }
    request->send(200);
    app.delay(2000, wifiSTA);
  });
  server.on("/reset", HTTP_POST, [] (AsyncWebServerRequest * request) {
    if (!checkAuth(request)) {
      request->send(401);
      return;
    }
    request->send(200);
    eejs.clear();
    app.delay(2000, wifiReset);
  });
  static_srv.setDefaultFile("index.html");
  if (system_username != "" && system_password != "") {
    static_srv.setAuthentication(system_username.c_str(), system_password.c_str());
  }
  set();
});
