#ifndef WATERLEVELSENSOR_H
#define WATERLEVELSENSOR_H

#include <stdint.h>

#define NUM_WATER_LEVELS 5 // Definicja liczby poziomów

/**
 * @brief Inicjalizuje piny dla czujnika poziomu wody.
 * Konfiguruje je jako INPUT_PULLUP.
 */
void waterLevelSensorSetup();

/**
 * @brief Odczytuje aktualny poziom wody.
 * Sprawdza stan pinów skonfigurowanych dla poziomów 1-5.
 * @return int Najwyższy wykryty poziom (1-5) lub 0, jeśli woda nie sięga nawet poziomu 1.
 */
int waterLevelSensorReadLevel();

#endif // WATERLEVELSENSOR_H