# OpenFlight Pinout

## Overview

This document defines the GPIO and SPI pin assignments used in the OpenFlight hardware.

The current hardware consists of:

- ESP32-WROOM-32 / ESP32 DevKit V1
- ST7789 240x320 SPI TFT

The ESP32 communicates with the ST7789 display using the SPI interface.

---

# ESP32 to ST7789 Pinout

| ESP32 Pin | ST7789 Pin | Signal | Description |
|---|---|---|---|
| 3V3 | VCC | 3.3V | TFT power |
| GND | GND | GND | Common ground |
| GPIO 18 | SCK / SCL | SPI Clock | SPI clock signal |
| GPIO 23 | SDA / SDI / MOSI | MOSI | SPI data from ESP32 |
| GPIO 19 | SDO / MISO | MISO | SPI data to ESP32 |
| GPIO 5 | CS | Chip Select | TFT chip selection |
| GPIO 27 | DC / A0 | Data/Command | Selects data or command |
| GPIO 26 | RESET / RES | Reset | TFT reset |
| 3V3 | LED / BL | Backlight | TFT backlight power |

---

# SPI Pin Configuration

OpenFlight uses the ESP32 hardware SPI pins:

```text
SCK  → GPIO 18
MOSI → GPIO 23
MISO → GPIO 19
CS   → GPIO 5