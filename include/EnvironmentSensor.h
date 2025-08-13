#ifndef ENVIRONMENTSENSOR_H
#define ENVIRONMENTSENSOR_H

#include <stdint.h>

/**
 * @brief Initializes environmental sensor (DHT11).
 */
void environmentSensorSetup();

/**
 * @brief Reads temperature and humidity from DHT11 sensor.
 *
 * @param temperature Reference to variable where temperature will be stored (°C).
 * @param humidity Reference to variable where humidity will be stored (%RH).
 * @return true if reading was successful, false in case of error.
 */
bool environmentSensorRead(float &temperature, float &humidity);

#endif // ENVIRONMENTSENSOR_H