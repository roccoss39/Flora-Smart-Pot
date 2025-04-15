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
int configGetSoilVccPin(); // Zwraca -1 jeśli nie używany

// Funkcje dostępowe dla czujnika poziomu wody
uint8_t configGetWaterLevelPin(int level); // Zwraca pin dla poziomu 1-5

// Funkcja dostępowa dla czasu uśpienia
uint32_t configGetSleepSeconds();

// W przyszłości dodaj tu gettery dla innych ustawień (MQTT, DHT itp.)

#endif // DEVICECONFIG_H