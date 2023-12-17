#include <ArduinoJson.h>
#include "HardwareSerial.h"
#include "functions.h"

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

// 发送 http 请求
// send_type: GET/POST
HttpResponse sendHttpRequest(String url, String send_type, String post_payload, String user_agent, int timeout) {
  /*
  警告：由于 ESP8266 内存空间问题和更新问题，以及校园网内发生 MITM 的概率不大，这里的 HTTPS 是不安全的。它不会验证服务器的身份，可能会导致中间人攻击或数据泄露。
  */
  WiFiClient wifiClient;
  if (url.startsWith("https")) {
    WiFiClientSecure wifiClient;
    wifiClient.setInsecure();  // 不安全的 HTTPS
  }

  HTTPClient httpClient;                // 实例化 http 客户端
  httpClient.begin(wifiClient, url);    // 设置发送 URL
  httpClient.setUserAgent(user_agent);  // 设置 UA
  httpClient.setTimeout(timeout);       // 设置超时

  Serial.print(F("Send "));
  Serial.print(send_type);
  Serial.print(F(" request to "));
  Serial.print(url);
  Serial.print(F(" with UA: "));
  Serial.print(user_agent);
  Serial.println();

  HttpResponse response;
  if (send_type == "GET") {
    response.status_code = httpClient.GET();  // 状态码
  } else if (send_type == "POST") {
    response.status_code = httpClient.POST(post_payload);  // 状态码
  } else {
    Serial.println(F("send_type could only be GET or POST."));
  }
  response.content = httpClient.getString();  // 响应内容
  httpClient.end();                           // 关闭连接

  return response;
}

// 检测网络通断
bool testNet() {
  static const vector<String> testServer = {
    "http://connect.rom.miui.com/generate_204",                    // 小米
    "http://connectivitycheck.platform.hicloud.com/generate_204",  // 华为
    "http://wifi.vivo.com.cn/generate_204"                         // vivo
  };

  bool connected = false;

  for (int i = 0; i < 2; i++) {  // 尝试2次
    String URL = testServer[random(0, testServer.size() - 1)];
    Serial.println(F("Start testing the Internet..."));

    HttpResponse response;
    response = sendHttpRequest(URL);  // 发送 HTTP 请求

    if (response.status_code == HTTP_CODE_NO_CONTENT) {  // 返回 204，连接成功
      connected = true;
      break;
    }
  }
  if (connected) {
    Serial.println(F("Test Internet connection successfully."));
  } else {
    Serial.println(F("No connection to the Internet."));
  }
  return connected;
}

// 获取当前 IP
// 方法1：meow
// 详见：https://github.com/Cookie-Cats/meow

String ICACHE_FLASH_ATTR meow(String meow_url) {
  // 定义反馈结果
  HttpResponse response;
  for (int i = 0; i < 2; i++) {  // 尝试2次
    Serial.print(F("Use "));
    Serial.print(meow_url);
    Serial.print(F(" to get IP.\n"));
    response = sendHttpRequest(meow_url);  // 发送 HTTP 请求

    if (response.status_code == HTTP_CODE_OK) {  // 返回 200，连接成功
      response.content.trim();
      break;
    } else {
      response.content = "0.0.0.0";
    }
  }
  if (response.content == "0.0.0.0") {  // 如果返回 "0.0.0.0"，则认为获取失败。meow 也是这样定义的。
    Serial.println(F("Can't connect to meow server."));
  } else {
    Serial.print(F("Get IP: "));
    Serial.print(response.content);
    Serial.println();
  }
  return response.content;
}

// 用于 http 服务器，获取文件类型
String ICACHE_FLASH_ATTR httpGetContentType(String filename) {
  if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  return "text/plain";
}

