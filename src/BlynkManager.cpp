#include "BlynkManager.h"
#include "secrets.h" // Dla danych autoryzacyjnych
#include <Arduino.h> // Dla Serial
#include "DeviceConfig.h" // Potrzebny do setterów konfiguracji
#include "PumpControl.h"  // Potrzebny do manualnego sterowania pompą

// --- WAŻNE: Konfiguracja Wirtualnych Pinów Blynk ---
// Ustaw tutaj numery VPIN zgodne z tym, co zdefiniowałeś w szablonie Blynk!
#define BLYNK_VPIN_SOIL         V0
#define BLYNK_VPIN_WATER_LEVEL  V1
#define BLYNK_VPIN_BATTERY      V2
#define BLYNK_VPIN_TEMPERATURE  V3
#define BLYNK_VPIN_HUMIDITY     V4
#define BLYNK_VPIN_TILT_ANGLE   V5
#define BLYNK_VPIN_TILT_ALERT   V6
#define BLYNK_VPIN_PUMP_STATUS  V7
// ----------------------------------------------------
// --- VPINy do sterowania ---
#define BLYNK_VPIN_PUMP_MANUAL  V10 // Przycisk (Button Widget)
#define BLYNK_VPIN_PUMP_DURATION V11 // Suwak/Pole numeryczne (np. Slider, Step, Numeric Input)
#define BLYNK_VPIN_SOIL_THRESHOLD V12 // Suwak/Pole numeryczne

// Przekierowanie logów Blynk do Serial (opcjonalne)
#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp32.h>

// Zmienne do przechowywania danych autoryzacyjnych
static char blynkAuthToken[35] = BLYNK_AUTH_TOKEN; // Zwiększ rozmiar w razie potrzeby

// Serwer Blynk (zazwyczaj domyślny dla chmury)
const char* blynk_server = "blynk.cloud"; // Lub własny serwer
uint16_t blynk_port = 80; // Domyślny port dla TCP

// // Jeśli chcesz używać domyślnych z secrets.h i blynk.cloud:
// #include <BlynkSimpleEsp32.h>

void blynkConfigure(const char* authToken, const char* templateId, const char* deviceName) {
     // Definicje dla Blynk.begin(), jeśli używasz go zamiast config/connect
     // Na razie ta funkcja może pozostać pusta, jeśli używamy stałych z secrets.h
     // Lub można tu skopiować authToken, jeśli przekazujemy go dynamicznie
     strncpy(blynkAuthToken, authToken, sizeof(blynkAuthToken)-1);
     blynkAuthToken[sizeof(blynkAuthToken)-1] = '\0';

     // Ustawienie ID i Nazwy - potrzebne, jeśli nie używasz BLYNK_... w kodzie
     // Blynk.setTemplateId(templateId);
     // Blynk.setDeviceName(deviceName);

     // Konfiguracja połączenia (zamiast Blynk.begin)
     Blynk.config(blynkAuthToken, blynk_server, blynk_port);
}

bool blynkConnect(unsigned long timeoutMs) {
    Serial.print("Łączenie z Blynk...");
    // Blynk.connect() zwraca true, jeśli udało się połączyć w ramach timeout
    bool result = Blynk.connect(timeoutMs);
    if (result) {
        Serial.println(" Połączono!");
    } else {
        Serial.println(" Błąd połączenia (timeout)!");
    }
    return result;
}

void blynkRun() {
    // Obsługuje komunikację Blynk w tle (musi być często wywoływane)
    if (Blynk.connected()) {
        Blynk.run();
    }
    // Można dodać logikę ponownego łączenia, jeśli rozłączono
    // else {
    //    blynkConnect(); // Spróbuj połączyć ponownie (ostrożnie, może blokować)
    // }
}

bool blynkIsConnected() {
    return Blynk.connected();
}

void blynkDisconnect() {
     if (Blynk.connected()) {
        Serial.println("Rozłączanie Blynk...");
        Blynk.disconnect();
        Serial.println("Blynk rozłączony.");
     }
}

