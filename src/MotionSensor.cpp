// #include "MotionSensor.h"
// #include <Wire.h>                // I2C
// #include <Adafruit_MPU6050.h>    // Biblioteka MPU6050
// #include <Adafruit_Sensor.h>     // Zależność

// Adafruit_MPU6050 mpu; // Obiekt czujnika
// bool isMpuInitialized = false;

// bool motionSensorSetup(uint8_t address) {
//     // Rozpocznij I2C (może być już rozpoczęte w innym miejscu)
//     // Wire.begin(); // Zazwyczaj nie trzeba tu wywoływać, jeśli jest w setup() main.cpp
//     // Lub jeśli chcemy być pewni:
//     // if (!Wire.isEnabled()) { // Sprawdź, czy I2C już działa
//     //      Wire.begin();
//     // }
//     // Na razie załóżmy, że Wire.begin() będzie w głównym setup

//     Serial.printf("  [MPU6050] Próba inicjalizacji czujnika pod adresem 0x%02X...\n", address);
//     if (!mpu.begin(address)) {
//         Serial.println("  [MPU6050] BŁĄD: Nie znaleziono czujnika MPU6050!");
//         isMpuInitialized = false;
//         return false;
//     }
//     Serial.println("  [MPU6050] Czujnik znaleziony i zainicjalizowany.");

//     // Ustawienie zakresów (przykładowe, można dostosować)
//     mpu.setAccelerometerRange(MPU6050_RANGE_8_G);  // Zakres akcelerometru: +/- 8G
//     mpu.setGyroRange(MPU6050_RANGE_500_DEG); // Zakres żyroskopu: +/- 500 stopni/s
//     mpu.setFilterBandwidth(MPU6050_BAND_21_HZ); // Filtr dolnoprzepustowy (redukuje szumy)

//     Serial.printf("  [MPU6050] Zakres akcelerometru: %d G\n", mpu.getAccelerometerRange());
//     Serial.printf("  [MPU6050] Zakres żyroskopu: %d deg/s\n", mpu.getGyroRange());

//     isMpuInitialized = true;
//     return true;
// }

// bool motionSensorReadRaw(float &ax, float &ay, float &az, float &gx, float &gy, float &gz) {
//     if (!isMpuInitialized) {
//         Serial.println("  [MPU6050] BŁĄD: Czujnik nie zainicjalizowany.");
//         return false;
//     }

//     sensors_event_t a, g, temp; // Struktury do przechowywania danych (temp nie używamy bezpośrednio)

//     // Odczytaj wszystkie dane jednym wywołaniem
//     mpu.getEvent(&a, &g, &temp);

//     // Przypisz wartości do zmiennych przekazanych przez referencję
//     ax = a.acceleration.x;
//     ay = a.acceleration.y;
//     az = a.acceleration.z;
//     gx = g.gyro.x;
//     gy = g.gyro.y;
//     gz = g.gyro.z;

//     // Wydrukuj surowe dane (opcjonalnie, do debugowania)
//     /*
//     Serial.printf("  [MPU6050] Accel X: %.2f Y: %.2f Z: %.2f m/s^2\n", ax, ay, az);
//     Serial.printf("  [MPU6050] Gyro  X: %.2f Y: %.2f Z: %.2f rad/s\n", gx, gy, gz);
//     */

//     return true; // Zakładamy, że getEvent zawsze się udaje, jeśli begin() zadziałało
// }

// float motionSensorCalculateTiltAngleZ(float ax, float ay, float az) {
//     // Oblicza kąt przechyłu względem pionowej osi Z (grawitacji)
//     // atan2 zwraca kąt w radianach, konwertujemy na stopnie
//     // Wynik 0 stopni oznacza, że oś Z jest wyrównana z grawitacją (płytka leży poziomo lub pionowo Z w dół)
//     // Wynik 90 stopni oznacza, że oś Z jest prostopadła do grawitacji (płytka na boku)
//     // Używamy sqrt(ax^2 + ay^2), aby uzyskać wektor przyspieszenia w płaszczyźnie XY
//     float angleRad = atan2(sqrt(ax * ax + ay * ay), az);
//     float angleDeg = angleRad * 180.0 / PI;
//     return angleDeg;
// }