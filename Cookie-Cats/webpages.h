#include <Arduino.h>

#ifndef FUNCTIONS_WEBPAGES
#define FUNCTIONS_WEBPAGES

// 读取网页
// 如果可以被找到，则返回网页内容
// 否则返回值为空
String ICACHE_FLASH_ATTR webPages(String &page);

#endif  // FUNCTIONS_WEBPAGES