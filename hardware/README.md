# PCB Design

The system is divided into two main PCB modules:

- **Transmitter Unit PCB**
- **Receiver Unit PCB**
---

## 🌦️ Sensors & Modules Used

### 📍 Environmental Sensors (Transmitter Side)
- **BMP280** – Atmospheric pressure and temperature (barometric sensing)  
- **Anemometer (RS485 based)** – Wind speed measurement  
- **Wind Direction sensor (RS485 based)** – Wind direction measurement
- **NEO 8MN GPS Module** – synchronize with UTC time  

---

### 📡 Communication Modules
- **RS485 to TTL Converter** – Long-distance sensor communication interface  
- **LoRa Module (SX1278 / compatible)** – Long-range wireless data transmission between transmitter and receiver  
- **ESP32-WROOM-32** – Main wireless processing and IoT connectivity unit  
- **UART / Serial Communication Interfaces** – Inter-module data exchange  

---

### ⚙️ Processing & Control Units
- **ESP32-WROOM-32** – Primary receiver-side processing and network handling  
- **Arduino Nano (Receiver Unit)** – Secondary control and data handling  
- **Arduino Nano (SD Control / Interface handling)** – Storage management support  

---

### 💾 Storage & Display Modules
- **SD Card Module** – Local data logging and backup storage  
- **OLED Display (I2C, SSD1306)** – Real-time weather data visualization  
- **Rotary Encoder Module** – Menu navigation and display control  

---

### ⚡ Power Management
- **Buck Converter Modules (LM2596 / similar)**  
  - 12V → 5V regulation  
  - 12V → 3.3V regulation  
- **Voltage Regulation Capacitors & Filtering Network** – Noise suppression and stability  

---
## 🧩 Transmitter PCB Design

The transmitter board is responsible for collecting sensor data and sending it via long-range communication.

### Key Features:
- RS485 to TTL communication module for long-distance sensor interfacing
- Central microcontroller unit for data acquisition and processing
- Dual output power regulation (12V → 5V & 3.3V) using buck converters
- Strategic component zoning for reduced interference
- Wide power traces for high current handling
- Ground plane (copper fill) for EMI reduction and stable reference ground
- Decoupling capacitors placed near IC power pins for noise filtering
- Manhattan-style routing (horizontal/vertical traces) for single-layer fabrication
- Mechanical mounting holes for structural stability

---

## 📶 Receiver PCB Design

The receiver board acts as the central processing and monitoring station of the AWOS.

### Key Features:
- LoRa module for long-range wireless data reception
- ESP32-WROOM-32 for processing and connectivity
- Receiver-NANO microcontroller for control tasks
- Ethernet module for primary wired communication (secure, no third-party dependency)
- WiFi as secondary backup connectivity
- SD card module for local data logging
- OLED display for real-time data visualization
- Rotary encoder interface for display navigation
- Extensive ground plane for noise reduction and heat dissipation
- Optimized trace routing for minimal signal interference
- Modular component grouping for easy debugging and scalability

---

## 🛠️ Design Tools

- **EasyEDA** – Schematic design and PCB layout
- Single-layer fabrication strategy for cost-effective manufacturing
- Industrial-style routing for improved reliability

---


