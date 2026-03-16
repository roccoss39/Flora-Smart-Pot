# Flora Smart Pot 🌱

An intelligent plant monitoring and irrigation system based on ESP32 with cloud connectivity via Blynk.  
Most parts are printed in 3D – see [Thingiverse thing:4921885](https://www.thingiverse.com/thing:4921885) for the original Flaura enclosure files.

---

## 📋 Features

- **Soil Moisture Monitoring** – automatic measurement with configurable dry/wet thresholds
- **Water Level Detection** – 5-level water-tank monitoring system
- **Automatic Irrigation** – PWM-controlled pump with customizable duration and power
- **Environmental Sensing** – temperature and humidity monitoring (DHT11)
- **Battery Monitoring** – real-time voltage tracking with low-battery alerts
- **Smart Alarms** – audio and visual alerts for low water, low battery, and dry soil
- **Cloud Connectivity** – real-time data transmission to Blynk cloud platform
- **Power Management** – deep-sleep mode for battery conservation
- **WiFi Configuration** – easy WiFi setup via captive portal (WiFiManager)
- **LED Status Indicators** – visual feedback for system status

---

## 🔧 Hardware Requirements

### Main Components

| Component | Notes |
|-----------|-------|
| **ESP32 Dev Board** | See supported boards below |
| **Soil Moisture Sensor** | Capacitive (recommended) or resistive |
| **DHT11** | Temperature + humidity |
| **Water Level Sensors** | 5-probe detection system |
| **Water Pump** | 5 V or 12 V DC, PWM-controlled via MOSFET |
| **Buzzer** | Passive, for audio alarms |
| **LED** | Status indicator (or use the on-board LED) |
| **Push Button** | Manual wake-up / reset |
| **LiPo / Li-ion Battery** | 3.7 V single cell |

### Optional Components

- **MPU6500** – motion sensor (code present, currently disabled)
- **External WiFi antenna** – for better range in thick enclosures

---

## 🖥️ Supported Boards

### Wemos LOLIN D32 ✅ (recommended)

| Feature | Detail |
|---------|--------|
| PlatformIO env | `lolin_d32` |
| Built-in LED | GPIO 5 |
| Built-in LiPo charger | TP4054, ~500 mA, PH-2 connector |
| Dedicated battery ADC | GPIO 35 (`_VBAT`) |
| Flash | 4 MB DIO |
| USB | Micro-USB with auto-reset |

```ini
pio run -e lolin_d32
pio run -e lolin_d32 --target upload
```

### ESP32 DevKit / LOLIN32 v1.0.0 ⚠️

```ini
pio run -e esp32dev
pio run -e esp32dev --target upload
```

> **⚠️ Known pin conflicts on this board (see table below):**  
> GPIO 2 (strapping) and GPIO 12 (strapping) are used in the *default* mapping.  
> If you experience boot failures, switch to the LOLIN D32 pin profile or remap manually.

---

## 📐 Pin Configuration

Two separate pin profiles are maintained in `include/pins.h` and selected at compile time via the `BOARD_LOLIN_D32` build flag.

### LOLIN D32 – Safe Pin Mapping

| Component | GPIO | ADC-safe with WiFi | Notes |
|-----------|------|--------------------|-------|
| Soil Sensor (ADC) | **36** | ✅ ADC1 | Input only |
| Soil Sensor (VCC) | **4** | – | 3.3 V switch |
| DHT11 Data | **33** | – | Moved from GPIO 2 (strapping) |
| DHT11 Power | **13** | – | Moved from GPIO 5 (LED_BUILTIN) |
| Water Level 1 | **25** | – | Moved from GPIO 12 (strapping) |
| Water Level 2 | **26** | – | |
| Water Level 3 | **27** | – | |
| Water Level 4 | **32** | – | Moved from GPIO 15 (strapping) |
| Water Level 5 | **14** | – | |
| Water Ground | **17** | – | |
| Pump Control | **18** | – | PWM via MOSFET |
| Battery ADC | **35** | ✅ ADC1 | Dedicated `_VBAT` pin on D32 |
| Buzzer | **19** | – | |
| LED | **5** | – | Built-in LED on LOLIN D32 |
| Button | **0** | – | Also BOOT button on board |
| I2C SDA | **21** | – | For optional MPU6500 |
| I2C SCL | **22** | – | |

### ESP32 DevKit / LOLIN32 v1.0.0 – Original Pin Mapping

| Component | GPIO | Status |
|-----------|------|--------|
| Soil Sensor (ADC) | A0 (36) | ✅ |
| Soil Sensor (VCC) | 4 | ✅ |
| DHT11 Data | **2** | ⚠️ strapping pin – must be LOW at boot |
| DHT11 Power | **5** | ✅ |
| Water Level 1 | **12** | ⚠️ strapping pin – must be LOW at boot |
| Water Level 2 | 13 | ✅ |
| Water Level 3 | 14 | ✅ |
| Water Level 4 | **15** | ⚠️ strapping pin – must be HIGH at boot |
| Water Level 5 | 16 | ✅ |
| Water Ground | 17 | ✅ |
| Pump Control | 18 | ✅ |
| Battery ADC | A3 (39) | ✅ |
| Buzzer | 19 | ✅ |
| LED | **21** | ⚠️ shared with I2C SDA |
| Button | 0 | ✅ |
| I2C SDA | 21 | ⚠️ shared with LED |
| I2C SCL | 22 | ✅ |

> **Note:** All IO pins run at **3.3 V**. Do not connect 5 V signals directly.

---

## 🚀 Quick Start

### 1. Hardware Setup

1. Connect all sensors according to the pin table for your board.
2. Ensure correct power supply (3.3 V for sensors, appropriate voltage for pump + MOSFET).
3. Install the water pump in the reservoir.
4. Insert the soil sensor into the plant pot.

### 2. Software Setup

1. Install [PlatformIO](https://platformio.org/) (VS Code extension or CLI).
2. Clone this repository:
   ```bash
   git clone https://github.com/roccoss39/Flora-Smart-Pot.git
   cd Flora-Smart-Pot
   ```
3. Copy `include/secrets.h.example` to `include/secrets.h` and fill in your Blynk credentials.
4. Select your board environment and upload:
   ```bash
   # LOLIN D32 (recommended)
   pio run -e lolin_d32 --target upload

   # Generic ESP32 DevKit
   pio run -e esp32dev --target upload
   ```

### 3. Initial Configuration

1. Power on the device.
2. Connect to the **"Flora-Setup"** WiFi access point.
3. Enter your home WiFi credentials in the captive portal.
4. The device will connect, sync with Blynk and start monitoring.

---

## ⚙️ Configuration

### Sensor Calibration

- **Soil Sensor** – hold the button to trigger dry/wet calibration sequence.
- **Water Level** – adjust thresholds in `include/config.h`.
- **Battery** – set low-voltage warning level in `include/config.h`.

### Blynk Setup

1. Create a free account at [blynk.io](https://blynk.io).
2. Create a new template and device, then copy the auth token.
3. Add the token to `include/secrets.h`:
   ```cpp
   #define BLYNK_AUTH_TOKEN "YourTokenHere"
   ```

---

## 📱 Blynk Virtual Pins

| Virtual Pin | Function | Type |
|-------------|----------|------|
| V0 | Soil Moisture (%) | Display |
| V1 | Water Level (1-5) | Display |
| V2 | Battery Voltage (V) | Display |
| V3 | Temperature (°C) | Display |
| V4 | Humidity (%) | Display |
| V5 | Pump Status | Display |
| V6 | Alarm Status | Display |
| V10 | Manual Pump Control | Button |
| V11 | Alarm Sound Enable | Switch |

---

## 🔋 Power Management

### Continuous Mode
- Stays awake, measures every 60 s (configurable).
- Real-time Blynk updates.

### Deep Sleep Mode
- Sleeps between measurements.
- Wakes on schedule or button press.
- Significantly extends battery life.

---

## 🚨 Alarm System

| Condition | Priority | Beeps |
|-----------|----------|-------|
| Low Battery | 🔴 High | 3 |
| Dry Soil | 🟡 Medium | 1 |
| Low Water | 🔵 Low | 2 |

Alarms also trigger: LED indication, Blynk notification, automatic pump activation (if water is available).

---

## 🛠️ Development

### Project Structure

```
├── include/
│   ├── config.h          # Thresholds, timings
│   ├── pins.h            # Board-specific pin definitions
│   └── secrets.h         # WiFi + Blynk credentials (not committed)
├── src/
│   ├── main.cpp
│   ├── SoilSensor.*
│   ├── WaterLevelSensor.*
│   ├── PumpControl.*
│   ├── AlarmManager.*
│   ├── BlynkManager.*
│   └── PowerManager.*
├── lib/                  # Local libraries
├── platformio.ini
└── README.md
```

### Build & Monitor

```bash
pio run -e lolin_d32 --target upload
pio device monitor -e lolin_d32
```

---

## 🔧 Troubleshooting

**Board won't boot / resets in a loop**  
→ Check strapping pins (GPIO 0, 2, 12, 15). Disconnect sensors from these pins during flashing.

**WiFi connection fails**  
→ ESP32 supports only 2.4 GHz networks. Check SSID/password in captive portal.

**Blynk connection fails**  
→ Verify `BLYNK_AUTH_TOKEN` in `secrets.h`. Check internet access.

**Pump not running**  
→ Check MOSFET wiring and power supply voltage. Test via V10 in Blynk.

**Sensor reads always 0 or 4095**  
→ Run calibration sequence. Make sure ADC pin is on ADC1 (GPIO 32-39).

---

## 📄 License

MIT License – see [LICENSE](LICENSE) for details.

## 🤝 Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

---

*Made with ❤️ for plant lovers 🌿*