/**
 * @file main.cpp
 * @brief Main program file for Flora Smart Pot
 * @version 1.0
 * @date 2025-05-01
 * 
 * Program controlling smart plant pot with moisture monitoring
 * and irrigation management.
 */

 #include <Arduino.h>
 #include <Wire.h>
 #include <esp_sleep.h>
 #include <cmath>
 #include <WiFi.h>
 #include <WiFiManager.h>
 #include <HTTPClient.h>
 #include <ArduinoJson.h>

 #if __has_include("secrets.h")
 #include "secrets.h"
 #endif
 
 // System modules
 #include "DeviceConfig.h"
 #include "SoilSensor.h"
 #include "WaterLevelSensor.h"
 #include "PumpControl.h"
 #include "BatteryMonitor.h"
 #include "EnvironmentSensor.h"
 #include "PowerManager.h"
 #include "AlarmManager.h"
 #include "ButtonManager.h"
 #include "LedManager.h"
 #include "BackendTasks.h"
 #include <Preferences.h>
 #include "test.h"  
 

 // Configuration constants

 constexpr uint16_t WEBPORTAL_TIMEOUT_SEC = 120;
 constexpr uint8_t WIFI_CONNECTION_TIMEOUT_SEC = 10;

 #ifndef FLORA_BACKEND_BASE_URL
 #define FLORA_BACKEND_BASE_URL "http://127.0.0.1:8080"
 #endif

 #ifndef FLORA_BACKEND_TOKEN
 #define FLORA_BACKEND_TOKEN "replace_me"
 #endif

 #ifndef FLORA_BACKEND_DEVICE_ID
 #define FLORA_BACKEND_DEVICE_ID "flora-1"
 #endif
 
 /**
  * @struct SensorData
  * @brief Structure storing data from all sensors
  */
 struct SensorData {
     int soilMoisture = -1;
     int waterLevel = -1;
     float batteryVoltage = -1.0f;
     float temperature = NAN;
     float humidity = NAN;
     bool dhtOk = false;
     
     /**
      * @brief Checks if data is valid
      * @return true if basic data is available
      */
     bool isValid() const {
         return soilMoisture >= 0 && waterLevel >= 0 && batteryVoltage > 0;
     }
 };
 
 // Local static variables
 namespace {
     SensorData g_latestSensorData;
     unsigned long g_lastMeasurementTime = 0;
     bool g_isMeasuring = false;
     bool g_isConnectingWifi = false;
 }

 // Function declarations
 SensorData performMeasurement();
 void displayMeasurements(const SensorData& data);
 void print_wakeup_reason();
 void updateLedBasedOnState();
 bool setupWiFiConnection();
 void handleMeasurementCycle();
 void setMeasuringStatus(bool isActive);
 void setConnectingWifiStatus(bool isActive);
 bool backendSendTelemetry(const SensorData& data);

 /**
  * @brief Device configuration at startup
  */
 void setup() {
     Serial.begin(115200);
     delay(100);
   
     //clearPreferencesData("flaura_cfg_1"); // comment in normal mode
     Serial.println(F("\n--- Flora Smart Pot - Main Start ---"));
     print_wakeup_reason();
    
     // I2C initialization
     Wire.begin();
     delay(100);
     
     // Load configuration
     configSetup();

     backendTasksSetup();

     testPrintConfig();

     // Module initialization
     ledManagerSetup(configGetLedPin(), HIGH);
     soilSensorSetup();
     waterLevelSensorSetup();
     pumpControlSetup();
     batteryMonitorSetup();
     environmentSensorSetup();
     alarmManagerSetup();
     buttonSetup();
 
     ledManagerBlink(100);  // Sygnalizacja inicjalizacji
 
     // Pierwszy pomiar po uruchomieniu
     Serial.println(F("\n--- Pierwszy pomiar po starcie ---"));
     g_latestSensorData = performMeasurement();
 
     // Aktualizacja stanu alarmu na podstawie pierwszego pomiaru
     alarmManagerUpdate(
         g_latestSensorData.waterLevel, 
         g_latestSensorData.batteryVoltage, 
         g_latestSensorData.soilMoisture
     );
 
     if (alarmManagerIsAlarmActive()) {
         Serial.println(F("[SETUP] Wykryto aktywny alarm!"));
     }
 
     // Display measurement results
     displayMeasurements(g_latestSensorData);
 
     // Konfiguracja sieci WiFi
     bool wifiConnected = setupWiFiConnection();
     
     // Operations after WiFi connection attempt
     if (wifiConnected) {
         Serial.println(F("Połączenie WiFi aktywne (Blynk wyłączony w main)."));
         
         // 1. Najpierw wysyłamy obecny stan czujników
         backendSendTelemetry(g_latestSensorData);
         
         // 2. NOWOŚĆ: Pobieramy ustawienia z apki (Tryb ciągły, czas pompy itd.)!
         // To nadpisze stare ustawienia w pamięci Flash.
         fetchAndApplyConfiguration(); 
         
     } else {
         Serial.println(F("Pomijam wysyłkę pierwszych danych - brak połączenia WiFi."));         
     }
     
     g_lastMeasurementTime = millis();

     // 3. Sprawdzamy ręczne komendy (np. "Podlej teraz")
     fetchAndExecuteCommands(g_latestSensorData.waterLevel);

     // Kontrola pompy na podstawie pierwszego pomiaru
     pumpControlActivateIfNeeded(g_latestSensorData.soilMoisture, g_latestSensorData.waterLevel);
 
     // Decision about operation mode (active/sleep)
     // UWAGA: configIsContinuousMode() teraz zwróci świeżutką wartość, którą pobraliśmy 20 linijek wyżej!
     const bool shouldSleep = !configIsContinuousMode() && 
                             !alarmManagerIsAlarmActive() && 
                             !pumpControlIsRunning();
                             
     if (shouldSleep) {
         ledManagerTurnOff();
         Serial.println(F("Konfiguruję wybudzanie i przechodzę w Deep Sleep..."));
         powerManagerGoToDeepSleep();
     } else {
         if (pumpControlIsRunning()) {
             Serial.println(F("Pompa pracuje - pozostaję w trybie aktywnym."));
         } else {
             Serial.println(F("Tryb ciągły aktywny. Dalsza praca w loop()."));
         }
     }
 }

 
 /**
  * @brief Main program loop
  */
 void loop() {
     // Update basic components
     ledManagerUpdate();
     pumpControlUpdate();
     
     // Network handling
     if (WiFi.status() == WL_CONNECTED) {
         // WiFi connected; cloud handling moved out of main (Blynk disabled here)
         fetchAndApplyConfiguration();
         fetchAndExecuteCommands(g_latestSensorData.waterLevel);
     } else if (!alarmManagerIsAlarmActive() && !pumpControlIsRunning()) {
         Serial.println(F("Brak aktywnego alarmu oraz połączenia z siecią - włączam tryb uśpienia"));
         ledManagerTurnOff();
         configSetContinuousMode(false);
     }
 
     // Handle measurement cycles in continuous mode
     if (configIsContinuousMode()) {
         uint32_t interval = configGetBlynkSendIntervalSec() * 1000;
         if (interval == 0) interval = 60000;  // Default interval 60s
 
         // Check if it's time for measurement
         if ((millis() - g_lastMeasurementTime > interval) || buttonWasPressed()) {
             handleMeasurementCycle();
         }
     } else {
         // Handle Deep Sleep mode
         if (!pumpControlIsRunning() && !alarmManagerIsAlarmActive()) {
             Serial.println(F("[Loop] Pompa zakończyła pracę w trybie Deep Sleep, przechodzę do uśpienia..."));
             ledManagerTurnOff();
             powerManagerGoToDeepSleep();
         }
     }
        
     // Aktualizacja stanu alarmu
     bool alarmStateChanged = alarmManagerUpdate(
         g_latestSensorData.waterLevel, 
         g_latestSensorData.batteryVoltage, 
         g_latestSensorData.soilMoisture
     );
     
     // Log alarm state change (cloud integration currently disabled in main)
     if (alarmStateChanged) {
         Serial.printf("[Loop] Zmiana stanu alarmu: %s\n", alarmManagerIsAlarmActive() ? "AKTYWNY" : "NIEAKTYWNY");
         backendSendTelemetry(g_latestSensorData);
     }
     
     updateLedBasedOnState();
     delay(10);  // Small delay for loop stability
 }
 
 /**
  * @brief Konfiguracja i próba połączenia z siecią WiFi
  * @return true jeśli połączenie udane
  */
 bool setupWiFiConnection() {
     bool connectSuccess = false;
     esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  
     WiFi.mode(WIFI_STA);
     WiFiManager wm;
     wm.setConnectTimeout(WIFI_CONNECTION_TIMEOUT_SEC);
     
     // Konfiguracja portalu na podstawie przyczyny wybudzenia
     if (wakeup_reason == ESP_SLEEP_WAKEUP_UNDEFINED) {
         // Reset lub Power-On
         Serial.println(F("[WiFi] Wykryto Reset/Power-On. Ustawiam timeout portalu."));
         wm.setConfigPortalTimeout(WEBPORTAL_TIMEOUT_SEC);
     } else {
         // Wybudzenie z Deep Sleep
         Serial.println(F("[WiFi] Wykryto wybudzenie z Deep Sleep. Wyłączam portal konfiguracyjny."));
         wm.setConfigPortalTimeout(0);
         wm.setEnableConfigPortal(false);
     }
  
     wm.setWiFiAutoReconnect(true);
  
     // Nazwa sieci AP
     String apName = "Flaura-Wifi-" + String((uint32_t)ESP.getEfuseMac(), HEX);
  
     Serial.println(F("Próba auto-połączenia z zapisaną siecią WiFi..."));
     setConnectingWifiStatus(true);
     
     if (wm.autoConnect(apName.c_str())) {
         Serial.println(F("\nPołączono z WiFi!"));
         Serial.print(F("Adres IP: "));
         Serial.println(WiFi.localIP());
         connectSuccess = true;
         
         // Synchronizacja czasu po udanym połączeniu WiFi
         powerManagerSyncTime();
     } else {
         Serial.println(F("\nOSTRZEŻENIE: Nie udało się połączyć z WiFi lub skonfigurować."));
         Serial.println(F("Przechodzę w tryb OFFLINE."));
         connectSuccess = false;
     }
     
     setConnectingWifiStatus(false);
     return connectSuccess;
 }
 
 /**
  * @brief Wykonanie pełnego cyklu pomiarowego
  */
