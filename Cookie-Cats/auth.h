#include <Arduino.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include "structures.h"
#include "functions.h"

#ifndef FUNCTIONS_AUTH
#define FUNCTIONS_AUTH

// 先检测是否联网，如果网络断开则认证
void checkNetAndAuth(Configuration &config, WiFiClient &wifiClient);

// 认证
bool auth(Configuration &config, WiFiClient &wifiClient);

#endif  // FUNCTIONS_AUTH