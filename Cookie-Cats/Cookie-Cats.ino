#include <LittleFS.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <TickTwo.h>
#include <ArduinoJson.h>
#include "functions.h"
#include "structures.h"
#include "auth.h"

#define VERSION "PIONEER_0.1_alpha_signed"
#define CONTRIBUTER "77, QiQi, and Cookie-Cats Org"
#define SERIAL_BAUD 115200  // 串口波特率

#define ALLOW_OTA_UPDATE false  // 允许 OTA 升级

using namespace std;

WiFiClient wifiClient;

// 在 80 端口实例化 http 服务器
ESP8266WebServer httpserver(80);

// 设置密钥
PracticalCrypto secret;

// 实例化配置项
Configuration configuration;

// 认证程序是否启动
bool startAuth = false;
// 无论如何都创建用于认证的 Ticker 对象，每 20s 检查一次，是否运行根据 startAuth 判断。
TickTwo CheckNetAndAuth([] {
  checkNetAndAuth(configuration, wifiClient);
},
                        20000);

// 设置 OTA 更新
#define UPDATE_URL "http://update.cookiecats.diazepam.cc"
#define ATOMIC_FS_UPDATE  // 允许压缩

BearSSL::PublicKey signPubKey("-----BEGIN PUBLIC KEY-----\n"
                              "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAsrdoN7eiznoUr5mtJa6W\n"
                              "FP/KbMWrtIRoblgbHjq43cs8+AwQ9Jebzl8OH9eyVEk8/mLDYtfJeAp1ma5Mwr8U\n"
                              "TQOKRYe9H+f2haUkf76pmdnU4xvHs5pLWHLuvy+MfCpyi3GTE19ydrCsnLoCZUlV\n"
                              "AZdhvhyCiIsrx3tocrnNkm6Okbh6ZsSdRS8eepOKIBH7UCXrsC8daR1e59rATCe8\n"
                              "ULxx6LsFPLDq1ls06RbDUDH4o2Pb2BbqA6XcQSqwxDp/JHjaJ/1w2juXxFHtc8TK\n"
                              "88drmrIYLPs1QZ1wMieumMR11L9AM6M96CLCa+6oE+yzshEXj0ScA40y/5SsQiMz\n"
                              "rwIDAQAB\n"
                              "-----END PUBLIC KEY-----");
BearSSL::HashSHA256 firmwareHash;
BearSSL::SigningVerifier sign(&signPubKey);

