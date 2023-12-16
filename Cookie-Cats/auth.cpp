#include <stdint.h>
#include <ArduinoJson.h>
#include "pins_arduino.h"
#include "HardwareSerial.h"
#include "auth.h"

using namespace std;

// drcom 网页认证
bool ICACHE_FLASH_ATTR drcomWebAuth(String authURL, WiFiClient &wifiClient) {
  Serial.println(F("Sending auth message..."));
  HttpResponse response;
  response = sendHttpRequest(authURL, wifiClient);  // 发送 HTTP 请求

  bool authSuccess = false;
  if (response.status_code == HTTP_CODE_OK) {  // 返回 200，连接成功
    Serial.println(F("Connected to auth server."));
    // 如果返回值包含："result":1 则认为认证成功
    int index = response.content.indexOf("\"result\":1");
    if (index != -1) {  // 认证成功
      Serial.println(F("Authentication success."));
      authSuccess = true;
    } else {
      Serial.println(F("Authentication failed but not accuracy."));
    }
  } else {
    Serial.println(F("Failed to connect to auth server."));
  }
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
  Serial.println(F("Try to get IP by Drcom API..."));
  HttpResponse response;
  response = sendHttpRequest(apiURL, wifiClient);  // 发送 HTTP 请求

  String IP;
  if (response.status_code == HTTP_CODE_OK) {  // 返回 200，连接成功
    Serial.println(F("Connected to Drcom status API."));
    // 寻找本机 IP
    response.content.trim();                                 // 删除前后空格
    response.content.remove(0, 7);                           // 删除 “dr1002(”
    response.content.remove(response.content.length() - 1);  // 删除末尾括号
    DynamicJsonDocument responseJson(1024);                  // 创建 Json 对象并解码
    DeserializationError error = deserializeJson(responseJson, response.content);
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
  if (IP == "") {
    IP = "0.0.0.0";  // 0.0.0.0 被定义为获取失败
  }
  return IP;
}

// --------------------------------------------------------------------------------------------------------------------------------
// 注意：这个是 DrCom 认证的参考实现

// 中国药科大学
// 状态：已实现|已验证
bool ICACHE_FLASH_ATTR authCPU(Configuration &config, WiFiClient &wifiClient) {
  if (config.carrier == "SchoolNetwork") {                 // 中国药科大学校园网
    String IP;                                             // 获取 IP
    if (config.IP_Obtain_Method.first == "unnecessary") {  // 如果设置为 unnecessary，则从 Drcom 的 API 获取
      String apiURL = "http://192.168.199.21/drcom/chkstatus?callback=dr1002";
      IP = getDrcomIp(apiURL, wifiClient);
    } else {
      IP = config.IP_Obtain_Method.second;
    }
    // 构造认证 URL
    String authURL = "http://192.168.199.21:801/eportal/?c=Portal&a=login&callback=dr1004&login_method=1&user_account=,0," + config.username + "&user_password=" + config.password + "&wlan_user_ip=" + IP + "&wlan_user_mac=000000000000&jsVersion=3.3.3&v=10379";
    return drcomWebAuth(authURL, wifiClient);
  } else {                                                      // 中国药科大学宿舍网
    String carrier;                                             // 转换运营商到认证格式
    if (config.carrier == "ChinaTelecom") carrier = "telecom";  // 已在读取 config 时做过判断，carrier 只能为 "ChinaTelecom", "ChinaUnicom", "ChinaMobile", "SchoolNetwork", ""
    else if (config.carrier == "ChinaUnicom") carrier = "unicom";
    else if (config.carrier == "ChinaMobile") carrier = "cmcc";
    String authURL = "http://172.17.253.3/drcom/login?callback=dr1003&DDDDD=" + config.username + "%40" + carrier + "&upass=" + config.password + "&0MKKey=123456&R1=0&R2=&R3=0&R6=1&para=00&v6ip=&terminal_type=2&lang=zh-cn&jsVersion=4.1.3&v=5336&lang=zh";
    return drcomWebAuth(authURL, wifiClient);
  }
}
// --------------------------------------------------------------------------------------------------------------------------------

// 南京邮电大学
// 参考自：https://github.com/Lintkey/njupt_net
// 状态：正在实现|未验证
bool ICACHE_FLASH_ATTR authNJUPT(Configuration &config, WiFiClient &wifiClient) {
  String carrier;
  if (config.carrier == "ChinaTelecom") carrier = "@njxy";      // 电信
  else if (config.carrier == "ChinaMobile") carrier = "@cmcc";  // 移动
  else if (config.carrier == "SchoolNetwork") carrier = "";     // 校园网

  // 构造认证 URL
  // TODO: 可能可以使用 DrCom API 获取 IP，需要贵学校帮助
  String authURL = "https://p.njupt.edu.cn:802/eportal/portal/login?callback=dr1003&login_method=1&user_account=" + config.username + carrier + "&user_password=" + config.password + "&wlan_user_ip=" + config.IP_Obtain_Method.second + "&wlan_user_ipv6=&wlan_user_mac=000000000000&wlan_ac_ip=&wlan_ac_name=&jsVersion=4.1.3&terminal_type=1&lang=zh-cn&v=6407&lang=zh";

  HttpResponse response;
  response = sendHttpRequest(authURL, wifiClient);  // 发送认证请求

  // 测试网络连通情况
  // 由于未知认证成功的特征，因此采用是否可以连通外网判断是否认证成功
  return testNet(wifiClient);
}

// 自动认证
// 警告：使用前需要判断 school、username、password 是否为空
bool auth(Configuration &config, WiFiClient &wifiClient) {
  Serial.println(F("Starting authoriation..."));
  if (config.school == "CPU") {  // 中国药科大学
    return authCPU(config, wifiClient);
  } else if (config.school == "NJUPT") {
    return authNJUPT(config, wifiClient);  // 南京邮电大学
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
