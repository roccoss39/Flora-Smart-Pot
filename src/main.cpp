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
#include "ButtonManager.h"
#include "LedManager.h"

// Wbudowana dioda LED
const uint8_t DIODA_PIN = 22; //TO MOVE
#define WEBPORTAL_TIMEOUT 12 //TO CHANGE
#define WIFI_CONNECTION_TIMEOUT 10



struct SensorData {
    int soilMoisture = -1;
    int waterLevel = -1;
    float batteryVoltage = -1.0f;
    float temperature = NAN; // Użyj NAN jako wartość domyślną/błędną dla float
    float humidity = NAN;
    bool dhtOk = false;
    // Można dodać inne odczyty w przyszłości
} g_latestSensorData; 

// --- Deklaracje nowych funkcji ---
SensorData performMeasurement();
void displayMeasurements(const SensorData& data);
void print_wakeup_reason();
void updateLedBasedOnState(); 
void setMeasuringStatus(bool isActive);
void setConnectingWifiStatus(bool isActive);


// Czas ostatniego cyklu pomiarowego
static int unsigned long lastMeasurementTime = 0;
static bool _isMeasuring = false;
static bool _isConnectingWifi = false;

// --- Główna funkcja Setup ---
void setup() {
    Serial.begin(115200);
    delay(100);
    clearPreferencesData("flaura_cfg_1");
    Serial.println("\n--- Flaura Smart Pot - Główny Start ---");
    print_wakeup_reason();

    Wire.begin();
    delay(100);
    
    configSetup(); // Wczytaj konfigurację

    // Inicjalizuj moduły
    ledManagerSetup(DIODA_PIN, HIGH);

    soilSensorSetup();
    waterLevelSensorSetup();
    pumpControlSetup();
    batteryMonitorSetup();
    environmentSensorSetup();
    alarmManagerSetup();
    buttonSetup();

    ledManagerBlink(100); // Mignięcie 100ms

    
    // --- ZAWSZE WYKONAJ PIERWSZY POMIAR ---
    Serial.println("\n--- Pierwszy pomiar/cykl po starcie/wybudzeniu ---");
    g_latestSensorData = performMeasurement();


    // Aktualizuj stan alarmu na podstawie pierwszego pomiaru
    alarmManagerUpdate(g_latestSensorData.waterLevel, g_latestSensorData.batteryVoltage, g_latestSensorData.soilMoisture);

    // Sprawdź alarm i ewentualnie wymuś tryb ciągły
     if (alarmManagerIsAlarmActive()) {
        Serial.println("[SETUP] Wykryto aktywny alarm!");
     }

    // Wyświetl wyniki pierwszego pomiaru
    displayMeasurements(g_latestSensorData); // Wywołaj funkcję wyświetlającą

     // --- Logika połączenia WiFi - POPRAWIONA WERSJA ---
     bool connectSuccess = false;
     esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
 
     WiFi.mode(WIFI_STA);
     WiFiManager wm; // Utwórz obiekt WiFiManager
     wm.resetSettings(); // Odkomentuj do testów resetowania ustawień WiFi
     wm.setConnectTimeout(WIFI_CONNECTION_TIMEOUT); // Czas próby połączenia z zapisaną siecią (sekundy)
     if (wakeup_reason == ESP_SLEEP_WAKEUP_UNDEFINED) {
         // Przyczyna: Reset lub Power-On
         Serial.println("[WiFi] Wykryto Reset/Power-On. Ustawiam timeout portalu na 120s.");
         wm.setConfigPortalTimeout(WEBPORTAL_TIMEOUT); 
        } else {
         // Przyczyna: Wybudzenie z Deep Sleep (Timer, GPIO itp.)
         Serial.println("[WiFi] Wykryto wybudzenie z Deep Sleep. Ustawiam timeout portalu na 0s.");
         wm.setConfigPortalTimeout(0); // Portal NIE powinien uruchomić się automatycznie po nieudanym autoConnect
         wm.setEnableConfigPortal(false);   
        }
 
     wm.setWiFiAutoReconnect(true); // Włącz automatyczne ponowne łączenie w tle
 
     // Nazwa sieci AP, która pojawi się+ (tylko po resecie/power-on), gdy urządzenie nie będzie mogło się połączyć
     String apName = "Flaura-Wifi-" + String((uint32_t)ESP.getEfuseMac(), HEX);
 
     Serial.println("Próba auto-połączenia z zapisaną siecią WiFi...");
     setConnectingWifiStatus(true);
     if (wm.autoConnect(apName.c_str())) {
         // Sukces - połączono z zapisaną siecią LUB skonfigurowano przez portal
         Serial.println("\nPołączono z WiFi!");
         Serial.print("Adres IP: ");
         Serial.println(WiFi.localIP());
         connectSuccess = true;
     } else {
         // Niepowodzenie - nie połączono z zapisaną siecią, a portal (jeśli był aktywny) zakończył się bez sukcesu (timeout/anulowanie)
         // LUB (po deep sleep) - nie połączono z zapisaną siecią, portal nie został uruchomiony.
         Serial.println("\nOSTRZEŻENIE: Nie udało się połączyć z WiFi lub skonfigurować.");
         Serial.println("Przechodzę w tryb OFFLINE.");
         connectSuccess = false;
     }
     setConnectingWifiStatus(false);
     // Dalsza logika zależna od connectSuccess
     if (connectSuccess) {
         Serial.println("Przechodzę do konfiguracji Blynk i synchronizacji czasu...");
         blynkConfigure(BLYNK_AUTH_TOKEN, BLYNK_TEMPLATE_ID, BLYNK_DEVICE_NAME);
         if (!blynkConnect()) {
              Serial.println("OSTRZEŻENIE: Nie udało się połączyć z Blynk mimo połączenia WiFi.");
         } else {
              Serial.println("Połączono z Blynk.");
              powerManagerSyncTime();
         }
     } else {
         Serial.println("Brak połączenia WiFi, pomijam konfigurację Blynk i synchronizację czasu.");
     }
     // --- KONIEC POPRAWIONEJ WERSJI LOGIKI WiFi ---
    // --- Operacje po próbie połączenia ---
    // Wyślij pierwsze dane do Blynk TYLKO jeśli jest połączenie
    if (connectSuccess && blynkIsConnected()) {
        Serial.println("Wysyłanie pierwszych danych do Blynk...");
        // Użyj danych z g_latestSensorData
        blynkSendSensorData(g_latestSensorData.soilMoisture, g_latestSensorData.waterLevel, g_latestSensorData.batteryVoltage,
            g_latestSensorData.temperature, g_latestSensorData.humidity,
            pumpControlIsRunning(), alarmManagerIsAlarmActive());
    } else {
        Serial.println("Pomijam wysyłkę pierwszych danych - brak połączenia.");         
    }
    lastMeasurementTime = millis();
    // Kontrola pompy na podstawie pierwszego pomiaru - ZAWSZE
    // Użyj danych z g_latestSensorData
    pumpControlActivateIfNeeded(g_latestSensorData.soilMoisture, g_latestSensorData.waterLevel);

    // --- Przejście w Deep Sleep (zależne od trybu) ---
    if (!configIsContinuousMode() && !alarmManagerIsAlarmActive()) {
        if (pumpControlIsRunning()) {
            Serial.println("Pompa pracuje - pozostaję w trybie aktywnym (przejście do loop).");
        } else {
            ledManagerTurnOff();
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
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// --- Główna pętla Loop ---
void loop() {
    ledManagerUpdate();
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
    } else if (!alarmManagerIsAlarmActive() && (!pumpControlIsRunning()))   {
        Serial.println("Brak aktywnego alarmu oraz połączenia z siecią - włączam tryb uśpienia");
        ledManagerTurnOff();
        configSetContinuousMode(false);
    }

    // --- GŁÓWNA LOGIKA CYKLICZNA (Pomiar, Sterowanie) ---
    if (configIsContinuousMode()) {
        // --- Tryb Ciągły ---

        uint32_t interval = configGetBlynkSendIntervalSec() * 1000;
        if (interval == 0) interval = 60000;

        // Sprawdź, czy nadszedł czas na kolejny cykl pomiarowy
        if ((millis() - lastMeasurementTime > interval) || buttonWasPressed()) {

            // --- Krok 1: Odczytaj sensory (niezależnie od WiFi) ---
            g_latestSensorData = performMeasurement();
            // Zaktualizuj ostatnie znane wartości (dla alarmu na końcu pętli)
            if (g_latestSensorData.soilMoisture >= 0) g_latestSensorData.soilMoisture = g_latestSensorData.soilMoisture;
            g_latestSensorData.waterLevel = g_latestSensorData.waterLevel;
            if (g_latestSensorData.batteryVoltage > 0) g_latestSensorData.batteryVoltage = g_latestSensorData.batteryVoltage;

            // --- Krok 2: Wyświetl wyniki lokalnie (niezależnie od WiFi) ---
            displayMeasurements(g_latestSensorData); // Wywołaj funkcję wyświetlającą

            // --- Krok 3: Wyślij dane do Blynk (TYLKO jeśli jest połączenie) ---
            if (WiFi.status() == WL_CONNECTED && blynkIsConnected()) {
                 blynkSendSensorData(g_latestSensorData.soilMoisture, g_latestSensorData.waterLevel, g_latestSensorData.batteryVoltage,
                                    g_latestSensorData.temperature, g_latestSensorData.humidity,
                                    pumpControlIsRunning(), alarmManagerIsAlarmActive());
            } else {
                 Serial.println("Pomijam wysyłkę do Blynk - brak połączenia.");
            }
            
            Serial.print("Stan alarmu: ");
            Serial.println(alarmManagerIsAlarmActive());

            // Użyj danych z g_latestSensorData
            pumpControlActivateIfNeeded(g_latestSensorData.soilMoisture, g_latestSensorData.waterLevel);

            // --- Krok 5: Zaktualizuj czas ostatniego pomiaru ---
            lastMeasurementTime = millis();
            
        } 
    } else {
        // --- Tryb Deep Sleep ---
        if (!pumpControlIsRunning() && !alarmManagerIsAlarmActive()) {
             Serial.println("[Loop] Pompa zakończyła pracę w trybie Deep Sleep, przechodzę do uśpienia...");
             ledManagerTurnOff();
             if (blynkIsConnected()){
                 blynkDisconnect();
             }
             powerManagerGoToDeepSleep();
        }
    }
    bool alarmStateChanged = alarmManagerUpdate(g_latestSensorData.waterLevel, g_latestSensorData.batteryVoltage, g_latestSensorData.soilMoisture);
    
    if (alarmStateChanged) {
        blynkSendSensorData(g_latestSensorData.soilMoisture, g_latestSensorData.waterLevel, g_latestSensorData.batteryVoltage,
            g_latestSensorData.temperature, g_latestSensorData.humidity,
            pumpControlIsRunning(), alarmManagerIsAlarmActive());
        
    }
    updateLedBasedOnState();
    delay(10);

} // Koniec loop()

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SensorData performMeasurement() {
    //Serial.println("Rozpoczynam odczyt sensorów...");
    setMeasuringStatus(true);

    SensorData data; // Struktura na wyniki

    data.soilMoisture = soilSensorReadPercent();
    data.waterLevel = waterLevelSensorReadLevel();
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

    setMeasuringStatus(false);
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

  void setMeasuringStatus(bool isActive) {
    _isMeasuring = isActive;
    updateLedBasedOnState();
    
    if (isActive) {
        Serial.println("[Status] Rozpoczęto pomiar");
    }
}

void setConnectingWifiStatus(bool isActive) {
    _isConnectingWifi = isActive;
    updateLedBasedOnState();
    
    if (isActive) {
        Serial.println("[Status] Rozpoczęto łączenie z WiFi");
    } else {
        Serial.println("[Status] Zakończono łączenie z WiFi");
    }
}

// --- Funkcja zarządzająca stanem LED na podstawie flag ---
void updateLedBasedOnState() {
    if (_isMeasuring || _isConnectingWifi) {
        ledManagerSetState(LED_ON); // Priorytet: Włączona podczas pomiaru/łączenia
    } else if (alarmManagerIsAlarmActive()) {
        ledManagerSetState(LED_BLINKING_FAST); // Migaj szybko, jeśli jest alarm
    } else {
        ledManagerSetState(LED_OFF); // W przeciwnym razie zgaś
    }
}