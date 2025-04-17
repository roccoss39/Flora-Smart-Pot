#ifndef DEVICECONFIG_H
#define DEVICECONFIG_H

#include <stdint.h>

// Funkcje do zarządzania konfiguracją
void configSetup(); // Ładuje konfigurację lub zapisuje domyślną
bool configIsContinuousMode(); // Sprawdza, czy włączyć tryb ciągły

// Funkcje dostępowe do ustawień czujnika wilgotności gleby
uint8_t configGetSoilPin();
int configGetSoilDryADC();
int configGetSoilWetADC();
int configGetSoilVccPin(); // Zwraca 255 (uint8_t max) lub -1 (int) w razie błędu/nieużywania

// Funkcje dostępowe dla czujnika poziomu wody
uint8_t configGetWaterLevelPin(int level); // Zwraca pin dla poziomu 1-5, lub 255 dla błędu

uint8_t configGetBatteryAdcPin(); // Zwraca pin ADC do pomiaru baterii

uint8_t configGetDhtPin();

// Funkcje dostępowe dla sterowania pompą
uint8_t configGetPumpPin();
uint32_t configGetPumpRunMillis();
int configGetSoilThresholdPercent(); // Próg wilgotności do uruchomienia pompy

// Funkcja dostępowa dla czasu uśpienia
uint32_t configGetSleepSeconds();

#endif // DEVICECONFIG_H