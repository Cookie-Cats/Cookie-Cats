#include "functions.h"

using namespace std;

// 获得随机 UA
String randomUA() {
  static const vector<String> UAs = {
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/118.0.0.0 Safari/537.36",
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/105.0.0.0 Safari/537.36 Edg/105.0.1343.27",
    "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/51.0.2704.106 Safari/537.36 OPR/38.0.2220.41"
  };
  return UAs[random(0, UAs.size() - 1)];
}

// 检测网络通断
bool testNet(WiFiClient wifiClient) {
  Serial.println("Start testing the Internet");
  static const vector<String> testServer = {
    "http://connect.rom.miui.com/generate_204",                    // 小米
    "http://connectivitycheck.platform.hicloud.com/generate_204",  // 华为
    "http://wifi.vivo.com.cn/generate_204"                         // vivo
  };

  bool connected = false;

  for (int i = 0; i < 2; i++) {  // 尝试2次
    // 实例化 http 客户端
    HTTPClient httpClient;

    String URL = testServer[random(0, testServer.size() - 1)];
    httpClient.begin(wifiClient, URL);
    Serial.print("Use ");
    Serial.print(URL);
    Serial.print(" to test.\n");

    String UA = randomUA();
    httpClient.setUserAgent(UA);
    Serial.print("User-Agent: ");
    Serial.print(UA);
    Serial.println();

    httpClient.setTimeout(5000);  // 超时 5 秒

    Serial.println("Sending get");
    int responseCode = httpClient.GET();  // 发送请求

    Serial.println("Stop httpclient");
    httpClient.end();  // 关闭连接

    if (responseCode == HTTP_CODE_NO_CONTENT) {  // 返回 204，连接成功
      connected = true;
      break;
    }
  }
  if (connected) {
    Serial.print("Test Internet connection successfully.\n");
  } else {
    Serial.print("No connection to the Internet.\n");
  }
  return connected;
}

// 获取当前 IP
// 方法1：meow
// 详见：https://github.com/Cookie-Cats/meow

String meow(String meow_url, WiFiClient wifiClient) {
  // 定义反馈结果
  String responsePayload = "0.0.0.0";
  for (int i = 0; i < 2; i++) {  // 尝试2次
    // 实例化 http 客户端
    HTTPClient httpClient;

    httpClient.begin(wifiClient, meow_url);
    Serial.print("Use ");
    Serial.print(meow_url);
    Serial.print(" to get IP.\n");

    Serial.println("Send get.");
    int responseCode = httpClient.GET();  // 发送请求

    if (responseCode == HTTP_CODE_OK) {  // 返回 200，连接成功
      responsePayload = httpClient.getString();
      responsePayload.trim();
      break;
    }

    Serial.println("Stop httpclient.\n");
    httpClient.end();  // 关闭连接
  }
  if (responsePayload == "0.0.0.0") {  // 如果返回 "0.0.0.0"，则认为获取失败。meow 也是这样定义的。
    Serial.println("Can't connect to meow server.");
  } else {
    Serial.print("Get IP: ");
    Serial.print(responsePayload);
    Serial.println();
  }
  return responsePayload;
}
