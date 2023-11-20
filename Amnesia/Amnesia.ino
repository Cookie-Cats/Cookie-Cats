#include <LittleFS.h>

/*
  Amnesia 用于擦除 ESP8266 的文件系统和 WiFi 凭据
  灵感来源于：https://github.com/SpacehuhnTech/esp8266_deauther/blob/v2/Reset_Sketch
*/

void setup() {
  Serial.begin(115200);  // 开启串口

  Serial.println();
  Serial.println("Starting Amnesia...");
  Serial.println("WARNING: Cookie-Cats will forget anything it remembered.");

  // 格式化 LittleFS
  LittleFS.begin();
  Serial.println("LittlsFS started.");
  LittleFS.format();
  Serial.println("LittlsFS cleaned.");

  // 清除 WiFi 配置
  ESP.eraseConfig();
  Serial.println("WiFi credentials erased.");

  Serial.println("DONE!");

  // 启动 LED
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // 每隔 500ms 闪灯
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
}