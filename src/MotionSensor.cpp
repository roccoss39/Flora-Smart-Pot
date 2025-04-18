#include "MotionSensor.h"

// Konstruktor
MotionSensor::MotionSensor(uint8_t addr) {
    _mpu_addr = addr;
    // Inicjalizacja zmiennych na 0
    _rawAccX = _rawAccY = _rawAccZ = 0;
    _rawGyroX = _rawGyroY = _rawGyroZ = 0;
    _accX_g = _accY_g = _accZ_g = 0.0f;
    _gyroX_dps = _gyroY_dps = _gyroZ_dps = 0.0f;
    _pitch = _roll = 0.0f;
}

// Inicjalizacja
bool MotionSensor::begin(int sda_pin, int scl_pin) {
    Wire.begin(sda_pin, scl_pin);
    // Wire.setClock(400000); // Opcjonalnie

    // Obudź MPU
    // Używamy writeRegister dla uproszczenia i sprawdzenia
    if (!writeRegister(PWR_MGMT_1, 0x00)) {
         Serial.println("Error writing to PWR_MGMT_1 to wake up MPU!");
         return false; // Błąd zapisu
    }
    delay(100); // Daj MPU chwilę na stabilizację po obudzeniu

    // Sprawdź, czy odczyt działa (opcjonalnie, można dodać odczyt WHO_AM_I)
    uint8_t testRead = readRegister(PWR_MGMT_1);
    if (testRead == 0xFF) { // 0xFF oznacza błąd odczytu w naszej funkcji pomocniczej
        Serial.println("Error reading back from MPU after wake up!");
        return false;
    }
    // Można by tu sprawdzić, czy testRead == 0, ale nie jest to krytyczne

    return true; // Inicjalizacja (obudzenie) udana
}

// Odczyt danych
bool MotionSensor::readData() {
    bool readError = false;

    // --- Odczyt Akcelerometru (Raw) ---
    Wire.beginTransmission(_mpu_addr);
    Wire.write(ACCEL_XOUT_H);
    byte status_acc_ptr = Wire.endTransmission(false);
    if (status_acc_ptr != 0) { readError = true; }
    else {
        byte bytes_read_accel = Wire.requestFrom(_mpu_addr, (uint8_t)6, (uint8_t)true); // Jawne rzutowanie dla pewności
        if (bytes_read_accel == 6) {
            _rawAccX = (int16_t)(Wire.read() << 8 | Wire.read());
            _rawAccY = (int16_t)(Wire.read() << 8 | Wire.read());
            _rawAccZ = (int16_t)(Wire.read() << 8 | Wire.read());
        } else { readError = true; }
    }

    // --- Odczyt Żyroskopu (Raw) ---
    if (!readError) {
        Wire.beginTransmission(_mpu_addr);
        Wire.write(GYRO_XOUT_H);
        byte status_gyro_ptr = Wire.endTransmission(false);
        if (status_gyro_ptr != 0) { readError = true; }
        else {
            byte bytes_read_gyro = Wire.requestFrom(_mpu_addr, (uint8_t)6, (uint8_t)true);
            if (bytes_read_gyro == 6) {
                _rawGyroX = (int16_t)(Wire.read() << 8 | Wire.read());
                _rawGyroY = (int16_t)(Wire.read() << 8 | Wire.read());
                _rawGyroZ = (int16_t)(Wire.read() << 8 | Wire.read());
            } else { readError = true; }
        }
    }

    if (readError) {
        Serial.println("I2C read error occurred in MotionSensor::readData()!");
        return false;
    }

    // --- Konwersja i Kalibracja ---
    _accX_g = ((float)_rawAccX / _accel_scale_factor_g) - _accX_bias;
    _accY_g = ((float)_rawAccY / _accel_scale_factor_g) - _accY_bias;
    _accZ_g = ((float)_rawAccZ / _accel_scale_factor_g); // Z bez odejmowania offsetu 1g
    _gyroX_dps = ((float)_rawGyroX / _gyro_scale_factor_dps) - _gyroX_bias;
    _gyroY_dps = ((float)_rawGyroY / _gyro_scale_factor_dps) - _gyroY_bias;
    _gyroZ_dps = ((float)_rawGyroZ / _gyro_scale_factor_dps) - _gyroZ_bias;

    // Oblicz kąty po odczytaniu danych
    calculateAngles();

    return true; // Odczyt udany
}

