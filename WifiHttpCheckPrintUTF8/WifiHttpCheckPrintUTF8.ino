#include <U8g2lib.h>
#include <Wire.h>
//
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
//
#include <NTPClient.h>
#include <WiFiUdp.h>
//
#include <ESP8266WebServer.h> // Include the library
//
#include <EEPROM.h>

#define SCL 12 /*D5=SCL=GPIO12*/
#define SDA 14 /*D6=SDA=GPIO14*/

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /*clock=*/SCL, /*data=*/SDA, /*reset=*/U8X8_PIN_NONE);

/* biz config */
const char* ssid = "Ziroom-new";
const char* password = "ziru16011601";
// const char* ssid = "HW";
// const char* password = "rootroot";

typedef struct {
    char name[50];
    char host[100];
    char result[100];
} HostInfo;

// 初始化结构体数组
HostInfo hosts[] = {
    {"Baidu",  "baidu.com",  ""},
    {"Bing",   "bing.com",   ""},
    {"Google", "google.com", ""}
};

/* render */
char title[128] = "";
bool isWifiConnected = false;
char latestTimeConnectOK[128] = "";


// NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600 * 8, 1000); // UTC+8 timezone, update every minute

// server
ESP8266WebServer server(80); // Create a web server object that listens for HTTP request on port 80


#define EEPROM_SIZE 512 // 定义 EEPROM 大小
void writeStringToEEPROM(int startAddress, String data) {
  for (int i = 0; i < data.length(); i++) {
    EEPROM.write(startAddress + i, data[i]);
  }
  EEPROM.write(startAddress + data.length(), '\0'); // 结束符
  EEPROM.commit();
}
String readStringFromEEPROM(int startAddress) {
  String data = "";
  char ch;
  int i = 0;

  while (true) {
    ch = EEPROM.read(startAddress + i);
    if (ch == '\0') {
      break;
    }
    data += ch;
    i++;
  }

  return data;
}

void setup(void) {
    // serial
    Serial.begin(115200);
    delay(10);
    Serial.print("setup.");

    // eeprom 
    EEPROM.begin(EEPROM_SIZE);

    // 写入字符串到 EEPROM
    writeStringToEEPROM(0, "Hello World");
    writeStringToEEPROM(128, "ESP8266");

    String str1 = readStringFromEEPROM(0);
    String str2 = readStringFromEEPROM(128);

    // Serial.println("String 1: " + str1);
    // Serial.println("String 2: " + str2);

    // render
    u8g2.begin();
    u8g2.enableUTF8Print();

    // led
    pinMode(LED_BUILTIN, OUTPUT); /* 设置内置LED为输出模式 */

    // wifi
    WiFi.begin(ssid, password);

    // NTP
    timeClient.begin();

    // Define route for restart
    server.on("/restart", HTTP_GET, []() {
        server.send(200, "text/plain", "Restarting...");
        delay(1000);
        ESP.restart();
    });

    server.begin(); // Start the server
}

void tryConnAndMeasure(HostInfo* hostInfo) {
    WiFiClient client;
    unsigned long startTime = millis();
    
    if (client.connect(hostInfo->host, 80)) {
        unsigned long elapsedTime = millis() - startTime;
        sprintf(hostInfo->result, "%-10s: %d", hostInfo->name, elapsedTime);
    } else {
        sprintf(hostInfo->result, "%-10s: %s", hostInfo->name, "fail");
    }
}

void ledBlink( int slp ) {
    digitalWrite(LED_BUILTIN, LOW); /* 亮灯 */
    delay(slp);
    digitalWrite(LED_BUILTIN, HIGH); /* 灭灯 */
}

void loop(void) {
    // log
    Serial.print("loop.");

    // Handle client requests
    server.handleClient();

    // led
    if (isWifiConnected) {
      ledBlink(1000);
    } else {
      ledBlink(100);
    }
    

    // biz
    if (WiFi.status() == WL_CONNECTED) {
        isWifiConnected = true;
        timeClient.update();

        IPAddress ip = WiFi.localIP();
        sprintf(latestTimeConnectOK, "%s", timeClient.getFormattedTime());
        sprintf(title, "%s - %d.%d", latestTimeConnectOK, ip[2], ip[3]); // WiFi.localIP().toString().c_str()
    } else {
        isWifiConnected = false;
        sprintf(title, "%s . NotConnectedSince", latestTimeConnectOK);
    }

    // render part01
    u8g2.setFont(u8g2_font_unifont_t_chinese2);
    u8g2.setFontDirection(0);
    u8g2.clearBuffer();
    u8g2.setCursor(0, 15);
    u8g2.print(title);
    // u8g2.sendBuffer();

    // u8g2.setCursor(0, 30);
    // u8g2.print(timeClient.getFormattedTime());
    if (isWifiConnected) {
        for (int i = 0; i < 3; i++) {
            tryConnAndMeasure(&hosts[i]);
            u8g2.setCursor(0, 30 + i * 15);
            u8g2.print(hosts[i].result); //u8g2.printf("你好,%8d", 88 );
        }
        u8g2.sendBuffer();
    } else {
        // strcpy(body01, "");
    }

    // delay(100);
}
