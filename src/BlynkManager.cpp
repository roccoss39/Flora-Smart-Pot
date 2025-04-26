#include "BlynkManager.h"
#include "secrets.h"
#include <Arduino.h>
#include "DeviceConfig.h"
#include "PumpControl.h"
#include "AlarmManager.h" // Potrzebny do alarmManagerIsAlarmActive()
#include <Preferences.h> // Nie jest bezpośrednio potrzebny tutaj, ale często używany razem

// --- Konfiguracja Wirtualnych Pinów Blynk ---
#define BLYNK_VPIN_SOIL         V0
#define BLYNK_VPIN_WATER_LEVEL  V1
#define BLYNK_VPIN_BATTERY      V2
#define BLYNK_VPIN_TEMPERATURE  V3
#define BLYNK_VPIN_HUMIDITY     V4
#define BLYNK_VPIN_TILT_ANGLE   V5  // Aktualnie nieużywane w main.cpp
#define BLYNK_VPIN_TILT_ALERT   V6  // Aktualnie nieużywane w main.cpp
#define BLYNK_VPIN_PUMP_STATUS  V7
// <<< NOWOŚĆ: VPIN dla wskaźnika ogólnego stanu alarmu >>>
#define BLYNK_VPIN_ALARM_STATUS V8  // WYBIERZ WOLNY PIN! (np. LED widget)

// --- VPINy do sterowania ---
#define BLYNK_VPIN_PUMP_MANUAL  V10 // Przycisk
#define BLYNK_VPIN_PUMP_DURATION V11 // Suwak/Input (ms)
#define BLYNK_VPIN_SOIL_THRESHOLD V12 // Suwak/Input (%)
#define BLYNK_VPIN_ALARM_BAT_THRESHOLD V13 // Suwak/Input (mV) - WYBIERZ WOLNY PIN!
#define BLYNK_VPIN_ALARM_SOIL_THRESHOLD V14 // Suwak/Input (%) - WYBIERZ WOLNY PIN!
#define BLYNK_VPIN_CONTINUOUS_MODE V15 // Przełącznik (Switch)
#define BLYNK_VPIN_ALARM_SOUND_ENABLE V16 // Przełącznik (Switch)

#define BLYNK_VPIN_MEASUREMENT_HOUR V20 // Input/Widget
#define BLYNK_VPIN_MEASUREMENT_MINUTE V21 // Input/Widget

// --- Pozostałe ustawienia Blynk ---
#define BLYNK_PRINT Serial // Przekierowanie logów Blynk
#include <BlynkSimpleEsp32.h>

static char blynkAuthToken[35] = BLYNK_AUTH_TOKEN;
const char* blynk_server = "blynk.cloud";
uint16_t blynk_port = 80;

// --- Implementacje funkcji ---

void blynkConfigure(const char* authToken, const char* templateId, const char* deviceName) {
     strncpy(blynkAuthToken, authToken, sizeof(blynkAuthToken)-1);
     blynkAuthToken[sizeof(blynkAuthToken)-1] = '\0';
     Blynk.config(blynkAuthToken, blynk_server, blynk_port);
}

bool blynkConnect(unsigned long timeoutMs) {
    Serial.print("Łączenie z Blynk...");
    bool result = Blynk.connect(timeoutMs);
    if (result) {
        Serial.println(" Połączono!");
    } else {
        Serial.println(" Błąd połączenia (timeout)!");
    }
    return result;
}

