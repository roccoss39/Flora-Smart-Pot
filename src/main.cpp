/**
 * @file main.cpp
 * @brief Główny plik programu Flaura Smart Pot
 * @version 1.0
 * @date 2025-05-01
 * 
 * Program kontrolujący inteligentną doniczkę z funkcjami monitorowania wilgotności,
 * zarządzania nawadnianiem i komunikacją z chmurą Blynk.
 */

 #include <Arduino.h>
 #include <Wire.h>
 #include <esp_sleep.h>
 #include <cmath>
 #include <WiFi.h>
 #include <WiFiManager.h>
 
 // Moduły systemowe
 #include "DeviceConfig.h"
 #include "secrets.h"  // Zawiera BLYNK_AUTH_TOKEN
 #include "BlynkManager.h"
 #include "SoilSensor.h"
 #include "WaterLevelSensor.h"
 #include "PumpControl.h"
 #include "BatteryMonitor.h"
 #include "EnvironmentSensor.h"
 #include "PowerManager.h"
 #include "AlarmManager.h"
 #include "ButtonManager.h"
 #include "LedManager.h"
 #include <Preferences.h>
 
 // Stałe konfiguracyjne
 constexpr uint8_t LED_PIN = 22;
 constexpr uint16_t WEBPORTAL_TIMEOUT_SEC = 1;
 constexpr uint8_t WIFI_CONNECTION_TIMEOUT_SEC = 10;
 constexpr uint16_t BLYNK_RECONNECT_INTERVAL_MS = 15000;
 
 /**
  * @struct SensorData
  * @brief Struktura przechowująca dane ze wszystkich sensorów
  */
 struct SensorData {
     int soilMoisture = -1;
     int waterLevel = -1;
     float batteryVoltage = -1.0f;
     float temperature = NAN;
     float humidity = NAN;
     bool dhtOk = false;
     
     /**
      * @brief Sprawdza czy dane są prawidłowe
      * @return true jeśli podstawowe dane są dostępne
      */
     bool isValid() const {
         return soilMoisture >= 0 && waterLevel >= 0 && batteryVoltage > 0;
     }
 };
 
 // Lokalne zmienne statyczne
 namespace {
     SensorData g_latestSensorData;
     unsigned long g_lastMeasurementTime = 0;
     unsigned long g_lastBlynkReconnectAttempt = 0;
     bool g_isMeasuring = false;
     bool g_isConnectingWifi = false;
 }
 
 // Deklaracje funkcji
 SensorData performMeasurement();
 void displayMeasurements(const SensorData& data);
 void print_wakeup_reason();
 void updateLedBasedOnState();
 bool setupWiFiConnection();
 void handleMeasurementCycle();
 void setMeasuringStatus(bool isActive);
 void setConnectingWifiStatus(bool isActive);
 
 /**
  * @brief Konfiguracja urządzenia przy starcie
  */
 void setup() {
     Serial.begin(115200);
     delay(100);
     
     Serial.println(F("\n--- Flaura Smart Pot - Główny Start ---"));
     print_wakeup_reason();
 
     // Inicjalizacja I2C
     Wire.begin();
     delay(100);
     
     // Wczytanie konfiguracji
     configSetup();
 
     // Inicjalizacja modułów
     ledManagerSetup(LED_PIN, HIGH);
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
 
     // Wyświetlenie wyników pomiaru
     displayMeasurements(g_latestSensorData);
 
     // Konfiguracja sieci WiFi
     bool wifiConnected = setupWiFiConnection();
     
     // Operacje po próbie połączenia WiFi
     if (wifiConnected && blynkIsConnected()) {
         Serial.println(F("Wysyłanie pierwszych danych do Blynk..."));
         blynkSendSensorData(
             g_latestSensorData.soilMoisture, 
             g_latestSensorData.waterLevel, 
             g_latestSensorData.batteryVoltage,
             g_latestSensorData.temperature, 
             g_latestSensorData.humidity,
             pumpControlIsRunning(), 
             alarmManagerIsAlarmActive()
         );
     } else {
         Serial.println(F("Pomijam wysyłkę pierwszych danych - brak połączenia."));         
     }
     
     g_lastMeasurementTime = millis();
     
     // Kontrola pompy na podstawie pierwszego pomiaru
     pumpControlActivateIfNeeded(g_latestSensorData.soilMoisture, g_latestSensorData.waterLevel);
 
     // Decyzja o trybie pracy (aktywny/uśpienie)
     const bool shouldSleep = !configIsContinuousMode() && 
                             !alarmManagerIsAlarmActive() && 
                             !pumpControlIsRunning();
                             
     if (shouldSleep) {
         ledManagerTurnOff();
         Serial.println(F("Konfiguruję wybudzanie i przechodzę w Deep Sleep..."));
         
         if (blynkIsConnected()) {
             blynkDisconnect();
         }
         
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
  * @brief Główna pętla programu
  */
 void loop() {
     // Aktualizacja podstawowych komponentów
     ledManagerUpdate();
     pumpControlUpdate();
     
     // Obsługa sieci
     if (WiFi.status() == WL_CONNECTED) {
         // Próba ponownego połączenia z Blynk jeśli rozłączony
         if (!blynkIsConnected()) {
             unsigned long currentTime = millis();
             if (currentTime - g_lastBlynkReconnectAttempt > BLYNK_RECONNECT_INTERVAL_MS) {
                 Serial.println(F("[Loop] Blynk rozłączony (WiFi jest), próba połączenia..."));
                 blynkConnect(1000);
                 g_lastBlynkReconnectAttempt = currentTime;
             }
         }
         blynkRun();
     } else if (!alarmManagerIsAlarmActive() && !pumpControlIsRunning()) {
         Serial.println(F("Brak aktywnego alarmu oraz połączenia z siecią - włączam tryb uśpienia"));
         ledManagerTurnOff();
         configSetContinuousMode(false);
     }
 
     // Obsługa cykli pomiarowych w trybie ciągłym
     if (configIsContinuousMode()) {
         uint32_t interval = configGetBlynkSendIntervalSec() * 1000;
         if (interval == 0) interval = 60000;  // Domyślny interwał 60s
 
         // Sprawdzenie czy nadszedł czas na pomiar
         if ((millis() - g_lastMeasurementTime > interval) || buttonWasPressed()) {
             handleMeasurementCycle();
         }
     } else {
         // Obsługa trybu Deep Sleep
         if (!pumpControlIsRunning() && !alarmManagerIsAlarmActive()) {
             Serial.println(F("[Loop] Pompa zakończyła pracę w trybie Deep Sleep, przechodzę do uśpienia..."));
             ledManagerTurnOff();
             
             if (blynkIsConnected()) {
                 blynkDisconnect();
             }
             
             powerManagerGoToDeepSleep();
         }
     }
     
     // Aktualizacja stanu alarmu
     bool alarmStateChanged = alarmManagerUpdate(
         g_latestSensorData.waterLevel, 
         g_latestSensorData.batteryVoltage, 
         g_latestSensorData.soilMoisture
     );
     
     // Wysyłka danych do Blynk przy zmianie stanu alarmu
     if (alarmStateChanged && blynkIsConnected()) {
         blynkSendSensorData(
             g_latestSensorData.soilMoisture, 
             g_latestSensorData.waterLevel, 
             g_latestSensorData.batteryVoltage,
             g_latestSensorData.temperature, 
             g_latestSensorData.humidity,
             pumpControlIsRunning(), 
             alarmManagerIsAlarmActive()
         );
     }
     
     updateLedBasedOnState();
     delay(10);  // Małe opóźnienie dla stabilności pętli
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
         
         // Konfiguracja Blynk
         blynkConfigure(BLYNK_AUTH_TOKEN, BLYNK_TEMPLATE_ID, BLYNK_DEVICE_NAME);
         
         if (!blynkConnect()) {
             Serial.println(F("OSTRZEŻENIE: Nie udało się połączyć z Blynk mimo połączenia WiFi."));
         } else {
             Serial.println(F("Połączono z Blynk."));
             powerManagerSyncTime();
         }
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
     // Odczyt sensorów
     g_latestSensorData = performMeasurement();
     
     // Wyświetlenie wyników
     displayMeasurements(g_latestSensorData);
 
     // Wysyłka danych do Blynk
     if (WiFi.status() == WL_CONNECTED && blynkIsConnected()) {
         blynkSendSensorData(
             g_latestSensorData.soilMoisture, 
             g_latestSensorData.waterLevel, 
             g_latestSensorData.batteryVoltage,
             g_latestSensorData.temperature, 
             g_latestSensorData.humidity,
             pumpControlIsRunning(), 
             alarmManagerIsAlarmActive()
         );
     } else {
         Serial.println(F("Pomijam wysyłkę do Blynk - brak połączenia."));
     }
     
     Serial.print(F("Stan alarmu: "));
     Serial.println(alarmManagerIsAlarmActive());
 
     // Kontrola pompy
     pumpControlActivateIfNeeded(g_latestSensorData.soilMoisture, g_latestSensorData.waterLevel);
 
     // Aktualizacja czasu ostatniego pomiaru
     g_lastMeasurementTime = millis();
 }
 
 /**
  * @brief Odczyt wszystkich sensorów
  * @return Struktura z danymi pomiarowymi
  */
 SensorData performMeasurement() {
     setMeasuringStatus(true);
 
     SensorData data;
 
     // Pomiar wilgotności gleby
     data.soilMoisture = soilSensorReadPercent();
     
     // Pomiar poziomu wody
     data.waterLevel = waterLevelSensorReadLevel();
     
     // Pomiar napięcia baterii
     data.batteryVoltage = batteryMonitorReadVoltage();
 
     // Pomiar temperatury i wilgotności powietrza
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
     
     // Wyświetlenie danych sensorów
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
         ledManagerSetState(LED_ON);  // Priorytet: pomiar/łączenie
     } else if (alarmManagerIsAlarmActive()) {
         ledManagerSetState(LED_BLINKING_FAST);  // Alarm
     } else {
         ledManagerSetState(LED_OFF);  // Normalny stan
     }
 }