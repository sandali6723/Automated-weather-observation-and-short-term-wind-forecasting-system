#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
TwoWire I2COLED = TwoWire(0);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &I2COLED, -1);

// Rotary Encoder Pins
#define CLK 13
#define DT 14
#define SW 25

// Serial Pins
#define RXD2 16  // From Nano (Weather Data)
#define TXD2 17
#define RXD1 33  // To Nano (SD Logger)
#define TXD1 32

// W5500 Ethernet Module
#define W5500_CS    5
#define W5500_RST   4

// W5500 Registers
#define W5500_MR        0x0000
#define W5500_GAR       0x0001
#define W5500_SUBR      0x0005
#define W5500_SHAR      0x0009
#define W5500_SIPR      0x000F
#define W5500_VERSIONR  0x0039

#define S0_MR           0x0000
#define S0_CR           0x0001
#define S0_SR           0x0003
#define S0_PORT         0x0004
#define S0_DIPR         0x000C
#define S0_DPORT        0x0010
#define S0_TX_WR        0x0024
#define S0_RX_RSR       0x0026

#define COMMON_R        0x00
#define COMMON_W        0x04
#define S0_R            0x08
#define S0_W            0x0C
#define S0_TX_W         0x14
#define SOCK_OPEN       0x01
#define SOCK_CONNECT    0x04
#define SOCK_CLOSE      0x10
#define SOCK_SEND       0x20

// Network Configuration
byte mac[6] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
byte ip[4] = {192, 168, 4, 177};
byte gateway[4] = {192, 168, 4, 1};
byte subnet[4] = {255, 255, 255, 0};
byte serverIP[4] = {192, 168, 4, 5}; // Node.js Backend Server IP
uint16_t serverPort = 3000;

// Weather Data Variables
String utcDate = "";
String utcTime = "";
String localTime = "";
String latitude = "";
String longitude = "";
String temperature = "";
String humidity = "";
String pressure = "";
String dewPoint = "";
String windSpeed = "";
String windDirection = "";
String voltage = "";
String current = "";
String power = "";
String powerStatus = "";
String commMode = "";

// Control Variables
int page = 0;
int lastClkState;
bool buttonPressed = false;
unsigned long lastOLEDUpdate = 0;
unsigned long lastBackendSend = 0;
unsigned long lastSDSend = 0;
unsigned long lastNanoData = 0;

const unsigned long oledUpdateInterval = 1000;
const unsigned long backendSendInterval = 5000;  // Send to backend every 5 sec
const unsigned long sdSendInterval = 30000;      // Send to SD logger every 30 sec
const unsigned long nanoTimeout = 10000;

// System Status
bool ethernetConnected = false;
bool nanoConnected = false;
bool backendConnected = false;

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);  // From Weather Nano
  Serial1.begin(9600, SERIAL_8N1, RXD1, TXD1);  // To SD Logger Nano

  // Initialize I2C for OLED
  I2COLED.begin(4, 15);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED initialization failed!");
  }

  // Initialize Encoder
  pinMode(CLK, INPUT_PULLUP);
  pinMode(DT, INPUT_PULLUP);
  pinMode(SW, INPUT_PULLUP);
  lastClkState = digitalRead(CLK);

  displayStartupMessage();

  // Initialize W5500
  Serial.println("\n ESP32 W5500 Ethernet Initializing...");
  pinMode(W5500_RST, OUTPUT);
  digitalWrite(W5500_RST, LOW);
  delay(10);
  digitalWrite(W5500_RST, HIGH);
  delay(100);

  pinMode(W5500_CS, OUTPUT);
  digitalWrite(W5500_CS, HIGH);

  SPI.begin(18, 19, 23, 5);

  if (!initW5500()) {
    Serial.println(" W5500 Init Failed!");
    ethernetConnected = false;
  } else {
    ethernetConnected = true;
    printNetworkInfo();
    Serial.println("\n Sending data to Backend: http://" +
                   String(serverIP[0]) + "." + String(serverIP[1]) + "." +
                   String(serverIP[2]) + "." + String(serverIP[3]) + ":" +
                   String(serverPort));
  }

  Serial.println("\n System Ready");
  Serial.println("=================================");
}

void loop() {
  handleEncoder();
  handleSerialInput();
 
  // Check Nano connection status
  if (millis() - lastNanoData > nanoTimeout && lastNanoData > 0) {
    nanoConnected = false;
  }
 
  // Send to Backend Server
  if (ethernetConnected && temperature != "" &&
      millis() - lastBackendSend >= backendSendInterval) {
    sendToBackendServer();
    lastBackendSend = millis();
  }
 
  // Send to SD Logger Nano
  if (utcDate != "" && utcTime != "" &&
      millis() - lastSDSend >= sdSendInterval) {
    sendToSDLogger();
    lastSDSend = millis();
  }
 
  updateOLEDDisplay();
}

