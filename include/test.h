/**
 * @file test.h
 * @brief Nagłówek pliku testowego Flora Smart Pot
 */

#pragma once
#include <stdint.h>

#ifdef TEST_MODE

// Wypisuje aktualne wartości testowe do Serial
void testPrintConfig();

// Settery – zmiana wartości testowych w runtime
void testSetSoilMoisture(int percent);
void testSetWaterLevel(int level);
void testSetBatteryVoltage(float voltage);
void testSetTemperature(float temp);
void testSetHumidity(float hum);
void testSetDhtError(bool error);
void testSetPumpRunning(bool running);

#else

// Tryb testowy wyłączony – puste inline, zero kosztu
inline void testPrintConfig() {}

#endif // TEST_MODE