#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <TickTwo.h>
#include <ArduinoJson.h>
#include "functions.h"
#include "structures.h"
#include "auth.h"
#include "webpages.h"

#define VERSION "PIONEER_alpha_prerelease_017"
#define CONTRIBUTER "77, QiQi, and Cookie-Cats Org"
#define SERIAL_BAUD 115200  // 串口波特率

// 调试：发布前请确认次选项为 true
#define ALLOW_OTA_UPDATE true  // 允许 OTA 升级

using namespace std;

// 在 80 端口实例化 http 服务器
ESP8266WebServer httpserver(80);

// 设置密钥
PracticalCrypto secret;

// 实例化配置项
Configuration config;

// 认证程序是否启动
bool startAuth = false;
// 无论如何都创建用于认证的 Ticker 对象，每 20s 检查一次，是否运行根据 startAuth 判断。
TickTwo CheckNetAndAuth([] {
  checkNetAndAuth(config);
},
                        20000);

// 设置 OTA 更新
#define UPDATE_URL "http://update.cookiecats.diazepam.cc"

#define ATOMIC_FS_UPDATE  // 允许压缩

const char pubkey[] PROGMEM = R"EOF(
-----BEGIN PUBLIC KEY-----
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAsrdoN7eiznoUr5mtJa6W
FP/KbMWrtIRoblgbHjq43cs8+AwQ9Jebzl8OH9eyVEk8/mLDYtfJeAp1ma5Mwr8U
TQOKRYe9H+f2haUkf76pmdnU4xvHs5pLWHLuvy+MfCpyi3GTE19ydrCsnLoCZUlV
AZdhvhyCiIsrx3tocrnNkm6Okbh6ZsSdRS8eepOKIBH7UCXrsC8daR1e59rATCe8
ULxx6LsFPLDq1ls06RbDUDH4o2Pb2BbqA6XcQSqwxDp/JHjaJ/1w2juXxFHtc8TK
88drmrIYLPs1QZ1wMieumMR11L9AM6M96CLCa+6oE+yzshEXj0ScA40y/5SsQiMz
rwIDAQAB
-----END PUBLIC KEY-----
)EOF";

BearSSL::PublicKey signPubKey(pubkey);
BearSSL::HashSHA256 firmwareHash;
BearSSL::SigningVerifier sign(&signPubKey);

