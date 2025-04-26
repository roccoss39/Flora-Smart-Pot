#include "AlarmManager.h"
#include <Arduino.h>
#include "DeviceConfig.h"

static uint8_t buzzerPin;
static bool isAlarmActive = false;
static unsigned long lastBeepCycleTime = 0;
static unsigned long lastSingleBeepTime = 0;
static const unsigned long BEEP_INTERVAL = 10000;    // Przerwa między cyklami (ms)
static const unsigned long BEEP_DURATION = 300;     // Długość pojedynczego pikania (ms)
static const unsigned long BEEP_PAUSE = 400;         // Pauza między piknięciami w cyklu (ms)

static bool lowWaterAlarm = false;
static bool lowBatteryAlarm = false;
static bool lowSoilAlarm = false;

static int beepCount = 0;           // Ile piknięć ma być w tym cyklu
static int beepsRemaining = 0;       // Ile piknięć zostało do wykonania
static bool buzzerOn = false;        // Czy buzzer jest aktualnie włączony
static unsigned long buzzerStateChangeTime = 0; // Czas ostatniej zmiany stanu buzzera

void alarmManagerSetup() {
    buzzerPin = configGetBuzzerPin();
    if (buzzerPin != 255) {
        pinMode(buzzerPin, OUTPUT);
        digitalWrite(buzzerPin, LOW);
        Serial.printf("  [Alarm] Skonfigurowano pin Buzzera: %d\n", buzzerPin);
    } else {
        Serial.println("  [Alarm] OSTRZEŻENIE: Pin Buzzera nie jest skonfigurowany!");
    }
    isAlarmActive = false;
    lowWaterAlarm = false;
    lowBatteryAlarm = false;
    lowSoilAlarm = false;
    lastBeepCycleTime = 0;
}

void alarmManagerUpdate(int waterLevel, float batteryVoltage, int soilMoisture) {
    if (buzzerPin == 255) {
        return;
    }

    int lowSoilThreshold = configGetLowSoilPercent();
    int lowBatteryThresholdMv = configGetLowBatteryMilliVolts();
    int currentBatteryMv = (int)(batteryVoltage * 1000);

    bool currentLowWater = false;
    bool currentLowBattery = false;
    bool currentLowSoil = false;

    if (waterLevel <= 0) {
        currentLowWater = true;
        if (!lowWaterAlarm) {
            Serial.println("[Alarm] Przyczyna: NISKI POZIOM WODY!");
        }
    }
    lowWaterAlarm = currentLowWater;

    if (batteryVoltage > 0.1) {
        if (currentBatteryMv < lowBatteryThresholdMv) {
            currentLowBattery = true;
            if (!lowBatteryAlarm) {
                Serial.printf("[Alarm] Przyczyna: NISKA BATERIA (%.2fV < %.2fV)!\n", batteryVoltage, lowBatteryThresholdMv / 1000.0);
            }
        }
    }
    lowBatteryAlarm = currentLowBattery;

    if (soilMoisture >= 0) {
        if (soilMoisture < lowSoilThreshold) {
            currentLowSoil = true;
            if (!lowSoilAlarm) {
                Serial.printf("[Alarm] Przyczyna: NISKA WILGOTNOŚĆ GLEBY (%d%% < %d%%)!\n", soilMoisture, lowSoilThreshold);
            }
        }
    }
    lowSoilAlarm = currentLowSoil;

    bool previousAlarmState = isAlarmActive;
    isAlarmActive = lowWaterAlarm || lowBatteryAlarm || lowSoilAlarm;

    if (isAlarmActive) {
        unsigned long currentTime = millis();
        
        if (!previousAlarmState) {
            Serial.println("[Alarm] ALARM AKTYWOWANY!");
            lastBeepCycleTime = currentTime - BEEP_INTERVAL - 1; // wymuś natychmiastowy start
        }

        // Obsługa rozpoczynania nowego cyklu pikania
        if (currentTime - lastBeepCycleTime >= BEEP_INTERVAL && beepsRemaining == 0) {
            if (lowBatteryAlarm) {
                beepCount = 3;
            } else if (lowWaterAlarm) {
                beepCount = 2;
            } else if (lowSoilAlarm) {
                beepCount = 1;
            } else {
                beepCount = 0;
            }

            if (beepCount > 0) {
                beepsRemaining = beepCount;
                buzzerOn = false;
                buzzerStateChangeTime = currentTime;
            }
            lastBeepCycleTime = currentTime;
        }

        // Obsługa pojedynczych piknięć w cyklu
        if (beepsRemaining > 0) {
            if (!buzzerOn && (currentTime - buzzerStateChangeTime >= BEEP_PAUSE)) {
                // Włącz buzzer
                digitalWrite(buzzerPin, HIGH);
                buzzerOn = true;
                buzzerStateChangeTime = currentTime;
            } else if (buzzerOn && (currentTime - buzzerStateChangeTime >= BEEP_DURATION)) {
                // Wyłącz buzzer
                digitalWrite(buzzerPin, LOW);
                buzzerOn = false;
                beepsRemaining--;
                buzzerStateChangeTime = currentTime;
            }
        }
    } else {
        if (previousAlarmState) {
            Serial.println("[Alarm] Stan normalny - alarm dezaktywowany.");
            digitalWrite(buzzerPin, LOW);
            beepsRemaining = 0;
            buzzerOn = false;
        }
    }
}

bool alarmManagerIsAlarmActive() {
    return isAlarmActive;
}
