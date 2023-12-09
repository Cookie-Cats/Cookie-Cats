#include <stdint.h>
#include <ArduinoJson.h>
#include "pins_arduino.h"
#include "HardwareSerial.h"
#include "auth.h"

using namespace std;

// drcom 网页认证
bool ICACHE_FLASH_ATTR drcomWebAuth(String authURL, WiFiClient &wifiClient) {
  HTTPClient httpClient;                  // 开启 http 客户端
  httpClient.begin(wifiClient, authURL);  // 构建请求
  httpClient.setUserAgent(randomUA());    // 设置 UA
  httpClient.setTimeout(5000);            // 超时 5 秒

  Serial.println(F("Sending auth message..."));
  int responseCode = httpClient.GET();  // 发送请求

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
    } else {
      Serial.println(F("Authentication failed but not accuracy."));
    }
  } else {
    Serial.println(F("Failed to connect to auth server."));
  }
  httpClient.end();  // 关闭连接
  return authSuccess;
}

// drcom 获取 IP 的 API
/*
1. 认证成功前，发送 Get 请求
  命令：curl -G -d "callback=dr1002" -H "User-Agent: 【某UA】" http://192.168.199.21/drcom/chkstatus
  返回值：
     dr1002({"result":0, ..., "ss5":"【本机ipv4】", ...})
2. 认证成功后，发送 Get 请求
  命令：curl -G -d "callback=dr1002" -H "User-Agent: 【某UA】" http://192.168.199.21/drcom/chkstatus
  返回值：
     dr1002({"result":1, ..., "ss5":"【本机ipv4】", ...})
     
返回值中 "ss5" 的项为 IP
*/
String ICACHE_FLASH_ATTR getDrcomIp(String apiURL, WiFiClient &wifiClient) {
  HTTPClient httpClient;                 // 开启 http 客户端
  httpClient.begin(wifiClient, apiURL);  // 构建请求
  httpClient.setUserAgent(randomUA());   // 设置 UA
  httpClient.setTimeout(5000);           // 超时 5 秒

  Serial.println(F("Try to get IP by Drcom API..."));
  int responseCode = httpClient.GET();  // 发送请求

  String IP;
  if (responseCode == HTTP_CODE_OK) {  // 返回 200，连接成功
    Serial.println(F("Connected to Drcom status API."));
    // 寻找本机 IP
    String responsePayload;
    responsePayload = httpClient.getString();
    responsePayload.trim();                                // 删除前后空格
    responsePayload.remove(0, 7);                          // 删除 “dr1002(”
    responsePayload.remove(responsePayload.length() - 1);  // 删除末尾括号
    DynamicJsonDocument responseJson(1024);                // 创建 Json 对象并解码
    DeserializationError error = deserializeJson(responseJson, responsePayload);
    IP = responseJson["ss5"].as<String>();
    if (error || (IP == "")) {  // 如果解析出错
      Serial.println(F("Cannot get local IP."));
    } else {  // 如果可以正确解析
      Serial.print(F("Get IP: "));
      Serial.print(IP);
      Serial.println();
    }
  } else {
    Serial.println(F("Failed to connect to Drcom status API."));
  }
  httpClient.end();  // 关闭连接
  if (IP == "") {
    IP = "0.0.0.0";  // 0.0.0.0 被定义为获取失败
  }
  return IP;
}

// 中国药科大学公共网
bool ICACHE_FLASH_ATTR authChinaPharmaceuticalUniversityPublic(Configuration &config, WiFiClient &wifiClient) {
  // 获取 IP
  String IP;
  if (config.IP_Obtain_Method.first == "unnecessary") {  // 如果设置为 unnecessary，则从 Drcom 的 API 获取
    String apiURL = "http://192.168.199.21/drcom/chkstatus?callback=dr1002";
    IP = getDrcomIp(apiURL, wifiClient);
  } else {
    IP = config.IP_Obtain_Method.second;
  }
  // 构造认证 URL
  String authURL = "http://192.168.199.21:801/eportal/?c=Portal&a=login&callback=dr1004&login_method=1&user_account=,0," + config.username + "&user_password=" + config.password + "&wlan_user_ip=" + IP + "&wlan_user_mac=000000000000&jsVersion=3.3.3&v=10379";
  return drcomWebAuth(authURL, wifiClient);
}

// 中国药科大学宿舍网
bool ICACHE_FLASH_ATTR authChinaPharmaceuticalUniversityDormitory(Configuration &config, WiFiClient &wifiClient) {
  String carrier;                                             // 转换运营商到认证格式
  if (config.carrier == "ChinaTelecom") carrier = "telecom";  // 已在读取 config 时做过判断，carrier 只能为 "ChinaTelecom", "ChinaUnicom", "ChinaMobile", ""
  else if (config.carrier == "ChinaUnicom") carrier = "unicom";
  else if (config.carrier == "ChinaMobile") carrier = "cmcc";

  // 构造认证 URL
  String authURL = "http://172.17.253.3/drcom/login?callback=dr1003&DDDDD=" + config.username + "%40" + carrier + "&upass=" + config.password + "&0MKKey=123456&R1=0&R2=&R3=0&R6=1&para=00&v6ip=&terminal_type=2&lang=zh-cn&jsVersion=4.1.3&v=5336&lang=zh";
  return drcomWebAuth(authURL, wifiClient);
}

// 自动认证
// 警告：使用前需要判断 school、username、password 是否为空
bool auth(Configuration &config, WiFiClient &wifiClient) {
  Serial.println(F("Starting authoriation..."));
  if (config.school == "ChinaPharmaceuticalUniversityDormitory") {  // 中国药科大学宿舍网
    return authChinaPharmaceuticalUniversityDormitory(config, wifiClient);
  } else if (config.school == "ChinaPharmaceuticalUniversityPublic") {  // 中国药科大学公共网
    return authChinaPharmaceuticalUniversityPublic(config, wifiClient);
  } else {
    return false;
  }
}

// 先检测是否联网，如果网络断开则认证
void checkNetAndAuth(Configuration &config, WiFiClient &wifiClient) {
  if (testNet(wifiClient)) {
    Serial.println(F("Connected to the Internet. No need to auth."));
    blink(1);  // 如果可以联网，则闪烁2次
  } else {
    Serial.println(F("No Connection to the Internet. Starting auth..."));
    blink(2);  // 如果不可以联网，则闪烁2次
    auth(config, wifiClient);
  }
}
