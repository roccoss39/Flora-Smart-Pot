// #include "MotionSensor.h"
// #include <Arduino.h>
// #include <Wire.h>
// // Używamy głównego nagłówka biblioteki SparkFun (może wymagać dostosowania nazwy, jeśli jest inna)
// // Najczęściej spotykany w nowszych wersjach to ten z DMP:

// #include "quaternionFilters.h"
// #include "MPU9250.h"
// // Jeśli powyższy include nie działa, spróbuj:
// // #include <MPU9250.h> // (jeśli taki plik istnieje w bibliotece SparkFun)

// MPU9250_DMP imu; // Tworzymy obiekt klasy z biblioteki SparkFun
// bool isMpuInitialized = false;

// // Stałe do konwersji jednostek
// const float G_TO_MS2 = 9.80665;
// // Biblioteka SparkFun często zwraca radiany/s dla żyroskopu, sprawdźmy
// // const float DEG_TO_RAD_CONST = PI / 180.0; // Może nie być potrzebne

// bool motionSensorSetup() {
//     // Adres jest zwykle domyślny (0x68 lub 0x69), begin() go znajduje
//     Serial.println("  [MPU6500/9250] Próba inicjalizacji czujnika (SparkFun)...");

//     // Wywołaj begin() - zwraca status INV_SUCCESS (który = 0) jeśli OK
//     // Funkcja ta sama konfiguruje I2C (przez Wire) i sprawdza WHO_AM_I
//     inv_error_t status = imu.begin();

//     if (status != INV_SUCCESS) {
//         Serial.print("  [MPU6500/9250] BŁĄD: Inicjalizacja MPU nie powiodła się! Kod błędu: ");
//         Serial.println(status);
//         isMpuInitialized = false;
//         return false;
//     }

//     Serial.println("  [MPU6500/9250] Inicjalizacja (imu.begin) OK.");

//     // Ustawienie zakresów i filtrów - używamy funkcji z API SparkFun
//     // (Zakładając, że chcemy te same zakresy co poprzednio)
//     if (imu.setAccelFSR(8) != INV_SUCCESS) { // Zakres w G
//          Serial.println("  [MPU6500/9250] OSTRZEŻENIE: Nie udało się ustawić zakresu akcelerometru.");
//     }
//      if (imu.setGyroFSR(500) != INV_SUCCESS) { // Zakres w stopniach/s
//          Serial.println("  [MPU6500/9250] OSTRZEŻENIE: Nie udało się ustawić zakresu żyroskopu.");
//     }
//     // Ustawienie filtra dolnoprzepustowego (w Hz)
//      if (imu.setLPF(42) != INV_SUCCESS) { // Np. 42Hz, można dobrać inną wartość (5, 10, 21, 42, 98, 188)
//          Serial.println("  [MPU6500/9250] OSTRZEŻENIE: Nie udało się ustawić filtra LPF.");
//      }
//     // Ustawienie częstotliwości próbkowania (w Hz)
//      if (imu.setSampleRate(50) != INV_SUCCESS) { // Np. 50Hz
//           Serial.println("  [MPU6500/9250] OSTRZEŻENIE: Nie udało się ustawić częstotliwości próbkowania.");
//      }
//     // Włączenie czujników
//      if (imu.setSensors(INV_XYZ_ACCEL | INV_XYZ_GYRO | INV_XYZ_COMPASS) != INV_SUCCESS){ // Włączamy też kompas, choć go nie używamy - może być wymagane
//          Serial.println("  [MPU6500/9250] OSTRZEŻENIE: Nie udało się włączyć sensorów.");
//      }


//     Serial.println("  [MPU6500/9250] Czujnik zainicjalizowany i skonfigurowany (SparkFun).");
//     isMpuInitialized = true;
//     return true;
// }

// bool motionSensorReadRaw(float &ax, float &ay, float &az, float &gx, float &gy, float &gz) {
//     if (!isMpuInitialized) {
//         return false;
//     }

//     // Sprawdź, czy nowe dane są dostępne
//     if (imu.dataReady()) {
//         // Odczytaj dane (aktualizuje publiczne zmienne obiektu imu)
//         // UPDATE_COMPASS może być niepotrzebne, jeśli nie ma magnetometru
//         imu.update(UPDATE_ACCEL | UPDATE_GYRO | UPDATE_COMPASS | UPDATE_TEMP);

//         // Pobierz wartości z publicznych zmiennych członkowskich obiektu imu
//         // Jednostki: ax, ay, az w [G]; gx, gy, gz w [deg/s]

//         // Przyspieszenie (G) -> m/s^2
//         ax = imu.ax * G_TO_MS2;
//         ay = imu.ay * G_TO_MS2;
//         az = imu.az * G_TO_MS2;

//         // Prędkość kątowa (deg/s) -> rad/s
//         // Sprawdźmy, czy biblioteka nie zwraca rad/s bezpośrednio (mało prawdopodobne)
//         // Jeśli imu.gx jest w dps:
//         gx = imu.gx * DEG_TO_RAD_CONST;
//         gy = imu.gy * DEG_TO_RAD_CONST;
//         gz = imu.gz * DEG_TO_RAD_CONST;

//         return true; // Sukces - odczytano nowe dane
//     }

//     return false; // Brak nowych danych
// }

// float motionSensorReadTemperature() {
//     if (!isMpuInitialized) {
//        return NAN; // Używamy NAN z <cmath>
//    }
//     // Zakładamy, że imu.update() zostało wywołane w motionSensorReadRaw()
//     // i zaktualizowało publiczną zmienną 'temperature'
//     // Sprawdź, czy ta zmienna istnieje w bibliotece SparkFun (powinna)
//     return imu.temperature; // Zwraca stopnie Celsjusza
// }

// // Funkcja obliczająca kąt - bez zmian
// float motionSensorCalculateTiltAngleZ(float ax, float ay, float az) {
//     if (az == 0.0) return 90.0;
//     // Upewnij się, że <cmath> jest dołączone w main.cpp lub tutaj
//     float angleRad = atan2(sqrt(ax * ax + ay * ay), abs(az));
//     // Używamy PI z Arduino.h lub math.h
//     float angleDeg = angleRad * 180.0 / PI;
//     return angleDeg;
// }