// ================== W5500 Functions ==================
bool initW5500() {
  writeCommonReg(W5500_MR, 0x80);
  delay(50);

  uint8_t version = readCommonReg(W5500_VERSIONR);
  if (version != 0x04) {
    Serial.print("Unexpected W5500 Version: 0x");
    Serial.println(version, HEX);
  }

  for (int i = 0; i < 6; i++) writeCommonReg(W5500_SHAR + i, mac[i]);
  for (int i = 0; i < 4; i++) writeCommonReg(W5500_SIPR + i, ip[i]);
  for (int i = 0; i < 4; i++) writeCommonReg(W5500_GAR + i, gateway[i]);
  for (int i = 0; i < 4; i++) writeCommonReg(W5500_SUBR + i, subnet[i]);

  return true;
}

void sendToBackendServer() {
  // Build JSON payload
  String json = "{";
  json += "\"utcDate\":\"" + utcDate + "\",";                 // Keep as text
  json += "\"utcTime\":\"" + utcTime + "\",";  
  json += "\"localTime\":\"" + localTime + "\",";               // Keep as text
  json += "\"temperature\":\"" + extractNumber(temperature) + "\",";  
  json += "\"humidity\":\"" + extractNumber(humidity) + "\",";  
  json += "\"pressure\":\"" + extractNumber(pressure) + "\",";  
  json += "\"windSpeed\":\"" + extractNumber(windSpeed) + "\",";  
  json += "\"windDirection\":\"" + extractNumber(windDirection) + "\",";  
  //json += "\"windDirectionDegrees\":" + String(windDirectionDegrees) + ",";    
  json += "\"dewPoint\":\"" + extractNumber(dewPoint) + "\",";  
  json += "\"voltage\":\"" + extractNumber(voltage) + "\",";  
  json += "\"current\":\"" + extractNumber(current) + "\",";  
  json += "\"power\":\"" + extractNumber(power) + "\",";  
  json += "\"powerStatus\":\"" + powerStatus + "\",";
  json += "\"commMode\":\"" + commMode + "\",";            // Keep as text (e.g. Battery/CEB)
  json += "\"latitude\":\"" + latitude + "\",";                // Keep as text (may include °)
  json += "\"longitude\":\"" + longitude + "\"";                // Keep as text (may include °)
  json += "}";

  // Build HTTP POST request
  String request = "POST /api/weather/update HTTP/1.1\r\n";
  request += "Host: " + String(serverIP[0]) + "." + String(serverIP[1]) + "." +
             String(serverIP[2]) + "." + String(serverIP[3]) + "\r\n";
  request += "Content-Type: application/json\r\n";
  request += "Content-Length: " + String(json.length()) + "\r\n";
  request += "Connection: close\r\n\r\n";
  request += json;

  // Send via W5500
  if (connectToBackend()) {
    sendSocketData((uint8_t*)request.c_str(), request.length());
    Serial.println("Data sent to Backend: " + temperature);
    backendConnected = true;
    delay(50);
    closeSocket();
  } else {
    Serial.println("Backend connection failed");
    backendConnected = false;
  }
}

bool connectToBackend() {
  writeSocketReg(0, S0_CR, SOCK_CLOSE);
  delay(10);
 
  writeSocketReg(0, S0_MR, 0x01); // TCP mode
  writeSocketReg(0, S0_PORT, 0x10);
  writeSocketReg(0, S0_PORT + 1, 0x00);
 
  // Set destination IP and port
  for (int i = 0; i < 4; i++) {
    writeSocketReg(0, S0_DIPR + i, serverIP[i]);
  }
  writeSocketReg(0, S0_DPORT, (serverPort >> 8) & 0xFF);
  writeSocketReg(0, S0_DPORT + 1, serverPort & 0xFF);
 
  writeSocketReg(0, S0_CR, SOCK_OPEN);
  delay(10);
 
  writeSocketReg(0, S0_CR, SOCK_CONNECT);
 
  // Wait for connection
  for (int i = 0; i < 50; i++) {
    delay(10);
    uint8_t status = readSocketReg(0, S0_SR);
    if (status == 0x17) return true; // ESTABLISHED
    if (status == 0x00) return false; // CLOSED
  }
 
  return false;
}

void closeSocket() {
  writeSocketReg(0, S0_CR, SOCK_CLOSE);
  delay(10);
}

// ================== SPI Low-Level Functions ==================
void writeCommonReg(uint16_t addr, uint8_t data) {
  SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
  digitalWrite(W5500_CS, LOW);
  SPI.transfer16(addr);
  SPI.transfer(COMMON_W);
  SPI.transfer(data);
  digitalWrite(W5500_CS, HIGH);
  SPI.endTransaction();
}