// Funkcja wysyłająca dane - dostosuj VPINy na górze pliku!
void blynkSendSensorData(int soil, int waterLvl, float batteryV, float temp, float humid, float tilt, bool tiltAlert, bool pumpStatus) {
    if (!Blynk.connected()) {
        Serial.println("Blynk nie połączony, pomijam wysyłanie danych.");
        return;
    }

    Serial.println("Wysyłanie danych do Blynk...");

    // Sprawdź poprawność wartości przed wysłaniem
    if (soil >= 0) Blynk.virtualWrite(BLYNK_VPIN_SOIL, soil);
    Blynk.virtualWrite(BLYNK_VPIN_WATER_LEVEL, waterLvl);
    if (batteryV > 0) Blynk.virtualWrite(BLYNK_VPIN_BATTERY, batteryV);
    if (!isnan(temp)) Blynk.virtualWrite(BLYNK_VPIN_TEMPERATURE, temp);
    if (!isnan(humid)) Blynk.virtualWrite(BLYNK_VPIN_HUMIDITY, humid);
    if (!isnan(tilt)) Blynk.virtualWrite(BLYNK_VPIN_TILT_ANGLE, tilt);
    Blynk.virtualWrite(BLYNK_VPIN_TILT_ALERT, tiltAlert ? 1 : 0);
    Blynk.virtualWrite(BLYNK_VPIN_PUMP_STATUS, pumpStatus ? 1 : 0);

    Serial.println("Dane wysłane.");
}


BLYNK_WRITE(BLYNK_VPIN_PUMP_MANUAL) {
  int value = param.asInt(); // Wartość z przycisku (zwykle 1 gdy wciśnięty)
  Serial.printf("Otrzymano komendę na V%d: %d\n", BLYNK_VPIN_PUMP_MANUAL, value);

  if (value == 1) {
    // Sprawdź poziom wody przed manualnym włączeniem!
    // Potrzebujemy dostępu do ostatniego odczytu poziomu wody.
    // To jest trudne do zrobienia tutaj w izolowanym module.
    // Lepsze rozwiązanie: Przekaż logikę do PumpControl lub zrób odczyt tutaj.
    // Na razie prostsze: ufamy użytkownikowi lub pomijamy sprawdzanie wody przy manualnym.
    Serial.println("Próba manualnego uruchomienia pompy...");
    uint32_t duration = configGetPumpRunMillis(); // Użyj aktualnie ustawionego czasu
    pumpControlManualTurnOn(duration);
    // Od razu zaktualizuj status pompy w Blynk
    Blynk.virtualWrite(BLYNK_VPIN_PUMP_STATUS, pumpControlIsRunning() ? 1 : 0);
  }
  // Zwykle przycisk nie wysyła 0 przy zwolnieniu, więc nie potrzebujemy else
}

// Handler dla zmiany czasu pracy pompy (VPIN V11)
BLYNK_WRITE(BLYNK_VPIN_PUMP_DURATION) {
  uint32_t newDurationMs = param.asInt(); // Odczytaj wartość z suwaka/pola (w ms)
  Serial.printf("Otrzymano nowy czas pracy pompy na V%d: %d ms\n", BLYNK_VPIN_PUMP_DURATION, newDurationMs);
  configSetPumpRunMillis(newDurationMs); // Użyj settera z DeviceConfig do zapisania wartości
}

// Handler dla zmiany progu wilgotności (VPIN V12)
BLYNK_WRITE(BLYNK_VPIN_SOIL_THRESHOLD) {
  int newThreshold = param.asInt(); // Odczytaj wartość z suwaka/pola (%)
  Serial.printf("Otrzymano nowy próg wilgotności na V%d: %d %%\n", BLYNK_VPIN_SOIL_THRESHOLD, newThreshold);
  configSetSoilThresholdPercent(newThreshold); // Użyj settera z DeviceConfig
}

// Handler wywoływany po połączeniu z Blynk
BLYNK_CONNECTED() {
  Serial.println("Połączono z serwerem Blynk. Synchronizuję ustawienia...");
  // Synchronizuj wartości z serwerem - odczyta aktualne ustawienia suwaków z Blynk
  Blynk.syncVirtual(BLYNK_VPIN_PUMP_DURATION);
  Blynk.syncVirtual(BLYNK_VPIN_SOIL_THRESHOLD);
  // Można też zsynchronizować statusy wyjściowe, np. status pompy
  Blynk.virtualWrite(BLYNK_VPIN_PUMP_STATUS, pumpControlIsRunning() ? 1 : 0);
}