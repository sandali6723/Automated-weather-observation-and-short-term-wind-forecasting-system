#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <LoRa.h>
#include <ModbusMaster.h>
#include <SoftwareSerial.h>
#include <math.h>
#include <Adafruit_INA219.h>

// ---------- I2C Buses ----------
TwoWire I2C_INA(1);  // Use I2C bus 1 for INA219

// ---------- Sensor objects ----------
Adafruit_AHTX0 aht;
Adafruit_BMP280 bmp;
Adafruit_INA219 ina219;       // INA219
TinyGPSPlus gps;

// ---------- Serial ports ----------
HardwareSerial gpsSerial(2);   // GPS UART2 (pins 16 RX, 17 TX)
HardwareSerial rs485_1(1);     // RS485 Wind Speed UART1 (pins 32 RX, 33 TX)
SoftwareSerial rs485_2(15, 4); // RS485 Wind Dir
SoftwareSerial nanoSerial(-1, 13);  // Nano TX only

// ---------- Modbus objects ----------
ModbusMaster windSpeedSensor;
ModbusMaster windDirSensor;

// ---------- Pin assignments ----------
#define SDA_PIN 21
#define SCL_PIN 22
#define RS485_1_RX 32
#define RS485_1_TX 33
#define NANO_TX 13
#define POWER_PIN 39  // GPIO39 for power source detection

#define LORA_SCK 5
#define LORA_MISO 19
#define LORA_MOSI 27
#define LORA_CS 18
#define LORA_RST 14
#define LORA_IRQ 26
#define LORA_FREQUENCY 433E6

// ---------- Status flags ----------
bool aht_ok = false;
bool bmp_ok = false;
bool ina_ok = false;

// ---------- Functions ----------
double calculateDewPoint(double temp, double humidity) {
    const double a = 17.27;
    const double b = 237.7;
    double alpha = ((a * temp) / (b + temp)) + log(humidity / 100.0);
    return (b * alpha) / (a - alpha);
}

// Convert UTC to local time (Sri Lanka +5:30)
String convertToLocalTime(int utcHour, int utcMinute, int utcSecond) {
    int localHour = utcHour + 5;
    int localMinute = utcMinute + 30;
    int localSecond = utcSecond;

    if (localMinute >= 60) {
        localMinute -= 60;
        localHour += 1;
    }
    if (localHour >= 24) localHour -= 24;

    char buf[9];
    sprintf(buf, "%02d:%02d:%02d", localHour, localMinute, localSecond);
    return String(buf);
}

void setup() {
    Serial.begin(115200);

    // Initialize main I2C for AHT20 and BMP280
    Wire.begin(SDA_PIN, SCL_PIN);

    // Initialize custom I2C for INA219
    I2C_INA.begin(23, 25);  // SDA = 23, SCL = 25

    gpsSerial.begin(9600, SERIAL_8N1, 16, 17);
    rs485_1.begin(4800, SERIAL_8N1, RS485_1_RX, RS485_1_TX);
    rs485_2.begin(4800);
    nanoSerial.begin(9600);

    windSpeedSensor.begin(1, rs485_1);
    windDirSensor.begin(1, rs485_2);

    pinMode(POWER_PIN, INPUT);

    Serial.println("ESP32 Weather Station Initializing...");

    // AHT20
    if (!aht.begin()) {
        Serial.println("AHT20 not found!");
        aht_ok = false;
    } else {
        Serial.println("AHT20 initialized.");
        aht_ok = true;
    }

    // BMP280
    if (!bmp.begin(0x77)) {
        Serial.println("BMP280 not found!");
        bmp_ok = false;
    } else {
        Serial.println("BMP280 initialized.");
        bmp_ok = true;
    }

    // INA219
    if (!ina219.begin(&I2C_INA)) {
        Serial.println("INA219 not found!");
        ina_ok = false;
    } else {
        Serial.println("INA219 initialized.");
        ina_ok = true;
    }

    // LoRa
    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
    LoRa.setPins(LORA_CS, LORA_RST, LORA_IRQ);
    if (!LoRa.begin(LORA_FREQUENCY)) {
        Serial.println("LoRa initialization failed!");
        while (1);
    }
    Serial.println("LoRa initialized.");

    // Send CSV header to Nano
    String csvHeader = "UTC_Date,UTC_Time,Local_Time,Latitude,Longitude,Temperature,Humidity,Pressure,DewPoint,WindSpeed,WindDirection,Voltage,Current,Power,powerSource";
    nanoSerial.println(csvHeader);
    Serial.println("CSV header sent to Nano: " + csvHeader);
}

