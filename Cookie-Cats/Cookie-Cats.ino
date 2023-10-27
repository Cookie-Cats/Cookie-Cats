#include <LittleFS.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include "functions.h"

using namespace std;

#define WIFI_SSID "OpenWrt"
#define WIFI_PASS "okgogogo"

#define AP_SSID "ESP"
#define AP_PASS "okgogogo"

WiFiClient wifiClient;

// 在 80 端口实例化 http 服务器
ESP8266WebServer httpserver(80);

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

  // 控制面板主页。通过 "/" 或 "/index.html" 访问
  httpserver.on("/", HTTP_GET, []() {
    httpserver.sendHeader("Location", "/index.html");
    httpserver.send(302, "text/plain", "");
  });

  // 网络状态
  // API。通过访问 "/status/network" 得到
  httpserver.on("/status/network", HTTP_GET, []() {
    if (testNet(wifiClient)) {
      httpserver.send(200, "text/plain", "true");
    } else {
      httpserver.send(200, "text/plain", "false");
    }
  });

  // 获取IP
  // API，通过访问 "/status/ip" 得到
  // TODO：目前仅支持meow，之后将增加其他方法
  httpserver.on("/status/ip", HTTP_GET, []() {
    httpserver.send(200, "text/plain", meow("http://192.168.10.151:8080", wifiClient));
  });

  // 处理页面请求
  httpserver.onNotFound([]() {
    // 获取用户请求网址信息
    String webAddress = httpserver.uri();

    bool fileReadOK = false;
    String localAddress = "/web-controller/compressed/" + webAddress + ".gz";  // 默认文件以 gz 压缩

    if (LittleFS.exists(localAddress)) {             // 如果访问的文件可以在LittleFS中找到
      File file = LittleFS.open(localAddress, "r");  // 则尝试打开该文件
      httpserver.streamFile(file, getContentType(webAddress));
      file.close();
    }

    if (!fileReadOK) {
      httpserver.send(404, "text/plain", "404 Not found.");
    }
  });

  httpserver.begin();

  Serial.println("HTTP server started on :80");
}

void loop(void) {
  httpserver.handleClient();  // 处理客户端请求
}

// 获取文件类型
String getContentType(String filename) {
  if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/gzip";
  return "text/plain";
}
