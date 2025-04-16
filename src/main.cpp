#include <Arduino.h>
#include "DeviceConfig.h"
#include "SoilSensor.h"
#include "WaterLevelSensor.h" // Używa stałej NUM_WATER_LEVELS z tego pliku
#include "PowerManager.h"
#include "PumpControl.h"
#include "BatteryMonitor.h"

// Przyszłe include'y dla komunikacji
// #include "WiFiManager.h"
// #include "MQTTManager.h"

void setup() {
    Serial.begin(115200);
    Serial.println("\n--- Flaura Smart Pot - Główny Start ---");

    configSetup();

    soilSensorSetup();
    waterLevelSensorSetup();
    pumpControlSetup();
    batteryMonitorSetup();
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

        int currentMoisture = soilSensorReadPercent();
        int currentWaterLevel = waterLevelSensorReadLevel();
        float currentBatteryVoltage = batteryMonitorReadVoltage();

        Serial.println("--- Wyniki pomiarów ---");
        Serial.printf("Wilgotność gleby: %d %%\n", currentMoisture);
        Serial.printf("Poziom wody: %d / %d\n", currentWaterLevel, NUM_WATER_LEVELS);
        Serial.printf("Napięcie baterii: %.2f V\n", currentBatteryVoltage); // TODO: DODAC DO DEEP SLEEP
        Serial.println("-----------------------");

        // Uruchom pompę jeśli potrzeba (tylko w trybie auto - do dodania)
        pumpControlActivateIfNeeded(currentMoisture, currentWaterLevel);

    } else {
        // W trybie Deep Sleep ta pętla nie powinna być często osiągana
        delay(1000);
    }
}