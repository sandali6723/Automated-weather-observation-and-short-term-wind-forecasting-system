# Firmware 

## 📡 System Overview

The firmware is designed for a multi-node embedded architecture:

- **3 × Arduino Nano modules**
- **2 × ESP32 modules (ESP32-WROOM-32)**

Each module performs a dedicated role within the AWOS network, ensuring reliable environmental data acquisition, transmission, processing, and visualization.

---

## 🧩 System Architecture

### 🔹 Arduino Nano Modules (Field Nodes)
The Nano units are responsible for:

- Reading sensor data (weather-related parameters via OLED )
- Handling wired RS485 communication (via TTL-RS485 converters)
- Sending processed sensor data to the transmitter ESP32
---

### 🔹 ESP32 – Transmitter Unit (Field Node)
This ESP32 acts as the main transmitter-side controller:

- Collects data from sensors
- Processes and formats sensor data
- Sends data via **LoRa communication module**
- Handles serial communication with RS485-TTL interface
- Manages timing and synchronization using GPS (if integrated)

---

### 🔹 ESP32 – Receiver Unit (Central Processing Node)
This ESP32 is used in the receiver station:

- Receives data via LoRa communication
- Processes incoming environmental data
- Sends data to display and storage modules
- Handles Ethernet / WiFi connectivity (primary and backup communication)
- Supports integration with SD card logging system
- Provides interface for OLED display visualization

---

## ⚙️ Development Environment

- **IDE:** Arduino IDE
- **Programming Language:** C / C++
- **Board Support:**
  - Arduino Nano (ATmega328P)
  - ESP32-WROOM-32
- **Libraries Used:**
  - LoRa library
  - ESP32 core libraries
  - SoftwareSerial / HardwareSerial
  - Wire / SPI libraries
  - SD card library
  - OLED display libraries (e.g., Adafruit SSD1306 / U8g2)

---


