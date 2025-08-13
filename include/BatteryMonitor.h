#ifndef BATTERYMONITOR_H
#define BATTERYMONITOR_H

#include <stdint.h>

/**
 * @brief Initializes battery monitoring module.
 */
void batteryMonitorSetup();

/**
 * @brief Reads and returns current battery voltage.
 * Takes voltage divider into account.
 * @return float Measured battery voltage in Volts (V).
 */
float batteryMonitorReadVoltage();

/**
 * @brief Reads raw value from ADC converter.
 * @return int ADC value (0-4095).
 */
int batteryMonitorReadRawADC();

#endif // BATTERYMONITOR_H