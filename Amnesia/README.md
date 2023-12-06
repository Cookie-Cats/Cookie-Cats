# Amnesia

Amnesia 用于擦除 ESP8266 的文件系统和 WiFi 凭据。

## 使用方法

### 方法一：直接上传固件

1. 从此 README 同目录下获取符合 ESP8266 内存大小的 Amnesia_*MB.bin 文件。

2. 检查文件完整性（非必需）

   * 在线工具
     * 打开 [http://www.metools.info/code/c92.html](http://www.metools.info/code/c92.html)；
     * 点击`选择文件`，上传固件；
     * 更改`哈希算法`为 SHA256；
     * 点击`SHA计算`，将`文件SHA值`与 [sha256sum](https://github.com/Cookie-Cats/Cookie-Cats/blob/main/Amnesia/sha256sum) 文件内容对比。

   * Linux

     ```bash
     # 在 Amnesia 文件夹下
     sha256sum -c sha256sum
     ```

   * MacOS

     ```bash
     # 在 Amnesia 文件夹下
     sha256sum -c sha256sum
     ```

   * Windows

     ```cmd
     # 在 Amnesia 文件夹下
     certutil -hashfile $FILE_NAME SHA256
     # 对比是否与 sha256sum 文件中 SHA256 校验和相同
     ```

3. 上传固件

   1. 使用 Chrome 或 Edge 打开 [esp.huhn.me](https://esp.huhn.me/)；

   2. 使用 USB 连接 CookieCats 到电脑；

   3. 点击 `CONNECT`，选择 CookieCats 连接的串口，点击`连接`。

      * 要找出与您的开发板相对应的串行端口，请查看列表（未插入您的开发板），然后插入您的开发板并观察列表中出现的新串行端口。

      * 如果你没有看到 CookieCats 连接的串口：

        1. 如果你使用 Windows 为 D1 mini 上传（蓝色开发板），其使用 CH340 作为USB串口转换器，请安装[驱动](http://www.wch-ic.com/downloads/CH341SER_ZIP.html)。MacOS 无需安装驱动。

        2. 如果你使用其他USB串口转换器，请按要求安装固件：

           * CH340/CH341： [http://www.wch-ic.com/downloads/CH341SER_ZIP.html](http://www.wch-ic.com/downloads/CH341SER_ZIP.html)

           * CP210x：[https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)

           * FTDI：[https://ftdichip.com/drivers](https://ftdichip.com/drivers)

   4. 上传 Amnesia_*MB.bin 固件；

   5. 点击 `PROGRAM`，直到 Output 中出现：

      ```
      Done!
      To run the new firmware please reset your device.
      ```

      则上传成功；

   6. 按重置按钮，或断电后重新连接 CookieCats。**如果 CookieCats 的 LED 每隔 500ms 闪亮一次，则为重置成功**。

### 方法二：使用 Arduino 上传

1. 使用 Arduino IDE 打开 Amnesia.ino；
2. 上传固件。

---

之后你可以使用相同的方法上传 CookieCats 的固件。你可以在[这里](https://update.cookiecats.diazepam.cc/)下载最新 CookieCats 的固件。

## 鸣谢

* [SpacehuhnTech/esp8266_deauther](https://github.com/SpacehuhnTech/esp8266_deauther)

* [SpacehuhnTech/espwebtool](https://github.com/SpacehuhnTech/espwebtool)