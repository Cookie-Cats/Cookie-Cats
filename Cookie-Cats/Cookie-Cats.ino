#include <LittleFS.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <vector>

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

  httpserver.on("/index.html", HTTP_GET, []() {
    File file = LittleFS.open("/web/index.html", "r");
    httpserver.streamFile(file, "text/plain");
    file.close();
  });

  // 网络状态
  // API。通过访问 "/status/network" 得到
  httpserver.on("/status/network", HTTP_GET, []() {
    if (testNet()) {
      httpserver.send(200, "text/plain", "true");
    } else {
      httpserver.send(200, "text/plain", "false");
    }
  });

  // 获取IP
  // API，通过访问 "/status/ip" 得到
  // TODO：目前仅支持meow，之后将增加其他方法
  httpserver.on("/status/ip", HTTP_GET, []() {
    httpserver.send(200, "text/plain", meow("http://192.168.10.151:8080"));
  });

  // 404
  httpserver.onNotFound([]() {
    httpserver.send(404, "text/plain", "404 Not found.");
  });

  httpserver.begin();

  Serial.println("HTTP server started on :80");
}

void loop(void) {
  httpserver.handleClient();  // 处理客户端请求
}

// 获得随机 UA
String randomUA() {
  const vector<String> UAs = {
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/118.0.0.0 Safari/537.36",
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/105.0.0.0 Safari/537.36 Edg/105.0.1343.27",
    "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/51.0.2704.106 Safari/537.36 OPR/38.0.2220.41"
  };
  return UAs[random(0, UAs.size() - 1)];
}

// 检测网络通断
bool testNet() {
  Serial.println("Start testing the Internet");
  const vector<String> testServer = {
    "http://connect.rom.miui.com/generate_204",                    // 小米
    "http://connectivitycheck.platform.hicloud.com/generate_204",  // 华为
    "http://wifi.vivo.com.cn/generate_204"                         // vivo
  };

  bool connected = false;

  for (int i = 0; i < 2; i++) {  // 尝试2次
    // 实例化 http 客户端
    HTTPClient httpClient;

    String URL = testServer[random(0, testServer.size() - 1)];
    httpClient.begin(wifiClient, URL);
    Serial.print("Use ");
    Serial.print(URL);
    Serial.print(" to test.\n");

    String UA = randomUA();
    httpClient.setUserAgent(UA);
    Serial.print("User-Agent: ");
    Serial.print(UA);
    Serial.println();

    httpClient.setTimeout(5000);  // 超时 5 秒

    Serial.println("Sending get");
    int responseCode = httpClient.GET();  // 发送请求

    Serial.println("Stop httpclient");
    httpClient.end();  // 关闭连接

    if (responseCode == HTTP_CODE_NO_CONTENT) {  // 返回 204，连接成功
      connected = true;
      break;
    }
  }
  if (connected) {
    Serial.print("Test Internet connection successfully.\n");
  } else {
    Serial.print("No connection to the Internet.\n");
  }
  return connected;
}


// 获取当前 IP
// 方法1：meow
// 详见：https://github.com/Cookie-Cats/meow

String meow(String meow_url) {
  // 定义反馈结果
  String responsePayload = "0.0.0.0";
  for (int i = 0; i < 2; i++) {  // 尝试2次
    // 实例化 http 客户端
    HTTPClient httpClient;

    httpClient.begin(wifiClient, meow_url);
    Serial.print("Use ");
    Serial.print(meow_url);
    Serial.print(" to get IP.\n");

    Serial.println("Send get");
    int responseCode = httpClient.GET();  // 发送请求

    if (responseCode == HTTP_CODE_OK) {  // 返回 200，连接成功
      responsePayload = httpClient.getString();
      responsePayload.trim();
      break;
    }

    Serial.println("Stop httpclient\n");
    httpClient.end();  // 关闭连接
  }
  if (responsePayload == "0.0.0.0") {  // 如果返回 "0.0.0.0"，则认为获取失败。meow 也是这样定义的。
    Serial.println("Can't connect to meow server.");
  } else {
    Serial.print("Get IP: ");
    Serial.print(responsePayload);
    Serial.println();
  }
  return responsePayload;
}
