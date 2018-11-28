/*
  Board(s): ESP8266, ESP32 (Untested)
  Depends:
    - Reactduino: https://github.com/Reactduino/Reactduino
    - ESPAsyncTCP: https://github.com/me-no-dev/ESPAsyncTCP
    - ESPAsyncWebServer: https://github.com/me-no-dev/ESPAsyncWebServer
    - ArduinoJson: https://github.com/bblanchon/ArduinoJson
*/
#include <Arduino.h>
#include <Reactduino.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#if HAS_WWW
#include <FS.h>
#endif

extern void set();
String new_ssid = "";
String new_psk = "";

AsyncWebServer server(80);

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

Reactduino app([] () {
  WiFi.begin();
  app.delay(6000, wifiCheck);
  server.on("/connect", HTTP_POST, [] (AsyncWebServerRequest * request) {}, NULL, [] (AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
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
    request->send(200);
    app.delay(2000, wifiReset);
  });
#if HAS_WWW
  SPIFFS.begin();
  server.serveStatic("/", SPIFFS, "/www/").setDefaultFile("index.html");
#endif
  set();
});
