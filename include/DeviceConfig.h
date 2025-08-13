#ifndef DEVICECONFIG_H
#define DEVICECONFIG_H

#include <stdint.h>

// -----------------------------------------------------------------------------
// Configuration initialization and cleanup
// -----------------------------------------------------------------------------
/**
 * @brief Loads (and creates default if needed) all settings from flash.
 */
void configSetup();

/**
 * @brief Is device working in continuous mode (without deep sleep)?
 */
bool configIsContinuousMode();

/**
 * @brief Sets and saves device operation mode.
 * @param enabled true = continuous, false = Deep Sleep 
 */
void configSetContinuousMode(bool enabled);

/**
 * @brief Is alarm sound enabled?
 */
bool configIsAlarmSoundEnabled();

/**
 * @brief Clears all saved data in given Preferences namespace.
 */
void clearPreferencesData(const char* namespaceToClear);


// -----------------------------------------------------------------------------
// Getters – general
// -----------------------------------------------------------------------------
/** Daily measurement time – hour (0–23) */
int  configGetMeasurementHour();
/** Daily measurement time – minute (0–59) */
int  configGetMeasurementMinute();
/** Blynk sending interval in seconds */
uint32_t configGetBlynkSendIntervalSec();
/** Sleep time after cycle completion (in seconds) */
uint32_t configGetSleepSeconds();
uint8_t configGetLedPin();

// -----------------------------------------------------------------------------
// Getters – soil moisture sensor
// -----------------------------------------------------------------------------
/** Soil sensor ADC pin number */
uint8_t configGetSoilPin();
/** "Dry" calibration (ADC) */
int     configGetSoilDryADC();
/** "Wet" calibration (ADC) */
int     configGetSoilWetADC();
/** Soil sensor power pin (VCC) */
int     configGetSoilVccPin();
/** Soil moisture threshold (%) for alarm/pump */
int     configGetSoilThresholdPercent();


// -----------------------------------------------------------------------------
// Getters – water level sensor
// -----------------------------------------------------------------------------
/** Digital pin number for water level 1–5 */
uint8_t  configGetWaterLevelPin(int level);
/** Common probe pin number (analog) */
uint8_t  configGetWaterLevelGroundPin();
/** ADC water detection threshold (0–4095) */
uint16_t configGetWaterLevelThreshold();


// -----------------------------------------------------------------------------
// Getters – pump
// -----------------------------------------------------------------------------
/** Pump control pin number */
uint8_t  configGetPumpPin();
/** Pump run time in milliseconds */
uint32_t configGetPumpRunMillis();
/** Maximum pump power (duty cycle 0–255) */
uint8_t  configGetPumpDutyCycle();


// -----------------------------------------------------------------------------
// Getters – power and auxiliary sensors
// -----------------------------------------------------------------------------
/** Battery measurement ADC pin number */
uint8_t  configGetBatteryAdcPin();
/** DHT11 pin number (temperature/humidity) */
uint8_t  configGetDhtPin();
/** DHT11 power pin number (optional) */
uint8_t  configGetDhtPowerPin();
/** MPU6500 INT pin number */
uint8_t  configGetMpuIntPin();
/** Button pin number (EXT0 wakeup) */
uint8_t  configGetButtonPin();
/** Buzzer pin number */
uint8_t  configGetBuzzerPin();
/** Low battery voltage threshold (mV) */
int      configGetLowBatteryMilliVolts();
/** Low soil moisture threshold (%) for alarm */
int      configGetLowSoilPercent();


// -----------------------------------------------------------------------------
// Setters – soil moisture sensor
// -----------------------------------------------------------------------------
/** Sets saved "dry" ADC calibration */
void configSetSoilDryADC(int value);
/** Sets saved "wet" ADC calibration */
void configSetSoilWetADC(int value);
/** Sets soil moisture threshold (%) */
void configSetSoilThresholdPercent(int threshold);


// -----------------------------------------------------------------------------
// Setters – water level sensor
// -----------------------------------------------------------------------------
/** Sets common water probe pin */
void configSetWaterLevelGroundPin(uint8_t pin);
/** Sets ADC water detection threshold */
void configSetWaterLevelThreshold(uint16_t threshold);


// -----------------------------------------------------------------------------
// Setters – pump
// -----------------------------------------------------------------------------
/** Sets pump run time (ms) */
void configSetPumpRunMillis(uint32_t durationMs);
/** Sets pump power (duty cycle 0–255) */
void configSetPumpDutyCycle(uint8_t duty);


// -----------------------------------------------------------------------------
// Setters – alarm and Blynk
// -----------------------------------------------------------------------------
/** Sets whether alarm sound is enabled */
void configSetAlarmSoundEnabled(bool enabled);
/** Sets low battery voltage threshold (mV) */
void configSetLowBatteryMilliVolts(int mv);
/** Sets low soil moisture threshold (%) */
void configSetLowSoilPercent(int percent);

/**
 * @brief Sets daily measurement time.
 * @return true if correct (0–23 for hour, 0–59 for minute)
 */
bool configSetMeasurementTime(int hour, int minute);

#endif // DEVICECONFIG_H
