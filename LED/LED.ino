 
void setup() {
  pinMode(LED_BUILTIN, OUTPUT); // 设置内置LED为输出模式
}

int i = 0;
void loop() {
  i ++;
  if (i>30) i=0;

  digitalWrite(LED_BUILTIN, HIGH);   // 关闭 LED
  delay(i*100);
  digitalWrite(LED_BUILTIN, LOW);    // 打开 LED
  delay(i*100);
}
