#include <U8g2lib.h>
#include <Wire.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#define SCL 12 /*D5=SCL=GPIO12*/
#define SDA 14 /*D6=SDA=GPIO14*/

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /*clock=*/SCL, /*data=*/SDA, /*reset=*/U8X8_PIN_NONE);


/* biz config */
const char* ssid = "HW";
const char* password = "rootroot";
const char* host01 = "baidu.com";
const char* host02 = "bing.com";
const char* host03 = "google.com";

/* render */
char title[128] = "";
char body01[128] =  "" ;
char body02[128] =  "" ;
char body03[128] =  "" ;
bool isWifiConnected = false;

void setup(void) {
  // serial
  Serial.begin(115200);
  delay(10);
  Serial.print("setup.");

  // render
  u8g2.begin();
  u8g2.enableUTF8Print();

  // led
  pinMode(LED_BUILTIN, OUTPUT); /* 设置内置LED为输出模式 */

  // wifi
  WiFi.begin(ssid, password);
}


void connectAndMeasure(const char* host, char* body) {
  WiFiClient client;
  unsigned long startTime = millis();
  
  if (client.connect(host, 80)) {
    unsigned long elapsedTime = millis() - startTime;
    sprintf(body, "%-10s: %d", host, elapsedTime);
  } else {
    sprintf(body, "%-10s: %s", host, "fail");
  }
}

void loop(void) {
  // log
  Serial.print("loop.");

  // led
  digitalWrite(LED_BUILTIN, LOW ); /* 亮灯 */
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH );/* 灭灯 */

  // biz
  if ( WiFi.status() == WL_CONNECTED ) {
    isWifiConnected = true;
    sprintf(title, "%s", WiFi.localIP().toString().c_str());
  } else {
    isWifiConnected = false;
    strcpy(title, "Not Connected");
  }

  if (isWifiConnected) {
    connectAndMeasure(host01, body01);
    connectAndMeasure(host02, body02);
    connectAndMeasure(host03, body03);
  }

  // render
  u8g2.setFont(u8g2_font_unifont_t_chinese2);
  u8g2.setFontDirection(0);
  u8g2.clearBuffer();
  u8g2.setCursor(0, 15);
  u8g2.print(title);
  u8g2.setCursor(0, 30);
  u8g2.print(body01);          //u8g2.printf("你好,%8d", 88 );
  u8g2.setCursor(0, 45);
  u8g2.print(body02);
  u8g2.setCursor(0, 60);
  u8g2.print(body03);
  u8g2.sendBuffer();

  // 
  delay(100);
}