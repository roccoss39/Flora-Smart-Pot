# Flora Smart Pot 🌱

An intelligent plant monitoring and irrigation system based on ESP32 with cloud connectivity via Blynk.

## 📋 Features

- **Soil Moisture Monitoring** - Automatic soil moisture measurement with configurable thresholds
- **Water Level Detection** - 5-level water tank monitoring system
- **Automatic Irrigation** - PWM-controlled pump with customizable duration and power
- **Environmental Sensing** - Temperature and humidity monitoring (DHT11)
- **Battery Monitoring** - Real-time battery voltage tracking with low battery alerts
- **Smart Alarms** - Audio and visual alerts for low water, low battery, and dry soil
- **Cloud Connectivity** - Real-time data transmission to Blynk cloud platform
- **Power Management** - Deep sleep mode for battery conservation
- **WiFi Configuration** - Easy WiFi setup via captive portal
- **LED Status Indicators** - Visual feedback for system status

## 🔧 Hardware Requirements

### Main Components
- **ESP32 Development Board** (ESP32-DevKit or similar)
- **Soil Moisture Sensor** (Capacitive or resistive)
- **DHT11** - Temperature/Humidity sensor
- **Water Level Sensor** - 5-level detection system
- **Water Pump** - 12V DC pump with PWM control
- **Buzzer** - For audio alarms
- **LED** - Status indicator
- **Push Button** - Manual control and wake-up
- **Battery** - Li-ion or Li-Po for portable operation

### Optional Components
- **MPU6500** - Motion sensor (currently disabled)
- **External antenna** - For better WiFi range

## 📐 Pin Configuration

| Component | Default Pin | Configurable |
|-----------|-------------|--------------|
| Soil Sensor (ADC) | A0 | ✅ |
| Soil Sensor (VCC) | GPIO 4 | ✅ |
| DHT11 Data | GPIO 2 | ✅ |
| DHT11 Power | GPIO 5 | ✅ |
| Water Level 1-5 | GPIO 12-16 | ✅ |
| Water Ground | GPIO 17 | ✅ |
| Pump Control | GPIO 18 | ✅ |
| Battery ADC | A3 | ✅ |
| Buzzer | GPIO 19 | ✅ |
| LED | GPIO 21 | ✅ |
| Button | GPIO 0 | ✅ |
| I2C SDA | GPIO 21 | ✅ |
| I2C SCL | GPIO 22 | ✅ |

## 🚀 Quick Start

### 1. Hardware Setup
1. Connect all sensors according to pin configuration
2. Ensure proper power supply (3.3V for sensors, 12V for pump)
3. Install water pump in reservoir
4. Place soil sensor in plant pot

### 2. Software Setup
1. Install [PlatformIO](https://platformio.org/) or Arduino IDE
2. Clone this repository
3. Configure your Blynk credentials in `include/secrets.h`
4. Upload the firmware to ESP32

### 3. Initial Configuration
1. Power on the device
2. Connect to "Flora-Setup" WiFi network
3. Configure your home WiFi credentials
4. The device will automatically connect and start monitoring

## ⚙️ Configuration

### WiFi Setup
- On first boot, device creates "Flora-Setup" access point
- Connect and configure your WiFi credentials
- Device remembers settings for future use

### Blynk Setup
1. Create account at [Blynk.io](https://blynk.io)
2. Create new project and get auth token
3. Update `include/secrets.h` with your credentials
4. Configure virtual pins in Blynk app

### Sensor Calibration
- **Soil Sensor**: Use button to calibrate dry/wet values
- **Water Level**: Adjust threshold in configuration
- **Battery**: Set low voltage warning level

## 📱 Blynk Integration

### Virtual Pins
| Pin | Function | Type |
|-----|----------|------|
| V0 | Soil Moisture (%) | Display |
| V1 | Water Level (1-5) | Display |
| V2 | Battery Voltage (V) | Display |
| V3 | Temperature (°C) | Display |
| V4 | Humidity (%) | Display |
| V5 | Pump Status | Display |
| V6 | Alarm Status | Display |
| V10 | Manual Pump Control | Button |
| V11 | Alarm Sound Enable | Switch |

## 🔋 Power Management

### Continuous Mode
- Device stays awake and monitors continuously
- Regular measurements every 60 seconds (configurable)
- Real-time Blynk updates

### Deep Sleep Mode
- Automatic sleep when no alarms active
- Wakes up for scheduled measurements
- Button wake-up for manual control
- Significant battery life extension

## 🚨 Alarm System

### Alarm Conditions
- **Low Water Level** (Level 0) - 2 beeps
- **Low Battery** - 3 beeps (highest priority)
- **Dry Soil** - 1 beep

### Alarm Behavior
- Audio alerts (if enabled)
- LED status indication
- Blynk notifications
- Automatic pump activation (if water available)

## 🛠️ Development

### Building
```bash
# Using PlatformIO
pio run

# Upload to device
pio run --target upload

# Monitor serial output
pio device monitor
```

### Project Structure
```
├── include/           # Header files
├── src/              # Source files
├── lib/              # Libraries
├── platformio.ini    # Build configuration
└── README.md         # This file
```

### Key Modules
- `DeviceConfig` - Configuration management
- `SoilSensor` - Soil moisture monitoring
- `WaterLevelSensor` - Water level detection
- `PumpControl` - Irrigation control
- `AlarmManager` - Alert system
- `BlynkManager` - Cloud connectivity
- `PowerManager` - Sleep management

## 🔧 Troubleshooting

### Common Issues

**WiFi Connection Failed**
- Check credentials in captive portal
- Ensure 2.4GHz network (ESP32 doesn't support 5GHz)
- Reset WiFi settings if needed

**Blynk Connection Failed**
- Verify auth token in `secrets.h`
- Check internet connectivity
- Ensure Blynk server is accessible

**Sensor Reading Errors**
- Check wiring connections
- Verify power supply voltage
- Calibrate sensors if needed

**Pump Not Working**
- Check 12V power supply
- Verify pump wiring
- Test manual pump control via Blynk

### Debug Mode
Enable detailed logging by setting `Serial.begin(115200)` and monitor output.

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🤝 Contributing

1. Fork the repository
2. Create feature branch (`git checkout -b feature/amazing-feature`)
3. Commit changes (`git commit -m 'Add amazing feature'`)
4. Push to branch (`git push origin feature/amazing-feature`)
5. Open Pull Request

## 📞 Support

For support and questions:
- Open an issue on GitHub
- Check troubleshooting section
- Review PlatformIO documentation

## 🔄 Version History

- **v1.0** - Initial release with basic monitoring
- **v1.1** - Added Blynk integration
- **v1.2** - Power management improvements
- **v1.3** - Enhanced alarm system

---

**Made with ❤️ for plant lovers** 🌿