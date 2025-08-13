// AlarmManager.cpp
// Główne problemy:
// - Komentarze w języku polskim
// - Brak wyraźnego rozgraniczenia części prywatnej i publicznej
// - Używanie magicznych liczb

// Proponowane poprawki:
#include "AlarmManager.h"
#include <Arduino.h>
#include "DeviceConfig.h"

// Constants
static const unsigned long BEEP_INTERVAL = 10000; // Interval between cycles (ms)
static const unsigned long BEEP_DURATION = 300;   // Single beep duration (ms)
static const unsigned long BEEP_PAUSE = 400;      // Pause between beeps in cycle (ms)

// Priority level constants
static const int BATTERY_ALARM_PRIORITY = 3;
static const int WATER_ALARM_PRIORITY = 2;
static const int SOIL_ALARM_PRIORITY = 1;

// Private variables
static uint8_t buzzerPin;
static bool isAlarmActive = false;
static unsigned long lastBeepCycleTime = 0;

static bool lowWaterAlarm = false;
static bool lowBatteryAlarm = false;
static bool lowSoilAlarm = false;

static int beepCount = 0;
static int beepsRemaining = 0;
static bool buzzerOn = false;
static unsigned long buzzerStateChangeTime = 0;

void alarmManagerSetup() {
    buzzerPin = configGetBuzzerPin();
    if (buzzerPin != 255) { // Check if pin is valid
        pinMode(buzzerPin, OUTPUT);
        digitalWrite(buzzerPin, LOW); // Ensure it's off at start
        Serial.printf("  [Alarm] Buzzer pin configured: %d\n", buzzerPin);
    } else {
        Serial.println("  [Alarm] WARNING: Buzzer pin not configured!");
    }
    isAlarmActive = false;
    lowWaterAlarm = false;
    lowBatteryAlarm = false;
    lowSoilAlarm = false;
    lastBeepCycleTime = 0;
    beepsRemaining = 0;
    buzzerOn = false;
}

bool alarmManagerUpdate(int waterLevel, float batteryVoltage, int soilMoisture) {
    // If buzzer is not configured, do nothing
    if (buzzerPin == 255) {
        return false;
    }

    unsigned long currentTime = millis();
    bool previousAlarmState = isAlarmActive;

    // --- Check alarm conditions ---
    int lowSoilThreshold    = configGetLowSoilPercent();
    int lowBatteryThreshold = configGetLowBatteryMilliVolts();
    int currentBatteryMv    = (int)(batteryVoltage * 1000);

    bool currentLowWater   = (waterLevel <= 0);
    bool currentLowBattery = (batteryVoltage > 0.1 && currentBatteryMv < lowBatteryThreshold);
    bool currentLowSoil    = (soilMoisture >= 0 && soilMoisture < lowSoilThreshold);

    // --- Direct logging for each cause ---
    if (currentLowWater && !lowWaterAlarm) {
        Serial.println("[Alarm] Cause: LOW WATER LEVEL!");
    }
    if (currentLowBattery && !lowBatteryAlarm) {
        Serial.printf("[Alarm] Cause: LOW BATTERY (%.2fV < %.2fV)!\n",
                      batteryVoltage, lowBatteryThreshold / 1000.0);
    }
    if (currentLowSoil && !lowSoilAlarm) {
        Serial.printf("[Alarm] Cause: LOW SOIL MOISTURE (%d%% < %d%%)!\n",
                      soilMoisture, lowSoilThreshold);
    }

    // --- Update cause flags ---
    lowWaterAlarm   = currentLowWater;
    lowBatteryAlarm = currentLowBattery;
    lowSoilAlarm    = currentLowSoil;

    // --- General alarm state ---
    isAlarmActive = lowWaterAlarm || lowBatteryAlarm || lowSoilAlarm;

    // Log alarm state change
    if (isAlarmActive != previousAlarmState) {
        Serial.printf("[Alarm] Alarm state change: %s -> %s\n",
                      previousAlarmState ? "ACTIVE" : "INACTIVE",
                      isAlarmActive   ? "ACTIVE" : "INACTIVE");
    }

    // --- Buzzer control ---
    bool soundEnabled = configIsAlarmSoundEnabled();

    if (isAlarmActive && soundEnabled) {
        // If alarm just activated, prepare new beep cycle
        if (!previousAlarmState) {
            Serial.println("[Alarm] ALARM ACTIVATED! Starting beeping.");
            lastBeepCycleTime = currentTime - BEEP_INTERVAL - 1;
        }
        // New cycle
        if (currentTime - lastBeepCycleTime >= BEEP_INTERVAL && beepsRemaining == 0) {
            // Set number of beeps according to priority
            if (lowBatteryAlarm)    beepCount = BATTERY_ALARM_PRIORITY;
            else if (lowWaterAlarm) beepCount = WATER_ALARM_PRIORITY;
            else if (lowSoilAlarm)  beepCount = SOIL_ALARM_PRIORITY;
            beepsRemaining = beepCount;
            buzzerOn = false;
            buzzerStateChangeTime = currentTime;
            lastBeepCycleTime = currentTime;
        }
        // Execute beeps
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
        // If alarm inactive or sound disabled – ensure buzzer is off
        if (buzzerOn || beepsRemaining > 0) {
            digitalWrite(buzzerPin, LOW);
            buzzerOn = false;
            beepsRemaining = 0;
            if (isAlarmActive && !soundEnabled) {
                Serial.println("[Alarm] Sound disabled – alarm is silent.");
            } else if (!isAlarmActive) {
                Serial.println("[Alarm] Alarm deactivated – turning off buzzer.");
            }
        }
    }

    return (isAlarmActive != previousAlarmState);
}

bool alarmManagerIsAlarmActive() {
    return isAlarmActive;
}