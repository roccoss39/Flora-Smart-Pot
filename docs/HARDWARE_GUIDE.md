# Hardware Setup Guide

Complete guide for assembling your Flora Smart Pot hardware.

## 📦 Bill of Materials (BOM)

### Essential Components

| Component | Quantity | Description | Estimated Cost |
|-----------|----------|-------------|----------------|
| ESP32 DevKit | 1 | Main microcontroller | $8-15 |
| Soil Moisture Sensor | 1 | Capacitive type recommended | $3-5 |
| Water Level Sensor | 1 | 5-level detection | $5-8 |
| Water Pump | 1 | 12V DC submersible pump | $8-12 |
| Push Button | 1 | Momentary switch | $0.50 |
| Resistors | Various | Pull-up/down resistors | $1-2 |
| Jumper Wires | 20+ | Male-to-male, male-to-female | $2-3 |
| Breadboard/PCB | 1 | For prototyping | $2-5 |
| Power Supply | 1 | 12V for pump, 5V for ESP32 | $5-10 |

### Optional Components

| Component | Quantity | Description | Purpose |
|-----------|----------|-------------|---------|
| DHT11/DHT22 | 1 | Temperature/humidity sensor | Required only for `lolin_d32` profile |
| Buzzer | 1 | 5V active buzzer | Required only for `lolin_d32` profile |
| LED | 1 | Status indicator | Required only for `lolin_d32` profile |
| MPU6500 | 1 | Motion sensor | Optional future extension |
| Li-ion Battery | 1 | 18650 3.7V | Portable operation |
| Battery Holder | 1 | 18650 holder with wires | Battery mounting |
| Voltage Regulator | 1 | 3.3V LDO regulator | Battery operation |
| Enclosure | 1 | Waterproof case | Protection |
| LCD Display | 1 | 16x2 I2C LCD | Local display |

## 🔌 Wiring Diagram

### ESP32 Pin Connections (actual firmware defaults)

> Firmware ma **dwa profile pinów**:
> - `env:lolin_d32` (`-D BOARD_LOLIN_D32`) – pełna funkcjonalność (DHT + buzzer + LED + MPU pin)
> - `env:lolin32` (bez flagi) – profil zgodny z **oryginalnym Flaura** (bez DHT, bez MPU, bez buzzera, bez LED statusowej)

#### `lolin_d32` (Wemos LOLIN D32)

| Component | GPIO | Notes |
|-----------|------|-------|
| Soil Sensor Signal | 34 | ADC input |
| Soil Sensor VCC | 4 | Power control |
| Water Level 1..5 | 33, 25, 26, 27, 14 | 5 sond poziomu |
| Water Level Ground Probe | 32 | Analog reference probe |
| Pump Control | 15 | PWM output |
| Battery ADC | 35 | `_VBAT` (divider on board) |
| DHT11 Data | 16 | Enabled in this profile |
| DHT11 Power | 17 | Enabled in this profile |
| MPU INT | 13 | Reserved for motion module |
| Buzzer | 23 | Enabled in this profile |
| LED | 5 | Built-in LED |
| Button | 0 | Boot button |

#### `lolin32` (LOLIN32 v1.0.0, zgodny z oryginalnym Flaura)

| Component | GPIO | Notes |
|-----------|------|-------|
| Soil Sensor Signal | 33 | ADC input |
| Soil Sensor VCC | 19 | Power control |
| Water Level (100%, 75%, 50%, 25%, 10%) | 13, 14, 27, 26, 25 | Zgodnie z oryginalnym `Flaura_Blynk.ino` |
| Water Level Ground Probe | 35 | ADC input |
| Pump Control | 23 | PWM output |
| Battery ADC | 32 | ADC input |
| Button | 0 | Boot button |
| DHT11 | — | Disabled (`255`) |
| MPU INT | — | Disabled (`255`) |
| Buzzer | — | Disabled (`255`) |
| LED status | — | Disabled (`255`) |

## 🔧 Assembly Instructions

### Step 1: Prepare the ESP32
1. **Flash the firmware** first to test basic functionality
2. **Test WiFi connection** using serial monitor
3. **Verify pin functionality** with simple blink test

### Step 2: Soil Moisture Sensor
1. **Connect sensor pins:**
   - `lolin_d32`: VCC → GPIO 4
   - `lolin32`: VCC → GPIO 19
   - GND → ESP32 GND
   - `lolin_d32`: Signal → GPIO 34
   - `lolin32`: Signal → GPIO 33

2. **Add pull-up resistor** (10kΩ) between Signal and VCC if needed
3. **Test reading** - should show values 0-4095

### Step 3: DHT11 Temperature/Humidity Sensor
> Only for `lolin_d32` profile.

1. **Connect sensor pins:**
   - VCC → GPIO 17
   - GND → ESP32 GND
   - Data → GPIO 16

2. **Add pull-up resistor** (4.7kΩ) between Data and VCC
3. **Test reading** - should show temperature and humidity

