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
// const char* targetSsid = "Ziroom-new";
// const char* targetPass = "ziru16011601";
const char* ap_ssid = "00IOT";
const char* ap_password = "rootroot";

typedef struct {
    char name[50];
    char host[100];
    char result[100];
} HostInfo;

// 初始化结构体数组
HostInfo hosts[] = {
    {"Baidu", "baidu.com", ""},
    {"Bing", "bing.com", ""},
    {"Google", "google.com", ""}
};

/* render */
char title[128] = "";

bool wifiModeClient = true; /* false: in mode AP */
bool isWifiConnected = false;
char latestTimeConnectOK[128] = "";

// NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600 * 8, 1000); // UTC+8 timezone, update every minute

// server
ESP8266WebServer server(80); // Create a web server object that listens for HTTP request on port 80

#define EEPROM_SIZE 512 // 定义 EEPROM 大小
#define SSID_IDX 0
#define PASS_IDX 64

void writeE2prom(int address, int value) {
    EEPROM.write(address, value);
    EEPROM.commit();
}

int readE2prom(int address) {
    return EEPROM.read(address);
}

void writeStringToE2prom(int address, const String &data) {
    for (int i = 0; i < data.length(); i++) {
        EEPROM.write(address + i, data[i]);
    }
    EEPROM.write(address + data.length(), '\0'); // 写入字符串终止符
    EEPROM.commit(); // 确保数据被写入
}

String readStringFromE2prom(int address) {
    char data[100]; // 假设字符串的最大长度为100
    int len = 0;
    unsigned char k;
    k = EEPROM.read(address);
    while (k != '\0' && len < 500) { // 读取直到字符串终止符或达到最大长度
        k = EEPROM.read(address + len);
        data[len] = k;
        len++;
    }
    data[len] = '\0'; // 确保字符串以终止符结束
    return String(data);
}

void handleRestart() {
    server.send(200, "text/plain", "Restarting...");
    delay(200);
    ESP.restart();
}

void handleRoot() {
    server.send(200, "text/html", "<h1>Hello, send a GET request to /write?data=abcd to write to EEPROM or /read to read EEPROM</h1>");
}

void handleWriteE2promSsid() {
    if (server.hasArg("data")) {
        String data = server.arg("data");
        writeStringToE2prom(SSID_IDX, data);
        server.send(200, "text/plain", "ssid Data written to EEPROM: " + data);
    } else {
        server.send(400, "text/plain", "ssid Bad Request: data parameter missing");
    }
}

void handleWriteE2promPass() {
    if (server.hasArg("data")) {
        String data = server.arg("data");
        writeStringToE2prom(PASS_IDX, data);
        server.send(200, "text/plain", "pass Data written to EEPROM: " + data);
    } else {
        server.send(400, "text/plain", "pass Bad Request: data parameter missing");
    }
}

void handleReadE2promSsid() {
    String data = readStringFromE2prom(SSID_IDX);
    server.send(200, "text/plain", "Ssid Data read from E2prom: " + data);
}

void handleReadE2promPass() {
    String data = readStringFromE2prom(PASS_IDX);
    server.send(200, "text/plain", "pass Data read from E2prom: " + data);
}

void setup(void) {
    // serial
    Serial.begin(115200);
    delay(3000);
    Serial.print("setup.");

    // eeprom 
    EEPROM.begin(EEPROM_SIZE);

    // render
    u8g2.begin();
    u8g2.enableUTF8Print();

    // led
    pinMode(LED_BUILTIN, OUTPUT); /* 设置内置LED为输出模式 */

    // wifi
    String ssidFromE2ROM = readStringFromE2prom(SSID_IDX);
    Serial.printf("e2prom ssid: %s \n", ssidFromE2ROM.c_str());
    String passFromE2ROM = readStringFromE2prom(PASS_IDX);
    Serial.printf("e2prom pass: %s \n", passFromE2ROM.c_str());

    // 尝试连接WiFi
    WiFi.begin(ssidFromE2ROM, passFromE2ROM);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("");
        Serial.print("WiFi connected, ");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        // NTP
        timeClient.begin();
    } else {
        Serial.println("");
        Serial.printf("Failed to connect to WiFi[%s], Starting AP mode. \n", ssidFromE2ROM.c_str() );
        wifiModeClient = false;
        startAP();
    }

    // Define route for restart
    server.on("/", handleRoot);
    server.on("/restart", handleRestart);
    server.on("/write/ssid", handleWriteE2promSsid);
    server.on("/write/pass", handleWriteE2promPass);
    server.on("/read/ssid", handleReadE2promSsid);
    server.on("/read/pass", handleReadE2promPass);
    // server.on("/config/wifi/ap/0/ssid/write", );
    // server.on("/config/wifi/ap/0/pass/write", );
    // server.on("/config/wifi/ap/1/ssid/write", );
    // server.on("/config/wifi/ap/1/pass/write", );
    server.begin();
    Serial.println("HTTP server started");
}

void tryConnAndMeasure(HostInfo* hostInfo) {
    WiFiClient client;
    unsigned long startTime = millis();

    if (client.connect(hostInfo->host, 80)) {
        unsigned long elapsedTime = millis() - startTime;
        sprintf(hostInfo->result, "%-7s: %d", hostInfo->name, elapsedTime);
    } else {
        sprintf(hostInfo->result, "%-7s: %s", hostInfo->name, "fail");
    }
}

void ledBlink(int slp) {
    digitalWrite(LED_BUILTIN, LOW); /* 亮灯 */
    delay(slp);
    digitalWrite(LED_BUILTIN, HIGH); /* 灭灯 */
}

void loop(void) {
    // Serial.print("loop.");

    // Handle client requests
    server.handleClient();

    if ( wifiModeClient && isWifiConnected) { /* 作为 wifi client 已连 */
      ledBlink(100);
    } else if ( wifiModeClient && !isWifiConnected) { /* 作为 wifi client  未连 */
      ledBlink(2000);
    } else if ( !wifiModeClient ){ /* 作为AP */
      ledBlink(1000);
    } 

    // biz title
    if (!wifiModeClient) {
        sprintf(title, "Mode AP");
    } else if (WiFi.status() == WL_CONNECTED) {
        isWifiConnected = true;
        timeClient.update();

        IPAddress ip = WiFi.localIP();
        sprintf(latestTimeConnectOK, "%s", timeClient.getFormattedTime());
        sprintf(title, "%s %d.%d", latestTimeConnectOK, ip[2], ip[3]); // WiFi.localIP().toString().c_str()
    } else {
        isWifiConnected = false;
        sprintf(title, "%s NotConnectedSince", latestTimeConnectOK);
    }

    // render title
    u8g2.setFont(u8g2_font_unifont_t_chinese2);
    u8g2.setFontDirection(0);
    u8g2.clearBuffer();
    u8g2.setCursor(0, 15);
    u8g2.print(title);

    // biz render host status
    if ( wifiModeClient && isWifiConnected) {
        for (int i = 0; i < 3; i++) {
            tryConnAndMeasure(&hosts[i]);
            u8g2.setCursor(0, 30 + i * 15);
            u8g2.print(hosts[i].result); // u8g2.printf("你好,%8d", 88 );
        }
    }

    u8g2.sendBuffer();
}

void startAP() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ap_ssid, ap_password);

    Serial.printf("AP mode started, ap_ssid = %s , ap_password = %s\n", ap_ssid, ap_password);
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
}
