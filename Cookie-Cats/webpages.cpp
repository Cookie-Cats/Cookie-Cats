#include "webpages.h"

using namespace std;

// 读取网页
// 如果可以被找到，则返回网页内容
// 否则返回值为空
String ICACHE_FLASH_ATTR webPages(String &page) {
  if (page == "/index.html") {
    String index = F("<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>Cookie-Cats</title></head><body><h1>这是一个测试网页</h1><p>再期待一会！</p></body></html>");
    return index;
  } else {
    String noFound = "";
    return noFound;
  }
}