void loop() {
    String dataPacket = "------ Weather Data ------\n";
    String csvData = "";

    // ===== GPS =====
    while (gpsSerial.available() > 0) {
        gps.encode(gpsSerial.read());
    }

    String utcDate = "N/A", utcTime = "N/A", localTime = "N/A";
    String latitude = "N/A", longitude = "N/A";

    // UTC Time & Date
    if (gps.date.isValid() && gps.time.isValid()) {
        utcDate = String(gps.date.day()) + "/" + String(gps.date.month()) + "/" + String(gps.date.year());
        utcTime = String(gps.time.hour()) + ":" + String(gps.time.minute()) + ":" + String(gps.time.second());
        localTime = convertToLocalTime(gps.time.hour(), gps.time.minute(), gps.time.second());

        dataPacket += "UD: " + utcDate + "\n";
        dataPacket += "UT: " + utcTime + "\n";
        dataPacket += "LT: " + localTime + "\n";
    } else {
        dataPacket += "UTC Date: N/A\nUTC Time: N/A\nLocal Time: N/A\n";
    }

    // Latitude & Longitude
    if (gps.location.isValid()) {
        latitude = String(gps.location.lat(), 6);
        longitude = String(gps.location.lng(), 6);

        dataPacket += "Lat: " + latitude + "°\n";
        dataPacket += "Long: " + longitude + "°\n";
    } else {
        dataPacket += "GPS Location: Invalid\n";
    }

    // ===== AHT20 =====
    String temperature = "N/A", humidity = "N/A", dewPoint = "N/A";
    if (aht_ok) {
        sensors_event_t humidityEvent, tempEvent;
        aht.getEvent(&humidityEvent, &tempEvent);

        double tempVal = tempEvent.temperature;
        double humVal = humidityEvent.relative_humidity;
        double dewVal = calculateDewPoint(tempVal, humVal);

        temperature = String(tempVal, 1);
        humidity = String(humVal, 1);
        dewPoint = String(dewVal, 2);

        dataPacket += "Temp: " + temperature + " °C\n";
        dataPacket += "Hum: " + humidity + " %\n";
        dataPacket += "Dew: " + dewPoint + " °C\n";
    } else {
        dataPacket += "Temp: N/A\nHum: N/A\nDew: N/A\n";
    }

    // ===== BMP280 =====
    String pressure = "N/A";
    if (bmp_ok) {
        float pressureVal = bmp.readPressure() / 100.0;
        pressure = String(pressureVal, 2);
        dataPacket += "Press: " + pressure + " hPa\n";
    } else {
        dataPacket += "Press: N/A\n";
    }

    // ===== Wind Speed =====
    String windSpeed = "N/A";
    uint8_t result = windSpeedSensor.readHoldingRegisters(0x0000, 1);
    if (result == windSpeedSensor.ku8MBSuccess) {
        float wsVal = windSpeedSensor.getResponseBuffer(0) / 10.0;
        windSpeed = String(wsVal, 1);
        dataPacket += "WS: " + windSpeed + " m/s\n";
    } else {
        dataPacket += "WS: N/A\n";
    }

    // ===== Wind Direction =====
    String windDir = "N/A";
    result = windDirSensor.readHoldingRegisters(0x0001, 1);
    if (result == windDirSensor.ku8MBSuccess) {
        windDir = String(windDirSensor.getResponseBuffer(0));
        dataPacket += "WD: " + windDir + "°\n";
    } else {
        dataPacket += "WD: N/A\n";
    }

    // ===== INA219 =====
    String voltage = "N/A", current = "N/A", power = "N/A";
    if (ina_ok) {
        float voltVal = ina219.getBusVoltage_V();
        float currVal = ina219.getCurrent_mA() / 1000.0;
        float powerVal = ina219.getPower_mW() / 1000.0;

        voltage = String(voltVal, 2);
        current = String(currVal, 3);
        power = String(powerVal, 3);

        dataPacket += "V: " + voltage + " V\n";
        dataPacket += "C: " + current  + " A\n";
        dataPacket += "P: " + power + " W\n";
    } else {
        dataPacket += "V: N/A\nC: N/A\nP: N/A\n";
    }

    // ===== Power Source =====
    String powerSource = (digitalRead(POWER_PIN) == 0) ? "Battery" : "CEB";
    dataPacket += "PS: " + powerSource + "\n";

    // ===== Output to Serial =====
    Serial.print(dataPacket);

    // ===== Send via LoRa =====
    LoRa.beginPacket();
    LoRa.print(dataPacket);
    LoRa.endPacket();
    Serial.println("Data sent via LoRa!");

    // ===== Send CSV to Nano =====
    csvData = utcDate + "," + utcTime + "," + localTime + ","+ latitude + "," + longitude + "," + temperature + "," + humidity + "," + pressure + "," + dewPoint + "," + windSpeed + "," + windDir + "," + voltage + "," + current + "," + power + "," + powerSource;
    nanoSerial.println(csvData);

    delay(5000); // 5 seconds
}
