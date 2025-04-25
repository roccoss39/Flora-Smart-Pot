#include <Arduino.h>
#include <Wire.h>              // Nadal potrzebne, jeśli planujesz inne urządzenia I2C
#include <esp_sleep.h>       // Dla Deep Sleep i Wakeup Cause
#include <cmath>             // Dla abs(), isnan(), sqrt(), atan2()

// Nasze moduły
#include "DeviceConfig.h"
#include "secrets.h"          // Dla danych WiFi i Blynk
#include "WiFiManager.h"
#include "BlynkManager.h"
#include "SoilSensor.h"
#include "WaterLevelSensor.h" // Definiuje NUM_WATER_LEVELS
#include "PumpControl.h"
#include "BatteryMonitor.h"
#include "EnvironmentSensor.h" // Dla DHT11
// #include "MotionSensor.h"   // <<--- ZAKOMENTOWANY LUB USUNIĘTY
#include "PowerManager.h"

// Funkcja pomocnicza do drukowania przyczyny wybudzenia
void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  Serial.print("Przyczyna uruchomienia/wybudzenia: ");
  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("EXT0 (zewnętrzne przerwanie 0)"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("EXT1 (zewnętrzne przerwanie 1)"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("ULP Program"); break;
    default : Serial.printf("Reset lub Power On (przyczyna nr %d)\n", wakeup_reason); break;
  }
}

// Wbudowana dioda LED
#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

// Zmienna do śledzenia czasu ostatniej wysyłki do Blynk
unsigned long lastBlynkSend = 0;

void setup() {
    Serial.begin(115200);
    Serial.println("\n--- Flaura Smart Pot - Główny Start ---");
    print_wakeup_reason();

    Wire.begin(); // Inicjalizuj I2C raz (może się przydać w przyszłości)
    delay(100);

    configSetup(); // Wczytaj konfigurację

    // Inicjalizuj wszystkie moduły (oprócz MPU)
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH); // Zaświeć na czas setup

    soilSensorSetup();
    waterLevelSensorSetup();
    pumpControlSetup();
    batteryMonitorSetup();
    environmentSensorSetup();
    // motionSensorSetup(); // <<--- ZAKOMENTOWANE LUB USUNIĘTE

    digitalWrite(LED_BUILTIN, LOW); // Zgaś po inicjalizacji

    // --- Logika połączenia i pierwszego pomiaru ---
    bool runMeasurement = true;
    bool connectSuccess = false;

    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    if (cause == ESP_SLEEP_WAKEUP_UNDEFINED || configIsContinuousMode()) {
        if (wifiConnect(SECRET_SSID, SECRET_PASS)) {
            blynkConfigure(BLYNK_AUTH_TOKEN, BLYNK_TEMPLATE_ID, BLYNK_DEVICE_NAME);
            connectSuccess = blynkConnect(); // Spróbuj połączyć z Blynk
        } else {
            runMeasurement = false;
            Serial.println("Brak WiFi, pomijam cykl pomiarowy przy starcie.");
        }
    } else {
         // Wybudzono z Deep Sleep - konfiguracja Blynk wystarczy przed wysłaniem
         blynkConfigure(BLYNK_AUTH_TOKEN, BLYNK_TEMPLATE_ID, BLYNK_DEVICE_NAME);
         // Zakładamy, że pomiar i próba wysłania nastąpią
    }

    if (runMeasurement) {
        Serial.println("\n--- Pierwszy pomiar/cykl po starcie/wybudzeniu ---");
        digitalWrite(LED_BUILTIN, HIGH);

        // Odczyty sensorów (bez MPU)
        int currentMoisture = soilSensorReadPercent();
        int currentWaterLevel = waterLevelSensorReadLevel();
        float currentBatteryVoltage = batteryMonitorReadVoltage();
        float currentTemperatureDHT;
        float currentHumidityDHT;
        bool dhtOk = environmentSensorRead(currentTemperatureDHT, currentHumidityDHT);

        digitalWrite(LED_BUILTIN, LOW);

        // Wyświetl wyniki lokalnie
        Serial.println("--- Wyniki pomiarów ---");
        if(currentMoisture >= 0) Serial.printf("Wilgotność gleby: %d %%\n", currentMoisture); else Serial.println("Wilgotność gleby: Błąd odczytu");
        Serial.printf("Poziom wody: %d / %d\n", currentWaterLevel, NUM_WATER_LEVELS);
        if(currentBatteryVoltage > 0) Serial.printf("Napięcie baterii: %.2f V\n", currentBatteryVoltage); else Serial.println("Napięcie baterii: Błąd odczytu / Odłączona");
        if (dhtOk && !isnan(currentTemperatureDHT) && !isnan(currentHumidityDHT)) {
            Serial.printf("Temperatura (DHT11): %.1f C\n", currentTemperatureDHT);
            Serial.printf("Wilgotność pow. (DHT11): %.1f %%\n", currentHumidityDHT);
         } else {
             Serial.println("Temperatura/Wilgotność (DHT11): Błąd odczytu");
         }
         // Usunięto sekcję MPU
        Serial.println("-----------------------");

        // Wyślij dane do Blynk
        // Spróbuj połączyć WiFi/Blynk jeśli potrzeba (np. po wybudzeniu)
        if (!wifiIsConnected()){ wifiConnect(SECRET_SSID, SECRET_PASS, 5000); }
        if (wifiIsConnected()) {
            if (!blynkIsConnected()) { blynkConnect(3000); }
            // Wyślij dane (bez MPU)
             blynkSendSensorData(currentMoisture, currentWaterLevel, currentBatteryVoltage,
                                  currentTemperatureDHT, currentHumidityDHT,
                                  NAN, false, pumpControlIsRunning()); // Przekazujemy NAN dla tilt i false dla alertu
             lastBlynkSend = millis();
        }

        // Kontrola pompy (bez sprawdzania przechyłu)
        pumpControlActivateIfNeeded(currentMoisture, currentWaterLevel);

    } // koniec if(runMeasurement)

    // --- Przejście w Deep Sleep (tylko w tym trybie) ---
    if (!configIsContinuousMode()) {
      // Sprawdź, czy pompa nie pracuje
      if (pumpControlIsRunning()) {
          Serial.println("Pompa pracuje - pozostaję w trybie aktywnym do zakończenia pracy pompy.");
          // Przejdź do loop() zamiast deep sleep
      } else {
          digitalWrite(LED_BUILTIN, HIGH); delay(50); digitalWrite(LED_BUILTIN, LOW);
          Serial.println("Rozłączam WiFi/Blynk i konfiguruję wybudzanie (tylko timer)...");
          blynkDisconnect();
          wifiDisconnect();
          powerManagerGoToDeepSleep(); // Konfiguruje timer i idzie spać
      }
    } else {
      Serial.println("Tryb ciągły aktywny. Dalsza praca w loop().");
  }
} // Koniec setup()