// 读取配置文件
void ICACHE_FLASH_ATTR readConfigurationFromFile(File& file, Configuration& configuration, PracticalCrypto& secret) {
  Serial.println(F("Found config.json, reading..."));
  DynamicJsonDocument config(1024);                            // 分配一个 JSON 文档对象
  DeserializationError error = deserializeJson(config, file);  // 解析文件内容为 JSON 对象
  if (error) {
    Serial.println(F("Parsing config.json fails, using the default configuration."));
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
      Serial.print(F("Set Cookie_Cat_SSID into: "));
      Serial.print(configuration.Cookie_Cat_SSID);
      Serial.println();
    } else {
      Serial.println(F("Cookie_Cat_SSID not set, use CookieCat instead."));
    }
  } else {
    Serial.println(F("No Cookie_Cat_SSID found in config.json, use CookieCat instead."));
  }

  // 自身 WiFi 密码
  if (config.containsKey("Cookie_Cat_PASSWORD")) {
    const char* value = (const char*)config["Cookie_Cat_PASSWORD"];
    if (value && strlen(value) > 0) {
      configuration.Cookie_Cat_PASSWORD = value;
      Serial.print(F("Set Cookie_Cat_PASSWORD into: "));
      Serial.print(configuration.Cookie_Cat_PASSWORD);
      Serial.println();
    } else {
      configuration.Cookie_Cat_PASSWORD = "cookiecat";
      Serial.println(F("Cookie_Cat_PASSWORD not set or NULL. Empty password is not allowed. Use cookiecat as default."));
    }
  } else {
    configuration.Cookie_Cat_PASSWORD = "cookiecat";
    Serial.println(F("No Cookie_Cat_PASSWORD found in config.json, Use cookiecat as default."));
  }

  // 连接 WiFi SSID
  if (config.containsKey("WiFi_SSID")) {
    const char* value = (const char*)config["WiFi_SSID"];
    configuration.WiFi_SSID = value;
    Serial.print(F("Set WiFi_SSID into: "));
    Serial.print(configuration.WiFi_SSID);
    Serial.println();
  } else {
    Serial.println(F("No WiFi_SSID found in config.json"));
  }

  // 连接 WiFi 密码
  if (config.containsKey("WiFi_PASSWORD")) {
    const char* value = (const char*)config["WiFi_PASSWORD"];
    if (value && strlen(value) > 0) {
      configuration.WiFi_PASSWORD = value;
      Serial.print(F("Set WiFi_PASSWORD into: "));
      Serial.print(configuration.WiFi_PASSWORD);
      Serial.println();
    } else {
      Serial.println(F("WiFi_PASSWORD not set."));
    }
  } else {
    Serial.println(F("No WiFi_PASSWORD found in config.json"));
  }

  // 认证用户名
  if (config.containsKey("username")) {
    const char* value = (const char*)config["username"];
    if (value && strlen(value) > 0) {
      configuration.username = value;
      Serial.print(F("Set username into: "));
      Serial.print(configuration.username);
      Serial.println();
    } else {
      Serial.println(F("username not set."));
    }
  } else {
    Serial.println(F("No username found in config.json"));
  }

  // 认证密码
  if (config.containsKey("password")) {
    const char* value = (const char*)config["password"];
    if (value && strlen(value) > 0) {
      configuration.password = secret.decrypt(value);
      Serial.println(F("The password is set."));
    } else {
      configuration.password = value;
      Serial.println(F("password not set or NULL."));
    }
  } else {
    Serial.println(F("No password found in config.json"));
  }

  // 运营商
  if (config.containsKey("carrier")) {
    vector<String> Carriers = { "ChinaTelecom", "ChinaUnicom", "ChinaMobile", "SchoolNetwork" };
    const char* carrier = (const char*)config["carrier"];

    auto it = find(Carriers.begin(), Carriers.end(), carrier);  // 遍历运营商列表
    if (it != Carriers.end()) {                                 // 如果找到了元素
      configuration.carrier = carrier;
    } else {  // 如果没有找到元素
      Serial.println(F("Carrier not set or wrong set. Carrier could only be one of ChinaTelecom, ChinaUnicom, ChinaMobile, SchoolNetwork."));
    }
  } else {
    Serial.println(F("No carrier found in config.json"));
  }

  // 学校
  if (config.containsKey("school")) {
    vector<String> Schools = { "CPU", "NJUPT" };
    String school = (const char*)config["school"];

    // --------------------------------------------------------------------------------------------------------------------------------
    /*
    兼容代码：
    PIONEER_0.1_alpha_prerelease_016 及以前版本配置文件此选项包含 ChinaPharmaceuticalUniversityDormitory 或 ChinaPharmaceuticalUniversityPublic
    之后版本这两个选项均指向 CPU
    此兼容代码可能在 PIONEER_0.1_alpha 版本去除
    */
    if ((school == "ChinaPharmaceuticalUniversityDormitory") || (school == "ChinaPharmaceuticalUniversityPublic")) {
      Serial.println(F("Old config found. Automatic conversion to the new format."));
      school = "CPU";
      config["school"] = "CPU";
      File configFile = LittleFS.open("/config.json", "w");
      serializeJson(config, configFile);  // 写入新配置
      configFile.close();
    }
    // --------------------------------------------------------------------------------------------------------------------------------

    auto it = find(Schools.begin(), Schools.end(), school);  // 遍历支持的学校列表
    if (it != Schools.end()) {                               // 如果找到了元素
      Serial.print(F("Set school into: "));
      Serial.print(school);
      Serial.println();
      configuration.school = school;
    } else {  // 如果没有找到元素
      Serial.println(F("School not set or unsupported."));
    }
  } else {
    Serial.println(F("No school found in config.json"));
  }

  // IP 获取方式
  if (config.containsKey("IP_Obtain_Method")) {
    JsonObject IP_Obtain_Method = config["IP_Obtain_Method"];
    if (IP_Obtain_Method.containsKey("meow")) {  // 采用 meow
      const char* url = (const char*)IP_Obtain_Method["meow"];
      configuration.IP_Obtain_Method = { "meow", url };
      Serial.print(F("Set meow URL: "));
      Serial.print(url);
      Serial.println();
    } else if (IP_Obtain_Method.containsKey("manual")) {  // 采用 manual
      const char* ip = (const char*)IP_Obtain_Method["manual"];
      configuration.IP_Obtain_Method = { "manual", ip };
      Serial.print(F("Get manual input IP: "));
      Serial.print(ip);
      Serial.println();
    } else if (IP_Obtain_Method.containsKey("unnecessary")) {         // 采用 unnecessary
      configuration.IP_Obtain_Method = { "unnecessary", "0.0.0.0" };  // 默认 IP 0.0.0.0
      Serial.println(F("Set IP_Obtain_Method into unnecessary."));
    }
  } else {
    Serial.println(F("IP_Obtain_Method not set or wrong set. IP_Obtain_Method could only be one of meow, manual, unnecessary."));
  }

  // 是否允许 OTA 更新
  // 默认为 true
  if (config.containsKey("allowOTA")) {
    const char* value = (const char*)config["allowOTA"];
    if (strcmp(value, "false") == 0) {
      configuration.allowOTA = false;
      Serial.println(F("Disable OTA update."));
    } else {
      configuration.allowOTA = true;
      Serial.println(F("Enable OTA update."));
    }
  } else {
    configuration.allowOTA = true;
    Serial.println(F("No OTA config set, use true as default."));
  }

  Serial.println(F("Read config.json completed."));
}