void handleMeasurementCycle() {
     // Read sensors
     g_latestSensorData = performMeasurement();
     
     // Display results
     displayMeasurements(g_latestSensorData);
 
     backendSendTelemetry(g_latestSensorData);
     
     Serial.print(F("Stan alarmu: "));
     Serial.println(alarmManagerIsAlarmActive());
 
     // Kontrola pompy
     pumpControlActivateIfNeeded(g_latestSensorData.soilMoisture, g_latestSensorData.waterLevel);
 
     // Aktualizacja czasu ostatniego pomiaru
     g_lastMeasurementTime = millis();
}

/**
 * @brief Wysyła telemetry snapshot do backendu mobile_backend
 */
bool backendSendTelemetry(const SensorData& data) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println(F("[Backend] Brak WiFi - pomijam telemetry push."));
        return false;
    }

    HTTPClient http;
    const String url = String(FLORA_BACKEND_BASE_URL) + "/api/flora/" + FLORA_BACKEND_DEVICE_ID + "/telemetry";
    http.begin(url);
    http.setTimeout(3000);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", String("Bearer ") + FLORA_BACKEND_TOKEN);

    const bool dhtValid = !isnan(data.temperature) && !isnan(data.humidity);
    const float temp = dhtValid ? data.temperature : 0.0f;
    const float humid = dhtValid ? data.humidity : 0.0f;

    char payload[512];
    snprintf(
        payload,
        sizeof(payload),
        "{\"snapshot\":{\"soilMoisturePercent\":%d,\"waterLevel\":%d,\"batteryVoltage\":%.2f,\"temperature\":%.2f,"
        "\"humidity\":%.2f,\"pumpRunning\":%s,\"alarmActive\":%s,\"updatedAt\":\"\"}}",
        data.soilMoisture,
        data.waterLevel,
        data.batteryVoltage,
        temp,
        humid,
        pumpControlIsRunning() ? "true" : "false",
        alarmManagerIsAlarmActive() ? "true" : "false");

    const int httpCode = http.POST((uint8_t*)payload, strlen(payload));
    if (httpCode > 0) {
        String response = http.getString();
        Serial.printf("[Backend] Telemetry HTTP %d\n", httpCode);
        if (httpCode >= 200 && httpCode < 300) {
            http.end();
            return true;
        }
        Serial.printf("[Backend] Odpowiedź: %s\n", response.c_str());
    } else {
        Serial.printf("[Backend] Błąd POST: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
    return false;
}
 
 /**
  * @brief Odczyt wszystkich sensorów
  * @return Struktura z danymi pomiarowymi
  */
 SensorData performMeasurement() {
     setMeasuringStatus(true);
 
     SensorData data;
 
     // Soil moisture measurement
     data.soilMoisture = soilSensorReadPercent();
     
     // Pomiar poziomu wody
     data.waterLevel = waterLevelSensorReadLevel();
     
     // Battery voltage measurement
     data.batteryVoltage = batteryMonitorReadVoltage();
 
     // Air temperature and humidity measurement
     float tempDHT, humDHT;
     data.dhtOk = environmentSensorRead(tempDHT, humDHT);
     
     if (data.dhtOk) {
         data.temperature = tempDHT;
         data.humidity = humDHT;
     } else {
         data.temperature = NAN;
         data.humidity = NAN;
     }
 
     setMeasuringStatus(false);
     Serial.println(F("Odczyt sensorów zakończony."));
     
     return data;
 }
 
 /**
  * @brief Wyświetla dane pomiarowe w konsoli
  * @param data Struktura z wynikami pomiarów
  */
 void displayMeasurements(const SensorData& data) {
     Serial.println(F("--- Wyniki pomiarów ---"));
     
     // Informacja o trybie pracy
     Serial.printf("  Tryb ciągły: %s (false = Deep Sleep)\n", configIsContinuousMode() ? "TAK" : "NIE");
     
     // Display sensor data
     if (data.soilMoisture >= 0) {
         Serial.printf("  Wilgotność gleby: %d %%\n", data.soilMoisture);
     } else {
         Serial.println(F("  Wilgotność gleby: Błąd odczytu"));
     }
     
     Serial.printf("  Poziom wody: %d / %d\n", data.waterLevel, NUM_WATER_LEVELS);
     
     if (data.batteryVoltage > 0) {
         Serial.printf("  Napięcie baterii: %.2f V\n", data.batteryVoltage);
     } else {
         Serial.println(F("  Napięcie baterii: Błąd odczytu / Odłączona"));
     }
     
     if (data.dhtOk && !isnan(data.temperature) && !isnan(data.humidity)) {
         Serial.printf("  Temperatura (DHT): %.1f C\n", data.temperature);
         Serial.printf("  Wilgotność pow. (DHT): %.1f %%\n", data.humidity);
     } else {
         Serial.println(F("  Temperatura/Wilgotność (DHT): Błąd odczytu"));
     }
     
     Serial.println(F("-----------------------"));
 }
 
 /**
  * @brief Wyświetla przyczynę uruchomienia ESP
  */
 void print_wakeup_reason() {
     esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
     
     Serial.print(F("Przyczyna uruchomienia/wybudzenia: "));
     
     switch (wakeup_reason) {
         case ESP_SLEEP_WAKEUP_EXT0:
             Serial.println(F("EXT0 (zewnętrzne przerwanie 0)"));
             break;
         case ESP_SLEEP_WAKEUP_EXT1:
             Serial.println(F("EXT1 (zewnętrzne przerwanie 1)"));
             break;
         case ESP_SLEEP_WAKEUP_TIMER:
             Serial.println(F("Timer"));
             break;
         case ESP_SLEEP_WAKEUP_TOUCHPAD:
             Serial.println(F("Touchpad"));
             break;
         case ESP_SLEEP_WAKEUP_ULP:
             Serial.println(F("ULP Program"));
             break;
         default:
             Serial.printf("Reset lub Power On (przyczyna nr %d)\n", wakeup_reason);
             break;
     }
 }
 
 /**
  * @brief Ustawia flagę stanu pomiaru
  * @param isActive true jeśli pomiar jest aktywny
  */
 void setMeasuringStatus(bool isActive) {
     g_isMeasuring = isActive;
     updateLedBasedOnState();
     
     if (isActive) {
         Serial.println(F("[Status] Rozpoczęto pomiar"));
     }
 }
 
 /**
  * @brief Ustawia flagę stanu połączenia WiFi
  * @param isActive true jeśli łączenie jest w trakcie
  */
 void setConnectingWifiStatus(bool isActive) {
     g_isConnectingWifi = isActive;
     updateLedBasedOnState();
     
     if (isActive) {
         Serial.println(F("[Status] Rozpoczęto łączenie z WiFi"));
     } else {
         Serial.println(F("[Status] Zakończono łączenie z WiFi"));
     }
 }
 
 /**
  * @brief Aktualizuje stan diody LED na podstawie stanu systemu
  */
 void updateLedBasedOnState() {
     if (g_isMeasuring || g_isConnectingWifi) {
         ledManagerSetState(LED_ON);  // Priority: measurement/connecting
     } else if (alarmManagerIsAlarmActive()) {
         ledManagerSetState(LED_BLINKING_FAST);  // Alarm
     } else {
         ledManagerSetState(LED_OFF);  // Normalny stan
     }
 }
