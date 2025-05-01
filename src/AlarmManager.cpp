#include "AlarmManager.h"
#include <Arduino.h>
#include "DeviceConfig.h" // Potrzebne dla configIsAlarmSoundEnabled()

static uint8_t buzzerPin;
static bool isAlarmActive = false; // Ogólny stan alarmu (czy jakikolwiek warunek jest spełniony)
static unsigned long lastBeepCycleTime = 0;
static const unsigned long BEEP_INTERVAL = 10000; // Przerwa między cyklami (ms)
static const unsigned long BEEP_DURATION = 300;  // Długość pojedynczego pikania (ms)
static const unsigned long BEEP_PAUSE = 400;      // Pauza między piknięciami w cyklu (ms)

static bool lowWaterAlarm = false;
static bool lowBatteryAlarm = false;
static bool lowSoilAlarm = false;

static int beepCount = 0;           // Ile piknięć ma być w tym cyklu
static int beepsRemaining = 0;      // Ile piknięć zostało do wykonania
static bool buzzerOn = false;       // Czy buzzer jest aktualnie włączony (stan fizyczny)
static unsigned long buzzerStateChangeTime = 0; // Czas ostatniej zmiany stanu buzzera

void alarmManagerSetup() {
    buzzerPin = configGetBuzzerPin();
    if (buzzerPin != 255) { // Sprawdź, czy pin jest prawidłowy
        pinMode(buzzerPin, OUTPUT);
        digitalWrite(buzzerPin, LOW); // Upewnij się, że jest wyłączony na starcie
        Serial.printf("  [Alarm] Skonfigurowano pin Buzzera: %d\n", buzzerPin);
    } else {
        Serial.println("  [Alarm] OSTRZEŻENIE: Pin Buzzera nie jest skonfigurowany!");
    }
    isAlarmActive = false;
    lowWaterAlarm = false;
    lowBatteryAlarm = false;
    lowSoilAlarm = false;
    lastBeepCycleTime = 0;
    beepsRemaining = 0; // Zresetuj licznik piknięć
    buzzerOn = false;   // Zresetuj stan buzzera
}

bool alarmManagerUpdate(int waterLevel, float batteryVoltage, int soilMoisture) {
    // Jeśli buzzer nie jest skonfigurowany, nic nie rób
    if (buzzerPin == 255) {
        return false;
    }

    unsigned long currentTime = millis();
    bool previousAlarmState = isAlarmActive;

    // --- Sprawdzenie warunków alarmowych ---
    int lowSoilThreshold    = configGetLowSoilPercent();
    int lowBatteryThreshold = configGetLowBatteryMilliVolts();
    int currentBatteryMv    = (int)(batteryVoltage * 1000);

    bool currentLowWater   = (waterLevel <= 0);
    bool currentLowBattery = (batteryVoltage > 0.1 && currentBatteryMv < lowBatteryThreshold);
    bool currentLowSoil    = (soilMoisture >= 0 && soilMoisture < lowSoilThreshold);

    // --- Bezpośrednie logowanie każdej przyczyny ---
    if (currentLowWater && !lowWaterAlarm) {
        Serial.println("[Alarm] Przyczyna: NISKI POZIOM WODY!");
    }
    if (currentLowBattery && !lowBatteryAlarm) {
        Serial.printf("[Alarm] Przyczyna: NISKA BATERIA (%.2fV < %.2fV)!\n",
                      batteryVoltage, lowBatteryThreshold / 1000.0);
    }
    if (currentLowSoil && !lowSoilAlarm) {
        Serial.printf("[Alarm] Przyczyna: NISKA WILGOTNOŚĆ GLEBY (%d%% < %d%%)!\n",
                      soilMoisture, lowSoilThreshold);
    }

    // --- Aktualizacja flag przyczyn ---
    lowWaterAlarm   = currentLowWater;
    lowBatteryAlarm = currentLowBattery;
    lowSoilAlarm    = currentLowSoil;

    // --- Ogólny stan alarmu ---
    isAlarmActive = lowWaterAlarm || lowBatteryAlarm || lowSoilAlarm;

    // Log zmiany stanu alarmu
    if (isAlarmActive != previousAlarmState) {
        Serial.printf("[Alarm] Zmiana stanu alarmu: %s -> %s\n",
                      previousAlarmState ? "AKTYWNY" : "NIEAKTYWNY",
                      isAlarmActive   ? "AKTYWNY" : "NIEAKTYWNY");
    }

    // --- Sterowanie buzzerem ---
    bool soundEnabled = configIsAlarmSoundEnabled();

    if (isAlarmActive && soundEnabled) {
        // Jeśli alarm dopiero się aktywował, przygotuj nowy cykl piknięć
        if (!previousAlarmState) {
            Serial.println("[Alarm] ALARM AKTYWOWANY! Rozpoczynam pikanie.");
            lastBeepCycleTime = currentTime - BEEP_INTERVAL - 1;
        }
        // Nowy cykl
        if (currentTime - lastBeepCycleTime >= BEEP_INTERVAL && beepsRemaining == 0) {
            // Ustal liczbę piknięć wg priorytetu
            if (lowBatteryAlarm)    beepCount = 3;
            else if (lowWaterAlarm) beepCount = 2;
            else if (lowSoilAlarm)  beepCount = 1;
            beepsRemaining = beepCount;
            buzzerOn = false;
            buzzerStateChangeTime = currentTime;
            lastBeepCycleTime = currentTime;
        }
        // Wykonanie piknięć
        if (beepsRemaining > 0) {
            if (!buzzerOn && currentTime - buzzerStateChangeTime >= BEEP_PAUSE) {
                digitalWrite(buzzerPin, HIGH);
                buzzerOn = true;
                buzzerStateChangeTime = currentTime;
            } else if (buzzerOn && currentTime - buzzerStateChangeTime >= BEEP_DURATION) {
                digitalWrite(buzzerPin, LOW);
                buzzerOn = false;
                beepsRemaining--;
                buzzerStateChangeTime = currentTime;
            }
        }
    } else {
        // Jeśli alarm nieaktywny lub dźwięk wyłączony – upewnij się, że buzzer wyłączony
        if (buzzerOn || beepsRemaining > 0) {
            digitalWrite(buzzerPin, LOW);
            buzzerOn = false;
            beepsRemaining = 0;
            if (isAlarmActive && !soundEnabled) {
                Serial.println("[Alarm] Dźwięk wyłączony – alarm milczy.");
            } else if (!isAlarmActive) {
                Serial.println("[Alarm] Alarm dezaktywowany – wyłączam buzzer.");
            }
        }
    }

    return (isAlarmActive != previousAlarmState);
}


// Zwraca ogólny stan alarmu (czy warunki są spełnione), niezależnie od dźwięku
bool alarmManagerIsAlarmActive() {
    return isAlarmActive;
}