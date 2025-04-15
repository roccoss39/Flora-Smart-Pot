#include <Arduino.h>
#include "DeviceConfig.h"     // Zarządzanie konfiguracją
#include "SoilSensor.h"       // Moduł czujnika wilgotności
#include "WaterLevelSensor.h" // Moduł czujnika poziomu wody
#include "PowerManager.h"     // Zarządzanie energią (Deep Sleep)

// W przyszłości dodaj include dla innych modułów:
// #include "DHTSensor.h"
// #include "MQTTManager.h"

void setup() {
    Serial.begin(115200);
    // while (!Serial); // Odkomentuj, jeśli chcesz czekać na otwarcie monitora
    Serial.println("\n--- Flaura Smart Pot - Główny Start ---");

    // 1. Wczytaj konfigurację (lub zapisz domyślną)
    configSetup();

    // 2. Zainicjalizuj moduły/czujniki
    soilSensorSetup();
    waterLevelSensorSetup();
    // W przyszłości:
    // dhtSensorSetup();
    // mqttManagerSetup();

    // 3. Wykonaj główną logikę cyklu (tylko jeśli NIE w trybie ciągłym)
    if (!configIsContinuousMode()) {
        Serial.println("Rozpoczynam cykl pomiarowy (Tryb Deep Sleep)...");

        int currentMoisture = soilSensorReadPercent();
        int currentWaterLevel = waterLevelSensorReadLevel();
        // W przyszłości:
        // float currentTemp = dhtSensorReadTemperature();
        // float currentHumidity = dhtSensorReadHumidity();

        Serial.println("--- Wyniki pomiarów ---");
        Serial.printf("Wilgotność gleby: %d %%\n", currentMoisture);
        Serial.printf("Poziom wody: %d / %d\n", currentWaterLevel, NUM_WATER_LEVELS); // Używamy stałej z WaterLevelSensor.h
        // ... (wyświetlanie innych wyników) ...
        Serial.println("-----------------------");


        // Tutaj miejsce na logikę wysyłania danych (np. MQTT)
        // if (/* Połączono z WiFi/MQTT */) {
        //    mqttManagerPublish(currentMoisture, currentWaterLevel, ...);
        // }

        // Przejdź w Deep Sleep
        powerManagerGoToDeepSleep();
    } else {
         Serial.println("Uruchomiono w trybie ciągłym. Pomiary będą wykonywane w pętli loop().");
    }
}

void loop() {
    // Kod w pętli loop() wykonuje się tylko w trybie ciągłym
    if (configIsContinuousMode()) {
        delay(5000); // Odstęp między pomiarami w trybie ciągłym
        Serial.println("\n--- Kolejny pomiar (tryb ciągły) ---");

        int currentMoisture = soilSensorReadPercent();
        int currentWaterLevel = waterLevelSensorReadLevel();
        // W przyszłości odczytaj inne czujniki...

        Serial.println("--- Wyniki pomiarów ---");
        Serial.printf("Wilgotność gleby: %d %%\n", currentMoisture);
        Serial.printf("Poziom wody: %d / %d\n", currentWaterLevel, NUM_WATER_LEVELS); // Używamy stałej z WaterLevelSensor.h
         // ... (wyświetlanie innych wyników) ...
        Serial.println("-------------------------------------");
    } else {
        // W trybie Deep Sleep ta pętla jest praktycznie nieużywana.
        // Można dodać tu np. miganie diodą co sekundę, aby pokazać, że ESP nieoczekiwanie tu trafił.
        delay(1000);
    }
}