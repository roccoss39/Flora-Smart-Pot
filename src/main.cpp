/*
#include <Arduino.h>
#include "DeviceConfig.h"
#include "SoilSensor.h"
#include "WaterLevelSensor.h" // Używa stałej NUM_WATER_LEVELS z tego pliku
#include "PowerManager.h"
#include "PumpControl.h"
#include "BatteryMonitor.h"
#include "EnvironmentSensor.h"
#include "MotionSensor.h"
#include <Wire.h> // Potrzebne dla I2C
#include <Adafruit_Sensor.h>

// Przyszłe include'y dla komunikacji
// #include "WiFiManager.h"
// #include "MQTTManager.h"

void setup() {
    Serial.begin(115200);
    Serial.println("\n--- Flaura Smart Pot - Główny Start ---");
    Wire.begin();

    configSetup();

    soilSensorSetup();
    waterLevelSensorSetup();
    pumpControlSetup();
    batteryMonitorSetup();
    environmentSensorSetup();
    motionSensorSetup(); 

    // ... (Inicjalizacja innych modułów) ...

    if (!configIsContinuousMode()) {
        Serial.println("Rozpoczynam cykl pomiarowy (Tryb Deep Sleep)...");
        int currentMoisture = soilSensorReadPercent();
        int currentWaterLevel = waterLevelSensorReadLevel();

        Serial.println("--- Wyniki pomiarów ---");
        Serial.printf("Wilgotność gleby: %d %%\n", currentMoisture);
        Serial.printf("Poziom wody: %d / %d\n", currentWaterLevel, NUM_WATER_LEVELS);
        Serial.println("-----------------------");

        // Uruchom pompę jeśli potrzeba (tylko w trybie auto - do dodania)
        pumpControlActivateIfNeeded(currentMoisture, currentWaterLevel);

        // ... (Wysyłanie danych) ...

        powerManagerGoToDeepSleep();
    } else {
         Serial.println("Uruchomiono w trybie ciągłym. Pomiary/kontrola w pętli loop().");
         Serial.println("Pamiętaj zmienić DEFAULT_CONTINUOUS_MODE na 'false' dla Deep Sleep!");
    }
}

void loop() {
    // W przyszłości: obsługa komunikacji (odbieranie komend MQTT/BLE)
    // handleCommunication();
    // Przykład:
    // if (receivedCommand == "PUMP_ON_MANUAL") { pumpControlManualTurnOn(configGetPumpRunMillis()); }
    // if (receivedCommand == "PUMP_OFF_MANUAL") { pumpControlManualTurnOff(); }

    if (configIsContinuousMode()) {
        delay(5000);
        Serial.println("\n--- Kolejny pomiar/cykl (tryb ciągły) ---");

        float currentTemperature;
        float currentHumidity;
        bool dhtOk = environmentSensorRead(currentTemperature, currentHumidity); // Odczytaj Temp/Wilg

        int currentMoisture = soilSensorReadPercent();
        int currentWaterLevel = waterLevelSensorReadLevel();
        float currentBatteryVoltage = batteryMonitorReadVoltage();

        float ax, ay, az, gx, gy, gz;
        bool mpuOk = motionSensorReadRaw(ax, ay, az, gx, gy, gz);

        Serial.println("--- Wyniki pomiarów ---");
        Serial.printf("Wilgotność gleby: %d %%\n", currentMoisture);
        Serial.printf("Poziom wody: %d / %d\n", currentWaterLevel, NUM_WATER_LEVELS);
        Serial.printf("Napięcie baterii: %.2f V\n", currentBatteryVoltage); // TODO: DODAC DO DEEP SLEEP
        if (dhtOk) { // Wyświetl tylko jeśli odczyt był poprawny
            Serial.printf("Temperatura: %.1f C\n", currentTemperature);
            Serial.printf("Wilgotność powietrza: %.1f %%\n", currentHumidity);
         } else {
            Serial.println("Temperatura: Błąd odczytu");
            Serial.println("Wilgotność powietrza: Błąd odczytu");
         }

         if (mpuOk) {
            float tiltAngle = motionSensorCalculateTiltAngleZ(ax, ay, az);
            Serial.printf("MPU6050 Accel (X,Y,Z): %.2f, %.2f, %.2f m/s^2\n", ax, ay, az);
            Serial.printf("MPU6050 Gyro  (X,Y,Z): %.2f, %.2f, %.2f rad/s\n", gx, gy, gz);
            Serial.printf("Obliczony kąt przechyłu (vs pion): %.1f stopni\n", tiltAngle);
    
            // Prosta logika wykrywania przechyłu
            const float TILT_THRESHOLD_DEGREES = 45.0; // Próg przechyłu w stopniach
            if (abs(tiltAngle) > TILT_THRESHOLD_DEGREES) {
                Serial.println("!!! ALERT: Doniczka znacznie przechylona lub przewrócona! Zatrzymuję pompę.");
                // Wywołaj funkcję natychmiastowego zatrzymania pompy
                pumpControlManualTurnOff();
                // Tutaj można dodać wysyłanie alertu MQTT/Blynk itp.
            }
    
        } else {
             Serial.println("MPU6050: Błąd odczytu danych.");
        }

        Serial.println("-----------------------");

        // Uruchom pompę jeśli potrzeba (tylko w trybie auto - do dodania)
        pumpControlActivateIfNeeded(currentMoisture, currentWaterLevel);

    } else {
        // W trybie Deep Sleep ta pętla nie powinna być często osiągana
        delay(1000);
    }
}

*/

