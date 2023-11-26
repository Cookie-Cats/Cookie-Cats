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
bool testNet(WiFiClient& wifiClient) {
  Serial.println(F("Start testing the Internet"));
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
    Serial.print(F("Use "));
    Serial.print(URL);
    Serial.print(F(" to test.\n"));

    String UA = randomUA();
    httpClient.setUserAgent(UA);
    Serial.print(F("User-Agent: "));
    Serial.print(UA);
    Serial.println();

    httpClient.setTimeout(5000);  // 超时 5 秒

    Serial.println(F("Sending get"));
    int responseCode = httpClient.GET();  // 发送请求

    Serial.println(F("Stop httpclient"));
    httpClient.end();  // 关闭连接

    if (responseCode == HTTP_CODE_NO_CONTENT) {  // 返回 204，连接成功
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

String ICACHE_FLASH_ATTR meow(String meow_url, WiFiClient& wifiClient) {
  // 定义反馈结果
  String responsePayload = "0.0.0.0";
  for (int i = 0; i < 2; i++) {  // 尝试2次
    // 实例化 http 客户端
    HTTPClient httpClient;

    httpClient.begin(wifiClient, meow_url);
    Serial.print(F("Use "));
    Serial.print(meow_url);
    Serial.print(F(" to get IP.\n"));

    Serial.println(F("Send get."));
    int responseCode = httpClient.GET();  // 发送请求

    if (responseCode == HTTP_CODE_OK) {  // 返回 200，连接成功
      responsePayload = httpClient.getString();
      responsePayload.trim();
      break;
    }

    Serial.println(F("Stop httpclient.\n"));
    httpClient.end();  // 关闭连接
  }
  if (responsePayload == "0.0.0.0") {  // 如果返回 "0.0.0.0"，则认为获取失败。meow 也是这样定义的。
    Serial.println(F("Can't connect to meow server."));
  } else {
    Serial.print(F("Get IP: "));
    Serial.print(responsePayload);
    Serial.println();
  }
  return responsePayload;
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
      Serial.println(F("Cookie_Cat_SSID not set."));
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
    vector<String> Carriers = { "ChinaTelecom", "ChinaUnicom", "ChinaMobile" };
    const char* carrier = (const char*)config["carrier"];

    auto it = find(Carriers.begin(), Carriers.end(), carrier);  // 遍历运营商列表
    if (it != Carriers.end()) {                                 // 如果找到了元素
      configuration.carrier = carrier;
    } else {  // 如果没有找到元素
      Serial.println(F("Carrier not set or wrong set. Carrier could only be one of ChinaTelecom, ChinaUnicom, ChinaMobile."));
    }
  } else {
    Serial.println(F("No carrier found in config.json"));
  }

  // 学校
  if (config.containsKey("school")) {
    vector<String> Schools = { "ChinaPharmaceuticalUniversityDormitory" };
    const char* school = (const char*)config["school"];

    auto it = find(Schools.begin(), Schools.end(), school);  // 遍历支持的学校列表
    if (it != Schools.end()) {                               // 如果找到了元素
      configuration.school = school;
    } else {  // 如果没有找到元素
      Serial.println(F("School not set or unsupported."));
    }
  } else {
    Serial.println(F("No school found in config.json"));
  }

  // IP 获取方式
  if (config.containsKey("IP_Obtain_Method_Content")) {
    const char* IP_Obtain_Method_Content = (const char*)config["IP_Obtain_Method_Content"];
    if (IP_Obtain_Method_Content && strlen(IP_Obtain_Method_Content) > 0) {  // 如果 IP_Obtain_Method_Content 不为空
      if (config.containsKey("IP_Obtain_Method")) {
        const char* IP_Obtain_Method = (const char*)config["IP_Obtain_Method"];
        vector<String> IP_Obtain_Methods = { "unnecessary", "meow", "manual" };
        auto it = find(IP_Obtain_Methods.begin(), IP_Obtain_Methods.end(), IP_Obtain_Method);
        if (it != IP_Obtain_Methods.end()) {  // 如果找到了方法
          configuration.IP_Obtain_Method = IP_Obtain_Method;
          configuration.IP_Obtain_Method_Content = IP_Obtain_Method_Content;
          Serial.print(F("Set IP_Obtain_Method to:"));
          Serial.print(IP_Obtain_Method);
          Serial.println();
          Serial.print(F("Set IP_Obtain_Method_Content to: "));
          Serial.print(IP_Obtain_Method_Content);
          Serial.println();
        } else {
          Serial.println(F("Unsupported IP_Obtain_Method. IP_Obtain_Method could only be one of meow, manual, unnecessary. Skip."));
        }
      } else {
        Serial.println(F("No IP_Obtain_Method found. Skip."));
      }
    } else {
      Serial.println(F("IP_Obtain_Method_Content is None. Skip."));
    }
  } else {
    Serial.println(F("No IP_Obtain_Method_Content found. Skip."));
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
void ICACHE_FLASH_ATTR otaUpdate(WiFiClient& wifiClient, String updateURL, String currentVersion) {
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
