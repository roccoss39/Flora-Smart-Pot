# Troubleshooting Guide

Common issues and solutions for Flora Smart Pot.

## 🚨 Quick Diagnostics

### System Status Check
1. **Power LED** - Should be solid or blinking
2. **Serial Output** - Check for error messages at 115200 baud
3. **WiFi Connection** - Verify network connectivity
4. **Sensor Readings** - Check if values are reasonable

### Emergency Reset
- Hold **BOOT button** for 10 seconds to factory reset
- Or add `clearPreferencesData("flaura_cfg_1");` in setup() temporarily

## 🔌 Hardware Issues

### ESP32 Won't Start

**Symptoms:**
- No serial output
- No LED activity
- Device appears dead

**Solutions:**
1. **Check Power Supply**
   ```
   - Verify 3.3V ±10% on ESP32 VCC pin
   - Ensure adequate current capacity (>500mA)
   - Check for voltage drops under load
   ```

2. **Check Boot Configuration**
   ```
   - GPIO 0 should be HIGH during boot
   - GPIO 2 should be floating or HIGH
   - Remove any loads on boot pins
   ```

3. **Verify Connections**
   ```
   - Ensure solid ground connections
   - Check for short circuits
   - Verify USB cable integrity
   ```

### Sensor Reading Issues

#### Soil Moisture Sensor

**Problem: Always reads 0% or 100%**
```
Solutions:
1. Check sensor power (GPIO 4 should go HIGH before reading)
2. Verify ADC pin connection (GPIO 36)
3. Calibrate sensor in dry and wet conditions
4. Replace sensor if physically damaged
```

**Problem: Erratic readings**
```
Solutions:
1. Add 100nF capacitor across sensor power pins
2. Increase delay between power-on and reading
3. Check for loose connections
4. Shield sensor cable from interference
```

#### DHT11 Temperature/Humidity

**Problem: Always returns NaN**
```
Solutions:
1. Check data pin connection (GPIO 2)
2. Verify 4.7kΩ pull-up resistor on data line
3. Ensure sensor power is stable 3.3V
4. Try different DHT11 sensor
5. Check timing - allow 2 seconds between readings
```

**Problem: Readings seem incorrect**
```
Solutions:
1. Compare with known good thermometer
2. Allow sensor to stabilize (5+ minutes)
3. Check for heat sources near sensor
4. Verify sensor is not in direct sunlight
```

#### Water Level Sensor

**Problem: No levels detected**
```
Solutions:
1. Check all level pin connections (GPIO 12-16)
2. Verify ground pin connection (GPIO 17)
3. Ensure pull-up resistors (10kΩ) on level pins
4. Test with multimeter - should read 3.3V when dry
5. Clean sensor contacts
```

**Problem: False level readings**
```
Solutions:
1. Adjust water detection threshold in config
2. Check for corrosion on sensor contacts
3. Ensure clean water (minerals affect conductivity)
4. Add debouncing delay in software
```

#### Battery Monitor

**Problem: Incorrect voltage readings**
```
Solutions:
1. Check voltage divider resistors (if used)
2. Verify ADC reference voltage (3.3V)
3. Calibrate with known voltage source
4. Check battery connection integrity
```

### Actuator Problems

#### Water Pump

**Problem: Pump doesn't run**
```
Solutions:
1. Check 12V power supply to pump
2. Verify MOSFET/relay operation (GPIO 18)
3. Test pump directly with 12V
4. Check for pump blockage or damage
5. Verify PWM signal with oscilloscope
```

**Problem: Pump runs but no water flow**
```
Solutions:
1. Check for air locks in tubing
2. Verify pump is submerged
3. Clean pump intake filter
4. Check tubing for kinks or blockages
5. Prime pump manually
```

#### Buzzer

**Problem: No sound from buzzer**
```
Solutions:
1. Check buzzer connection (GPIO 19)
2. Verify buzzer type (active vs passive)
3. Test with direct 3.3V connection
4. Check if alarm sound is enabled in config
5. Replace buzzer if damaged
```

#### LED

**Problem: LED not working**
```
Solutions:
1. Check LED polarity (anode to GPIO 21 via resistor)
2. Verify current limiting resistor (220Ω)
3. Test LED with direct connection
4. Check GPIO 21 output with multimeter
5. Replace LED if burned out
```

## 📡 Connectivity Issues

### WiFi Problems

**Problem: Can't connect to WiFi**
```
Diagnostics:
1. Check serial output for error messages
2. Verify network name and password
3. Ensure 2.4GHz network (ESP32 doesn't support 5GHz)
4. Check signal strength at device location

Solutions:
1. Reset WiFi settings: wm.resetSettings()
2. Move closer to router for testing
3. Try different WiFi network
4. Check for special characters in password
5. Restart router if needed
```

**Problem: WiFi keeps disconnecting**
```
Solutions:
1. Check power supply stability
2. Improve antenna connection
3. Reduce distance to router
4. Check for interference sources
5. Update router firmware
6. Use WiFi repeater if needed
```

