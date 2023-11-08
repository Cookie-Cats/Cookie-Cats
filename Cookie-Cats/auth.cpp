#include "HardwareSerial.h"
#include "auth.h"

using namespace std;

bool ICACHE_FLASH_ATTR authChinaPharmaceuticalUniversity(Configuration &config, WiFiClient &wifiClient) {
  String carrier;                                             // 转换运营商到认证格式
  if (config.carrier == "ChinaTelecom") carrier = "telecom";  // 已在读取 config 时做过判断，carrier 只能为 "ChinaTelecom", "ChinaUnicom", "ChinaMobile", ""
  else if (config.carrier == "ChinaUnicom") carrier = "unicom";
  else if (config.carrier == "ChinaMobile") carrier = "cmcc";

  // 构造认证 URL
  String authURL = "http://sushe.cpu.edu.cn/drcom/login?callback=dr1003&DDDDD=" + config.username + "%40" + carrier + "&upass=" + config.password + "&0MKKey=123456&R1=0&R2=&R3=0&R6=1&para=00&v6ip=&terminal_type=2&lang=zh-cn&jsVersion=4.1.3&v=5336&lang=zh";

  HTTPClient httpClient;                  // 开启 http 客户端
  httpClient.begin(wifiClient, authURL);  // 构建请求
  httpClient.setUserAgent(randomUA());    // 设置 UA
  httpClient.setTimeout(5000);            // 超时 5 秒

  Serial.println(F("Sending auth message..."));
  int responseCode = httpClient.GET();  // 发送请求
  httpClient.end();                     // 关闭连接

  bool authSuccess = false;
  if (responseCode == HTTP_CODE_OK) {  // 返回 200，连接成功
    Serial.println(F("Connected to auth server."));
    // 如果返回值包含："result":1 则认为认证成功
    String responsePayload;
    responsePayload = httpClient.getString();
    int index = responsePayload.indexOf("\"result\":1");
    if (index != -1) {  // 认证成功
      Serial.println(F("Authentication success."));
      authSuccess = true;
    }
  } else {
    Serial.println(F("Failed to connect to auth server."));
  }
  return authSuccess;
}

// 自动认证
// 警告：使用前需要判断 school、username、password 是否为空
bool auth(Configuration &config, WiFiClient &wifiClient) {
  Serial.println(F("Starting authoriation..."));
  if (config.school == "ChinaPharmaceuticalUniversity") {  // 中国药科大学
    return authChinaPharmaceuticalUniversity(config, wifiClient);
  } else {
    return false;
  }
}

// 先检测是否联网，如果网络断开则认证
void checkNetAndAuth(Configuration &config, WiFiClient &wifiClient) {
  if (testNet(wifiClient)) {
    Serial.println(F("Connected to the Internet. No need to auth."));
  } else {
    Serial.println(F("No Connection to the Internet. Starting auth..."));
    auth(config, wifiClient);
  }
}
