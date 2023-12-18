#include <Arduino.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <LittleFS.h>
#include <PracticalCrypto.h>
#include "structures.h"

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

// 获得随机 UA
String randomUA();

// 发送 http 请求
// send_type: GET/POST
HttpResponse sendHttpRequest(String url, String send_type = "GET", String post_payload = "", String user_agent = randomUA(), int timeout = 5000);

// 检测网络通断
bool ICACHE_FLASH_ATTR testNet();

// 获取当前 IP
// 方法1：meow
// 详见：https://github.com/Cookie-Cats/meow
String ICACHE_FLASH_ATTR meow(String meow_url);

// 用于 http 服务器，获取文件类型
String ICACHE_FLASH_ATTR httpGetContentType(String filename);

// 读取配置文件
void ICACHE_FLASH_ATTR readConfigurationFromFile(File &file, Configuration &configuration, PracticalCrypto &secret);

// OTA 更新
void ICACHE_FLASH_ATTR otaUpdate(String updateURL, String currentVersion);

// 读取是否存在 .secret 密钥文件
// 如果不存在，就生成一个
void ICACHE_FLASH_ATTR readSecret(PracticalCrypto &secret);

// 闪灯函数
// 参数为times，表示 led 闪烁的次数，闪灯间隔 200 毫秒
void blink(int times);

// 从 IP_Obtain_Method 获取 IP

// 当未设置 IP_Obtain_Method 或 IP_Obtain_Method 设置为 unnucessary 时会返回 "0.0.0.0"
String ICACHE_FLASH_ATTR getIPFromIPObtainMethod(Configuration &config);

#endif  // FUNCTIONS_H