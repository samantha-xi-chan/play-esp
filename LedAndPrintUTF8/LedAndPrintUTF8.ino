#include <U8g2lib.h>
#include <Wire.h>
 
#define SCL 12 /*D5=SCL=GPIO12*/
#define SDA 14/*D6=SDA=GPIO14*/

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /*clock=*/SCL, /*data=*/SDA, /*reset=*/U8X8_PIN_NONE);

void setup(void) {
  u8g2.begin();
  u8g2.enableUTF8Print();

  pinMode(LED_BUILTIN, OUTPUT); // 设置内置LED为输出模式
}

int i = 0;
int x = 0;
void loop(void) {
  i ++;
  x += 5;
  if ( x >= 100 ) {
    x = 0;
  }

  if ( i%2 == 0 ) {
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }

  u8g2.setFont(u8g2_font_unifont_t_chinese2);
  u8g2.setFontDirection(0);
  u8g2.clearBuffer();
  u8g2.setCursor(x, 15);
  u8g2.print("Hello World!");
  u8g2.setCursor(x, 40);
  u8g2.printf("你好,%8d", i);
  u8g2.sendBuffer();
  delay(100);
}