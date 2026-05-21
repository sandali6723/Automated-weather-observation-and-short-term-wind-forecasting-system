#include <SoftwareSerial.h>
#include <Wire.h>
#include <SSD1306Ascii.h>
#include <SSD1306AsciiWire.h>

// ---------- Display ----------
SSD1306AsciiWire oled;

// ---------- Pins ----------
#define CLK_PIN 3
#define DT_PIN 5
#define SW_PIN 7

// ---------- SoftwareSerial ----------
// ESP32 → Nano (RX pin 4)
SoftwareSerial espNano(4, -1);
// Nano → Nano2 (TX pin 2)
SoftwareSerial nanoOut(-1, 2);  

// ---------- Variables ----------
char receivedBuffer[128];  // enlarged buffer for full CSV line
byte bufferIndex = 0;
byte currentPage = 0;
byte lastCLK = HIGH;
bool newData = false;  // <--- NEW flag

// Data variables
float temp, hum, dew, press, volt, pwr, curr, wspeed;
int wdir;
char utctimeStr[9];   // HH:MM:SS
char localtimeStr[9]; // HH:MM:SS
char latStr[12], lonStr[12];
char dateStr[11];     // DD/MM/YYYY
char powerSourceStr[8]; // CEB or Battery

void setup() {
  Serial.begin(115200);
  espNano.begin(9600);
  nanoOut.begin(9600);

  // Initialize OLED
  Wire.begin();
  oled.begin(&Adafruit128x64, 0x3C);
  oled.setFont(System5x7);
  oled.clear();

  // Encoder pins
  pinMode(CLK_PIN, INPUT);
  pinMode(DT_PIN, INPUT);
  pinMode(SW_PIN, INPUT_PULLUP);
  lastCLK = digitalRead(CLK_PIN);

  // Initialize default values
  temp = hum = dew = press = volt = curr = wspeed = pwr = 0.0;
  wdir = 0;
  strcpy(utctimeStr, "00:00:00");
  strcpy(localtimeStr, "00:00:00");
  strcpy(latStr, "0.000000");
  strcpy(lonStr, "0.000000");
  strcpy(dateStr, "01/01/2024");
  strcpy(powerSourceStr, "CEB");

  updateDisplay();
  Serial.println(F("Nano RX Ready..."));
}

void loop() {
  // ---------- Receive CSV ----------
  while (espNano.available()) {
    char c = espNano.read();

    if (c == '\n') {
      receivedBuffer[bufferIndex] = '\0'; // terminate string

      // Print to Serial Monitor
      Serial.print(F("Received CSV: "));
      Serial.println(receivedBuffer);

      // Mark as new data (send once in loop)
      newData = true;

      // Parse for OLED display
      parseData();
      updateDisplay();

      bufferIndex = 0; // reset for next line
    }
    else if (bufferIndex < sizeof(receivedBuffer) - 1) {
      receivedBuffer[bufferIndex++] = c;
    }
  }

  // ---------- Forward only when new CSV received ----------
  if (newData) {
    nanoOut.println(receivedBuffer);  // send once
    newData = false;                  // reset flag
  }

  // ---------- Rotary encoder ----------
  byte currentCLK = digitalRead(CLK_PIN);
  if (currentCLK != lastCLK) {
    if (currentCLK == LOW) {
      if (digitalRead(DT_PIN) == HIGH) {
        currentPage = (currentPage + 1) % 4;
      } else {
        currentPage = (currentPage > 0) ? currentPage - 1 : 3;
      }
      updateDisplay();
    }
    delay(10);
  }
  lastCLK = currentCLK;

  // ---------- Button ----------
  if (digitalRead(SW_PIN) == LOW) {
    delay(200);
    updateDisplay();
  }
}

// ---------- Parse CSV for OLED ----------
void parseData() {
  char* ptr = receivedBuffer;
  byte field = 0;
  char tempStr[32];
  byte strIndex = 0;

  while (*ptr && field < 15) { // 15 fields in CSV
    if (*ptr == ',' || *ptr == '\0') {
      tempStr[strIndex] = '\0';

      switch (field) {
        case 0: strncpy(dateStr, tempStr, 10); dateStr[10]='\0'; break;
        case 1: strncpy(utctimeStr, tempStr, 8); utctimeStr[8]='\0'; break;
        case 2: strncpy(localtimeStr, tempStr, 8); localtimeStr[8]='\0'; break;
        case 3: strncpy(latStr, tempStr, 11); latStr[11] = '\0'; break;
        case 4: strncpy(lonStr, tempStr, 11); lonStr[11] = '\0'; break;
        case 5: temp = atof(tempStr); break;
        case 6: hum = atof(tempStr); break;
        case 7: press = atof(tempStr); break;
        case 8: dew = atof(tempStr); break;
        case 9: wspeed = atof(tempStr); break;
        case 10: wdir = atoi(tempStr); break;
        case 11: volt = atof(tempStr); break;
        case 12: curr = atof(tempStr); break;
        case 13: pwr = atof(tempStr); break;
        case 14: strncpy(powerSourceStr, tempStr, 7); powerSourceStr[7] = '\0'; break;
      }

      field++;
      strIndex = 0;
      if (*ptr == '\0') break;
    } else if (strIndex <  sizeof(tempStr) - 1) {
      tempStr[strIndex++] = *ptr;
    }
    ptr++;
  }
}

// ---------- OLED Display ----------
void updateDisplay() {
  oled.clear();

  switch (currentPage) {
    case 0: // TIME
      oled.println(F("=== TIME ==="));
      oled.print(F("UTC Time: ")); oled.println(utctimeStr);
      oled.print(F("Local Time: ")); oled.println(localtimeStr);
      oled.print(F("Date: ")); oled.println(dateStr);
      oled.println(); oled.println(F("Page 1/4"));
      break;

    case 1: // WEATHER
      oled.println(F("=== WEATHER ==="));
      oled.print(F("Temp: ")); oled.print(temp,1); oled.println(F("C"));
      oled.print(F("Humid: ")); oled.print(hum,1); oled.println(F("%"));
      oled.print(F("Press: ")); oled.print(press,0); oled.println(F("hPa"));
      oled.print(F("Dew: ")); oled.print(dew,1); oled.println(F("C"));
      oled.println(); oled.println(F("Page 2/4"));
      break;

    case 2: // WIND
      oled.println(F("=== WIND ==="));
      oled.print(F("Speed: ")); oled.print(wspeed,1); oled.println(F("m/s"));
      oled.print(F("Dir: ")); oled.print(wdir); oled.println(F("deg"));
      oled.println(); oled.println(F("Page 3/4"));
      break;

    case 3: // POWER
      oled.println(F("=== POWER ==="));
      oled.print(F("Volt: ")); oled.print(volt,2); oled.println(F("V"));
      oled.print(F("Curr: ")); oled.print(curr,2); oled.println(F("A"));
      oled.print(F("pwr: ")); oled.print(pwr,2); oled.println(F("W"));
      oled.print(F("Src: ")); oled.println(powerSourceStr);
      oled.println(); oled.println(F("Page 4/4"));
      break;
  }
}
