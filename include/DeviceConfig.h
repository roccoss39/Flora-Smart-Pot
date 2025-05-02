#ifndef DEVICECONFIG_H
#define DEVICECONFIG_H

#include <stdint.h>

// -----------------------------------------------------------------------------
// Inicjalizacja i czyszczenie konfiguracji
// -----------------------------------------------------------------------------
/**
 * @brief Wczytuje (i tworzy domyślną, jeśli trzeba) wszystkie ustawienia z flash.
 */
void configSetup();

/**
 * @brief Czy urządzenie pracuje w trybie ciągłym (bez deep sleep)?
 */
bool configIsContinuousMode();

/**
 * @brief Ustawia i zapisuje tryb pracy urządzenia.
 * @param enabled true = ciągły, false = Deep Sleep 
 */
void configSetContinuousMode(bool enabled);

/**
 * @brief Czy dźwięk alarmu jest włączony?
 */
bool configIsAlarmSoundEnabled();

/**
 * @brief Czyści wszystkie zapisane dane w danej przestrzeni nazw Preferences.
 */
void clearPreferencesData(const char* namespaceToClear);


// -----------------------------------------------------------------------------
// Gettery – ogólne
// -----------------------------------------------------------------------------
/** Czas codziennego pomiaru – godzina (0–23) */
int  configGetMeasurementHour();
/** Czas codziennego pomiaru – minuta (0–59) */
int  configGetMeasurementMinute();
/** Interwał wysyłania do Blynka w sekundach */
uint32_t configGetBlynkSendIntervalSec();
/** Czas uśpienia po zakończeniu cyklu (w sekundach) */
uint32_t configGetSleepSeconds();
uint8_t configGetLedPin();

// -----------------------------------------------------------------------------
// Gettery – czujnik wilgotności gleby
// -----------------------------------------------------------------------------
/** Numer pinu ADC czujnika gleby */
uint8_t configGetSoilPin();
/** Kalibracja „sucho” (ADC) */
int     configGetSoilDryADC();
/** Kalibracja „mokro” (ADC) */
int     configGetSoilWetADC();
/** Pin zasilania czujnika gleby (VCC) */
int     configGetSoilVccPin();
/** Próg wilgotności gleby (%) do alarmu/pompy */
int     configGetSoilThresholdPercent();


// -----------------------------------------------------------------------------
// Gettery – czujnik poziomu wody
// -----------------------------------------------------------------------------
/** Numer pinu cyfrowego dla poziomu wody 1–5 */
uint8_t  configGetWaterLevelPin(int level);
/** Numer pinu wspólnej sondy (analog) */
uint8_t  configGetWaterLevelGroundPin();
/** Próg ADC detekcji wody (0–4095) */
uint16_t configGetWaterLevelThreshold();


// -----------------------------------------------------------------------------
// Gettery – pompa
// -----------------------------------------------------------------------------
/** Numer pinu sterującego pompą */
uint8_t  configGetPumpPin();
/** Czas pracy pompy w milisekundach */
uint32_t configGetPumpRunMillis();
/** Maksymalna moc pompy (duty cycle 0–255) */
uint8_t  configGetPumpDutyCycle();


// -----------------------------------------------------------------------------
// Gettery – zasilanie i czujniki pomocnicze
// -----------------------------------------------------------------------------
/** Numer pinu ADC do pomiaru baterii */
uint8_t  configGetBatteryAdcPin();
/** Numer pinu DHT11 (temperatura/ wilgotność) */
uint8_t  configGetDhtPin();
/** Numer pinu zasilania DHT11 (opcjonalne) */
uint8_t  configGetDhtPowerPin();
/** Numer pinu INT od MPU6500 */
uint8_t  configGetMpuIntPin();
/** Numer pinu przycisku (wybudzanie EXT0) */
uint8_t  configGetButtonPin();
/** Numer pinu buzzera */
uint8_t  configGetBuzzerPin();
/** Próg niskiego napięcia baterii (mV) */
int      configGetLowBatteryMilliVolts();
/** Próg niskiej wilgotności gleby (%) dla alarmu */
int      configGetLowSoilPercent();


// -----------------------------------------------------------------------------
// Settery – czujnik wilgotności gleby
// -----------------------------------------------------------------------------
/** Ustawia zapisaną kalibrację ADC „sucho” */
void configSetSoilDryADC(int value);
/** Ustawia zapisaną kalibrację ADC „mokro” */
void configSetSoilWetADC(int value);
/** Ustawia próg wilgotności gleby (%) */
void configSetSoilThresholdPercent(int threshold);


// -----------------------------------------------------------------------------
// Settery – czujnik poziomu wody
// -----------------------------------------------------------------------------
/** Ustawia pin wspólnej sondy wody */
void configSetWaterLevelGroundPin(uint8_t pin);
/** Ustawia próg ADC detekcji wody */
void configSetWaterLevelThreshold(uint16_t threshold);


// -----------------------------------------------------------------------------
// Settery – pompa
// -----------------------------------------------------------------------------
/** Ustawia czas pracy pompy (ms) */
void configSetPumpRunMillis(uint32_t durationMs);
/** Ustawia moc (duty cycle 0–255) pompy */
void configSetPumpDutyCycle(uint8_t duty);


// -----------------------------------------------------------------------------
// Settery – alarm i Blynk
// -----------------------------------------------------------------------------
/** Ustawia, czy dźwięk alarmu jest włączony */
void configSetAlarmSoundEnabled(bool enabled);
/** Ustawia próg niskiego napięcia baterii (mV) */
void configSetLowBatteryMilliVolts(int mv);
/** Ustawia próg niskiej wilgotności gleby (%) */
void configSetLowSoilPercent(int percent);

/**
 * @brief Ustawia czas codziennych pomiarów.
 * @return true jeśli poprawnie (0–23 dla godziny, 0–59 dla minuty)
 */
bool configSetMeasurementTime(int hour, int minute);

#endif // DEVICECONFIG_H
