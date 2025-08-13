# Hardware Setup Guide

Complete guide for assembling your Flora Smart Pot hardware.

## 📦 Bill of Materials (BOM)

### Essential Components

| Component | Quantity | Description | Estimated Cost |
|-----------|----------|-------------|----------------|
| ESP32 DevKit | 1 | Main microcontroller | $8-15 |
| Soil Moisture Sensor | 1 | Capacitive type recommended | $3-5 |
| DHT11/DHT22 | 1 | Temperature/humidity sensor | $2-4 |
| Water Level Sensor | 1 | 5-level detection | $5-8 |
| Water Pump | 1 | 12V DC submersible pump | $8-12 |
| Buzzer | 1 | 5V active buzzer | $1-2 |
| LED | 1 | Status indicator (any color) | $0.50 |
| Push Button | 1 | Momentary switch | $0.50 |
| Resistors | Various | Pull-up/down resistors | $1-2 |
| Jumper Wires | 20+ | Male-to-male, male-to-female | $2-3 |
| Breadboard/PCB | 1 | For prototyping | $2-5 |
| Power Supply | 1 | 12V for pump, 5V for ESP32 | $5-10 |

### Optional Components

| Component | Quantity | Description | Purpose |
|-----------|----------|-------------|---------|
| Li-ion Battery | 1 | 18650 3.7V | Portable operation |
| Battery Holder | 1 | 18650 holder with wires | Battery mounting |
| Voltage Regulator | 1 | 3.3V LDO regulator | Battery operation |
| Enclosure | 1 | Waterproof case | Protection |
| LCD Display | 1 | 16x2 I2C LCD | Local display |

## 🔌 Wiring Diagram

### ESP32 Pin Connections

```
ESP32 GPIO    Component           Wire Color    Notes
-----------   -----------------   -----------   ----------------
GPIO 2        DHT11 Data          Yellow        Pull-up required
GPIO 4        Soil Sensor VCC     Red           Power control
GPIO 5        DHT11 VCC           Red           Optional power control
A0 (GPIO 36)  Soil Sensor Signal  White         ADC input
A3 (GPIO 39)  Battery Monitor     Orange        Voltage divider
GPIO 12       Water Level 1       Blue          Pull-up required
GPIO 13       Water Level 2       Green         Pull-up required
GPIO 14       Water Level 3       Yellow        Pull-up required
GPIO 15       Water Level 4       Orange        Pull-up required
GPIO 16       Water Level 5       Purple        Pull-up required
GPIO 17       Water Ground        Black         Common ground
GPIO 18       Pump Control        Red           PWM output
GPIO 19       Buzzer              Brown         Digital output
GPIO 21       LED / I2C SDA       Blue/White    Status indicator
GPIO 22       I2C SCL             Yellow/White  For future expansion
GPIO 0        Button              Gray          Boot button (built-in)
GND           Common Ground       Black         All grounds
3.3V          Sensor Power        Red           Sensor VCC
5V            Pump/Logic Power    Red           External power
```

## 🔧 Assembly Instructions

### Step 1: Prepare the ESP32
1. **Flash the firmware** first to test basic functionality
2. **Test WiFi connection** using serial monitor
3. **Verify pin functionality** with simple blink test

### Step 2: Soil Moisture Sensor
1. **Connect sensor pins:**
   - VCC → GPIO 4 (controllable power)
   - GND → ESP32 GND
   - Signal → A0 (GPIO 36)

2. **Add pull-up resistor** (10kΩ) between Signal and VCC if needed
3. **Test reading** - should show values 0-4095

### Step 3: DHT11 Temperature/Humidity Sensor
1. **Connect sensor pins:**
   - VCC → GPIO 5 (or 3.3V directly)
   - GND → ESP32 GND
   - Data → GPIO 2

2. **Add pull-up resistor** (4.7kΩ) between Data and VCC
3. **Test reading** - should show temperature and humidity

### Step 4: Water Level Sensor
1. **Connect level pins:**
   - Level 1 → GPIO 12
   - Level 2 → GPIO 13
   - Level 3 → GPIO 14
   - Level 4 → GPIO 15
   - Level 5 → GPIO 16
   - Ground → GPIO 17

2. **Add pull-up resistors** (10kΩ) on each level pin
3. **Test with water** - should detect levels 1-5

### Step 5: Water Pump
1. **Connect pump:**
   - Positive → Relay/MOSFET controlled by GPIO 18
   - Negative → 12V power supply ground

2. **Add protection:**
   - Flyback diode across pump terminals
   - MOSFET or relay for high current switching

3. **Test operation** - pump should run with PWM control

### Step 6: Audio/Visual Indicators
1. **Buzzer connection:**
   - Positive → GPIO 19
   - Negative → GND

2. **LED connection:**
   - Anode → GPIO 21 (through 220Ω resistor)
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