### Blynk Connection Issues

**Problem: Can't connect to Blynk**
```
Diagnostics:
1. Verify auth token in secrets.h
2. Check internet connectivity
3. Test with Blynk mobile app
4. Monitor serial output for error codes

Solutions:
1. Regenerate auth token in Blynk console
2. Check Blynk server status
3. Verify template ID and device name
4. Try different internet connection
5. Check firewall settings
```

**Problem: Data not updating in Blynk**
```
Solutions:
1. Check virtual pin assignments
2. Verify Blynk project configuration
3. Monitor send interval settings
4. Check for rate limiting
5. Verify data types match Blynk widgets
```

## ⚡ Power and Performance

### Battery Issues

**Problem: Short battery life**
```
Solutions:
1. Enable deep sleep mode
2. Reduce measurement frequency
3. Check for current leaks
4. Optimize sensor power control
5. Use higher capacity battery
6. Check charging system
```

**Problem: Device resets randomly**
```
Solutions:
1. Check power supply capacity
2. Add bulk capacitors (1000µF)
3. Verify stable voltage under load
4. Check for loose connections
5. Monitor for brownout conditions
```

### Memory Issues

**Problem: Device crashes or behaves erratically**
```
Solutions:
1. Check for memory leaks in code
2. Reduce string usage in Serial.print
3. Use F() macro for string literals
4. Monitor heap usage
5. Restart device periodically
```

## 🔧 Software Issues

### Compilation Errors

**Problem: Build fails**
```
Solutions:
1. Update PlatformIO core and platform
2. Clean build environment: pio run -t clean
3. Check library dependencies
4. Verify secrets.h exists and is configured
5. Check for syntax errors in modified code
```

### Runtime Errors

**Problem: Watchdog timer resets**
```
Solutions:
1. Add yield() or delay() in long loops
2. Reduce processing time in loop()
3. Check for infinite loops
4. Monitor task execution time
5. Use RTOS tasks for heavy processing
```

**Problem: Stack overflow**
```
Solutions:
1. Reduce local variable usage
2. Avoid deep recursion
3. Use dynamic allocation carefully
4. Monitor stack usage
5. Increase stack size if needed
```

## 📊 Diagnostic Tools

### Serial Monitor Commands
```cpp
// Add these to your code for debugging
Serial.println("=== System Status ===");
Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
Serial.printf("WiFi status: %d\n", WiFi.status());
Serial.printf("Blynk connected: %s\n", Blynk.connected() ? "YES" : "NO");
Serial.printf("Uptime: %lu ms\n", millis());
```

### Hardware Testing
```cpp
// Test individual components
void testHardware() {
    // Test LED
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    
    // Test buzzer
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    
    // Test sensors
    Serial.printf("Soil ADC: %d\n", analogRead(SOIL_PIN));
    Serial.printf("Battery ADC: %d\n", analogRead(BATTERY_PIN));
}
```

### Network Diagnostics
```cpp
void networkDiagnostics() {
    Serial.printf("WiFi SSID: %s\n", WiFi.SSID().c_str());
    Serial.printf("WiFi RSSI: %d dBm\n", WiFi.RSSI());
    Serial.printf("Local IP: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
    Serial.printf("DNS: %s\n", WiFi.dnsIP().toString().c_str());
}
```

## 🆘 Getting Help

### Information to Provide
When seeking help, include:
1. **Hardware setup** - Board type, sensor models, wiring
2. **Software version** - Firmware version, PlatformIO version
3. **Error messages** - Complete serial output
4. **Steps to reproduce** - What you were doing when issue occurred
5. **Environment** - Power supply, WiFi setup, physical conditions

### Useful Commands
```bash
# Get system information
pio system info

# Check library versions
pio lib list

# Clean and rebuild
pio run -t clean
pio run

# Monitor with timestamps
pio device monitor --filter time
```

### Community Resources
- GitHub Issues - Report bugs and feature requests
- PlatformIO Community - General ESP32 help
- Blynk Community - Cloud connectivity issues
- Arduino Forums - Hardware and coding help

## 🔄 Recovery Procedures

### Factory Reset
1. Add to setup(): `clearPreferencesData("flaura_cfg_1");`
2. Upload and run once
3. Remove the line and upload again
4. Reconfigure via WiFi portal

### Firmware Recovery
1. Hold BOOT button while connecting USB
2. Upload firmware via PlatformIO
3. Reset device after upload
4. Check serial output for proper boot

### Configuration Backup
```cpp
// Save current config before major changes
void backupConfig() {
    Serial.println("=== Current Configuration ===");
    Serial.printf("Continuous mode: %s\n", configIsContinuousMode() ? "YES" : "NO");
    Serial.printf("Soil threshold: %d%%\n", configGetSoilThresholdPercent());
    Serial.printf("Pump duration: %d ms\n", configGetPumpRunMillis());
    // Add other important settings
}
```

---

**Remember:** Most issues are related to power supply, connections, or configuration. Start with the basics! 🔧