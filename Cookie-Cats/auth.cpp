#include <stdint.h>
#include <ArduinoJson.h>
#include "pins_arduino.h"
#include "HardwareSerial.h"
#include "auth.h"

using namespace std;

// drcom 网页认证
bool ICACHE_FLASH_ATTR drcomWebAuth(String authURL) {
  Serial.println(F("Sending auth message..."));
  HttpResponse response;
  response = sendHttpRequest(authURL);  // 发送 HTTP 请求

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
String ICACHE_FLASH_ATTR getDrcomIp(String apiURL) {
  Serial.println(F("Try to get IP by Drcom API..."));
  HttpResponse response;
  response = sendHttpRequest(apiURL);  // 发送 HTTP 请求

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
bool ICACHE_FLASH_ATTR authCPU(Configuration &config) {
  if (config.carrier == "SchoolNetwork") {                 // 中国药科大学校园网   
    String IP = getIPFromIPObtainMethod(config);  // 获取 IP
    if (IP == "0.0.0.0") {  // 未设置 IP 获取方式或选择 unnecessary
      String apiURL = "http://192.168.199.21/drcom/chkstatus?callback=dr1002";
      IP = getDrcomIp(apiURL);
    }

    // 构造认证 URL
    String authURL = "http://192.168.199.21:801/eportal/?c=Portal&a=login&callback=dr1004&login_method=1&user_account=,0," + config.username + "&user_password=" + config.password + "&wlan_user_ip=" + IP + "&wlan_user_mac=000000000000&jsVersion=3.3.3&v=10379";
    return drcomWebAuth(authURL);                               // 发送认证请求
  } else {                                                      // 中国药科大学宿舍网
    String carrier;                                             // 转换运营商到认证格式
    if (config.carrier == "ChinaTelecom") carrier = "telecom";  // 已在读取 config 时做过判断，carrier 只能为 "ChinaTelecom", "ChinaUnicom", "ChinaMobile", "SchoolNetwork", ""
    else if (config.carrier == "ChinaUnicom") carrier = "unicom";
    else if (config.carrier == "ChinaMobile") carrier = "cmcc";
    String authURL = "http://172.17.253.3/drcom/login?callback=dr1003&DDDDD=" + config.username + "%40" + carrier + "&upass=" + config.password + "&0MKKey=123456&R1=0&R2=&R3=0&R6=1&para=00&v6ip=&terminal_type=2&lang=zh-cn&jsVersion=4.1.3&v=5336&lang=zh";
    return drcomWebAuth(authURL);  // 发送认证请求
  }
}
// --------------------------------------------------------------------------------------------------------------------------------

// 南京邮电大学
// 参考自：https://github.com/Lintkey/njupt_net
// 状态：正在实现|未验证
bool ICACHE_FLASH_ATTR authNJUPT(Configuration &config) {
  String username;
  if (config.carrier == "ChinaTelecom") username = ",0," + config.username + "@njxy";      // 电信
  else if (config.carrier == "ChinaMobile") username = ",0," + config.username + "@cmcc";  // 移动
  else if (config.carrier == "SchoolNetwork") username = config.username;                  // 校园网

  // 构造认证 URL
  // 待确认：似乎并没有可用的 DrCOM API 获取 IP
  String authURL = "https://p.njupt.edu.cn:802/eportal/portal/login?callback=dr1003&login_method=1&user_account=" + username + "&user_password=" + config.password + "&wlan_user_ip=" + config.IP_Obtain_Method.second + "&wlan_user_ipv6=&wlan_user_mac=000000000000&wlan_ac_ip=&wlan_ac_name=&jsVersion=4.1.3&terminal_type=1&lang=zh-cn&v=6407&lang=zh";

  return drcomWebAuth(authURL);
}

// 自动认证
// 警告：使用前需要判断 school、username、password 是否为空
bool auth(Configuration &config) {
  Serial.println(F("Starting authoriation..."));
  if (config.school == "CPU") {  // 中国药科大学
    return authCPU(config);
  } else if (config.school == "NJUPT") {
    return authNJUPT(config);  // 南京邮电大学
  } else {
    return false;
  }
}

// 先检测是否联网，如果网络断开则认证
void checkNetAndAuth(Configuration &config) {
  if (testNet()) {
    Serial.println(F("Connected to the Internet. No need to auth."));
    blink(1);  // 如果可以联网，则闪烁2次
  } else {
    Serial.println(F("No Connection to the Internet. Starting auth..."));
    blink(2);  // 如果不可以联网，则闪烁2次
    auth(config);
  }
}
