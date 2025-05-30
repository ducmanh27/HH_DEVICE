# Indoor Environment Monitoring System (Device node)

## Overview

This project is an **indoor environment monitoring system** that uses various sensors to measure environmental parameters such as:
- Temperature
- Humidity
- Air quality (CO₂, TVOC, PM2.5, etc.)
- Electric info (Voltage, current, power, energy, power factor, ...)

The data is collected and published periodically to an **MQTT broker** for remote monitoring, visualization, and analysis.

## Hardware Used

- 🧠 **Measurement MCU**: STM32F407 Discovery Board  
- 🔄 **Communication MCU**: ESP32 (built-in WiFi)  
- 🌡️ **Temperature & Humidity Sensor**: SHT2x  
- 💨 **Air Quality Sensor**: Sharp GP2Y1014  
- ⚡ **Power Measurement Sensor**: PZEM-004T (voltage, current, power, energy)  
- ⚡ **Power Supply**: 3.3V / 5V DC  

## Software

- Language: C++ (Arduino IDE / PlatformIO / STM32CubeIDE)
- Communication Protocol: **MQTT**
- Libraries:
  - `PubSubClient` for MQTT
  - `FreeRTOS`
  - `Driver for sensor` SHT2x, PZEM-004T, GP2Y1014
- MQTT Brokers:
  - Mosquitto (self-hosted)