// Obliczanie kątów (prosta metoda z akcelerometru)
void MotionSensor::calculateAngles() {
    // Te obliczenia działają najlepiej, gdy czujnik jest względnie nieruchomy.
    // Podatne na zakłócenia od przyspieszeń liniowych.
    // Wynik w stopniach.
    _roll = atan2(_accY_g, _accZ_g) * 180.0 / M_PI;
    _pitch = atan2(-_accX_g, sqrt(_accY_g * _accY_g + _accZ_g * _accZ_g)) * 180.0 / M_PI;
}

// Sprawdzanie przechyłu
bool MotionSensor::isTilted(float threshold_degrees) {
    // Upewnij się, że kąty są aktualne (można by to przenieść do readData)
    // calculateAngles(); // calculateAngles jest już wywoływane w readData()
    return (abs(_pitch) > threshold_degrees || abs(_roll) > threshold_degrees);
}

// Konfiguracja Wake on Motion (dla MPU-6500) - wymaga testowania i dostrojenia!
bool MotionSensor::setupWakeOnMotion(uint8_t threshold_mg_per_lsb) {
    Serial.println("Configuring Wake on Motion...");

    // 1. Reset device (optional but recommended)
    if (!writeRegister(PWR_MGMT_1, 0x80)) return false; // Reset
    delay(100);
    // Wake up again
    if (!writeRegister(PWR_MGMT_1, 0x00)) return false;
    delay(100);

    // Konfiguracja dla MPU-6500 Wake-on-Motion (bazując na datasheet i notach aplikacyjnych)
    // Używamy trybu "Cycle" z niską częstotliwością próbkowania akcelerometru

    // Ustaw akcelerometr w trybie low power ODR (Output Data Rate)
    // Przykład: 1.25 Hz ODR (0x07) - im niższa tym mniejszy pobór prądu,
    // ale wolniejsza reakcja na ruch. Wybierz eksperymentalnie.
    // 0 = 0.31 Hz, 1 = 0.62 Hz, ..., 6 = 20 Hz, 7 = 40 Hz ... 10 = 500Hz
    if (!writeRegister(LP_ACCEL_ODR, 0x06)) return false; // ok 20 Hz

    // Ustaw próg Wake-on-Motion
    // Jednostka: 1 LSB = 4mg (w zakresie +/-2g)
    // threshold_mg_per_lsb = 2 oznacza próg 2*4 = 8mg
    if (!writeRegister(ACCEL_WOM_THR, threshold_mg_per_lsb)) return false;

    // Włącz funkcję Wake-on-Motion dla akcelerometru
    // Ustaw bit ACCEL_INTEL_EN i ACCEL_INTEL_MODE (compare with previous sample)
    if (!writeRegister(ACCEL_INTEL_CTRL, 0xC0)) return false; // Enable WoM logic, compare current sample to previous sample

    // Skonfiguruj pin INT
    // Np. Active low, push-pull, latching until cleared
    // Bit 7=ACTL(0=active H), 6=OPEN(0=push-pull), 5=LATCH_INT_EN(1=latch), 4=INT_ANYRD_2CLEAR(0=cleared on status read)
    if (!writeRegister(INT_PIN_CFG, 0x20)) return false; // Latching, push-pull, active high

    // Włącz przerwanie Wake-on-Motion
    // Bit 6 = WOM_INT_EN
    if (!writeRegister(INT_ENABLE, 0x40)) return false; // Enable Wake On Motion interrupt only

    // Na koniec, przełącz MPU w tryb Cycle (niski pobór mocy, tylko akcelerometr aktywny)
    // Ustaw bit CYCLE w PWR_MGMT_1 (bit 5) i wyłącz SLEEP (bit 6 = 0)
    if (!writeRegister(PWR_MGMT_1, 0x20)) return false; // CYCLE=1, SLEEP=0

    Serial.println("Wake on Motion configured (hopefully).");
    return true;
}


// --- Prywatne funkcje pomocnicze ---

bool MotionSensor::writeRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(_mpu_addr);
  Wire.write(reg);
  Wire.write(value);
  byte status = Wire.endTransmission(true);
  return (status == 0); // Zwróć true jeśli zapis się powiódł (ACK)
}

uint8_t MotionSensor::readRegister(uint8_t reg) {
  Wire.beginTransmission(_mpu_addr);
  Wire.write(reg);
  byte status = Wire.endTransmission(false); // Wyślij restart
  if (status != 0) {
    return 0xFF; // Błąd przy wysyłaniu adresu rejestru
  }
  byte byteCount = Wire.requestFrom(_mpu_addr, (uint8_t)1, (uint8_t)true); // Poproś o 1 bajt
  if (byteCount == 1) {
    return Wire.read();
  } else {
    return 0xFF; // Błąd odczytu
  }
}