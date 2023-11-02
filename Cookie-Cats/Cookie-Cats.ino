#include <LittleFS.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include "functions.h"
#include "structures.h"

using namespace std;

WiFiClient wifiClient;

// 在 80 端口实例化 http 服务器
AsyncWebServer httpserver(80);

// 实例化 HTTP 客户端
// AsyncHTTPRequest HTTPClient;

// 实例化配置项
Configuration configuration;

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

    for (int i = 0; i < 5; i++) {  // 连接 WiFi，尝试 5 次
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected!");
        Serial.print("IP address for network ");
        Serial.print(configuration.WiFi_SSID);
        Serial.print(" : ");
        Serial.println(WiFi.localIP());
        break;
      } else {
        Serial.print(".");
        delay(100);
      }
    }
    if (WiFi.status() != WL_CONNECTED) {
      Serial.print("Cannot connect to ");
      Serial.print(configuration.WiFi_SSID);
      Serial.print(". Skip.");
      Serial.println();
    }
  } else {
    Serial.println("No WiFi access point configuration found. Skip.");
  }

  // 启动网络服务器

  // 控制面板主页。通过 "/" 或 "/index.html" 访问
  httpserver.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->redirect("/index.html");  // 重定向到 /index.html
  });

  // 网络状态
  // API。通过访问 "/status/network" 得到
  httpserver.on("/status/network", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (testNet(wifiClient)) {
      request->send(200, "text/plain", "true");
    } else {
      request->send(200, "text/plain", "false");
    }
  });

  // 获取IP
  // API，通过访问 "/status/ip" 得到
  // TODO：目前仅支持 meow 和 manual，之后将增加其他方法
  httpserver.on("/status/ip", HTTP_GET, [](AsyncWebServerRequest *request) {
    String ip;
    if (configuration.IP_Obtain_Method.first == "meow") {
      ip = meow(configuration.IP_Obtain_Method.second, wifiClient);
    } else if (configuration.IP_Obtain_Method.first == "manual") {
      ip = configuration.IP_Obtain_Method.second;
    } else {
      ip = "No IP method to found, please config IP method in config.json";
    }
    request->send(200, "text/plain", ip);
  });

  // 重启
  // API，访问 "/device/restart" 将立即重启
  httpserver.on("/device/restart", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Restart now.");
    ESP.restart();
  });

  // 获取 config.json 内容
  // API，访问 "/config/get"，将以 JSON 格式返回 config.json
  httpserver.on("/config/get", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (LittleFS.exists("/config.json")) {
      File jsonText = LittleFS.open("/config.json", "r");
      request->send(LittleFS, "/config.json", "application/json");
    } else {
      request->send(500, "application/json", "{\"error\":\"No config.json Found.\"}");
    }
  });

  // 保存 config.json 内容
  // API，访问 "/config/save"，将接收客户端 post 的 JSON 数据，并使用 ArduinoJson 读取
  // 如果 JSON 格式合法，将把接收到的 JSON 覆盖保存到 config.json
  // 警告：如果 JSON 格式不合法，则返回状态码 400，类型 application/json，内容 {"error":"Not a valid JSON object."}
  // 如果保存成功，返回状态码为 200，类型：application/json 内容：{"success":"config.json saved."}
  // 如果保存失败，则返回状态码 500，类型：application/json 内容：{"error":"Failed to save."}
  // 样例：
  // curl -X POST -H "Content-Type: application/json" -d '{"Cookie_Cat_SSID": "CookieCat","Cookie_Cat_PASSWORD": "cookiecat","WiFi_SSID": "OpenWrt","WiFi_PASSWORD": "okgogogo","username": "","password": "","carrier": "","school": "","IP_Obtain_Method": {"meow": "http://192.168.10.151:8080"}}' http://192.168.10.181/config/save
  httpserver.on(
    "/config/save", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      Serial.println("Receiving config.json...");
      // 创建一个 JSON 文档对象，用于存储接收到的 JSON 数据
      DynamicJsonDocument doc(1024);
      // 尝试从客户端读取 JSON 数据，并解析到文档对象中
      DeserializationError error = deserializeJson(doc, data);
      // 如果解析成功，说明 JSON 格式合法
      if (!error) {
        // 尝试打开 config.json 文件，如果不存在则创建一个新文件
        File jsonConfig = LittleFS.open("/config.json", "w");
        // 如果文件打开成功，说明可以写入数据
        if (jsonConfig) {
          // 将文档对象中的 JSON 数据序列化到文件中
          serializeJson(doc, jsonConfig);
          // 关闭文件
          jsonConfig.close();
          // 返回成功信息
          Serial.println("config.json saved.");
          request->send(200, "application/json", "{\"success\":\"config.json saved.\"}");
        } else {
          // 如果文件打开失败，说明无法写入数据
          // 返回失败信息
          Serial.println("Failed to save config.json");
          request->send(500, "application/json", "{\"error\":\"Failed to save.\"}");
        }
      } else {
        // 如果解析失败，说明 JSON 格式不合法
        // 返回失败信息
        Serial.println("Invalid JSON format.");
        request->send(500, "application/json", "{\"error\":\"Invalid JSON format.\"}");
      }
    });

  // 处理页面请求
  httpserver.onNotFound([](AsyncWebServerRequest *request) {
    // 获取用户请求网址信息
    String localAddress = "/web-controller/compressed/" + request->url() + ".bin";  // 默认文件以 gz 压缩

    // 如果文件存在，则发送；否则返回 404。
    if (LittleFS.exists(localAddress)) {
      AsyncWebServerResponse *response = request->beginResponse(LittleFS, localAddress, httpGetContentType(request->url()), false);
      response->addHeader("Content-Encoding", "gzip");
      request->send(response);
    } else {
      request->send(404, "text/plain", "404 Not found.");
    }
  });

  // 启动服务器
  httpserver.begin();
  Serial.println("HTTP server started on :80");

  Serial.println("Cookie-Cats Initialized Successfully.");
}

void loop(void) {
}