// OTA 更新
void ICACHE_FLASH_ATTR otaUpdate(String updateURL, String currentVersion) {
  WiFiClient wifiClient;
  t_httpUpdate_return ret = ESPhttpUpdate.update(wifiClient, updateURL, currentVersion);
  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.println(F("OTA Update failed."));
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println(F("Firmware is up to date."));
      break;
    case HTTP_UPDATE_OK:
      Serial.println(F("OTA Update ok."));  // May not be called since we reboot the ESP
      break;
  }
}

// 读取是否存在 .secret 密钥文件
// 如果不存在，就生成一个
void ICACHE_FLASH_ATTR readSecret(PracticalCrypto& secret) {
  // 读取是否存在 .secret 密钥文件
  // 如果不存在，就生成一个
  Serial.println(F("Loading secret..."));
  if (LittleFS.exists("/.secret")) {
    File file = LittleFS.open("/.secret", "r");
    String data = file.readString();
    secret.setKey(data);
    file.close();
    Serial.println(F("Found secret."));
  } else {
    File file = LittleFS.open("/.secret", "w");
    String key = secret.generateKey();  // 生成密钥
    yield();
    file.write(key.c_str());
    file.close();        // 写入密钥，关闭文件
    secret.setKey(key);  // 设置密钥
    Serial.println(F("Cannot find secret, generate a new one."));
  }
}

// 定义一个函数，参数为times，表示 led 闪烁的次数，闪灯间隔 200 毫秒
void blink(int times) {
  unsigned long last_change = 0;  // 定义一个变量，表示上一次改变led状态的时间，初始为0
  unsigned long interval = 200;   // 闪灯间隔 200 毫秒
  int count = 0;                  // 已经闪烁的次数
  bool LED_STATE = true;          // LED 状态

  if (digitalRead(LED_BUILTIN) == false) digitalWrite(LED_BUILTIN, true);  // 如果初始为 HIGH，则改为 LOW

  while (count < times) {                            // 使用一个循环，直到闪烁的次数达到参数times为止
    unsigned long current_time = millis();           // 获取当前的时间，单位为毫秒
    if (current_time - last_change >= interval) {    // 如果当前时间减去上一次改变led状态的时间大于或等于闪灯间隔
      LED_STATE = LED_STATE == true ? false : true;  // 则改变led的状态，如果原来是LOW，就变成HIGH，反之亦然
      digitalWrite(LED_BUILTIN, LED_STATE);          // 将led的状态写入led引脚
      last_change = current_time;                    // 更新上一次改变led状态的时间为当前时间
      if (LED_STATE == true) {                       // 如果led的状态是LOW，说明已经完成了一次闪烁，将闪烁次数加一
        count++;
        yield();
      }
    }
  }
  digitalWrite(LED_BUILTIN, true);  // 熄灯
}