void blynkRun() {
    if (Blynk.connected()) {
        Blynk.run();
    }
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

void blynkSendSensorData(int soil, int waterLvl, float batteryV, float temp, float humid, float tilt, bool tiltAlert, bool pumpStatus) {
    if (!Blynk.connected()) {
        //Serial.println("Blynk nie połączony, pomijam wysyłanie danych.");
        return;
    }

    //Serial.println("Wysyłanie danych do Blynk...");

    if (soil >= 0) Blynk.virtualWrite(BLYNK_VPIN_SOIL, soil);
    Blynk.virtualWrite(BLYNK_VPIN_WATER_LEVEL, waterLvl);
    if (batteryV > 0) Blynk.virtualWrite(BLYNK_VPIN_BATTERY, batteryV);
    if (!isnan(temp)) Blynk.virtualWrite(BLYNK_VPIN_TEMPERATURE, temp);
    if (!isnan(humid)) Blynk.virtualWrite(BLYNK_VPIN_HUMIDITY, humid);
    // Te linie są zachowane, ale main.cpp wysyła NAN i false
    if (!isnan(tilt)) Blynk.virtualWrite(BLYNK_VPIN_TILT_ANGLE, tilt);
    Blynk.virtualWrite(BLYNK_VPIN_TILT_ALERT, tiltAlert ? 1 : 0);
    // ---
    Blynk.virtualWrite(BLYNK_VPIN_PUMP_STATUS, pumpStatus ? 1 : 0);

    // <<< NOWOŚĆ: Wysyłanie ogólnego stanu alarmu >>>
    bool isAlarm = alarmManagerIsAlarmActive(); // Pobierz aktualny stan alarmu
    Blynk.virtualWrite(BLYNK_VPIN_ALARM_STATUS, isAlarm ? 1 : 0); // Wyślij 1 (ON) jeśli alarm jest aktywny, 0 (OFF) w przeciwnym razie

    //Serial.println("Dane wysłane.");
}

// --- Handlery BLYNK_WRITE ---

// Handler dla manualnego włączenia pompy
BLYNK_WRITE(BLYNK_VPIN_PUMP_MANUAL) {
  int value = param.asInt();
  Serial.printf("[Blynk] Otrzymano komendę na V%d (PumpManual): %d\n", BLYNK_VPIN_PUMP_MANUAL, value);
  if (value == 1) {
    Serial.println("[Blynk] Próba manualnego uruchomienia pompy...");
    uint32_t duration = configGetPumpRunMillis();
    pumpControlManualTurnOn(duration);
    Blynk.virtualWrite(BLYNK_VPIN_PUMP_STATUS, pumpControlIsRunning() ? 1 : 0);
  }
}

// Handler dla zmiany czasu pracy pompy
BLYNK_WRITE(BLYNK_VPIN_PUMP_DURATION) {
  uint32_t newDurationMs = param.asInt();
  Serial.printf("[Blynk] Otrzymano nowy czas pracy pompy na V%d: %d ms\n", BLYNK_VPIN_PUMP_DURATION, newDurationMs);
  configSetPumpRunMillis(newDurationMs);
}

// Handler dla zmiany progu wilgotności
BLYNK_WRITE(BLYNK_VPIN_SOIL_THRESHOLD) {
  int newThreshold = param.asInt();
  Serial.printf("[Blynk] Otrzymano nowy próg wilgotności na V%d: %d %%\n", BLYNK_VPIN_SOIL_THRESHOLD, newThreshold);
  configSetSoilThresholdPercent(newThreshold);
}

// Handler dla zmiany godziny pomiaru
BLYNK_WRITE(BLYNK_VPIN_MEASUREMENT_HOUR) {
    int newHour = param.asInt();
    Serial.printf("[Blynk] Otrzymano nową godzinę pomiaru na V%d: %d\n", BLYNK_VPIN_MEASUREMENT_HOUR, newHour);
    int currentMinute = configGetMeasurementMinute();
    configSetMeasurementTime(newHour, currentMinute);
}

// Handler dla zmiany minuty pomiaru
BLYNK_WRITE(BLYNK_VPIN_MEASUREMENT_MINUTE) {
    int newMinute = param.asInt();
    Serial.printf("[Blynk] Otrzymano nową minutę pomiaru na V%d: %d\n", BLYNK_VPIN_MEASUREMENT_MINUTE, newMinute);
    int currentHour = configGetMeasurementHour();
    configSetMeasurementTime(currentHour, newMinute);
}

// Handler dla przełącznika trybu ciągłego
BLYNK_WRITE(BLYNK_VPIN_CONTINUOUS_MODE) {
    bool isContinuous = param.asInt() == 1;
    Serial.printf("[Blynk] Otrzymano komendę zmiany trybu na V%d: %s\n",
                  BLYNK_VPIN_CONTINUOUS_MODE, isContinuous ? "Ciągły (WŁ)" : "Deep Sleep (WYŁ)");
    configSetContinuousMode(isContinuous);
}

// Handler dla przełącznika dźwięku alarmu
BLYNK_WRITE(BLYNK_VPIN_ALARM_SOUND_ENABLE) {
    bool soundEnabled = param.asInt() == 1;
    Serial.printf("[Blynk] Otrzymano komendę zmiany dźwięku alarmu na V%d: %s\n",
                  BLYNK_VPIN_ALARM_SOUND_ENABLE, soundEnabled ? "Włączony" : "Wyłączony");
    configSetAlarmSoundEnabled(soundEnabled);
}


// --- Handler BLYNK_CONNECTED ---

BLYNK_CONNECTED() {
    Serial.println("[Blynk] Połączono z serwerem. Synchronizuję widgety...");
    // Synchronizuj stan widgetów sterujących
    Blynk.syncVirtual(BLYNK_VPIN_PUMP_DURATION);
    Blynk.syncVirtual(BLYNK_VPIN_SOIL_THRESHOLD);
    Blynk.syncVirtual(BLYNK_VPIN_MEASUREMENT_HOUR);
    Blynk.syncVirtual(BLYNK_VPIN_MEASUREMENT_MINUTE);
    Blynk.syncVirtual(BLYNK_VPIN_ALARM_SOUND_ENABLE);
    Blynk.syncVirtual(BLYNK_VPIN_ALARM_BAT_THRESHOLD);
    Blynk.syncVirtual(BLYNK_VPIN_ALARM_SOIL_THRESHOLD);

    if (!alarmManagerIsAlarmActive())
    {
      Serial.printf("[Blynk] Brak aktywnego alarmu. Synchronizuję stan V%d (ContinuousMode) z serwera...\n", BLYNK_VPIN_CONTINUOUS_MODE);
      Blynk.syncVirtual(BLYNK_VPIN_CONTINUOUS_MODE);
    }
    else
    {
      Serial.printf("[Blynk] Alarm jest AKTYWNY! Wysyłam aktualny stan urządzenia (%s) do V%d (ContinuousMode)...\n", configIsContinuousMode() ? "CIĄGŁY" : "DEEP SLEEP", BLYNK_VPIN_CONTINUOUS_MODE);
      Blynk.virtualWrite(BLYNK_VPIN_CONTINUOUS_MODE, configIsContinuousMode());
    }

    // Aktualizuj wartości widgetów na podstawie bieżącej konfiguracji
    Blynk.virtualWrite(BLYNK_VPIN_PUMP_DURATION, configGetPumpRunMillis());
    Blynk.virtualWrite(BLYNK_VPIN_SOIL_THRESHOLD, configGetSoilThresholdPercent());
    Blynk.virtualWrite(BLYNK_VPIN_MEASUREMENT_HOUR, configGetMeasurementHour());
    Blynk.virtualWrite(BLYNK_VPIN_MEASUREMENT_MINUTE, configGetMeasurementMinute());
    Blynk.virtualWrite(BLYNK_VPIN_ALARM_SOUND_ENABLE, configIsAlarmSoundEnabled() ? 1 : 0);

    Blynk.virtualWrite(BLYNK_VPIN_ALARM_BAT_THRESHOLD, configGetLowBatteryMilliVolts());
    Blynk.virtualWrite(BLYNK_VPIN_ALARM_SOIL_THRESHOLD, configGetLowSoilPercent());

    // <<< NOWOŚĆ: Aktualizuj wskaźnik alarmu przy połączeniu >>>
    Blynk.virtualWrite(BLYNK_VPIN_ALARM_STATUS, alarmManagerIsAlarmActive() ? 1 : 0); //
    Blynk.virtualWrite(BLYNK_VPIN_PUMP_STATUS, pumpControlIsRunning() ? 1 : 0);
    Serial.println("[Blynk] Synchronizacja zakończona.");
}

// --- Funkcje pomocnicze do aktualizacji stanu w Blynk ---

void blynkUpdatePumpStatus(bool isRunning) {
    if (Blynk.connected()) {
        Blynk.virtualWrite(BLYNK_VPIN_PUMP_STATUS, isRunning ? 1 : 0);
    }
}

BLYNK_WRITE(BLYNK_VPIN_ALARM_BAT_THRESHOLD) {
  int newThresholdMv = param.asInt();
  Serial.printf("[Blynk] Otrzymano nowy próg alarmu baterii na V%d: %d mV\n", BLYNK_VPIN_ALARM_BAT_THRESHOLD, newThresholdMv);
  configSetLowBatteryMilliVolts(newThresholdMv);
}

// Handler dla zmiany progu alarmu wilgotności gleby (%)
BLYNK_WRITE(BLYNK_VPIN_ALARM_SOIL_THRESHOLD) {
  int newThresholdPercent = param.asInt();
  Serial.printf("[Blynk] Otrzymano nowy próg alarmu wilg. gleby na V%d: %d %%\n", BLYNK_VPIN_ALARM_SOIL_THRESHOLD, newThresholdPercent);
  configSetLowSoilPercent(newThresholdPercent);
}