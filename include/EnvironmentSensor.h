#ifndef ENVIRONMENTSENSOR_H
#define ENVIRONMENTSENSOR_H

#include <stdint.h>

/**
 * @brief Inicjalizuje czujnik środowiskowy (DHT11).
 */
void environmentSensorSetup();

/**
 * @brief Odczytuje temperaturę i wilgotność z czujnika DHT11.
 *
 * @param temperature Referencja do zmiennej, gdzie zostanie zapisana temperatura (°C).
 * @param humidity Referencja do zmiennej, gdzie zostanie zapisana wilgotność (%RH).
 * @return true jeśli odczyt był poprawny, false w przypadku błędu.
 */
bool environmentSensorRead(float &temperature, float &humidity);

#endif // ENVIRONMENTSENSOR_H