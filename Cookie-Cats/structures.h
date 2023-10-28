#ifndef STRUCTURES_H
#define STRUCTURES_H

enum class Carrier {
  ChinaTelecom,  // 中国电信
  ChinaUnicom,   // 中国联通
  ChinaMobile,   // 中国移动
  None           // 空值
};

class Configuration {
public:
  // 自身 WiFi
  String Cookie_Cat_SSID = "CookieCat";
  String Cookie_Cat_PASSWORD = "cookiecat";
  // 连接 WiFi
  String WiFi_SSID;
  String WiFi_PASSWORD;
  // 认证
  String username;
  String password;
  Carrier carrier;  // 运营商
  // IP 获取方式，目前只能是 meow、manual
  std::pair<String, String> IP_Obtain_Method;
};

#endif  // STRUCTURES_H
