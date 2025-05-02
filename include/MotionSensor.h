/*
#ifndef MOTION_SENSOR_H
#define MOTION_SENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include <math.h> // Potrzebne do atan2, sqrt

class MotionSensor {
public:
    // Konstruktor (może przyjmować adres I2C)
    MotionSensor(uint8_t addr = 0x68);

    // Inicjalizacja (inicjalizuje Wire, budzi MPU)
    // Zwraca true jeśli sukces, false jeśli błąd
    bool begin(int sda_pin = 21, int scl_pin = 22);

    // Odczytuje nowe dane z czujnika
    // Zwraca true jeśli sukces, false jeśli błąd odczytu
    bool readData();

    // Oblicza kąty nachylenia na podstawie danych akcelerometru
    void calculateAngles();

    // Sprawdza, czy czujnik jest przechylony powyżej progu
    bool isTilted(float threshold_degrees = 15.0);

    // Konfiguruje MPU do generowania przerwania przy stuknięciu/ruchu
    // Zwraca true jeśli konfiguracja się powiodła
    // threshold: czułość (0-255, niższa wartość = bardziej czuły)
    // duration: minimalny czas trwania ruchu/stuknięcia (nie zawsze dostępny/prost do ustawienia)
    //           W MPU-6500 ustawia się bardziej próg i częstotliwość próbkowania w trybie low power.
    bool setupWakeOnMotion(uint8_t threshold_mg_per_lsb = 2); // Próg w mg/LSB (domyślnie 2mg/LSB)

    bool enterCycleMode(); 
    
    // Gettery (funkcje dostępowe) do odczytanych wartości
    float getAccX_g() { return _accX_g; }
    float getAccY_g() { return _accY_g; }
    float getAccZ_g() { return _accZ_g; }
    float getGyroX_dps() { return _gyroX_dps; }
    float getGyroY_dps() { return _gyroY_dps; }
    float getGyroZ_dps() { return _gyroZ_dps; }
    float getPitch() { return _pitch; }
    float getRoll() { return _roll; }

private:
    uint8_t _mpu_addr; // Adres I2C

    // Zmienne na surowe dane
    int16_t _rawAccX, _rawAccY, _rawAccZ;
    int16_t _rawGyroX, _rawGyroY, _rawGyroZ;

    // Zmienne na dane w jednostkach fizycznych (nieskalibrowane)
    float _accX_g, _accY_g, _accZ_g;
    float _gyroX_dps, _gyroY_dps, _gyroZ_dps;

    // Zmienne na obliczone kąty
    float _pitch, _roll;

    // Zmienne na biasy (offsety) - używamy tych obliczonych wcześniej
    // Można je później uczynić konfigurowalnymi
    const float _accX_bias = -0.132;
    const float _accY_bias = -0.025;
    // const float _accZ_bias = 1.031; // Nie używamy do korekty Z w prosty sposób
    const float _gyroX_bias = 0.019;
    const float _gyroY_bias = 2.297;
    const float _gyroZ_bias = 0.326;

    // Współczynniki skalowania
    const float _accel_scale_factor_g = 16384.0;
    const float _gyro_scale_factor_dps = 131.0;

    // Adresy rejestrów (prywatne stałe)
    static const int PWR_MGMT_1   = 0x6B;
    static const int ACCEL_XOUT_H = 0x3B;
    static const int GYRO_XOUT_H  = 0x43;
    // Rejestry potrzebne do Wake-on-Motion dla MPU-6500
    static const int ACCEL_CONFIG2    = 0x1D;
    static const int INT_PIN_CFG      = 0x37;
    static const int INT_ENABLE       = 0x38;
    static const int ACCEL_WOM_THR    = 0x1F; // Threshold for wake on motion (1 LSb = 4mg)
    static const int LP_ACCEL_ODR     = 0x1E; // Low Power Accelerometer ODR Control
    static const int MOT_DETECT_CTRL  = 0x69; // Motion Detection Control (w 6050 to inaczej!) - użyjmy ACCEL_INTEL_CTRL? Sprawdźmy datasheet 6500. Tak, 0x69 to ACCEL_INTEL_CTRL w MPU-6500.
    static const int ACCEL_INTEL_CTRL = 0x69; // Accelerometer Intelligence Control


    // Prywatna funkcja pomocnicza do zapisu do rejestru
    bool writeRegister(uint8_t reg, uint8_t value);
    // Prywatna funkcja pomocnicza do odczytu z rejestru
    uint8_t readRegister(uint8_t reg); // Zwraca wartość lub 0xFF przy błędzie
};

#endif // MOTION_SENSOR_H
*/