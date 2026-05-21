# ✈️ Standalone Automated Weather Observation System (AWOS)
### Short-Term Wind Forecasting for Regional International Airports in Sri Lanka

## 📖 Overview

Many regional airports in Sri Lanka — including **Ratmalana**, **Jaffna**, and **Batticaloa** — lack Automated Weather Observation Systems (AWOS) and rely on error-prone manual observations. Commercial AWOS solutions are prohibitively expensive for these airports.

This project presents a **low-cost, standalone AWOS** designed for regional aviation applications. The system provides real-time meteorological monitoring with integrated machine learning-based short-term wind forecasting — achieving performance comparable to commercial AWOS at a significantly reduced cost.

> Field-tested at **Colombo International Airport, Ratmalana**, with validation against official meteorological department reference data and ATC tower instrumentation.

---

## 🌟 Key Features

- 📡 **Real-time acquisition** of temperature, humidity, atmospheric pressure, dew point, wind speed, and wind direction
- 🛰️ **GPS-based UTC time synchronization** for accurate, timezone-independent data logging
- 📶 **LoRa wireless communication** with wired backup channel for reliable data transmission
- 🔋 **Battery backup** for uninterrupted operation independent of external infrastructure
- 🌐 **Web-based dashboard** (Flask + Next.js) for real-time visualization, historical data browsing, and configurable alerts
- 🤖 **ML wind forecasting**: Random Forest for wind speed, SVR for wind direction (2-hour horizon)
- 📊 **Data export** in XLSX format with aviation-compatible formatting
- 🔔 **Threshold-based alerts** for abnormal weather conditions, sensor faults, and communication failures

---

## 🏗️ System Architecture

The system is composed of two primary subsystems:

<img width="383" height="393" alt="image" src="https://github.com/user-attachments/assets/caa0228a-2606-4ecc-8ba6-aa7fdae8a6c2" />
---

## 🤖 Machine Learning Models

### Wind Speed Forecasting

| Model | MAE (kt) | RMSE (kt) | Accuracy (%) |
|---|---|---|---|
| **Random Forest**| **1.135** | **1.568** | **85.35** |

### Wind Direction Forecasting (±45° tolerance)

| Model | Forecast Horizon | MAE (°) | RMSE (°) | Accuracy ±45° (%) |
|---|---|---|---|---|
| SVR | +120 min | **21.02** | **21.36** | **100.0** |

> **Note:** Sine-cosine transformation is applied to wind direction data to handle the 0°/360° angular discontinuity during preprocessing.

---

## 🛠️ Hardware Components

| Component | Description |
|---|---|
| ESP32 | Central data acquisition and processing microcontroller |
| AHT20 + BMP280 | Temperature, humidity & atmospheric pressure sensor |
| RS485 Anemometer | Industrial-grade wind speed measurement |
| RS485 Wind Vane (360°) | Wind direction sensor (10–30V) |
| GPS Module | UTC time synchronization |
| LoRa Module | Long-range, low-power wireless communication |
| Custom PCBs | Designed for field sensing unit and central monitoring unit |
| Nano Module + SD Card | Local data storage |
| OLED Display | On-unit status display |

---

## 💻 Software Stack

| Layer | Technology |
|---|---|
| Firmware | ESP32 (Arduino/C++), I²C, UART, RS485 |
| Primary Dashboard | Flask (Python), Ethernet |
| Database | Local database with 30-day FIFO buffer |
| ML Models | Python (Random Forest, SVR, scikit-learn) |
| Data Export | XLSX (Excel-compatible) |
| Training Data | Meteostat historical data + AWOS real-time observations (30-min intervals) |

---

## 📊 System Performance

- **Latency:** 0–10 seconds (near-instantaneous data transmission)
- **Wind speed forecast MAE:** 1.35 kt (field validation, 2-hour horizon)
- **Wind direction forecast MAE:** 10.29° (field validation, 2-hour horizon)
- **Recommended safety margins:** ±10° for direction, ±1.5 kt for speed (high-precision decisions)
- **Temperature, humidity, pressure, dew point:** Strong agreement with reference instruments
- **Pressure calibration:** Systematic offset of one unit identified and documented

---
