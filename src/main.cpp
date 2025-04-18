#include <Arduino.h>
#include <Wire.h>

// === Konfiguracja Pinów i Adresu ===
const int SDA_PIN = 21;
const int SCL_PIN = 22;
const int MPU_ADDR = 0x68;

// === Adresy Rejestrów ===
const int PWR_MGMT_1   = 0x6B;
const int ACCEL_XOUT_H = 0x3B;
const int GYRO_XOUT_H  = 0x43;

// === Współczynniki Skalowania ===
const float ACCEL_SCALE_FACTOR_G = 16384.0; // LSB/g
const float GYRO_SCALE_FACTOR_DPS = 131.0;  // LSB/(deg/s)

// === OBLICZONE BIASY (z Twoich danych "w spoczynku") ===
// Zaokrąglone dla czytelności
const float accX_bias = -0.132;
const float accY_bias = -0.025;
const float accZ_bias = 1.031; // Średnia wartość Z w spoczynku (bliska 1g)
const float gyroX_bias = 0.019;
const float gyroY_bias = 2.297; // Znaczący bias!
const float gyroZ_bias = 0.326;

// === Zmienne Globalne ===
int16_t rawAccX, rawAccY, rawAccZ;
int16_t rawGyroX, rawGyroY, rawGyroZ;
float accX_g, accY_g, accZ_g;
float gyroX_dps, gyroY_dps, gyroZ_dps;
// Dodane zmienne na skalibrowane wartości
float calAccX_g, calAccY_g, calAccZ_g;
float calGyroX_dps, calGyroY_dps, calGyroZ_dps;

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("\n--- MPU Calibrated Data Reader (Wire.h only) for ESP32 ---");

  Wire.begin(SDA_PIN, SCL_PIN);
  // Wire.setClock(400000);

  Serial.println("Initializing I2C and waking up MPU...");
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(PWR_MGMT_1);
  Wire.write(0);
  byte status = Wire.endTransmission(true);

  if (status == 0) {
    Serial.println("MPU wake-up successful!");
    Serial.println("Using pre-calculated biases based on provided data.");
    Serial.print("Gyro Y Bias to be subtracted: "); Serial.println(gyroY_bias); // Zwróć uwagę na ten bias
  } else {
    Serial.print("MPU wake-up failed! Error code: "); Serial.println(status);
    Serial.println("Check wiring. Halting.");
    while (1);
  }
  Serial.println("--------------------------------------------------------------------------");
  Serial.println("Format: CalAccX(g) CalAccY(g) CalAccZ(g) | CalGyroX(dps) CalGyroY(dps) CalGyroZ(dps)");
  Serial.println("--------------------------------------------------------------------------");
  delay(500);
}

void loop() {
  bool readError = false;

  // --- Odczyt Surowych Danych (tak jak poprzednio) ---
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(ACCEL_XOUT_H);
  byte status_acc_ptr = Wire.endTransmission(false);
  if (status_acc_ptr != 0) { readError = true; }
  else {
    byte bytes_read_accel = Wire.requestFrom((uint16_t)MPU_ADDR, (size_t)6, true);
    if (bytes_read_accel == 6) {
      rawAccX = (int16_t)(Wire.read() << 8 | Wire.read());
      rawAccY = (int16_t)(Wire.read() << 8 | Wire.read());
      rawAccZ = (int16_t)(Wire.read() << 8 | Wire.read());
    } else { readError = true; }
  }

  if (!readError) {
      Wire.beginTransmission(MPU_ADDR);
      Wire.write(GYRO_XOUT_H);
      byte status_gyro_ptr = Wire.endTransmission(false);
      if (status_gyro_ptr != 0) { readError = true; }
      else {
        byte bytes_read_gyro = Wire.requestFrom((uint16_t)MPU_ADDR, (size_t)6, true);
         if (bytes_read_gyro == 6) {
           rawGyroX = (int16_t)(Wire.read() << 8 | Wire.read());
           rawGyroY = (int16_t)(Wire.read() << 8 | Wire.read());
           rawGyroZ = (int16_t)(Wire.read() << 8 | Wire.read());
         } else { readError = true; }
      }
  }

  // --- Przetwarzanie, Kalibracja i Wyświetlanie ---
  if (!readError) {
    // Konwersja na jednostki fizyczne (tak jak poprzednio)
    accX_g = (float)rawAccX / ACCEL_SCALE_FACTOR_G;
    accY_g = (float)rawAccY / ACCEL_SCALE_FACTOR_G;
    accZ_g = (float)rawAccZ / ACCEL_SCALE_FACTOR_G;
    gyroX_dps = (float)rawGyroX / GYRO_SCALE_FACTOR_DPS;
    gyroY_dps = (float)rawGyroY / GYRO_SCALE_FACTOR_DPS;
    gyroZ_dps = (float)rawGyroZ / GYRO_SCALE_FACTOR_DPS;

    // *** ZASTOSOWANIE KALIBRACJI (Odejmowanie Biasu) ***
    calAccX_g = accX_g - accX_bias;
    calAccY_g = accY_g - accY_bias;
    // Dla AccZ odejmujemy różnicę od 1g, aby uzyskać odczyt względem idealnego poziomu
    // (Zakładając, że podczas pomiaru biasu oś Z była pionowo w górę)
    // Jeśli oś Z była w dół, odjęlibyśmy (accZ_bias - (-1.0)) czyli accZ_bias + 1.0
    // Prostsze podejście: po prostu odjąć bias AccZ, aby wycentrować wokół średniej spoczynkowej.
    // Zastosujmy prostsze: odjęcie biasu X i Y (powinny być 0), Z zostawiamy względem grawitacji
     calAccZ_g = accZ_g; // Nie korygujemy Z na razie, bo zawiera grawitację

    calGyroX_dps = gyroX_dps - gyroX_bias;
    calGyroY_dps = gyroY_dps - gyroY_bias; // Tutaj korekta będzie znacząca
    calGyroZ_dps = gyroZ_dps - gyroZ_bias;


    // Wyświetl SKALIBROWANE wartości
    Serial.print(calAccX_g, 3); Serial.print("\t");
    Serial.print(calAccY_g, 3); Serial.print("\t");
    Serial.print(calAccZ_g, 3); Serial.print("\t|\t"); // Z nadal nieskalibrowane względem 1g
    Serial.print(calGyroX_dps, 3); Serial.print("\t");
    Serial.print(calGyroY_dps, 3); Serial.print("\t"); // Ta wartość powinna być teraz bliska 0
    Serial.println(calGyroZ_dps, 3);

  } else {
      Serial.println("Read error occurred, skipping print.");
  }

  delay(150);
}