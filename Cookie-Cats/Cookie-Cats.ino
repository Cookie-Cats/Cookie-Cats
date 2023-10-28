#include <LittleFS.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include "functions.h"
#include "structures.h"

using namespace std;

WiFiClient wifiClient;

// 在 80 端口实例化 http 服务器
ESP8266WebServer httpserver(80);

Configuration configuration;  // 实例化配置项

void setup() {
  // 开启串口
  Serial.begin(9600);
  Serial.println();

  // 启动闪存文件系统
  if (LittleFS.begin()) {
    Serial.println("LittleFS Started.\n");
  } else {
    Serial.println("LittleFS Failed to Start.\n");
  }

  // 载入自定义配置
  Serial.println("Loading config.json...");
  if (LittleFS.exists("/config.json")) {             // 如果 config.json 可以在LittleFS中找到
    File file = LittleFS.open("/config.json", "r");  // 则尝试打开该文件
    configuration = readConfigurationFromFile(file, configuration);
    file.close();  // 关闭文件
  } else {
    Serial.println("No config.json found, use default settings.");
  }

  // 设置 WiFi
  WiFi.mode(WIFI_AP_STA);

  WiFi.softAP(configuration.Cookie_Cat_SSID, configuration.Cookie_Cat_PASSWORD);  // 设置 WiFi 接入点
  Serial.print("WiFi access point SSID: ");
  Serial.print(configuration.Cookie_Cat_SSID);
  Serial.println();
  Serial.print("WiFi access point Password: ");
  Serial.print(configuration.Cookie_Cat_PASSWORD);
  Serial.println();
  Serial.print("Cookie-Cat is on:");
  Serial.print(WiFi.softAPIP());
  Serial.println();

  if (configuration.WiFi_SSID != "") {  // 连接 WiFi
    WiFi.begin(configuration.WiFi_SSID, configuration.WiFi_PASSWORD);
    Serial.print("Connecting to ");
    Serial.print(configuration.WiFi_SSID);
    Serial.println();
    while (WiFi.status() != WL_CONNECTED) {
      delay(100);
      Serial.print(".");
    }
    Serial.println("\nConnected!");
    Serial.print("IP address for network ");
    Serial.print(configuration.WiFi_SSID);
    Serial.print(" : ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("No WiFi access point configuration found. Skip.");
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
  // TODO：目前仅支持 meow 和 manual，之后将增加其他方法
  httpserver.on("/status/ip", HTTP_GET, []() {
    String ip;
    if (configuration.IP_Obtain_Method.first == "meow") {
      ip = meow(configuration.IP_Obtain_Method.second, wifiClient);
    } else if (configuration.IP_Obtain_Method.first == "manual") {
      ip = configuration.IP_Obtain_Method.second;
    } else {
      ip = "No IP method to found, please config IP method in config.json";
    }
    httpserver.send(200, "text/plain", ip);
  });

  // 重启
  // API，访问 "/device/restart" 将立即重启
  httpserver.on("/device/restart", HTTP_GET, []() {
    httpserver.send(200, "text/plain", "Restart now.");
    ESP.restart();
  });

  // 获取 config.json 内容
  // API，访问 "/config/get"，将以 JSON 格式返回 config.json
  httpserver.on("/config/get", HTTP_GET, []() {
    if (LittleFS.exists("/config.json")) {
      File jsonText = LittleFS.open("/config.json", "r");
      httpserver.send(200, "application/json", jsonText);
    } else {
      httpserver.send(500, "application/json", "{\"error\":\"No config.json Found.\"}");
    }
  });

  // 处理页面请求
  httpserver.onNotFound([]() {
    // 获取用户请求网址信息
    String webAddress = httpserver.uri();

    bool fileReadOK = false;
    String localAddress = "/web-controller/compressed/" + webAddress + ".gz";  // 默认文件以 gz 压缩

    if (LittleFS.exists(localAddress)) {             // 如果访问的文件可以在LittleFS中找到
      File file = LittleFS.open(localAddress, "r");  // 则尝试打开该文件
      httpserver.streamFile(file, httpGetContentType(webAddress));
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
