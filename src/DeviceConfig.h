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

#endif // DEVICECONFIG_H