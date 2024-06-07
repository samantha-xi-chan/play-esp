#include <ESP8266WiFi.h>
#include <WiFiClient.h>

const char* ssid = "HW";
const char* password = "rootroot";
const char* host = "www.baidu.com";  // 目标网站

void setup() {
  Serial.begin(115200);
  delay(10);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("✖️");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  WiFiClient client;
  unsigned long startTime = millis();
  
  if (client.connect(host, 80)) {
    unsigned long elapsedTime = millis() - startTime;
    Serial.print("Connected to ");
    Serial.print(host);
    Serial.print(" in ");
    Serial.print(elapsedTime);
    Serial.println(" ms");

    // 发送HTTP请求
    client.print(String("GET / HTTP/1.1\r\n") +
                 "Host: " + host + "\r\n" + 
                 "Connection: close\r\n\r\n");
    
    // 读取响应
    while (client.connected() && !client.available()) {
      delay(1);
    }
    while (client.available()) {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
    
    client.stop();
  } else {
    Serial.println("Connection failed");
  }
  
  // 每隔10秒钟测试一次
  delay(10000);
}
