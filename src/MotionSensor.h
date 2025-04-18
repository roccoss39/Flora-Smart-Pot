#ifndef MOTIONSENSOR_H
#define MOTIONSENSOR_H

#include <stdint.h>

/**
 * @brief Inicjalizuje czujnik MPU-6050.
 * Rozpoczyna komunikację I2C.
 * @param address Adres I2C czujnika (domyślnie 0x68).
 * @return true jeśli inicjalizacja się powiodła, false w przeciwnym razie.
 */
bool motionSensorSetup(uint8_t address = 0x68);

/**
 * @brief Odczytuje surowe dane z akcelerometru i żyroskopu.
 * @param ax Referencja do zmiennej dla przyspieszenia X (m/s^2).
 * @param ay Referencja do zmiennej dla przyspieszenia Y (m/s^2).
 * @param az Referencja do zmiennej dla przyspieszenia Z (m/s^2).
 * @param gx Referencja do zmiennej dla prędkości kątowej X (rad/s).
 * @param gy Referencja do zmiennej dla prędkości kątowej Y (rad/s).
 * @param gz Referencja do zmiennej dla prędkości kątowej Z (rad/s).
 * @return true jeśli odczyt był poprawny, false w przypadku błędu.
 */
bool motionSensorReadRaw(float &ax, float &ay, float &az, float &gx, float &gy, float &gz);

/**
 * @brief Oblicza prosty kąt przechyłu względem pionu (osi Z).
 * Używa tylko danych z akcelerometru. Wynik może być niestabilny przy ruchu.
 * @param ax Przyspieszenie X.
 * @param ay Przyspieszenie Y.
 * @param az Przyspieszenie Z.
 * @return float Kąt przechyłu w stopniach (0 = pionowo, 90 = poziomo).
 */
float motionSensorCalculateTiltAngleZ(float ax, float ay, float az);

#endif // MOTIONSENSOR_H