#include "HardwareSerial.h"
#include "functions.h"
#include <ArduinoJson.h>

using namespace std;

// 获得随机 UA
String randomUA() {
  static const vector<String> UAs = {
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/118.0.0.0 Safari/537.36",
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/105.0.0.0 Safari/537.36 Edg/105.0.1343.27",
    "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/51.0.2704.106 Safari/537.36 OPR/38.0.2220.41"
  };
  return UAs[random(0, UAs.size() - 1)];
}

// 检测网络通断
bool testNet(WiFiClient wifiClient) {
  Serial.println("Start testing the Internet");
  static const vector<String> testServer = {
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

String meow(String meow_url, WiFiClient wifiClient) {
  // 定义反馈结果
  String responsePayload = "0.0.0.0";
  for (int i = 0; i < 2; i++) {  // 尝试2次
    // 实例化 http 客户端
    HTTPClient httpClient;

    httpClient.begin(wifiClient, meow_url);
    Serial.print("Use ");
    Serial.print(meow_url);
    Serial.print(" to get IP.\n");

    Serial.println("Send get.");
    int responseCode = httpClient.GET();  // 发送请求

    if (responseCode == HTTP_CODE_OK) {  // 返回 200，连接成功
      responsePayload = httpClient.getString();
      responsePayload.trim();
      break;
    }

    Serial.println("Stop httpclient.\n");
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

// 用于 http 服务器，获取文件类型
String httpGetContentType(String filename) {
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

// 读取配置文件
Configuration readConfigurationFromFile(File file, Configuration configuration) {
  Serial.println("Found config.json, reading...");
  DynamicJsonDocument config(1024);                            // 分配一个 JSON 文档对象
  DeserializationError error = deserializeJson(config, file);  // 解析文件内容为 JSON 对象
  if (error) {
    Serial.println("Parsing config.json fails, using the default configuration.");
  }

  // 读取内容并写配置
  // 因为这里内容不一定都包含，所以逐个写
  // 如果获取内容为空，则仍使用默认

  // 自身 WiFi 名
  if (config.containsKey("Cookie_Cat_SSID")) {
    const char* value = (const char*)config["Cookie_Cat_SSID"];
    // 判断值是否为空，如果不为空，则赋给String类型的变量
    if (value && strlen(value) > 0) {
      configuration.Cookie_Cat_SSID = value;
      Serial.print("Set Cookie_Cat_SSID into: ");
      Serial.print(configuration.Cookie_Cat_SSID);
      Serial.println();
    } else {
      Serial.println("Cookie_Cat_SSID not set.");
    }
  } else {
    Serial.println("No Cookie_Cat_SSID found in config.json, use CookieCat instead.");
  }

  // 自身 WiFi 密码
  if (config.containsKey("Cookie_Cat_PASSWORD")) {
    const char* value = (const char*)config["Cookie_Cat_PASSWORD"];
    if (value && strlen(value) > 0) {
      configuration.Cookie_Cat_PASSWORD = value;
      Serial.print("Set Cookie_Cat_PASSWORD into: ");
      Serial.print(configuration.Cookie_Cat_PASSWORD);
      Serial.println();
    } else {
      Serial.println("Cookie_Cat_PASSWORD not set.");
    }
  } else {
    Serial.println("No Cookie_Cat_PASSWORD found in config.json, use okgogogo instead.");
  }

  // 连接 WiFi SSID
  if (config.containsKey("WiFi_SSID")) {
    const char* value = (const char*)config["WiFi_SSID"];
    if (value && strlen(value) > 0) {
      configuration.WiFi_SSID = value;
      Serial.print("Set WiFi_SSID into: ");
      Serial.print(configuration.WiFi_SSID);
      Serial.println();
    } else {
      Serial.println("WiFi_SSID not set.");
    }
  } else {
    Serial.println("No WiFi_SSID found in config.json");
  }

  // 连接 WiFi 密码
  if (config.containsKey("WiFi_PASSWORD")) {
    const char* value = (const char*)config["WiFi_PASSWORD"];
    if (value && strlen(value) > 0) {
      configuration.WiFi_PASSWORD = value;
      Serial.print("Set WiFi_PASSWORD into: ");
      Serial.print(configuration.WiFi_PASSWORD);
      Serial.println();
    } else {
      Serial.println("WiFi_PASSWORD not set.");
    }
  } else {
    Serial.println("No WiFi_PASSWORD found in config.json");
  }

  // 认证用户名
  if (config.containsKey("username")) {
    const char* value = (const char*)config["username"];
    if (value && strlen(value) > 0) {
      configuration.username = value;
      Serial.print("Set username into: ");
      Serial.print(configuration.username);
      Serial.println();
    } else {
      Serial.println("username not set.");
    }
  } else {
    Serial.println("No username found in config.json");
  }

  // 认证密码
  if (config.containsKey("password")) {
    const char* value = (const char*)config["password"];
    if (value && strlen(value) > 0) {
      configuration.password = value;
      Serial.print("Set password into: ");
      Serial.print(configuration.password);
      Serial.println();
    } else {
      Serial.println("password not set.");
    }
  } else {
    Serial.println("No password found in config.json");
  }

  // 运营商
  if (config.containsKey("carrier")) {
    vector<String> Carriers = { "ChinaTelecom", "ChinaUnicom", "ChinaMobile" };
    const char* carrier = (const char*)config["carrier"];

    auto it = find(Carriers.begin(), Carriers.end(), carrier);  // 遍历运营商列表
    if (it != Carriers.end()) {                                 // 如果找到了元素
      configuration.carrier = carrier;
    } else {  // 如果没有找到元素
      Serial.println("Carrier not set or wrong set. Carrier could only be one of ChinaTelecom, ChinaUnicom, ChinaMobile.");
    }
  } else {
    Serial.println("No carrier found in config.json");
  }

  // 学校
  if (config.containsKey("school")) {
    vector<String> Schools = { "ChinaPharmaceuticalUniversity" };
    const char* school = (const char*)config["school"];

    auto it = find(Schools.begin(), Schools.end(), school);  // 遍历支持的学校列表
    if (it != Schools.end()) {                               // 如果找到了元素
      configuration.school = school;
    } else {  // 如果没有找到元素
      Serial.println("School not set or unsupported.");
    }
  } else {
    Serial.println("No school found in config.json");
  }

  // IP 获取方式
  if (config.containsKey("IP_Obtain_Method")) {
    JsonObject IP_Obtain_Method = config["IP_Obtain_Method"];
    if (IP_Obtain_Method.containsKey("meow")) {  // 采用 meow
      const char* url = (const char*)IP_Obtain_Method["meow"];
      configuration.IP_Obtain_Method = { "meow", url };
      Serial.print("Set meow URL: ");
      Serial.print(url);
      Serial.println();
    } else if (IP_Obtain_Method.containsKey("manual")) {  // 采用 manual
      const char* ip = (const char*)IP_Obtain_Method["manual"];
      configuration.IP_Obtain_Method = { "manual", ip };
      Serial.print("Get manual input IP: ");
      Serial.print(ip);
      Serial.println();
    }
  } else {
    Serial.println("No IP_Obtain_Method found in config.json");
  }
  Serial.println("Read config.json completed.");
  return configuration;
}