void loop() {
    if (configIsContinuousMode()) {
        // --- TRYB CIĄGŁY ---
        // Obsługa Blynk
        if(wifiIsConnected()) {
           if(!blynkIsConnected()) {
              Serial.println("Blynk rozłączony w loop(), próba połączenia...");
              blynkConnect(1000);
           }
           blynkRun(); // Obsługa komunikacji Blynk
        } else {
            static unsigned long lastWifiTry = 0;
            if(millis() - lastWifiTry > 60000) { // Próbuj co minutę
                Serial.println("Próba ponownego połączenia z WiFi w loop()...");
                wifiConnect(SECRET_SSID, SECRET_PASS, 5000);
                lastWifiTry = millis();
            }
        }

        pumpControlUpdate();
        // Okresowe wysyłanie danych do Blynk
        uint32_t interval = configGetBlynkSendIntervalSec() * 1000;
        if (interval == 0) interval = 60000;

        if (millis() - lastBlynkSend > interval) {
            Serial.println("\n--- Cykliczny pomiar i wysyłka danych (tryb ciągły) ---");
            digitalWrite(LED_BUILTIN, HIGH);

            // Odczyty sensorów (bez MPU)
            int currentMoisture = soilSensorReadPercent();
            int currentWaterLevel = waterLevelSensorReadLevel();
            float currentBatteryVoltage = batteryMonitorReadVoltage();
            float currentTemperatureDHT;
            float currentHumidityDHT;
            bool dhtOk = environmentSensorRead(currentTemperatureDHT, currentHumidityDHT);

            digitalWrite(LED_BUILTIN, LOW);

            // Wyświetlanie lokalne
            Serial.println("--- Wyniki pomiarów ---");
            if(currentMoisture >= 0) Serial.printf("Wilgotność gleby: %d %%\n", currentMoisture); else Serial.println("Wilgotność gleby: Błąd odczytu");
            Serial.printf("Poziom wody: %d / %d\n", currentWaterLevel, NUM_WATER_LEVELS);
            if(currentBatteryVoltage > 0) Serial.printf("Napięcie baterii: %.2f V\n", currentBatteryVoltage); else Serial.println("Napięcie baterii: Błąd odczytu / Odłączona");
           if (dhtOk && !isnan(currentTemperatureDHT) && !isnan(currentHumidityDHT)) {
                Serial.printf("Temperatura (DHT11): %.1f C\n", currentTemperatureDHT);
                Serial.printf("Wilgotność pow. (DHT11): %.1f %%\n", currentHumidityDHT);
             } else {
                 Serial.println("Temperatura/Wilgotność (DHT11): Błąd odczytu");
             }
            // Usunięto sekcję MPU
            Serial.println("-------------------------------------");

             // Wyślij dane do Blynk (jeśli jest połączenie)
             if (blynkIsConnected()) {
                 blynkSendSensorData(currentMoisture, currentWaterLevel, currentBatteryVoltage,
                                      currentTemperatureDHT, currentHumidityDHT,
                                      NAN, false, pumpControlIsRunning()); // Przekazujemy NAN dla tilt i false dla alertu
                 lastBlynkSend = millis();
             } else {
                  Serial.println("Pomijam wysyłkę - brak połączenia z Blynk.");
             }

            // Kontrola pompy
            pumpControlActivateIfNeeded(currentMoisture, currentWaterLevel);

        } // koniec if (millis() - lastBlynkSend > interval)

    } else {
      // Tryb Deep Sleep - ale pompa może być aktywna
      pumpControlUpdate(); // Aktualizacja stanu pompy
      
      // Jeśli pompa skończyła pracę, przejdź w deep sleep
      if (!pumpControlIsRunning()) {
          Serial.println("Pompa zakończyła pracę, przechodzę do deep sleep...");
          digitalWrite(LED_BUILTIN, HIGH); delay(50); digitalWrite(LED_BUILTIN, LOW);
          blynkDisconnect();
          wifiDisconnect();
          powerManagerGoToDeepSleep();
      } else {
          // Jeśli pompa jeszcze pracuje, pozwól jej działać
          Serial.println("Pompa nadal pracuje...");
          delay(1000); // Krótkie opóźnienie, aby nie generować zbyt wielu logów
      }
  }
} // Koniec loop()