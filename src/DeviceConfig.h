#ifndef DEVICECONFIG_H
#define DEVICECONFIG_H

#include <stdint.h>

// Funkcje do zarządzania konfiguracją
void configSetup();
bool configIsContinuousMode();

// Gettery dla czujnika wilgotności gleby
uint8_t configGetSoilPin();
int configGetSoilDryADC();
int configGetSoilWetADC();
int configGetSoilVccPin();

// Gettery dla czujnika poziomu wody
uint8_t configGetWaterLevelPin(int level); // Dla poziomów 1-5

// Gettery dla pompki
uint8_t configGetPumpPin();
uint32_t configGetPumpRunMillis();
int configGetSoilThresholdPercent();

// Getter dla pinu ADC baterii
uint8_t configGetBatteryAdcPin();

// Getter dla pinu DHT11
uint8_t configGetDhtPin();

// Getter dla pinu INT MPU6500
uint8_t configGetMpuIntPin();

// Getter dla czasu uśpienia
uint32_t configGetSleepSeconds();

uint32_t configGetBlynkSendIntervalSec();

void configSetPumpRunMillis(uint32_t durationMs);

void configSetSoilThresholdPercent(int threshold);

/**
 * @brief Pobiera ustawioną godzinę codziennego pomiaru
 * @return Godzina jako liczba 0-23
 */
int configGetMeasurementHour();

/**
 * @brief Pobiera ustawioną minutę codziennego pomiaru
 * @return Minuta jako liczba 0-59
 */
int configGetMeasurementMinute();

/**
 * @brief Ustawia czas codziennego pomiaru
 * @param hour Godzina (0-23)
 * @param minute Minuta (0-59)
 * @return true jeśli ustawiono poprawnie
 */
bool configSetMeasurementTime(int hour, int minute);

/**
 * @brief Ustawia i zapisuje tryb pracy urządzenia.
 * @param enabled true dla trybu ciągłego, false dla trybu Deep Sleep.
 */
void configSetContinuousMode(bool enabled); // <-- Dodaj tę linię

uint8_t configGetBuzzerPin();
int configGetLowBatteryMilliVolts(); // <-- DODAJ: Getter dla progu baterii (mV)
int configGetLowSoilPercent();  

#endif // DEVICECONFIG_H