#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#define WIFI_SSID "OpenWrt"
#define WIFI_PASS "okgogogo"

#define AP_SSID "ESP"
#define AP_PASS "okgogogo"

// 在 80 端口实例化 http 服务器
AsyncWebServer httpserver(80);


void setup() {
  // 开启串口
  Serial.begin(9600);
  Serial.println();

  // 开启接入点，连接 WiFi
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(AP_SSID, AP_PASS);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.print("IP address for network ");
  Serial.print(WIFI_SSID);
  Serial.print(" : ");
  Serial.println(WiFi.localIP());
  Serial.print("IP address for network ");
  Serial.print(AP_SSID);
  Serial.print(" : ");
  Serial.print(WiFi.softAPIP());
  Serial.println();

  // 启动闪存文件系统
  if (LittleFS.begin()) {
    Serial.println("LittleFS Started.\n");
  } else {
    Serial.println("LittleFS Failed to Start.\n");
  }

  // 启动网络服务器
  httpserver.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->redirect("/index.html");
  });
  httpserver.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", "text/plain");
  });

  httpserver.onNotFound(notFound);

  httpserver.begin();

  Serial.println("HTTP server started on :80\n");
}

void loop(void) {

}

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "404 Not found.");
}