void setup() {
  // 开启串口
  Serial.begin(SERIAL_BAUD);
  Serial.println();

  // 启动闪存文件系统
  if (LittleFS.begin()) {
    Serial.println(F("LittleFS Started."));
  } else {
    Serial.println(F("LittleFS Failed to Start."));
  }

  // 载入 secret
  readSecret(secret);

  // 载入自定义配置
  Serial.println(F("Loading config.json..."));
  if (LittleFS.exists("/config.json")) {                     // 如果 config.json 可以在LittleFS中找到
    File file = LittleFS.open("/config.json", "r");          // 则尝试打开该文件
    readConfigurationFromFile(file, configuration, secret);  // 读取配置文件
    file.close();                                            // 关闭文件
  } else {
    Serial.println(F("No config.json found, use default settings."));
  }

  // 设置 WiFi
  WiFi.mode(WIFI_AP_STA);

  WiFi.softAP(configuration.Cookie_Cat_SSID, configuration.Cookie_Cat_PASSWORD);  // 设置 WiFi 接入点
  Serial.print(F("WiFi access point SSID: "));
  Serial.print(configuration.Cookie_Cat_SSID);
  Serial.println();
  Serial.print(F("WiFi access point Password: "));
  Serial.print(configuration.Cookie_Cat_PASSWORD);
  Serial.println();
  Serial.print(F("Cookie-Cat is on:"));
  Serial.print(WiFi.softAPIP());
  Serial.println();

  // 连接 WiFi
  if (configuration.WiFi_SSID != "") {
    WiFi.begin(configuration.WiFi_SSID, configuration.WiFi_PASSWORD);
    Serial.print(F("Connecting to "));
    Serial.print(configuration.WiFi_SSID);
    Serial.println();
    delay(1000);  // 等待 WiFi 连接

    for (int i = 0; i < 500; i++) {  // 连接 WiFi，最多尝试 500 次
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println(F("Connected!"));
        Serial.print(F("IP address for network "));
        Serial.print(configuration.WiFi_SSID);
        Serial.print(F(": "));
        Serial.println(WiFi.localIP());
        break;
      } else {
        Serial.print(F("."));
        delay(100);
      }
    }
    if (WiFi.status() != WL_CONNECTED) {
      Serial.print(F("Cannot connect to "));
      Serial.print(configuration.WiFi_SSID);
      Serial.print(F(". Skip."));
      Serial.println();
    }
  } else {
    Serial.println(F("No WiFi access point config found. Skip."));
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
    Serial.println(F("Ready to restart. See you later!"));
    httpserver.send(200, "text/plain", "Restart now.");
    ESP.restart();
  });

  // 获取 config.json 内容
  // API，访问 "/config/get"，将以 JSON 格式返回 config.json
  httpserver.on("/config/get", HTTP_GET, []() {
    if (LittleFS.exists("/config.json")) {
      Serial.println(F("Sending config.json"));
      File jsonText = LittleFS.open("/config.json", "r");
      httpserver.send(200, "application/json", jsonText);
    } else {
      Serial.println(F("No config.json Found."));
      httpserver.send(500, "application/json", "{\"error\":\"No config.json Found.\"}");
    }
  });

  // 保存 config.json 内容
  // API，访问 "/config/save"，将接收客户端 post 的 JSON 数据，并使用 ArduinoJson 读取
  // 如果 JSON 格式合法，将把接收到的 JSON 覆盖保存到 config.json
  // 如果保存成功，返回状态码为 200，类型：application/json 内容：{"success":"config.json saved."}
  // 如果保存失败，则返回状态码 500，类型：application/json 内容：{"error":"Failed to save."}
  // 测试命令：curl -X POST -H "Content-Type: application/json" -d '{"Cookie_Cat_SSID": "CookieCat","Cookie_Cat_PASSWORD": "cookiecat","WiFi_SSID": "OpenWrt","WiFi_PASSWORD": "okgogogo","username": "","password": "","carrier": "","school": "","IP_Obtain_Method": {"meow": "http://192.168.10.151:8080"}},"allowOTA": "true"' http://192.168.10.181/config/save
  httpserver.on("/config/save", HTTP_POST, [] {
    Serial.println(F("Receiving config.json..."));
    DynamicJsonDocument doc(1024);                                               // 创建一个 JSON 文档对象，用于存储接收到的 JSON 数据
    DeserializationError error = deserializeJson(doc, httpserver.arg("plain"));  // 尝试从客户端读取 JSON 数据，并解析到文档对象中

    if (doc.containsKey("password")) {  // 如果用户填写了密码项，则进行加密
      doc["password"] = secret.encrypt(doc["password"]);
    }

    if (!error) {                                            // 如果解析成功，说明 JSON 格式合法
      File jsonConfig = LittleFS.open("/config.json", "w");  // 尝试打开 config.json 文件，如果不存在则创建一个新文件
      if (jsonConfig) {
        serializeJson(doc, jsonConfig);  // 将文档对象中的 JSON 数据序列化到文件中
        jsonConfig.close();              // 关闭文件
        // 返回成功信息
        Serial.println(F("config.json saved."));
        httpserver.send(200, "application/json", "{\"success\":\"config.json saved.\"}");
      } else {
        // 如果文件打开失败，说明无法写入数据
        // 返回失败信息
        Serial.println(F("Failed to save config.json"));
        httpserver.send(500, "application/json", "{\"error\":\"Failed to save.\"}");
      }
    } else {
      // 如果解析失败，说明 JSON 格式不合法
      // 返回失败信息
      Serial.println(F("Invalid JSON format."));
      httpserver.send(500, "application/json", "{\"error\":\"Invalid JSON format.\"}");
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
  Serial.println(F("HTTP server started on :80"));

  // 判断是否开启认证程序
  if ((configuration.school == "") || (configuration.username == "") || (configuration.password == "")) {
    Serial.println(F("No school or username or password set. Skip."));
  } else {
    Serial.println(F("Configuration of authoriation found. Set startAuth flag to true."));
    startAuth = true;
  }
  CheckNetAndAuth.start();  // 启动 Ticker 对象

  // OTA 升级
  Update.installSignature(&firmwareHash, &sign);
  if (ALLOW_OTA_UPDATE && configuration.allowOTA && (WiFi.status() == WL_CONNECTED)) {  // 仅当 ALLOW_OTA_UPDATE 与 configuration.allowOTA 设置为 true 且 WiFi 已连接时更新。因为更新服务器可能在内网，所以不要求可以联网。
    Serial.println(F("Starting OTA upgrades..."));
    otaUpdate(wifiClient, UPDATE_URL, VERSION);
  } else {
    Serial.println(F("Firmware is set to disallow OTA upgrades."));
  }

  Serial.println(F("Cookie-Cats Initialized Successfully.\n\n"));
  Serial.println(F("  _____            _    _              _____      _       "));
  Serial.println(F(" / ____|          | |  (_)            / ____|    | |      "));
  Serial.println(F("| |     ___   ___ | | ___  ___ ______| |     __ _| |_ ___ "));
  Serial.println(F("| |    / _ \\ / _ \\| |/ / |/ _ \\______| |    / _` | __/ __|"));
  Serial.println(F("| |___| (_) | (_) |   <| |  __/      | |___| (_| | |_\\__ \\"));
  Serial.println(F(" \\_____\\___/ \\___/|_|\\_\\_|\\___|       \\_____\\__,_|\\__|___/"));
  Serial.print(F("\n\n"));
  Serial.print(F("    Made with Love from "));
  Serial.print(CONTRIBUTER);
  Serial.print(F("     \n"));
  Serial.println(F("         Homepage: https://github.com/Cookie-Cats         "));
  Serial.print(F("                VERSION: "));
  Serial.print(VERSION);
  Serial.print(F("                \n"));
  Serial.println(F("                   Cookie-Cats started.                   "));
  Serial.println("\n");
}

void loop(void) {
  httpserver.handleClient();  // 处理客户端请求

  if (startAuth) CheckNetAndAuth.update();  // 判断是否运行认证程序
}
