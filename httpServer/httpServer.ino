#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "Ziroom-new";
const char* password = "ziru16011601";

ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  WiFi.begin(ssid, password);

  // 等待连接到WiFi网络
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // 打印并获取IP地址
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  EEPROM.begin(512); // 初始化EEPROM，大小为512字节

  // 设置HTTP服务器的处理程序
  server.on("/", handleRoot);
  server.on("/ip", handleGetIP);
  server.on("/write", handleWriteEEPROM);
  server.on("/read", handleReadEEPROM);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  blinkLED(); // 执行其他任务
}

void handleRoot() {
  server.send(200, "text/html", "<h1>Hello from ESP8266!</h1>");
}

void handleGetIP() {
  String ipAddress = WiFi.localIP().toString();
  server.send(200, "text/plain", "IP Address: " + ipAddress);
}

void handleWriteEEPROM() {
  if (server.hasArg("data")) {
    String data = server.arg("data");
    writeStringToEEPROM(0, data);
    server.send(200, "text/plain", "Data written to EEPROM: " + data);
  } else {
    server.send(400, "text/plain", "Bad Request: data parameter missing");
  }
}

void handleReadEEPROM() {
  String data = readStringFromEEPROM(0);
  server.send(200, "text/plain", "Data read from EEPROM: " + data);
}

void writeStringToEEPROM(int address, const String &data) {
  for (int i = 0; i < data.length(); i++) {
    EEPROM.write(address + i, data[i]);
  }
  EEPROM.write(address + data.length(), '\0'); // 写入字符串终止符
  EEPROM.commit(); // 确保数据被写入
}

String readStringFromEEPROM(int address) {
  char data[100]; // 假设字符串的最大长度为100
  int len = 0;
  unsigned char k;
  k = EEPROM.read(address);
  while (k != '\0' && len < 100) { // 读取直到字符串终止符或达到最大长度
    data[len] = k;
    len++;
    k = EEPROM.read(address + len);
  }
  data[len] = '\0'; // 确保字符串以终止符结束
  return String(data);
}

void blinkLED() {
  static unsigned long lastTime = 0;
  unsigned long currentTime = millis();

  if (currentTime - lastTime >= 1000) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); // 切换LED状态
    lastTime = currentTime;
  }
}
