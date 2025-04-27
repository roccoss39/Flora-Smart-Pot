#include <Arduino.h>
#include <Wire.h>
#include <esp_sleep.h>
#include <cmath>

#include <WiFi.h>
#include <WiFiManager.h>

// Nasze moduły
#include "DeviceConfig.h"
#include "secrets.h"         // Potrzebne dla BLYNK_AUTH_TOKEN
#include "BlynkManager.h"
#include "SoilSensor.h"
#include "WaterLevelSensor.h" // Definiuje NUM_WATER_LEVELS
#include "PumpControl.h"
#include "BatteryMonitor.h"
#include "EnvironmentSensor.h"
#include "PowerManager.h"
#include "AlarmManager.h"
#include <Preferences.h>

// --- NOWOŚĆ: Struktura do przechowywania danych z pomiarów ---
struct SensorData {
    int soilMoisture = -1;
    int waterLevel = -1;
    float batteryVoltage = -1.0f;
    float temperature = NAN; // Użyj NAN jako wartość domyślną/błędną dla float
    float humidity = NAN;
    bool dhtOk = false;
    // Można dodać inne odczyty w przyszłości
};

// --- Deklaracje nowych funkcji ---
SensorData performMeasurement();
void displayMeasurements(const SensorData& data);

// Funkcja pomocnicza do drukowania przyczyny wybudzenia
// ... (bez zmian) ...
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

// Czas ostatniego cyklu pomiarowego
unsigned long lastMeasurementTime = 0;

// --- Główna funkcja Setup ---
void setup() {
    Serial.begin(115200);
    Serial.println("\n--- Flaura Smart Pot - Główny Start ---");
    print_wakeup_reason();

    Wire.begin();
    delay(100);

    configSetup(); // Wczytaj konfigurację

    // Inicjalizuj moduły
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    soilSensorSetup();
    waterLevelSensorSetup();
    pumpControlSetup();
    batteryMonitorSetup();
    environmentSensorSetup();
    alarmManagerSetup();

    digitalWrite(LED_BUILTIN, LOW);

    // --- ZAWSZE WYKONAJ PIERWSZY POMIAR ---
    Serial.println("\n--- Pierwszy pomiar/cykl po starcie/wybudzeniu ---");
    SensorData initialData = performMeasurement(); // Wywołaj funkcję pomiarową

    // Aktualizuj stan alarmu na podstawie pierwszego pomiaru
    alarmManagerUpdate(initialData.waterLevel, initialData.batteryVoltage, initialData.soilMoisture);

    // Sprawdź alarm i ewentualnie wymuś tryb ciągły
     if (alarmManagerIsAlarmActive()) {
        Serial.println("[SETUP] Wykryto aktywny alarm!");
        if (!configIsContinuousMode()) {
            Serial.println("[SETUP] Przełączam na tryb ciągły z powodu alarmu...");
            configSetContinuousMode(true);
        } else {
            Serial.println("[SETUP] Już w trybie ciągłym.");
        }
     }

    // Wyświetl wyniki pierwszego pomiaru
    displayMeasurements(initialData); // Wywołaj funkcję wyświetlającą


    // --- Logika połączenia WiFi ---
    bool connectSuccess = false;
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    WiFi.mode(WIFI_STA);
    WiFiManager wm;
    // wm.resetSettings();

    wm.setConnectTimeout(10);
    if (wakeup_reason == ESP_SLEEP_WAKEUP_UNDEFINED) {
        Serial.println("[WiFi] Wykryto Reset/Power-On. Ustawiam timeout portalu na 120s.");
        wm.setConfigPortalTimeout(120);
    } else {
        Serial.println("[WiFi] Wykryto wybudzenie z Deep Sleep. Ustawiam timeout portalu na 0s.");
        wm.setConfigPortalTimeout(0);
    }
    //wm.setWiFiAutoReconnect(true);
    String apName = "Flaura-Wifi-" + String((uint32_t)ESP.getEfuseMac(), HEX);

    Serial.println("Próba połączenia z zapisaną siecią WiFi...");
    if (wm.autoConnect(apName.c_str())) {
        Serial.println("\nPołączono z WiFi!");
        Serial.print("Adres IP: ");
        Serial.println(WiFi.localIP());
        connectSuccess = true;

        blynkConfigure(BLYNK_AUTH_TOKEN, BLYNK_TEMPLATE_ID, BLYNK_DEVICE_NAME);
        if (!blynkConnect()) {
             Serial.println("OSTRZEŻENIE: Nie udało się połączyć z Blynk mimo połączenia WiFi.");
        } else {
             Serial.println("Połączono z Blynk.");
             powerManagerSyncTime();
        }
    } else {
        Serial.println("\nOSTRZEŻENIE: Nie udało się połączyć z WiFi.");
        Serial.println("Przechodzę w tryb OFFLINE.");
        connectSuccess = false;
    }

    // --- Operacje po próbie połączenia ---
    // Wyślij pierwsze dane do Blynk TYLKO jeśli jest połączenie
    if (connectSuccess && blynkIsConnected()) {
        Serial.println("Wysyłanie pierwszych danych do Blynk...");
        // Użyj danych z initialData
        blynkSendSensorData(initialData.soilMoisture, initialData.waterLevel, initialData.batteryVoltage,
                             initialData.temperature, initialData.humidity,
                             NAN, false, pumpControlIsRunning());
        lastMeasurementTime = millis();
    } else {
        Serial.println("Pomijam wysyłkę pierwszych danych - brak połączenia.");
        // Ustaw czas ostatniego pomiaru, nawet jeśli nie wysłano
         lastMeasurementTime = millis();
    }

    // Kontrola pompy na podstawie pierwszego pomiaru - ZAWSZE
    // Użyj danych z initialData
    pumpControlActivateIfNeeded(initialData.soilMoisture, initialData.waterLevel);

    // --- Przejście w Deep Sleep (zależne od trybu) ---
    if (!configIsContinuousMode()) {
        if (pumpControlIsRunning()) {
            Serial.println("Pompa pracuje - pozostaję w trybie aktywnym (przejście do loop).");
        } else {
            digitalWrite(LED_BUILTIN, HIGH); delay(50); digitalWrite(LED_BUILTIN, LOW);
            Serial.println("Konfiguruję wybudzanie i przechodzę w Deep Sleep...");
            if (blynkIsConnected()) {
                blynkDisconnect();
            }
            powerManagerGoToDeepSleep();
        }
    } else {
        Serial.println("Tryb ciągły aktywny. Dalsza praca w loop().");
    }

} // Koniec setup()

