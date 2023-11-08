#include <Arduino.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <vector>
#include <FS.h>
#include "structures.h"

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

// 获得随机 UA
String randomUA();

// 检测网络通断
bool testNet(WiFiClient &wifiClient);

// 获取当前 IP
// 方法1：meow
// 详见：https://github.com/Cookie-Cats/meow
String ICACHE_FLASH_ATTR meow(String meow_url, WiFiClient &wifiClient);

// 用于 http 服务器，获取文件类型
String httpGetContentType(String filename);

// 读取配置文件
void ICACHE_FLASH_ATTR readConfigurationFromFile(File &file, Configuration &configuration);

// OTA 更新
void otaUpdate(WiFiClient &wifiClient, String updateURL, String currentVersion);

#endif  // FUNCTIONS_H