#ifndef DEVICECONFIG_H
#define DEVICECONFIG_H

#include <stdint.h>

// Funkcje do zarządzania konfiguracją
void configSetup();
bool configIsContinuousMode();
void clearPreferencesData(const char* namespaceToClear);
// Gettery dla czujnika wilgotności gleby
uint8_t configGetSoilPin();
int configGetSoilDryADC();
int configGetSoilWetADC();
int configGetSoilVccPin();

// Gettery dla czujnika poziomu wody
uint8_t configGetWaterLevelPin(int level); // Dla poziomów 1-5

// Nowe gettery dla sondy poziomu wody
uint8_t  configGetWaterLevelGroundPin();   // wspólna sonda (wejście analogowe)
uint16_t configGetWaterLevelThreshold();    // próg ADC do detekcji wody

// Settery dla sondy poziomu wody
void configSetWaterLevelGroundPin(uint8_t pin);
void configSetWaterLevelThreshold(uint16_t threshold);

// Gettery dla pompki
uint8_t configGetPumpPin();
uint32_t configGetPumpRunMillis();
int configGetSoilThresholdPercent();

// Getter dla pinu ADC baterii
uint8_t configGetBatteryAdcPin();

// Getter dla pinu DHT11
uint8_t configGetDhtPin();
uint8_t configGetDhtPowerPin();
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
void configSetContinuousMode(bool enabled);

uint8_t configGetBuzzerPin();
int configGetLowBatteryMilliVolts(); // Getter dla progu baterii (mV)
int configGetLowSoilPercent();  

uint8_t configGetButtonPin();

bool configIsAlarmSoundEnabled();
void configSetAlarmSoundEnabled(bool enabled);
void configSetLowBatteryMilliVolts(int mv);
void configSetLowSoilPercent(int percent); // Próg dla alarmu wilgotności
/**
 * @brief Ustawia i zapisuje wartość ADC dla całkowicie suchej gleby.
 * @param value Odczyt ADC (0-4095).
 */
void configSetSoilDryADC(int value);

/**
 * @brief Ustawia i zapisuje wartość ADC dla całkowicie mokrej (nasyconej) gleby.
 * @param value Odczyt ADC (0-4095).
 */
void configSetSoilWetADC(int value);
// --- NOWOŚĆ: Getter dla mocy pompy ---
uint8_t configGetPumpDutyCycle(); // Zwraca wartość 0-255

// ... (istniejące deklaracje funkcji configSet...) ...

// --- NOWOŚĆ: Setter dla mocy pompy ---
void configSetPumpDutyCycle(uint8_t duty); // Ustawia wartość 0-255

#endif // DEVICECONFIG_H
