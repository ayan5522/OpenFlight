# OpenFlight Components

## Overview

OpenFlight is built using a combination of hardware and software components.

The main hardware system consists of an ESP32-WROOM-32 microcontroller and an ST7789 SPI TFT display. The Python-based flight simulator runs on a laptop and sends real-time telemetry to the ESP32 over Wi-Fi using TCP.

---

# Hardware Components

## 1. ESP32-WROOM-32

**Type:** Microcontroller  
**Role:** Main embedded controller

The ESP32-WROOM-32 acts as the central hardware controller of OpenFlight.

### Responsibilities

- Connects to Wi-Fi
- Creates the TCP server
- Receives telemetry from the Python simulator
- Parses telemetry data
- Updates the TFT display
- Communicates with the display using SPI

### Communication

```text
Python Simulator
       |
       | Wi-Fi / TCP
       v
ESP32-WROOM-32