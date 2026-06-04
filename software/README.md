# Software System

This repository contains the **software implementation** of the Automated Weather Observing System (AWOS), developed to receive, process, analyze, and visualize meteorological data transmitted from the ESP32-based hardware system.

The software is designed using a **two-tier client-server architecture**, enabling real-time monitoring, historical data analysis, and machine learning-based forecasting.

---

## 🏗️ System Architecture

The AWOS software follows a **two-layer architecture**:

### 🔹 Backend Layer (Server)
- Built using **Flask (Python)**
- Handles data processing, storage, and API services
- Communicates with ESP32 via TCP/IP Ethernet connection
- Provides RESTful endpoints for frontend access

### 🔹 Frontend Layer (User Interface)
- Built using **HTML5, CSS3, JavaScript**
- Runs in web browser or desktop app via **PyWebView**
- Provides real-time dashboard, history, forecasting, and settings modules

---

## 🌐 Network Configuration & ESP32 Communication

The ESP32 acts as a **TCP server over Ethernet**, transmitting sensor data to the backend system.

### 📡 Network Parameters:
- **ESP32 IP Address:** `192.168.4.177`
- **PC Client IP:** `192.168.4.5` (static configuration required)
- **TCP Port:** `5000`
- **Protocol:** TCP/IP
- **Data Format:** JSON

### 📊 Transmitted Data Includes:
- Temperature & Humidity
- Atmospheric Pressure
- Wind Speed & Wind Direction
- Dew Point
- Voltage, Current, Power Metrics
- GPS Coordinates
- UTC Timestamp

The system includes **auto-reconnection mechanisms** to ensure uninterrupted data flow.

---

## 🧠 Backend Server Architecture

The backend is developed using **Flask REST API** running on port `3000`.

### 🔧 Key Processing Functions:
- JSON data validation and parsing
- Unit conversion (e.g., m/s → knots for aviation use)
- Timestamp synchronization (UTC + local time)
- Real-time in-memory data storage
- Historical logging every 5 minutes

### 💾 Data Storage System:
- File-based database (`backup.json`)
- 30-day retention policy (FIFO mechanism)
- Maximum capacity: **8,640 records**
- Automatic backup every 30 minutes

### 📁 Data Schema (17 Fields):
- Timestamp, UTC Date, UTC Time, Local Time
- Temperature, Humidity, Pressure, Dew Point
- Wind Speed, Wind Direction
- Voltage, Current, Power, Power Status
- Communication Mode
- Latitude, Longitude

---

## 🔌 RESTful API Endpoints

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/api/weather/latest` | GET | Get current weather data |
| `/api/weather/history` | GET | Retrieve historical dataset |
| `/api/weather/forecast` | GET | ML-based wind prediction |
| `/api/weather/stats` | GET | Statistical analysis (min/max/avg) |
| `/api/weather/update` | POST | Receive data from ESP32 |
| `/api/weather/export` | GET | Export data as JSON/XLSX |
| `/api/weather/history` | DELETE | Clear historical records |
| `/api/health` | GET | System health check |

✔ CORS enabled for secure frontend-backend communication

---

## 🖥️ Frontend Architecture

The frontend is a **responsive web-based GUI** accessible via browser or desktop wrapper.

### ⚙️ Technologies:
- HTML5
- CSS3
- JavaScript
- Chart.js (data visualization)
- SheetJS (Excel export)
- PyWebView (desktop integration)

---

## 🧭 Navigation System

The interface uses a **sidebar-based navigation layout**:

1. **Dashboard**
   - Real-time weather visualization
   - Compass-based wind direction display
   - Live sensor monitoring

2. **History**
   - Paginated weather logs (50 records/page)
   - Excel export support

3. **Forecast**
   - Machine Learning-based predictions
   - Interactive time-series graphs

4. **Settings**
   - System configuration
   - Authentication-protected controls

---

## 📊 Dashboard Features

- Live updates every **5 seconds**
- Wind speed displayed in **knots (aviation standard)**
- Safety threshold alerts
- System status indicators:
  - Power status
  - Connectivity status
  - Location data
  - UTC time synchronization

---

## 📜 History Module

- Complete historical weather database access
- Pagination-based data browsing
- Export to Excel (XLSX format)
- Built using **SheetJS library**

---

## 🔮 Forecast Module (Machine Learning)

The forecasting system uses **ML models trained on historical weather data**.

### Models Used:
- 🌪️ Wind Speed Prediction → **Random Forest Regression**
- 🧭 Wind Direction Prediction → **Support Vector Regression (SVR)**

### Features:
- 2-hour ahead forecasting
- Updates every 5 minutes
- Confidence-based visualization
- Requires minimum **48 historical data points**

---

## ⚙️ Settings Module

Protected configuration panel including:

- Weather threshold settings
- Runway configuration
- System diagnostics (uptime, records, forecast status)
- Authentication system
- Password management
- Communication mode selection

All settings are stored in **browser local storage**.

---

## 📦 Software Dependencies

- Flask 2.0+
- Flask-CORS
- NumPy
- Pandas
- scikit-learn
- PyWebView

---

## 🚀 Key Features

- Real-time weather monitoring system
- Secure ESP32 Ethernet communication
- Machine learning-based forecasting
- Web + desktop hybrid interface
- Aviation-standard wind speed reporting
- Robust data logging with 30-day retention
- Fully modular and scalable architecture

---
