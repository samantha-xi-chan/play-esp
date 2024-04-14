
/*
   测试Case： 短接 开发板上 的 G 与 D6 [GPIO #12]
*/

struct Button {
  const uint8_t PIN;
  uint32_t numberKeyPresses;
  bool pressed;
};

Button button1 = {D6, 0, false};

void ICACHE_RAM_ATTR isr() {
  button1.numberKeyPresses++;
  button1.pressed = true;
}

void setup() {
  Serial.begin(115200);
  pinMode(button1.PIN, INPUT_PULLUP);
  attachInterrupt(button1.PIN, isr, FALLING);
}

void loop() {
  if (button1.pressed) {
      Serial.printf("Button has been pressed %u times\n", button1.numberKeyPresses);
      button1.pressed = false;
  }
}