uint8_t readCommonReg(uint16_t addr) {
  SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
  digitalWrite(W5500_CS, LOW);
  SPI.transfer16(addr);
  SPI.transfer(COMMON_R);
  uint8_t d = SPI.transfer(0x00);
  digitalWrite(W5500_CS, HIGH);
  SPI.endTransaction();
  return d;
}

void writeSocketReg(uint8_t sock, uint16_t addr, uint8_t data) {
  uint8_t ctrl = (sock << 5) | S0_W;
  SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
  digitalWrite(W5500_CS, LOW);
  SPI.transfer16(addr);
  SPI.transfer(ctrl);
  SPI.transfer(data);
  digitalWrite(W5500_CS, HIGH);
  SPI.endTransaction();
}

uint8_t readSocketReg(uint8_t sock, uint16_t addr) {
  uint8_t ctrl = (sock << 5) | S0_R;
  SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
  digitalWrite(W5500_CS, LOW);
  SPI.transfer16(addr);
  SPI.transfer(ctrl);
  uint8_t d = SPI.transfer(0x00);
  digitalWrite(W5500_CS, HIGH);
  SPI.endTransaction();
  return d;
}

void sendSocketData(uint8_t* data, uint16_t len) {
  uint16_t ptr = (readSocketReg(0, S0_TX_WR) << 8) | readSocketReg(0, S0_TX_WR + 1);
  uint8_t ctrl = S0_TX_W;
 
  SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
  digitalWrite(W5500_CS, LOW);
  SPI.transfer16(ptr);
  SPI.transfer(ctrl);
  for (uint16_t i = 0; i < len; i++) SPI.transfer(data[i]);
  digitalWrite(W5500_CS, HIGH);
  SPI.endTransaction();
 
  ptr += len;
  writeSocketReg(0, S0_TX_WR, (ptr >> 8) & 0xFF);
  writeSocketReg(0, S0_TX_WR + 1, ptr & 0xFF);
  writeSocketReg(0, S0_CR, SOCK_SEND);
  while (readSocketReg(0, S0_CR)) delay(1);
}

// ================== Data Processing ==================
void handleSerialInput() {
  while (Serial2.available()) {
    String line = Serial2.readStringUntil('\n');
    line.trim();
   
    if (line.length() > 0) {
      lastNanoData = millis();
      nanoConnected = true;
      Serial.println("[RX] " + line);
      parseSerialData(line);
    }
  }
}

void parseSerialData(String data) {
  data.trim();
 
  if (data.startsWith("UD:")) {
    utcDate = data.substring(4);
    utcDate.trim();
  }
  else if (data.startsWith("UT:")) {
    utcTime = data.substring(4);
    utcTime.trim();
  }
  else if (data.startsWith("LT:")) {
    localTime = data.substring(4);
    latitude.trim();
  }
  else if (data.startsWith("Lat:")) {
    latitude = data.substring(5);
    latitude.trim();
  }
  else if (data.startsWith("Long:")) {
    longitude = data.substring(6);
    longitude.trim();
  }
  else if (data.startsWith("Temp:")) {
    temperature = data.substring(6);
    temperature.trim();
  }
  else if (data.startsWith("Hum:")) {
    humidity = data.substring(5);
    humidity.trim();
  }
  else if (data.startsWith("Dew:")) {
    dewPoint = data.substring(5);
    dewPoint.trim();
  }
  else if (data.startsWith("Press:")) {
    pressure = data.substring(7);
    pressure.trim();
  }
  else if (data.startsWith("WS:")) {
    windSpeed = data.substring(4);
    windSpeed.trim();
  }
  else if (data.startsWith("WD:")) {
    windDirection = data.substring(4);
    windDirection.trim();
  }
  else if (data.startsWith("V:")) {
    voltage = data.substring(3);
    voltage.trim();
  }
  else if (data.startsWith("C:")) {
    current = data.substring(3);
    current.trim();
  }
  else if (data.startsWith("P:")) {
    power = data.substring(3);
    power.trim();
  }
  else if (data.startsWith("PS:")) {
    powerStatus = data.substring(4);
    powerStatus.trim();
  }
  else if (data.startsWith("Mode:")) {
    commMode = data.substring(6);
    commMode.trim();
  }
}

void sendToSDLogger() {
  // Send CSV format to SD Logger Nano
  String csv = checkField(utcDate) + "," +
               checkField(utcTime) + "," +
               checkField(latitude) + "," +
               checkField(longitude) + "," +
               checkField(extractNumber(temperature)) + "," +
               checkField(extractNumber(humidity)) + "," +
               checkField(extractNumber(pressure)) + "," +
               checkField(extractNumber(dewPoint)) + "," +
               checkField(extractNumber(windDirection)) + "," +
               checkField(extractNumber(windSpeed)) + "," +
               checkField(extractNumber(voltage)) + "," +
               checkField(extractNumber(current)) + "," +
               checkField(extractNumber(power)) + "," +
               checkField(powerStatus) + "," +
               checkField(commMode);
 
  Serial1.println(csv);
  Serial.println("CSV sent to SD Logger: " + csv.substring(0, 30) + "...");
}

