#ifndef SOILSENSOR_H
#define SOILSENSOR_H

// Funkcja inicjalizująca (np. konfigurująca pin VCC)
void soilSensorSetup();

// Funkcja odczytująca wilgotność
// Zwraca wartość w procentach (0-100)
int soilSensorReadPercent();

#endif // SOILSENSOR_H