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

extern void set();
String new_ssid = "";
String new_psk = "";

AsyncWebServer server(80);

void restart() {
  ESP.restart();
}

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

void addJsonReqHandler(const char* uri, WebRequestMethodComposite method, ArJsonRequestHandlerFunction callback) {
  AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler(uri, callback);
  handler->setMethod(method);
  server.addHandler(handler);
}

Reactduino app([] () {
  WiFi.begin();
  app.delay(6000, wifiCheck);
  addJsonReqHandler("/connect", HTTP_POST, [] (AsyncWebServerRequest * request, JsonVariant &requJV) {
    JsonObject& requJ = requJV.as<JsonObject>();
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
  server.on("/update", HTTP_POST, [] (AsyncWebServerRequest * request) {
    request->send(200);
    app.delay(2000, restart);
  }, [] (AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    if (!index) {
      uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
      Update.begin(maxSketchSpace);
      Update.runAsync(true);
    }
    Update.write(data, len);
    if (final) {
      Update.end(true);
    }
  });
  set();
});