### Step 4: Water Level Sensor
1. **Connect level pins:**
   - `lolin_d32`: L1..L5 → GPIO 33, 25, 26, 27, 14 and probe GND → GPIO 32
   - `lolin32`: L1..L5 → GPIO 13, 14, 27, 26, 25 and probe GND → GPIO 35

2. **Add pull-up resistors** (10kΩ) on each level pin
3. **Test with water** - should detect levels 1-5

### Step 5: Water Pump
1. **Connect pump:**
   - `lolin_d32`: Positive → Relay/MOSFET controlled by GPIO 15
   - `lolin32`: Positive → Relay/MOSFET controlled by GPIO 23
   - Negative → 12V power supply ground

2. **Add protection:**
   - Flyback diode across pump terminals
   - MOSFET or relay for high current switching

3. **Test operation** - pump should run with PWM control

### Step 6: Audio/Visual Indicators
> Only for `lolin_d32` profile.

1. **Buzzer connection:**
   - Positive → GPIO 23
   - Negative → GND

2. **LED connection:**
   - Anode → GPIO 5 (through 220Ω resistor), or use built-in LED
   - Cathode → GND

3. **Test alerts** - should beep and blink on alarms

### Step 7: Power System
1. **For mains operation:**
   - Use 12V power supply for pump
   - Use 5V regulator for ESP32
   - Common ground for all components

2. **For battery operation:**
   - 18650 Li-ion battery (3.7V)
   - Voltage divider for battery monitoring
   - 3.3V LDO regulator for stable power

## ⚡ Power Considerations

### Power Consumption
- **ESP32 Active:** ~160mA @ 3.3V
- **ESP32 Deep Sleep:** ~10µA @ 3.3V
- **DHT11:** ~1mA @ 3.3V
- **Soil Sensor:** ~20mA @ 3.3V (when powered)
- **Water Pump:** ~500-1000mA @ 12V
- **Total (without pump):** ~200mA @ 3.3V

### Battery Life Estimation
With 3000mAh 18650 battery:
- **Continuous operation:** ~15 hours
- **With deep sleep (1 hour intervals):** ~30 days
- **Optimized sleep mode:** ~60+ days

### Power Optimization Tips
1. **Use GPIO power control** for sensors
2. **Enable deep sleep** when possible
3. **Minimize WiFi usage** in battery mode
4. **Use efficient voltage regulators**
5. **Turn off unused peripherals**

## 🛡️ Safety Considerations

### Electrical Safety
- **Double-check connections** before powering on
- **Use proper fuses** for pump circuit
- **Isolate high voltage** from low voltage circuits
- **Add reverse polarity protection**

### Water Safety
- **Waterproof all connections** near water
- **Use IP65+ rated enclosures**
- **Keep electronics elevated** above water level
- **Use marine-grade connectors** for outdoor use

### Component Protection
- **Add flyback diodes** for inductive loads
- **Use pull-up/pull-down resistors** as needed
- **Implement overcurrent protection**
- **Add ESD protection** for exposed pins

## 🔍 Troubleshooting

### Common Issues

**ESP32 won't boot:**
- Check power supply voltage (3.3V ±10%)
- Verify GPIO 0 is not pulled low during boot
- Ensure proper ground connections

**Sensor readings incorrect:**
- Verify sensor power and ground
- Check ADC reference voltage
- Calibrate sensors in known conditions
- Add filtering capacitors if readings are noisy

**Pump not working:**
- Check 12V power supply
- Verify MOSFET/relay operation
- Test pump directly with 12V
- Check PWM signal with oscilloscope

**WiFi connection fails:**
- Verify antenna connection
- Check 2.4GHz network availability
- Ensure adequate power supply
- Move closer to router for testing

## 📐 Mechanical Assembly

### Enclosure Design
- **Main electronics box:** IP65 rated, ventilated
- **Sensor mounting:** Waterproof cable glands
- **Pump housing:** Submersible or external with tubing
- **Display window:** Clear acrylic for status LED

### Mounting Options
- **Desktop version:** Compact enclosure with external sensors
- **Garden version:** Weather-resistant with solar panel option
- **Portable version:** Battery-powered with carrying handle

## 🔧 Tools Required

### Basic Tools
- Soldering iron and solder
- Wire strippers
- Multimeter
- Breadboard or PCB
- Heat shrink tubing
- Cable ties

### Advanced Tools
- Oscilloscope (for debugging)
- 3D printer (for custom enclosures)
- PCB design software
- Hot air station (for SMD work)

## 📚 Additional Resources

- [ESP32 Pinout Reference](https://randomnerdtutorials.com/esp32-pinout-reference-gpios/)
- [DHT11 Datasheet](https://www.mouser.com/datasheet/2/758/DHT11-Technical-Data-Sheet-Translated-Version-1143054.pdf)
- [Water Pump Selection Guide](https://www.adafruit.com/product/1150)
- [PCB Design Guidelines](https://www.altium.com/documentation/altium-designer/pcb-design-guidelines)

---

**Safety First!** Always double-check connections and use proper safety equipment when working with electronics and water. 🔌💧
