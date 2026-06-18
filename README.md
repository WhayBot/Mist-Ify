# Mist-Ify

Automated mushroom cultivation control system based on ESP32 with a web dashboard.

## Features

- **Automated Humidity Control** - Water pump + mist maker with state machine
- **Web Dashboard** - Real-time monitoring, 24-hour chart, event log
- **WiFi AP Setup** - WiFi configuration without hardcoding
- **OTA Update** - Firmware update via WiFi (Arduino IDE or web upload)
- **Manual Override** - Manual control of pump and mist maker via web
- **Safety Alarm** - Buzzer alarm if mist maker timeouts after 5 minutes

## Components

| Component | Qty | Connection |
|-----------|-----|------------|
| ESP32 DOIT DevKit V1 | 1 | - |
| MOSFET Module (IRLZ44N) | 2 | SIG→GPIO25/26 |
| DHT22 (3-pin module) | 1 | DAT→GPIO4, VCC→3.3V |
| LCD 16x2 I2C | 1 | SDA→GPIO21, SCL→GPIO22 |
| 5V Water Pump | 1 | Via MOSFET 1 |
| 5V Mist Maker | 1 | Via MOSFET 2 |
| Buzzer Module (3-pin) | 1 | SIG→GPIO27 |
| 5V 3A USB PSU | 1 | Power for all components |

## Wiring

```
ESP32           MOSFET 1 (Pump)      MOSFET 2 (Mist)      Buzzer
─────           ────────────────     ────────────────     ──────
GPIO25 ──────── SIG                                      
GPIO26 ───────────────────────────── SIG
GPIO27 ──────────────────────────────────────────────── SIG
VIN ──── PSU 5V
GND ──── PSU GND

MOSFET 1 & 2:
  VCC, VIN ──── PSU 5V
  GND ──── PSU GND
  V+/V- ──── Load (Pump/Mist Maker)

DHT22: VCC→3.3V ESP32, GND→GND ESP32, DAT→GPIO4
LCD:   SDA→GPIO21, SCL→GPIO22, VCC→PSU 5V, GND→PSU GND
Buzzer: SIG→GPIO27, VIN→PSU 5V, GND→PSU GND
```

## Installation

### 1. Install Arduino IDE
Download from https://www.arduino.cc/en/software

### 2. Install ESP32 Board
1. File → Preferences → Additional Boards Manager URLs:
   ```
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```
2. Tools → Board → Boards Manager → Search for "esp32" → Install

### 3. Install Library
Tools → Manage Libraries → Install:
- **DHT sensor library** (by Adafruit)
- **LiquidCrystal I2C** (by Frank de Brabander)

### 4. Upload Firmware
1. Connect ESP32 via USB
2. Tools → Board → ESP32 Dev Module
3. Tools → Port → select ESP32 COM port
4. Sketch → Upload

### 5. Setup WiFi
1. After upload, ESP32 will create an AP: **Mist-Ify-Setup**
2. Connect your phone/laptop to the AP (password: `mistify1234`)
3. Open browser → `http://192.168.4.1`
4. Scan WiFi → Select network → Enter password → Connect
5. ESP32 will restart and connect to your home WiFi

### 6. Access Dashboard
Open browser → type ESP32 IP (see in Serial Monitor or LCD)

## OTA Update

### Via Arduino IDE
1. Tools → Port → select "mist-ify-ctrl" (network)
2. Upload as usual
3. OTA Password: `mistify`

### Via Web Dashboard
1. Open dashboard in browser
2. Scroll to the "Update Firmware" section
3. Select .bin file → Upload
4. Export .bin: Sketch → Export Compiled Binary

## Control Logic

```
IDLE ──[humidity ≤ 75%]──→ PUMPING (3 seconds)
                              │
                              ▼
                          SOAKING (3 seconds, water absorbs)
                              │
                              ▼
                          MISTING ──[humidity ≥ 80%]──→ IDLE
                              │
                              └──[timeout 5 minutes]──→ ALARM
                                                        │
                                                  [manual reset via web]
                                                        │
                                                        ▼
                                                      IDLE
```

## Configuration

Edit `config.h` to change:
- GPIO Pins
- Humidity thresholds (default: 75% low, 80% high)
- Pump duration, soak time, mist timeout
- AP mode SSID and password
- OTA Password
- Timezone

## Troubleshooting

| Issue | Solution |
|-------|----------|
| LCD not turning on | Check I2C address (run I2C scanner), check wiring |
| DHT22 error | Check connection, ensure 3-pin module, check pull-up resistor |
| WiFi not connecting | Reset WiFi via dashboard or hardcode in config.h |
| MOSFET not turning on | Check SIG pin, ensure IRLZ44N (not IRF520N) |
| Weak mist maker | Check PSU capacity, ensure minimum 2A |