String extractNumber(String input) {
  String result = "";
  bool decimalFound = false;
 
  for (int i = 0; i < input.length(); i++) {
    char c = input.charAt(i);
    if (isDigit(c) || (c == '.' && !decimalFound) || (c == '-' && i == 0)) {
      result += c;
      if (c == '.') decimalFound = true;
    } else if (result.length() > 0) {
      break;
    }
  }
 
  return result;
}

String checkField(String f) {
  return (f.length() > 0) ? f : "N/A";
}

// ================== Display Functions ==================
void displayStartupMessage() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("AWOS Starting...");
  display.println("Backend Mode");
  display.println("IP: 192.168.4.177");
  display.println("Server: .100:3000");
  display.println("Waiting for data");
  display.display();
  delay(3000);
}

void handleEncoder() {
  int currentClk = digitalRead(CLK);
  if (currentClk != lastClkState) {
    if (digitalRead(DT) != currentClk) {
      page++;
    } else {
      page--;
    }
   
    if (page < 0) page = 6;
    if (page > 6) page = 0;
   
    lastClkState = currentClk;
  }
 
  if (digitalRead(SW) == LOW && !buttonPressed) {
    buttonPressed = true;
    page = 0;
  } else if (digitalRead(SW) == HIGH) {
    buttonPressed = false;
  }
}

void updateOLEDDisplay() {
  if (millis() - lastOLEDUpdate < oledUpdateInterval) return;
  lastOLEDUpdate = millis();

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  switch (page) {
    case 0:
      display.println("=== SYSTEM INFO ===");
      display.println("IP: 192.168.4.5");
      display.println("Date: " + checkField(utcDate));
      display.println("UTC_Time: " + checkField(utcTime));
      display.println("Local_Time: " + checkField(localTime));
      display.println("...............");
      display.print("E:");
      display.print(ethernetConnected ? "OK" : "X");
      display.print(" N:");
      display.print(nanoConnected ? "OK" : "X");
      display.print(" B:");
      display.println(backendConnected ? "OK" : "X");
      display.println("Page 1/7");
      break;
     
    case 1:
      display.println("=== POWER STATUS ===");
      display.println("Voltage: " + checkField(voltage));
      display.println("Current: " + checkField(current));
      display.println("Power: " + checkField(power));
      display.println("Status: " + checkField(powerStatus));
      display.println("Comm: " + checkField(commMode));
      display.println("Page 2/7");
      break;
     
    case 2:
      display.println("=== TEMPERATURE ===");
      display.println("Temperature:");
      display.println("  " + checkField(temperature));
      display.println("Dew Point:");
      display.println("  " + checkField(dewPoint));
      display.println("");
      display.println("Page 3/7");
      break;
     
    case 3:
      display.println("=== HUMIDITY ===");
      display.println("Humidity:");
      display.println("  " + checkField(humidity));
      display.println("");
      display.println("");
      display.println("");
      display.println("Page 4/7");
      break;
     
    case 4:
      display.println("=== PRESSURE ===");
      display.println("Pressure:");
      display.println("  " + checkField(pressure));
      display.println("");
      display.println("");
      display.println("");
      display.println("Page 5/7");
      break;
     
    case 5:
      display.println("=== WIND DATA ===");
      display.println("Wind Speed:");
      display.println("  " + checkField(windSpeed));
      display.println("Wind Direction:");
      display.println("  " + checkField(windDirection));
      display.println("");
      display.println("Page 6/7");
      break;
     
    case 6:
      display.println("=== LOCATION ===");
      display.println("Latitude:");
      display.println("  " + checkField(latitude));
      display.println("Longitude:");
      display.println("  " + checkField(longitude));
      display.println("");
      display.println("Page 7/7");
      break;
  }

  display.display();
}

void printNetworkInfo() {
  Serial.println("======================================");
  Serial.println("ESP32 IP: " + formatIP(ip));
  Serial.println("MAC: " + formatMAC());
  Serial.println("Gateway: " + formatIP(gateway));
  Serial.println("Subnet: " + formatIP(subnet));
  Serial.println("Backend: " + formatIP(serverIP) + ":" + String(serverPort));
  Serial.println("======================================");
}

String formatIP(byte* addr) {
  return String(addr[0]) + "." + String(addr[1]) + "." + String(addr[2]) + "." + String(addr[3]);
}

String formatMAC() {
  String result = "";
  for (int i = 0; i < 6; i++) {
    if (i) result += ":";
    if (mac[i] < 16) result += "0";
    result += String(mac[i], HEX);
  }
  result.toUpperCase();
  return result;
}
