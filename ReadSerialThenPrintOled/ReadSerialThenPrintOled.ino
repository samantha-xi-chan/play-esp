#include <U8g2lib.h>
#include <Wire.h>
 
#define SCL 12 /*D5=SCL=GPIO12*/
#define SDA 14/*D6=SDA=GPIO14*/

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /*clock=*/SCL, /*data=*/SDA, /*reset=*/U8X8_PIN_NONE);

void setup(void) {
  u8g2.begin();
  u8g2.enableUTF8Print();

  Serial.begin(115200);
}

void loop() {
  int incomingByte = 0;

  if (Serial.available() > 0) {
    incomingByte = Serial.read();
    Serial.print("read: ");
    Serial.println(incomingByte, DEC);
  }

  u8g2.setFont(u8g2_font_unifont_t_chinese2);
  u8g2.setFontDirection(0);
  u8g2.clearBuffer();
  u8g2.setCursor(0, 20);
  u8g2.print(incomingByte, DEC);
  u8g2.sendBuffer();
  
  delay(100);
}