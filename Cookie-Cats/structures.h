#ifndef STRUCTURES_H
#define STRUCTURES_H

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
  // 运营商
  // 只能为 ChinaTelecom, ChinaUnicom, ChinaMobile 其一
  String carrier;  // 运营商
  // 学校
  // 目前支持的选项：ChinaPharmaceuticalUniversity
  String school;
  // IP 获取方式
  // 目前支持的选项：
  // meow: url
  // manual: ip
  // unnecessary 无value，指定为 0.0.0.0
  std::pair<String, String> IP_Obtain_Method;
};

#endif  // STRUCTURES_H