// --- Główna pętla Loop ---
void loop() {
    // Zmienne statyczne do przechowywania ostatnich odczytów dla alarmu
    static int lastMoisture = -1;
    static int lastWaterLevel = -1;
    static float lastBatteryVoltage = -1.0f;

    // --- ZAWSZE WYKONYWANE ---
    pumpControlUpdate();

    // --- OBSŁUGA SIECI (jeśli jest połączenie WiFi) ---
    if (WiFi.status() == WL_CONNECTED) {
       if (!blynkIsConnected()) {
          static unsigned long lastBlynkTry = 0;
          if (millis() - lastBlynkTry > 15000) {
              Serial.println("[Loop] Blynk rozłączony (WiFi jest), próba połączenia...");
              blynkConnect(1000);
              lastBlynkTry = millis();
          }
       }
       blynkRun();
    }

    // --- GŁÓWNA LOGIKA CYKLICZNA (Pomiar, Sterowanie) ---
    if (configIsContinuousMode()) {
        // --- Tryb Ciągły ---
        uint32_t interval = configGetBlynkSendIntervalSec() * 1000;
        if (interval == 0) interval = 60000;

        // Sprawdź, czy nadszedł czas na kolejny cykl pomiarowy
        if (millis() - lastMeasurementTime > interval) {

            // --- Krok 1: Odczytaj sensory (niezależnie od WiFi) ---
            SensorData currentData = performMeasurement(); // Wywołaj funkcję pomiarową

            // Zaktualizuj ostatnie znane wartości (dla alarmu na końcu pętli)
            if (currentData.soilMoisture >= 0) lastMoisture = currentData.soilMoisture;
            lastWaterLevel = currentData.waterLevel;
            if (currentData.batteryVoltage > 0) lastBatteryVoltage = currentData.batteryVoltage;

            // --- Krok 2: Wyświetl wyniki lokalnie (niezależnie od WiFi) ---
            displayMeasurements(currentData); // Wywołaj funkcję wyświetlającą

            // --- Krok 3: Wyślij dane do Blynk (TYLKO jeśli jest połączenie) ---
            if (WiFi.status() == WL_CONNECTED && blynkIsConnected()) {
                 Serial.println("Wysyłanie danych do Blynk...");
                 // Użyj danych z currentData
                 blynkSendSensorData(currentData.soilMoisture, currentData.waterLevel, currentData.batteryVoltage,
                                      currentData.temperature, currentData.humidity,
                                      NAN, false, pumpControlIsRunning());
            } else {
                 Serial.println("Pomijam wysyłkę do Blynk - brak połączenia.");
            }

            // --- Krok 4: Aktywuj pompę jeśli potrzeba (niezależnie od WiFi) ---
            // Użyj danych z currentData
            pumpControlActivateIfNeeded(currentData.soilMoisture, currentData.waterLevel);

            // --- Krok 5: Zaktualizuj czas ostatniego pomiaru ---
            lastMeasurementTime = millis();

        } // koniec if (czas na pomiar)

    } else {
        // --- Tryb Deep Sleep ---
        if (!pumpControlIsRunning()) {
             Serial.println("[Loop] Pompa zakończyła pracę w trybie Deep Sleep, przechodzę do uśpienia...");
             digitalWrite(LED_BUILTIN, HIGH); delay(50); digitalWrite(LED_BUILTIN, LOW);
             if (blynkIsConnected()){
                 blynkDisconnect();
             }
             powerManagerGoToDeepSleep();
        }
    }

    // --- ZAWSZE WYKONYWANE na końcu pętli ---
    // Aktualizacja stanu alarmu na podstawie ostatnich poprawnych odczytów
    alarmManagerUpdate(lastWaterLevel, lastBatteryVoltage, lastMoisture);

    delay(10);

} // Koniec loop()

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// --- NOWOŚĆ: Implementacja funkcji do wykonywania pomiarów ---
SensorData performMeasurement() {
    Serial.println("Rozpoczynam odczyt sensorów...");
    digitalWrite(LED_BUILTIN, HIGH); // Włącz LED na czas pomiaru

    SensorData data; // Struktura na wyniki

    data.soilMoisture = soilSensorReadPercent();
    data.waterLevel = waterLevelSensorReadLevel();
    data.batteryVoltage = batteryMonitorReadVoltage();

    // Odczyt DHT wymaga osobnego traktowania ze względu na dhtOk
    float tempDHT, humDHT;
    data.dhtOk = environmentSensorRead(tempDHT, humDHT);
    if (data.dhtOk) {
        data.temperature = tempDHT;
        data.humidity = humDHT;
    } else {
        data.temperature = NAN; // Upewnij się, że są NAN w razie błędu
        data.humidity = NAN;
    }

    digitalWrite(LED_BUILTIN, LOW); // Wyłącz LED
    Serial.println("Odczyt sensorów zakończony.");
    return data; // Zwróć strukturę z wynikami
}

