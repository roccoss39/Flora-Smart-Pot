#include <Arduino.h>
#include <Wire.h>

// === Konfiguracja Pinów i Adresu ===
const int SDA_PIN = 21; // Domyślny SDA dla ESP32
const int SCL_PIN = 22; // Domyślny SCL dla ESP32
const int MPU_ADDR = 0x68; // Adres I2C czujnika (AD0 = LOW)

// === Adresy Rejestrów MPU-6050 / MPU-6500 ===
const int PWR_MGMT_1   = 0x6B; // Rejestr Zarządzania Energią 1
const int ACCEL_XOUT_H = 0x3B; // Rejestr początkowy danych Akcelerometru (6 bajtów od tego adresu)
const int GYRO_XOUT_H  = 0x43; // Rejestr początkowy danych Żyroskopu (6 bajtów od tego adresu)
// const int TEMP_OUT_H   = 0x41; // Rejestr początkowy danych Temperatury (2 bajty) - opcjonalnie

// === Zmienne Globalne na Surowe Dane ===
int16_t rawAccX, rawAccY, rawAccZ; // 16-bitowe wartości ze znakiem
int16_t rawGyroX, rawGyroY, rawGyroZ;

void setup() {
  // Inicjalizacja Portu Szeregowego
  Serial.begin(115200); // Użyj tej prędkości w monitorze szeregowym
  while (!Serial) {
    delay(10); // Czekaj na połączenie
  }
  Serial.println("\n--- MPU Raw Data Reader (Wire.h only) for ESP32 ---");

  // Inicjalizacja Magistrali I2C z pinami ESP32
  Wire.begin(SDA_PIN, SCL_PIN);
  // Opcjonalnie ustaw wyższą prędkość zegara I2C (standard to 100kHz)
  // Wire.setClock(400000); // 400kHz

  Serial.println("Initializing I2C communication...");

  // Krok 1: Obudź czujnik MPU
  Wire.beginTransmission(MPU_ADDR);   // Zacznij transmisję do MPU (0x68)
  Wire.write(PWR_MGMT_1);           // Wskaż rejestr PWR_MGMT_1 (0x6B)
  Wire.write(0);                    // Zapisz wartość 0x00, aby wyłączyć tryb uśpienia
  byte status = Wire.endTransmission(true); // Zakończ transmisję

  // Sprawdź status operacji budzenia
  if (status == 0) {
    Serial.println("MPU wake-up successful!");
  } else {
    Serial.print("MPU wake-up failed! Wire.endTransmission error code: ");
    Serial.println(status);
    Serial.println("Check wiring (SDA->21, SCL->22, VCC->3.3V!, GND->GND) and I2C address.");
    Serial.println("Halting execution.");
    while (1) { delay(100); } // Zatrzymaj program
  }
  Serial.println("----------------------------------------------------");
  Serial.println("Reading Raw Sensor Data...");
  Serial.println("Format: AccX  AccY  AccZ  |  GyroX  GyroY  GyroZ");
  Serial.println("----------------------------------------------------");
  delay(500); // Krótka pauza przed pętlą główną
}

void loop() {
  bool readError = false;

  // --- Odczyt Danych Akcelerometru ---
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(ACCEL_XOUT_H); // Ustaw wskaźnik na początek danych akcelerometru (0x3B)
  byte status_acc_ptr = Wire.endTransmission(false); // Wyślij restart (nie kończ sesji)

  if (status_acc_ptr != 0) {
     Serial.print("I2C Error setting Accel pointer: "); Serial.println(status_acc_ptr);
     readError = true;
  } else {
     byte bytes_read_accel = Wire.requestFrom(MPU_ADDR, 6, true); // Poproś o 6 bajtów (AccX H/L, AccY H/L, AccZ H/L)
     if (bytes_read_accel == 6) {
       // Odczytaj i połącz bajty (starszy bajt << 8 | młodszy bajt)
       rawAccX = (int16_t)(Wire.read() << 8 | Wire.read());
       rawAccY = (int16_t)(Wire.read() << 8 | Wire.read());
       rawAccZ = (int16_t)(Wire.read() << 8 | Wire.read());
     } else {
       Serial.print("I2C Error reading Accel data, bytes read: "); Serial.println(bytes_read_accel);
       readError = true;
     }
  }

  // --- Odczyt Danych Żyroskopu ---
  // (Tylko jeśli odczyt akcelerometru nie napotkał błędu wskaźnika,
  //  chociaż błąd odczytu mógł wystąpić)
  if (!readError) {
      Wire.beginTransmission(MPU_ADDR);
      Wire.write(GYRO_XOUT_H); // Ustaw wskaźnik na początek danych żyroskopu (0x43)
      byte status_gyro_ptr = Wire.endTransmission(false); // Wyślij restart

      if (status_gyro_ptr != 0) {
         Serial.print("I2C Error setting Gyro pointer: "); Serial.println(status_gyro_ptr);
         readError = true;
      } else {
         byte bytes_read_gyro = Wire.requestFrom(MPU_ADDR, 6, true); // Poproś o 6 bajtów (GyroX H/L, GyroY H/L, GyroZ H/L)
         if (bytes_read_gyro == 6) {
           // Odczytaj i połącz bajty
           rawGyroX = (int16_t)(Wire.read() << 8 | Wire.read());
           rawGyroY = (int16_t)(Wire.read() << 8 | Wire.read());
           rawGyroZ = (int16_t)(Wire.read() << 8 | Wire.read());
         } else {
           Serial.print("I2C Error reading Gyro data, bytes read: "); Serial.println(bytes_read_gyro);
           readError = true;
         }
      }
  }

  // --- Wyświetl Surowe Dane (jeśli nie było błędów I2C) ---
  if (!readError) {
      Serial.print(rawAccX); Serial.print("\t"); // Użyj tabulatora jako separatora
      Serial.print(rawAccY); Serial.print("\t");
      Serial.print(rawAccZ); Serial.print("\t|\t");
      Serial.print(rawGyroX); Serial.print("\t");
      Serial.print(rawGyroY); Serial.print("\t");
      Serial.println(rawGyroZ);
  } else {
      Serial.println("Skipping print due to read error.");
      // Zeruj wartości w razie błędu, aby uniknąć używania starych danych
      rawAccX=0; rawAccY=0; rawAccZ=0; rawGyroX=0; rawGyroY=0; rawGyroZ=0;
  }

  delay(150); // Opóźnienie między cyklami odczytu
}