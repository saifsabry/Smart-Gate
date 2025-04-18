# 🔐 Smart Gate Control System

An IoT-based access control system that uses RFID authentication and servo motor control to manage secure gate entry, powered by the ESP32 microcontroller and a .NET backend.

## 📌 Overview

The Smart Gate Control System is designed to provide secure, contactless access to restricted areas. Using RFID tags and Wi-Fi communication, the system verifies users against a .NET-based backend before activating a servo motor to open or close the gate.

## 🚀 Features

- RFID authentication using RC522 module  
- Gate control with servo motor  
- Wi-Fi-enabled communication using ESP32  
- RESTful API integration with a .NET backend  
- Real-time decision-making and gate actuation  
- Modular and scalable design

## 🧰 Tech Stack

- **Hardware**: ESP32, RC522 RFID Reader, SG90 Servo Motor  
- **Software**: Arduino IDE (C++), .NET (Backend API), HTTP communication  
- **Protocols**: RESTful API over Wi-Fi  
- **Optional Add-ons**: LCD display, buzzer, LED indicators

## ⚙️ How It Works

1. User taps RFID card on the reader.  
2. ESP32 reads the card UID via the RC522 module.  
3. The UID is sent to the .NET backend via an HTTP POST request.  
4. Backend responds with access granted or denied.  
5. If access is granted, the ESP32 triggers the servo to open the gate.  
6. Access attempt is logged in the backend system (optional).

## 🛠️ Hardware Connections

| Component       | ESP32 Pin       |
|-----------------|------------------|
| RC522 RFID      | SPI (MOSI, MISO, SCK, SS) |
| Servo Motor     | PWM Pin (e.g., D5)       |
| Power Supply    | 5V and GND       |

## 📂 Project Structure