// --- NOWOŚĆ: Implementacja funkcji do wyświetlania pomiarów ---
void displayMeasurements(const SensorData& data) {
    Serial.println("--- Wyniki pomiarów ---");
    // Wyświetl tryb (można by go dodać do struktury, ale łatwiej pobrać bezpośrednio)
    Serial.printf("  Tryb ciągły: %s (false = Deep Sleep)\n", configIsContinuousMode() ? "TAK" : "NIE");
    // Wyświetl dane ze struktury
    if(data.soilMoisture >= 0) Serial.printf("  Wilgotność gleby: %d %%\n", data.soilMoisture); else Serial.println("  Wilgotność gleby: Błąd odczytu");
    Serial.printf("  Poziom wody: %d / %d\n", data.waterLevel, NUM_WATER_LEVELS);
    if(data.batteryVoltage > 0) Serial.printf("  Napięcie baterii: %.2f V\n", data.batteryVoltage); else Serial.println("  Napięcie baterii: Błąd odczytu / Odłączona");
    if (data.dhtOk && !isnan(data.temperature) && !isnan(data.humidity)) {
        Serial.printf("  Temperatura (DHT): %.1f C\n", data.temperature);
        Serial.printf("  Wilgotność pow. (DHT): %.1f %%\n", data.humidity);
    } else {
         Serial.println("  Temperatura/Wilgotność (DHT): Błąd odczytu");
    }
    Serial.println("-----------------------");
}