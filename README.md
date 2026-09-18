# OpenFlight

**OpenFlight: An Open-Source ESP-Based Real-Time Flight Simulator Telemetry Display**

OpenFlight connects [FlightGear](https://www.flightgear.org/) on a Windows PC to an ESP32 and a 2.4" ST7789 TFT, showing live **simulator** telemetry over Wi-Fi. It is for education, embedded systems, networking, and visualization—not for real aircraft or certified instruments.

## Problem statement

Flight simulators expose rich aircraft state, but a dedicated low-cost bench display is useful for labs and DIY avionics-style UI practice without claiming real-world certification.

## Objective

Build a reliable pipeline:

**FlightGear → UDP → Python bridge → TCP/Wi-Fi → ESP32 → SPI → TFT**

with clear logging, reconnection, and a readable aviation-style dashboard at 320×240 (landscape).

## Features (target)

- Nine telemetry fields: altitude, airspeed, heading, vertical speed, pitch, roll, throttle, gear, flaps
- Lightweight text protocol (newline-terminated)
- ESP32 TCP server; Python TCP client
- Connection and stale-data indicators on TFT and in Python logs
- Automatic recovery from Wi-Fi / TCP / FlightGear interruptions

## System architecture

See [docs/architecture/system-architecture.md](docs/architecture/system-architecture.md).

| Leg | Protocol | Endpoint |
|-----|----------|----------|
| FlightGear → Python | UDP | `127.0.0.1:5500` |
| Python → ESP32 | TCP | ESP32 IP, port `5000` |

## Hardware requirements

- ESP32 DevKit V1 (ESP32-WROOM-32)
- 2.4" TFT SPI 240×320, driver **ST7789**
- Wi-Fi access point (ESP32 and PC on same LAN)
- USB cable for programming / serial monitor

Details: [hardware/components.md](hardware/components.md), [hardware/pinout.md](hardware/pinout.md).

## Software requirements

| Component | Requirement |
|-----------|-------------|
| FlightGear | With Generic protocol support (version-specific setup in [flightgear/README.md](flightgear/README.md)) |
| Python | 3.10+ recommended (stdlib-first bridge) |
| Arduino IDE | ESP32 board support + Adafruit GFX + Adafruit ST7789 libraries |

## Folder structure

```text
OpenFlight/
├── README.md
├── LICENSE
├── .gitignore
├── flightgear/
│   ├── protocols/          # openflight.xml (Phase 9)
│   └── README.md
├── python_bridge/          # Phase 7+
├── esp32/OpenFlight_ESP32/ # Phase 2+
├── hardware/
├── docs/
├── tests/                  # Phase 18
└── data/sample_telemetry.txt
```

## Installation

**Status:** Phased build. Follow phases below; this README is updated through Phase 19.

1. Clone or copy this repository.
2. **ESP32:** Copy `esp32/OpenFlight_ESP32/config.h.example` → `config.h` and set Wi-Fi (never commit `config.h`).
3. **Python:** (Phase 7+) `cd python_bridge` and use `requirements.txt` if any deps are added.
4. **FlightGear:** (Phase 9+) Install `openflight.xml` per [flightgear/README.md](flightgear/README.md).

## FlightGear setup

See [flightgear/README.md](flightgear/README.md). Protocol file and verified property paths arrive in **Phase 9**.

## Python setup

See [python_bridge/README.md](python_bridge/README.md). Bridge code starts in **Phase 7**.

## ESP32 setup

1. Install Arduino IDE and [ESP32 board package](https://docs.espressif.com/projects/arduino-esp-core-esp32/en/latest/).
2. Library Manager: **Adafruit GFX**, **Adafruit ST7789** (and dependencies).
3. Copy `config.h.example` to `config.h` and set credentials.
4. Open `esp32/OpenFlight_ESP32/OpenFlight_ESP32.ino` when added in Phase 2+.

## TFT wiring

Use the pin map in [hardware/pinout.md](hardware/pinout.md). Touchscreen and microSD are unused in v1.

## Telemetry protocol

One line per update, comma-separated `KEY:value` pairs, newline terminated:

```text
ALT:12450,SPD:245,HDG:273,VS:650,PITCH:3.2,ROLL:-8.4,THR:72,GEAR:UP,FLAPS:5
```

| Key | Meaning | Display units |
|-----|---------|----------------|
| ALT | Altitude | ft |
| SPD | Airspeed | kt |
| HDG | Heading | degrees |
| VS | Vertical speed | fpm |
| PITCH | Pitch | degrees |
| ROLL | Roll | degrees |
| THR | Throttle | % |
| GEAR | Gear | UP / DOWN |
| FLAPS | Flaps | degrees (or sim representation) |

Sample: [data/sample_telemetry.txt](data/sample_telemetry.txt).

Unit normalization (e.g. throttle 0–1 → %, VS ft/s → fpm) is handled in Python and/or FlightGear XML as documented in later phases.

## How to run (full system)

Available after integration phases (12–11). Order:

1. Power ESP32; note IP on serial/TFT.
2. Start Python bridge with ESP32 IP.
3. Start FlightGear with Generic output to `127.0.0.1:5500`.

Exact commands will be added when the bridge and XML exist.

## Troubleshooting

| Symptom | Check |
|---------|--------|
| No UDP data | FlightGear running? `--generic=...` correct? Protocol file on FG search path? |
| ESP32 TCP fails | Same Wi-Fi? Firewall? Port 5000? IP in Python matches serial output? |
| TFT blank | Wiring vs [pinout.md](hardware/pinout.md); ST7789 (not ILI9341); 3V3 backlight |
| FG STALE on TFT | Simulator paused or protocol stopped; Python not forwarding |

## Testing

Incremental plan (Phases 2–18): ESP32 serial → Wi-Fi → TFT static UI → TCP → Python → FlightGear UDP → full chain → reconnection → soak test. See [tests/README.md](tests/README.md).

## Development phases

| Phase | Topic |
|-------|--------|
| 1 | Project structure (current) |
| 2 | ESP32 basic hardware test |
| 3 | ST7789 TFT test |
| 4 | Static dashboard |
| 5–6 | Wi-Fi + TCP server |
| 7–8 | Python → ESP32 → TFT |
| 9–12 | FlightGear XML + UDP + integration |
| 13–17 | Fields, UI, status, errors, reconnect |
| 18–20 | Tests, docs, release |

## Future scope

- Attitude / artificial horizon screen
- Touchscreen screen switching
- microSD logging

## Open-source technologies

| Project | Role | License (verify upstream) |
|---------|------|---------------------------|
| FlightGear | Simulator + Generic protocol | GPL (project policy) |
| Python | Bridge | PSF |
| Arduino-ESP32 / Espressif | Firmware framework | LGPL / project-specific |
| Adafruit GFX / ST7789 | Display | BSD |

This repository: [LICENSE](LICENSE) (MIT) for OpenFlight-authored code; third-party libraries retain their own licenses.

## License

OpenFlight project code and documentation in this repository are under the **MIT License** unless noted otherwise. See [LICENSE](LICENSE).
