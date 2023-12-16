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
  // 目前支持的选项：
  // * ChinaPharmaceuticalUniversityDormitory
  // * ChinaPharmaceuticalUniversityPublic
  String school;
  // IP 获取方式
  // IP_Obtain_Method: IP_Obtain_Method_Content
  // 目前支持的选项：
  // meow: url
  // manual: ip
  // unnecessary: 0.0.0.0
  std::pair<String, String> IP_Obtain_Method;
  // 允许 OTA 更新
  // 只有固件和用户都设置允许才会更新
  // 默认：true
  bool allowOTA;
};

class HttpResponse {
public:
  // 状态码
  int status_code;
  // 响应内容
  String content;
};

#endif  // STRUCTURES_H
