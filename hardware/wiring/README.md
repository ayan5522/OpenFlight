# OpenFlight Hardware Wiring Guide

## Hardware

- ESP32-WROOM-32 / ESP32 DevKit V1
- ST7789 240x320 SPI TFT
- Jumper wires
- USB cable

---

## ST7789 to ESP32 Wiring

| ST7789 | ESP32 | Function |
|---|---|---|
| VCC | 3V3 | Power |
| GND | GND | Ground |
| SCK / SCL | GPIO 18 | SPI Clock |
| SDA / SDI / MOSI | GPIO 23 | SPI MOSI |
| SDO / MISO | GPIO 19 | SPI MISO |
| CS | GPIO 5 | Chip Select |
| DC / A0 | GPIO 27 | Data / Command |
| RESET / RES | GPIO 26 | Reset |
| LED / BL | 3V3 | Backlight |

> Note: On an SPI TFT, SDA/SDI normally refers to SPI MOSI, not I2C SDA.

---

## Wiring Diagram

```text
ESP32-WROOM-32                 ST7789 TFT
----------------               ----------------
3V3       -------------------> VCC
GND       -------------------> GND
GPIO 18   -------------------> SCK / SCL
GPIO 23   -------------------> SDA / SDI / MOSI
GPIO 19   -------------------> SDO / MISO
GPIO 5    -------------------> CS
GPIO 27   -------------------> DC / A0
GPIO 26   -------------------> RESET / RES
3V3       -------------------> LED / BL