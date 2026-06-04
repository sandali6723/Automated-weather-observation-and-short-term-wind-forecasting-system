# PCB Design

The system is divided into two main PCB modules:

- **Transmitter Unit PCB**
- **Receiver Unit PCB**
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


