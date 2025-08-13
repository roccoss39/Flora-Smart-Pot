# API Reference

Complete reference for Flora Smart Pot software modules and functions.

## 📚 Table of Contents

- [Device Configuration](#device-configuration)
- [Sensor Modules](#sensor-modules)
- [Control Modules](#control-modules)
- [Communication Modules](#communication-modules)
- [Utility Modules](#utility-modules)

## 🔧 Device Configuration

### DeviceConfig.h

Central configuration management for all device settings.

#### Initialization Functions

```cpp
void configSetup();
```
**Description:** Loads configuration from flash memory, creates defaults if needed.
**Parameters:** None
**Returns:** void

```cpp
void clearPreferencesData(const char* namespaceToClear);
```
**Description:** Clears all saved data in specified Preferences namespace.
**Parameters:** 
- `namespaceToClear` - Namespace to clear
**Returns:** void

#### Mode Control

```cpp
bool configIsContinuousMode();
```
**Description:** Checks if device is in continuous mode (no deep sleep).
**Returns:** `true` if continuous mode, `false` if deep sleep mode

```cpp
void configSetContinuousMode(bool enabled);
```
**Description:** Sets and saves device operation mode.
**Parameters:**
- `enabled` - `true` for continuous, `false` for deep sleep
**Returns:** void

#### General Settings

```cpp
int configGetMeasurementHour();
int configGetMeasurementMinute();
```
**Description:** Gets daily measurement time.
**Returns:** Hour (0-23) or minute (0-59)

```cpp
uint32_t configGetBlynkSendIntervalSec();
```
**Description:** Gets Blynk sending interval.
**Returns:** Interval in seconds

```cpp
uint32_t configGetSleepSeconds();
```
**Description:** Gets sleep duration after cycle completion.
**Returns:** Sleep time in seconds

#### Pin Configuration

```cpp
uint8_t configGetSoilPin();
uint8_t configGetDhtPin();
uint8_t configGetPumpPin();
uint8_t configGetBuzzerPin();
uint8_t configGetLedPin();
uint8_t configGetButtonPin();
```
**Description:** Gets GPIO pin numbers for various components.
**Returns:** GPIO pin number (0-39) or 255 if not configured

## 🌱 Sensor Modules

### SoilSensor.h

Soil moisture monitoring with calibration support.

```cpp
void soilSensorSetup();
```
**Description:** Initializes soil sensor (configures VCC pin).
**Parameters:** None
**Returns:** void

```cpp
int soilSensorReadPercent();
```
**Description:** Reads soil moisture as percentage.
**Returns:** Moisture percentage (0-100) or -1 on error

### WaterLevelSensor.h

Multi-level water detection system.

```cpp
void waterLevelSensorSetup();
```
**Description:** Initializes water level sensor pins as INPUT_PULLUP.
**Parameters:** None
**Returns:** void

```cpp
int waterLevelSensorReadLevel();
```
**Description:** Reads current water level.
**Returns:** Highest detected level (1-5) or 0 if no water detected

### EnvironmentSensor.h

Temperature and humidity monitoring.

```cpp
void environmentSensorSetup();
```
**Description:** Initializes DHT11 environmental sensor.
**Parameters:** None
**Returns:** void

```cpp
bool environmentSensorRead(float &temperature, float &humidity);
```
**Description:** Reads temperature and humidity from DHT11.
**Parameters:**
- `temperature` - Reference to store temperature (°C)
- `humidity` - Reference to store humidity (%RH)
**Returns:** `true` if reading successful, `false` on error

### BatteryMonitor.h

Battery voltage monitoring with voltage divider support.

```cpp
void batteryMonitorSetup();
```
**Description:** Initializes battery monitoring module.
**Parameters:** None
**Returns:** void

```cpp
float batteryMonitorReadVoltage();
```
**Description:** Reads current battery voltage.
**Returns:** Battery voltage in Volts (V), accounts for voltage divider

```cpp
int batteryMonitorReadRawADC();
```
**Description:** Reads raw ADC value from battery monitor.
**Returns:** Raw ADC value (0-4095)

## 🎛️ Control Modules

### PumpControl.h

PWM-controlled water pump management.

```cpp
void pumpControlSetup();
```
**Description:** Initializes pump control with PWM capabilities.
**Parameters:** None
**Returns:** void

```cpp
void pumpControlActivateIfNeeded(int currentSoilMoisture, int currentWaterLevel);
```
**Description:** Automatically activates pump if conditions are met.
**Parameters:**
- `currentSoilMoisture` - Current soil moisture (%)
- `currentWaterLevel` - Current water level (0-5)
**Returns:** void

```cpp
void pumpControlManualTurnOn(uint32_t durationMillis);
```
**Description:** Manually turns on pump for specified duration.
**Parameters:**
- `durationMillis` - Duration in milliseconds
**Returns:** void

```cpp
void pumpControlManualTurnOff();
```
**Description:** Immediately turns off pump.
**Parameters:** None
**Returns:** void

```cpp
bool pumpControlIsRunning();
```
**Description:** Checks if pump is currently running.
**Returns:** `true` if pump is running, `false` otherwise

```cpp
void pumpControlUpdate();
```
**Description:** Updates pump state, call regularly in loop().
**Parameters:** None
**Returns:** void

### AlarmManager.h

Audio and visual alarm system with priority levels.

```cpp
void alarmManagerSetup();
```
**Description:** Initializes alarm module (configures buzzer pin).
**Parameters:** None
**Returns:** void

```cpp
bool alarmManagerUpdate(int waterLevel, float batteryVoltage, int soilMoisture);
```
**Description:** Updates alarm state based on sensor readings.
**Parameters:**
- `waterLevel` - Current water level (0-5)
- `batteryVoltage` - Current battery voltage (V)
- `soilMoisture` - Current soil moisture (%)
**Returns:** `true` if alarm state changed, `false` otherwise

```cpp
bool alarmManagerIsAlarmActive();
```
**Description:** Returns current alarm state.
**Returns:** `true` if any alarm condition is met, `false` otherwise

### LedManager.h

LED status indication with multiple states.

#### Enums

```cpp
enum LedState {
    LED_OFF,
    LED_ON,
    LED_BLINKING_SLOW,
    LED_BLINKING_FAST
};
```

#### Functions

```cpp
void ledManagerSetup(uint8_t pin, uint8_t ledOnState = HIGH);
```
**Description:** Initializes LED manager.
**Parameters:**
- `pin` - GPIO pin number for LED
- `ledOnState` - State that turns LED on (HIGH or LOW)
**Returns:** void

```cpp
void ledManagerSetState(LedState state);
```
**Description:** Sets desired LED state.
**Parameters:**
- `state` - New LED state from LedState enum
**Returns:** void

```cpp
void ledManagerUpdate();
```
**Description:** Updates LED state, call regularly in loop().
**Parameters:** None
**Returns:** void

```cpp
void ledManagerTurnOn();
void ledManagerTurnOff();
```
**Description:** Convenience functions for LED control.
**Parameters:** None
**Returns:** void

```cpp
LedState ledManagerGetCurrentState();
```
**Description:** Gets current LED state.
**Returns:** Current LedState

### ButtonManager.h

Button input handling with debouncing.

```cpp
void buttonSetup();
```
**Description:** Initializes button module with pull-up configuration.
**Parameters:** None
**Returns:** void

```cpp
bool buttonWasPressed();
```
**Description:** Checks for button press in continuous mode.
**Returns:** `true` if button was just pressed (falling edge), `false` otherwise

## 📡 Communication Modules

### BlynkManager.h

Cloud connectivity and data transmission.

```cpp
void blynkConfigure(const char* authToken, const char* templateId, const char* deviceName);
```
**Description:** Configures Blynk authentication data.
**Parameters:**
- `authToken` - Blynk device auth token
- `templateId` - Blynk template ID
- `deviceName` - Blynk device name
**Returns:** void

```cpp
bool blynkConnect(unsigned long timeoutMs = 5000);
```
**Description:** Connects to Blynk server.
**Parameters:**
- `timeoutMs` - Connection timeout in milliseconds
**Returns:** `true` if connection successful, `false` otherwise

```cpp
void blynkRun();
```
**Description:** Main Blynk handler, call regularly in loop().
**Parameters:** None
**Returns:** void

```cpp
bool blynkIsConnected();
```
**Description:** Checks Blynk connection status.
**Returns:** `true` if connected, `false` otherwise

```cpp
void blynkDisconnect();
```
**Description:** Disconnects from Blynk server.
**Parameters:** None
**Returns:** void

```cpp
void blynkSendSensorData(int soil, int waterLvl, float batteryV, float temp, float humid, bool pumpStatus, bool isAlarmCurrentlyActive);
```
**Description:** Sends sensor data to Blynk virtual pins.
**Parameters:**
- `soil` - Soil moisture percentage
- `waterLvl` - Water level (0-5)
- `batteryV` - Battery voltage
- `temp` - Temperature
- `humid` - Humidity
- `pumpStatus` - Pump running status
- `isAlarmCurrentlyActive` - Alarm active status
**Returns:** void

## ⚡ Utility Modules

### PowerManager.h

Power management and deep sleep control.

```cpp
void powerManagerGoToDeepSleep();
```
**Description:** Enters deep sleep mode for configured duration.
**Parameters:** None
**Returns:** void (function doesn't return, device resets on wake)

```cpp
bool powerManagerSyncTime();
```
**Description:** Synchronizes time with NTP server.
**Returns:** `true` if synchronization successful, `false` otherwise

```cpp
uint64_t powerManagerGetTimeToNextMeasurement();
```
**Description:** Calculates time until next scheduled measurement.
**Returns:** Time in microseconds until next measurement

```cpp
void powerManagerPrintCurrentTime();
```
**Description:** Prints current system time to serial.
**Parameters:** None
**Returns:** void

```cpp
void powerManagerConfigureButtonWakeup();
```
**Description:** Configures button for EXT0 wakeup from deep sleep.
**Parameters:** None
**Returns:** void

## 📊 Data Structures

### SensorData (main.cpp)

```cpp
struct SensorData {
    int soilMoisture = -1;
    int waterLevel = -1;
    float batteryVoltage = -1.0f;
    float temperature = NAN;
    float humidity = NAN;
    bool dhtOk = false;
    
    bool isValid() const;
};
```

**Description:** Structure for storing all sensor readings.

**Members:**
- `soilMoisture` - Soil moisture percentage (-1 if invalid)
- `waterLevel` - Water level 0-5 (-1 if invalid)
- `batteryVoltage` - Battery voltage in volts (-1 if invalid)
- `temperature` - Temperature in Celsius (NAN if invalid)
- `humidity` - Humidity percentage (NAN if invalid)
- `dhtOk` - DHT sensor reading status

**Methods:**
- `isValid()` - Returns true if basic sensor data is available

## 🔧 Configuration Constants

### Default Pin Assignments

```cpp
// Soil Sensor
#define DEFAULT_SOIL_PIN         36  // A0
#define DEFAULT_SOIL_VCC_PIN     4

// DHT11 Sensor
#define DEFAULT_DHT_PIN          2
#define DEFAULT_DHT_POWER_PIN    5

// Water Level Sensor
#define DEFAULT_WATER_LEVEL_1    12
#define DEFAULT_WATER_LEVEL_2    13
#define DEFAULT_WATER_LEVEL_3    14
#define DEFAULT_WATER_LEVEL_4    15
#define DEFAULT_WATER_LEVEL_5    16
#define DEFAULT_WATER_GROUND     17

// Control Outputs
#define DEFAULT_PUMP_PIN         18
#define DEFAULT_BUZZER_PIN       19
#define DEFAULT_LED_PIN          21

// Input
#define DEFAULT_BUTTON_PIN       0

// Battery Monitor
#define DEFAULT_BATTERY_ADC_PIN  39  // A3
```

### Default Thresholds

```cpp
#define DEFAULT_SOIL_THRESHOLD_PERCENT     30
#define DEFAULT_LOW_SOIL_PERCENT          20
#define DEFAULT_LOW_BATTERY_MILLIVOLTS    3200
#define DEFAULT_PUMP_RUN_MILLIS           5000
#define DEFAULT_PUMP_DUTY_CYCLE           200
#define DEFAULT_BLYNK_SEND_INTERVAL_SEC   60
#define DEFAULT_SLEEP_SECONDS             3600
```

## 🚨 Error Codes

### Return Values

- **-1**: General error or invalid reading
- **0**: Success or no detection (context dependent)
- **255**: Invalid pin configuration
- **NAN**: Invalid floating-point reading

### Common Error Conditions

- **Sensor not responding**: Check wiring and power
- **ADC reading out of range**: Verify voltage levels
- **WiFi connection failed**: Check credentials and signal
- **Blynk connection failed**: Verify auth token and internet
- **Pump not responding**: Check power supply and connections

---

**Note:** All functions are non-blocking unless specifically noted. Call update functions regularly in the main loop for proper operation.