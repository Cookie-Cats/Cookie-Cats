#include <Arduino.h>
#include <WiFiClient.h>
#include <vector>
#include <ESP8266HTTPClient.h>

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

// 获得随机 UA
String randomUA();

// 检测网络通断
bool testNet(WiFiClient wifiClient);

// 获取当前 IP
// 方法1：meow
// 详见：https://github.com/Cookie-Cats/meow
String meow(String meow_url, WiFiClient wifiClient);

#endif // FUNCTIONS_H