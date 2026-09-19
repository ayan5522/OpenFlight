# OpenFlight

**OpenFlight: An Open-Source ESP-Based Real-Time Flight Simulator Telemetry Display**

OpenFlight is a lightweight flight simulation and embedded telemetry display project that connects a custom Python-based flight simulator to an ESP32 and a 2.4" ST7789 TFT display.

The Python simulator generates real-time flight telemetry and sends it directly to the ESP32 over Wi-Fi using TCP. The ESP32 receives, parses, and displays the telemetry on the ST7789 TFT using SPI.

OpenFlight is intended for education, embedded systems, networking, simulation, and real-time visualization. It is not a real aircraft instrument and is not intended for certified aviation use.

---

## Problem Statement

Flight simulators provide a large amount of flight information, but displaying important telemetry on a separate low-cost physical display provides a useful platform for learning embedded systems, wireless communication, real-time data processing, and aviation-style user interfaces.

OpenFlight addresses this by creating a lightweight software flight simulator connected to a physical ESP32-based telemetry display.

---

## Objective

The objective of OpenFlight is to build a complete real-time telemetry pipeline:

**Python Flight Simulator → TCP/Wi-Fi → ESP32 → SPI → ST7789 TFT**

The system provides:

- Lightweight flight simulation
- Real-time telemetry generation
- Wireless telemetry transmission
- ESP32-based telemetry processing
- Physical TFT telemetry visualization
- Flicker-reduced real-time display updates

---

## Current System Architecture

```text
+--------------------------------+
|      Python Flight Simulator   |
|             Pygame             |
+---------------+----------------+
                |
                | Wi-Fi / TCP
                | Port 5000
                v
+--------------------------------+
|          ESP32-WROOM-32        |
|                                |
|  TCP Server                    |
|  Telemetry Parser              |
|  Display Controller            |
+---------------+----------------+
                |
                | SPI
                v
+--------------------------------+
|          ST7789 TFT            |
|          240 x 320             |
|       Landscape: 320 x 240     |
+--------------------------------+