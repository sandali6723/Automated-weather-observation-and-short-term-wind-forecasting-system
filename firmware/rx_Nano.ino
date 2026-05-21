#include <SPI.h>
#include <LoRa.h>
#include <SoftwareSerial.h>

// ---------- LoRa Module Pins ----------
#define LORA_SCK 13
#define LORA_MISO 12
#define LORA_MOSI 11
#define LORA_CS 10
#define LORA_RST 9
#define LORA_IRQ 2
#define LORA_FREQUENCY 433E6

// ---------- ESP32 Communication Pins ----------
#define ESP_TX 3
#define ESP_RX 4
SoftwareSerial espSerial(ESP_TX, ESP_RX);

// ---------- TTL Converter Pins ----------
#define RX_PIN 8
#define TX_PIN -1 // Not used
SoftwareSerial mySerial(RX_PIN, TX_PIN);

// ---------- Timing Variables ----------
unsigned long lastLoRaTime = 0;
const unsigned long LORA_TIMEOUT = 40000; // 40 seconds
bool loRaAvailable = true;
bool usingTTL = false;

// ---------- Convert TTL CSV to Readable ----------
String csvToReadable(String csvLine) {
    String readable = "";
    int fieldIndex = 0;
    int lastIndex = 0;
    int commaIndex = csvLine.indexOf(',');

    while (commaIndex != -1) {
        String field = csvLine.substring(lastIndex, commaIndex);
        switch (fieldIndex) {
            case 0: readable += "UD: " + field + "\n"; break;
            case 1: readable += "UT: " + field + "\n"; break;
            case 2: readable += "LT: " + field + "\n"; break;
            case 3: readable += "Lat: " + field + "\n"; break;
            case 4: readable += "Long: " + field + "\n"; break;
            case 5: readable += "Temp: " + field + " °C\n"; break;
            case 6: readable += "Hum: " + field + " %\n"; break;
            case 7: readable += "Press: " + field + " hPa\n"; break;
            case 8: readable += "Dew: " + field + " °C\n"; break;
            case 9: readable += "WS: " + field + "\n"; break;
            case 10: readable += "WD: " + field + "\n"; break;
            case 11: readable += "V: " + field + " V\n"; break;
            case 12: readable += "C: " + field + " A\n"; break;
            case 13: readable += "P: " + field + " W\n"; break;
            case 14: readable += "PS: " + field + "\n"; break;
        }
        lastIndex = commaIndex + 1;
        commaIndex = csvLine.indexOf(',', lastIndex);
        fieldIndex++;
    }

    // Add last field if exists
    if (lastIndex < csvLine.length() && fieldIndex == 14) {
        String field = csvLine.substring(lastIndex);
        readable += "PS: " + field + "\n";
    }

    // Append mode at end
    readable += "Mode: Wired\n";

    return readable;
}

void setup() {
    Serial.begin(9600);      // Serial monitor
    espSerial.begin(9600);   // ESP32 communication
    mySerial.begin(9600);    // TTL communication

    Serial.println("Starting LoRa + TTL Failover System...");

    // Initialize LoRa
    SPI.begin();
    LoRa.setPins(LORA_CS, LORA_RST, LORA_IRQ);
    if (!LoRa.begin(LORA_FREQUENCY)) {
        Serial.println("LoRa initialization failed! Switching to TTL only.");
        loRaAvailable = false;
        usingTTL = true;
    } else {
        Serial.println("LoRa Ready - Primary Mode");
        lastLoRaTime = millis();
    }

    Serial.println("TTL Ready - Backup Mode");
    Serial.println("System Ready - Waiting for data...");
}

void loop() {
    // ---------- LoRa Mode ----------
    if (!usingTTL && loRaAvailable) {
        int packetSize = LoRa.parsePacket();
        if (packetSize) {
            String receivedData = "";
            while (LoRa.available()) {
                receivedData += (char)LoRa.read();
            }

            // Add communication mode
            String finalData = receivedData + "Mode: Wireless\n";

            // Display both raw and forwarded versions
            Serial.println("Raw LoRa: " + receivedData);
            Serial.println("Forwarded: " + finalData);

            espSerial.println(finalData);    // Send to ESP32

            lastLoRaTime = millis(); // Reset LoRa timeout
        } else {
            // If timeout, switch to TTL
            if (millis() - lastLoRaTime >= LORA_TIMEOUT) {
                Serial.println(">>> No LoRa data for 40s - Switching to TTL backup <<<");
                usingTTL = true;
            }
        }
    }

    // ---------- TTL Mode ----------
    else if (usingTTL) {
        if (mySerial.available()) {
            String data = mySerial.readStringUntil('\n'); // Read one line
            data.trim();

            if (data.length() > 0) {
                // Convert to readable before sending
                String readableData = csvToReadable(data);

                // Display both raw and readable versions
                Serial.println("Raw TTL: " + data);
                Serial.println("Readable TTL:\n" + readableData);

                espSerial.println(readableData); // Send readable to ESP32
            }
        }

        // Keep listening for LoRa to come back
        int packetSize = LoRa.parsePacket();
        if (packetSize) {
            String receivedData = "";
            while (LoRa.available()) {
                receivedData += (char)LoRa.read();
            }

            Serial.println(">>> LoRa signal restored! Switching back to LoRa <<<");
            usingTTL = false;

            // Send this recovered LoRa data right away
            String finalData = receivedData + "Mode: Wireless\n";
            Serial.println("Raw LoRa: " + receivedData);
            Serial.println("Forwarded: " + finalData);

            espSerial.println(finalData);

            lastLoRaTime = millis();
        }
    }
}
