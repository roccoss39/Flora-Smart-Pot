#ifndef BATTERYMONITOR_H
#define BATTERYMONITOR_H

#include <stdint.h>

/**
 * @brief Inicjalizuje moduł monitorowania baterii.
 */
void batteryMonitorSetup();

/**
 * @brief Odczytuje i zwraca aktualne napięcie baterii.
 * Uwzględnia dzielnik napięcia.
 * @return float Zmierzone napięcie baterii w Woltach (V).
 */
float batteryMonitorReadVoltage();

/**
 * @brief Odczytuje surową wartość z przetwornika ADC.
 * @return int Wartość ADC (0-4095).
 */
int batteryMonitorReadRawADC();

#endif // BATTERYMONITOR_H