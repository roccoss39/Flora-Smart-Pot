// #ifndef MOTIONSENSOR_H
// #define MOTIONSENSOR_H

// #include <stdint.h>

// /**
//  * @brief Inicjalizuje czujnik MPU-6500/9250.
//  * @return true jeśli inicjalizacja się powiodła, false w przeciwnym razie.
//  */
// bool motionSensorSetup();

// /**
//  * @brief Odczytuje najnowsze dostępne dane z akcelerometru i żyroskopu.
//  * @param ax Referencja do zmiennej dla przyspieszenia X (m/s^2).
//  * @param ay Referencja do zmiennej dla przyspieszenia Y (m/s^2).
//  * @param az Referencja do zmiennej dla przyspieszenia Z (m/s^2).
//  * @param gx Referencja do zmiennej dla prędkości kątowej X (rad/s).
//  * @param gy Referencja do zmiennej dla prędkości kątowej Y (rad/s).
//  * @param gz Referencja do zmiennej dla prędkości kątowej Z (rad/s).
//  * @return true jeśli odczytano nowe dane, false jeśli nowe dane nie były dostępne lub wystąpił błąd.
//  */
// bool motionSensorReadRaw(float &ax, float &ay, float &az, float &gx, float &gy, float &gz);

// /**
//  * @brief Odczytuje temperaturę z wewnętrznego czujnika MPU.
//  * Wymaga wcześniejszego udanego wywołania motionSensorReadRaw (lub ręcznego odczytu temperatury).
//  * @return float Temperatura w stopniach Celsjusza, lub NAN w przypadku błędu inicjalizacji.
//  */
// float motionSensorReadTemperature();

// /**
//  * @brief Oblicza prosty kąt przechyłu względem pionu (osi Z).
//  * @param ax Przyspieszenie X (m/s^2).
//  * @param ay Przyspieszenie Y (m/s^2).
//  * @param az Przyspieszenie Z (m/s^2).
//  * @return float Kąt przechyłu w stopniach (0 = pionowo Z w dół).
//  */
// float motionSensorCalculateTiltAngleZ(float ax, float ay, float az);

// #endif // MOTIONSENSOR_H