/*
#include <Arduino.h>
#include <Wire.h>

void setup() {
  Wire.begin(); // Uruchom I2C (domyślne piny 21, 22)
  Serial.begin(115200);
  while (!Serial); // Poczekaj na monitor
  Serial.println("\n--- Skaner I2C ---");
}

void loop() {
  byte error, address;
  int nDevices;

  Serial.println("Skanowanie...");
  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    // Wire.requestFrom() zwraca 0 jeśli urządzenie nie odpowiedziało
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) { // Sukces - urządzenie odpowiedziało
      Serial.print("Znaleziono urządzenie I2C pod adresem 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println("  !");
      nDevices++;
    }
    else if (error == 4) { // Inny błąd (np. problem z połączeniem)
      Serial.print("Nieznany błąd przy adresie 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("Nie znaleziono żadnych urządzeń I2C.\n");
  else
    Serial.println("Skanowanie zakończone.\n");

  delay(5000); // Poczekaj 5 sekund przed kolejnym skanowaniem
}

*/

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Minimalny Test MPU-6050");

  Wire.begin(); // Uruchom I2C

  // Spróbuj zainicjować MPU na domyślnym adresie 0x68
  if (!mpu.begin()) {
    Serial.println("BŁĄD KRYTYCZNY: Nie można zainicjować MPU-6050! Sprawdź połączenia.");
    while (1) { // Zatrzymaj program
      delay(10);
    }
  }
  Serial.println("MPU-6050 zainicjalizowany pomyślnie.");

  // Ustaw zakresy (opcjonalnie)
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp); // Odczytaj dane

  Serial.print("Akcelerometr - X: "); Serial.print(a.acceleration.x);
  Serial.print(" Y: "); Serial.print(a.acceleration.y);
  Serial.print(" Z: "); Serial.print(a.acceleration.z);
  Serial.println(" m/s^2");

  Serial.print("Żyroskop - X: "); Serial.print(g.gyro.x);
  Serial.print(" Y: "); Serial.print(g.gyro.y);
  Serial.print(" Z: "); Serial.print(g.gyro.z);
  Serial.println(" rad/s");

  Serial.println("");
  delay(500); // Odczytuj co pół sekundy
}