void setup() {
  // 开启串口
  Serial.begin(SERIAL_BAUD);
  Serial.println();

  // 设置 LED 引脚为输出模式
  // 使用内置 LED 为输出引脚
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, true);  // 熄灯

  // 启动闪存文件系统
  if (LittleFS.begin()) {
    Serial.println(F("LittleFS Started."));
  } else {
    Serial.println(F("LittleFS Failed to Start."));
  }

  // 载入 secret
  readSecret(secret);
  // 载入证书
  Update.installSignature(&firmwareHash, &sign);

  // 载入自定义配置
  Serial.println(F("Loading config.json..."));
  if (LittleFS.exists("/config.json")) {              // 如果 config.json 可以在LittleFS中找到
    File file = LittleFS.open("/config.json", "r");   // 则尝试打开该文件
    readConfigurationFromFile(file, config, secret);  // 读取配置文件
    file.close();                                     // 关闭文件
  } else {
    Serial.println(F("No config.json found, use default settings."));
  }

  // 设置 WiFi
  WiFi.mode(WIFI_AP_STA);
  WiFi.hostname("Cookie-Cats");

  WiFi.softAP(config.Cookie_Cat_SSID, config.Cookie_Cat_PASSWORD);  // 设置 WiFi 接入点
  Serial.print(F("WiFi access point SSID: "));
  Serial.print(config.Cookie_Cat_SSID);
  Serial.println();
  Serial.print(F("WiFi access point Password: "));
  Serial.print(config.Cookie_Cat_PASSWORD);
  Serial.println();
  Serial.print(F("Cookie-Cat is on:"));
  Serial.print(WiFi.softAPIP());
  Serial.println();

  // 连接 WiFi
  if (config.WiFi_SSID != "") {
    WiFi.begin(config.WiFi_SSID, config.WiFi_PASSWORD);
    Serial.print(F("Connecting to "));
    Serial.print(config.WiFi_SSID);
    delay(1000);  // 等待 WiFi 连接

    for (int i = 0; i < 500; i++) {  // 连接 WiFi，最多尝试 500 次
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println(F("Connected!"));
        Serial.print(F("IP address for network "));
        Serial.print(config.WiFi_SSID);
        Serial.print(F(": "));
        Serial.println(WiFi.localIP());
        break;
      } else {
        Serial.print(F("."));
        delay(100);
      }
    }
    if (WiFi.status() != WL_CONNECTED) {
      WiFi.disconnect(true);  // 停止连接 WiFi
      Serial.print(F("Cannot connect to "));
      Serial.print(config.WiFi_SSID);
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
    if (testNet()) {
      httpserver.send(200, "text/plain", "true");
    } else {
      httpserver.send(500, "text/plain", "false");
    }
  });

  // 获取IP
  // API，通过访问 "/status/ip" 得到
  // 当 config.IP_Obtain_Method 为 ununcessary 时返回 0.0.0.0
  // TODO：目前仅支持 meow 和 manual，之后将增加其他方法
  httpserver.on("/status/ip", HTTP_GET, []() {
    httpserver.send(200, "text/plain", getIPFromIPObtainMethod(config));
  });

  // 返回路由器分配给 CookieCats 的 IP
  // API，访问 "/status/deviceip" 返回路由器分配给 CookieCats 的 IP
  // 如果 CookieCats 已连接 WiFi，则返回路由器分配给 CookieCats 的 IP ；否则返回 "0.0.0.0"
  httpserver.on("/status/deviceip", HTTP_GET, []() {
    String localIP = WiFi.localIP().toString();
    if (localIP == "(IP unset)") {
      httpserver.send(500, "text/plain", "0.0.0.0");
    } else {
      httpserver.send(200, "text/plain", localIP);
    }
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
      jsonText.close();
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
  // 测试命令：curl -X POST -H "Content-Type: application/json" -d '{"Cookie_Cat_SSID":"CookieCat","Cookie_Cat_PASSWORD":"cookiecat","WiFi_SSID":"","WiFi_PASSWORD":"","username":"","password":"","carrier":"","school":"","IP_Obtain_Method":{"meow":"http://192.168.10.151:8080"},"allowOTA":"true"}' http://192.168.4.1/config/save
  httpserver.on("/config/save", HTTP_POST, [] {
    Serial.println(F("Receiving config.json..."));
    DynamicJsonDocument doc(1024);                                               // 创建一个 JSON 文档对象，用于存储接收到的 JSON 数据
    DeserializationError error = deserializeJson(doc, httpserver.arg("plain"));  // 尝试从客户端读取 JSON 数据，并解析到文档对象中

    if (doc.containsKey("password") && doc["password"] != "") {  // 如果用户填写了密码项，则进行加密
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
      jsonConfig.close();
    } else {
      // 如果解析失败，说明 JSON 格式不合法
      // 返回失败信息
      Serial.println(F("Invalid JSON format."));
      httpserver.send(500, "application/json", "{\"error\":\"Invalid JSON format.\"}");
    }
  });

  // 重置配置
  // API，访问 "/config/rmconfig"，将删除 LittleFS 上保存的 config.json
  httpserver.on("/config/rmconfig", HTTP_GET, []() {
    if (LittleFS.exists("/config.json")) {
      LittleFS.remove("/config.json");
      Serial.println(F("Removed config.json"));
      httpserver.send(200, "text/plain", "Removed config.json");
      delay(1000);    // 确保 HTTP 响应
      ESP.restart();  // 重启以刷新配置
    } else {
      Serial.println(F("No config.json Found."));
      httpserver.send(500, "text/plain", "No config.json found.");
    }
  });

  // 返回固件版本
  // API，访问 "/firmware/version" 将返回固件版本和作者信息
  httpserver.on("/firmware/version", HTTP_GET, []() {
    char version[128];
    strcat(version, "Cookie-Cats(");
    strcat(version, VERSION);
    strcat(version, ")by ");
    strcat(version, CONTRIBUTER);
    httpserver.send(200, "text/plain", version);
  });

  // 是否允许自动更新
  // API，访问 "/firmware/allowupdate" 将返回是否允许自动更新
  httpserver.on("/firmware/allowupdate", HTTP_GET, []() {
    if (ALLOW_OTA_UPDATE && config.allowOTA && (WiFi.status() == WL_CONNECTED)) {
      httpserver.send(200, "text/plain", "true");
    } else {
      httpserver.send(500, "text/plain", "false");
    }
  });

  // 强制更新固件
  // API，访问 "/firmware/update" 将强制更新，忽略固件和用户设置
  // 警告：返回值不能表示是否更新。
  httpserver.on("/firmware/update", HTTP_GET, []() {
    httpserver.send(200, "text/plain", "Ok.");
    Serial.println(F("Updating firmware..."));
    otaUpdate(UPDATE_URL, VERSION);
  });

  // 返回认证程序是否启动
  // API，访问 "/auth/status" 返回认证程序状态
  httpserver.on("/auth/status", HTTP_GET, []() {
    if (startAuth) {
      httpserver.send(200, "text/plain", "true");
    } else {
      httpserver.send(500, "text/plain", "false");
    }
  });

  // 处理页面请求
  httpserver.onNotFound([]() {
    String webAddress = httpserver.uri();  // 获取用户请求网址信息
    String page = webPages(webAddress);    // 读取网页

    if (page == "") {  // 如果没找到网页
      httpserver.send(404, "text/plain", "404 Not found.");
    } else {  // 如果可以找到网页
      httpserver.send(200, httpGetContentType(webAddress), page);
    }
  });

  httpserver.begin();
  Serial.println(F("HTTP server started on :80"));

  // 判断是否开启认证程序
  if ((config.school == "") || (config.username == "") || (config.password == "")) {
    Serial.println(F("No school or username or password set. Skip."));
  } else {
    Serial.println(F("Configuration of authoriation found. Set startAuth flag to true."));
    startAuth = true;
  }
  CheckNetAndAuth.start();  // 启动 Ticker 对象

  // OTA 升级
  if (ALLOW_OTA_UPDATE && config.allowOTA && (WiFi.status() == WL_CONNECTED)) {  // 仅当 ALLOW_OTA_UPDATE 与 config.allowOTA 设置为 true 且 WiFi 已连接时更新。因为更新服务器可能在内网，所以不要求可以联网。
    Serial.println(F("Starting OTA upgrades..."));
    otaUpdate(UPDATE_URL, VERSION);
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

  yield();
  blink(5);  // 闪烁 5 次，代表初始化成功

  yield();
  if (startAuth) checkNetAndAuth(config);  // 立即认证

  /*
  调试区
  需要调试的代码请在这里进行调试。
  发布新版本前请确认这里没有需要执行的内容。
  */
  // -------------------------------------------------------------DEBUG--------------------------------------------------------------
  // Serial.println(F("-------DEBUG--------"));
  // Serial.println(F("-----DEBUG END------"));
  // -----------------------------------------------------------DEBUG END------------------------------------------------------------
}

void loop(void) {
  httpserver.handleClient();  // 处理客户端请求

  if (startAuth) CheckNetAndAuth.update();  // 判断是否运行认证程序
}