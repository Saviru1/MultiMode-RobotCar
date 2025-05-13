# 🤖 Autonomous Robot Controller

A smart robot system built with Arduino and ESP8266, capable of autonomous navigation, object detection, line following, and manual remote control — all managed through a responsive web interface. 🧠🌐

---

## 📸 Interface Previews

### 🧭 Modes Tab
![Modes Tab](gui_design_png/main_controller_tab.png)

### 🎮 Remote Control
![Remote Control](gui_design_png/remote_control_tab.png)

### 📊 Status Monitor
![Status Monitor](gui_design_png/status_tab.png)

---

## 🛠️ Hardware Overview

### 📷 Circuit Diagram
![Circuit Diagram](circuit_diagram_image.png)

**Main Components:**

- 🧠 Arduino UNO (central control)
- 📡 ESP8266 (for web interface & remote commands)
- 🔌 L298N Motor Driver (dual H-bridge for 4 DC motors)
- 🦾 4x TT Gear Motors with wheels
- 🎯 6x IR Sensors (line following, edge detection)
- 🚧 Ultrasonic Sensor (distance measurement)
- 🔁 Servo Motor (to rotate ultrasonic sensor for scanning)
- 🔋 2x 18650 Li-ion Batteries (power supply)
- 🟢 Power Switch
- 🔌 Resistors, jumpers, etc.

---

## 🚗 Features

- **🧭 Multiple Operation Modes:**
  - Obstacle Avoidance
  - Line Following
  - Obstacle + Line Hybrid
  - Object Tracking (using ultrasonic + servo sweep)
  - Remote Manual Control
  - Battery Monitoring

- **🌐 Web-Based UI:**
  - Real-time control and monitoring
  - Speed slider control
  - Tabs for Modes, Remote, and System Status

- **⚙️ Intelligent Control System:**
  - Auto/manual toggle
  - Battery level status
  - Modular logic for easy upgrades

---

## 💻 Software Stack

| Layer        | Tech                      |
|--------------|---------------------------|
| Controller   | Arduino UNO               |
| Web Server   | ESP8266 / NodeMCU         |
| Frontend     | HTML, CSS, JavaScript     |
| Firmware     | Arduino IDE (C/C++)       |
| Communication| Serial/Wi-Fi              |

---

## 🧪 Getting Started

### 1. 🧩 Hardware Setup
- Assemble the circuit as shown in the diagram.
- Ensure the batteries and motor driver are properly connected.
- Upload the Arduino sketch to the UNO.
- Upload the web interface to ESP8266 (e.g., via SPIFFS or `LittleFS`).

### 2. 🧑‍💻 Code Upload
- Open `firmware/robot_controller.ino` in Arduino IDE.
- Flash code to Arduino UNO and ESP8266.

### 3. 🌐 Access Web Interface
- Connect to ESP8266 Wi-Fi hotspot or get its IP from serial monitor.
- Visit `http://<esp_ip>` in your browser.
- Start driving or select a mode!

---

## 📁 Folder Structure

autonomous-robot-controller/
│
├── code
| ├── arduino_uno
| ├── esp8266_code
├── gui_design_png
| |── main_controller_tab
| ├── remote_controller_tab
| ├── status_tab
├── .gitignore
|__ LICENSE
|__ README.md
|__ circuit_diagram_image
│__ multi_mode_robot_working_video.mp4



---

## ✨ Future Work

- 🛰️ Add GPS tracking
- 🔋 Real-time battery voltage sensing
- 📷 Integrate camera stream (ESP32-CAM)
- 📱 Mobile app (Android) to control robot

---

## 🙌 Contributors

- 👤 Saviru Subasith Ferdinando

---

## 📃 License

This project is licensed under the MIT License. Feel free to modify and use it for personal or educational purposes.

---




