#include "AlarmManager.h"
#include <Arduino.h>
#include "DeviceConfig.h" // Potrzebny do pobrania pinów i progów

static uint8_t buzzerPin;
static bool isAlarmActive = false; // Ogólny stan alarmu
static unsigned long lastBeepTime = 0;
const unsigned long BEEP_INTERVAL = 3000; // Interwał pikania w ms (3 sekundy)
const int BEEP_DURATION = 150;           // Czas trwania pikania w ms

// Flagi dla poszczególnych przyczyn alarmu (dla logowania)
static bool lowWaterAlarm = false;
static bool lowBatteryAlarm = false;
static bool lowSoilAlarm = false;

void alarmManagerSetup() {
    buzzerPin = configGetBuzzerPin();
    if (buzzerPin != 255) { // Używamy 255 jako wskaźnik błędu/braku konfiguracji
        pinMode(buzzerPin, OUTPUT);
        digitalWrite(buzzerPin, LOW); // Upewnij się, że buzzer jest cicho na starcie
        Serial.printf("  [Alarm] Skonfigurowano pin Buzzera: %d\n", buzzerPin);
    } else {
        Serial.println("  [Alarm] OSTRZEŻENIE: Pin Buzzera nie jest skonfigurowany!");
    }
    // Reset stanów przy inicjalizacji
    isAlarmActive = false;
    lowWaterAlarm = false;
    lowBatteryAlarm = false;
    lowSoilAlarm = false;
    lastBeepTime = 0;
}

// Zmieniono argumenty funkcji:
void alarmManagerUpdate(int waterLevel, float batteryVoltage, int soilMoisture) {
    if (buzzerPin == 255) {
        return; // Nie rób nic, jeśli pin nie jest skonfigurowany
    }

    // Pobierz progi alarmowe z konfiguracji
    int lowSoilThreshold = configGetLowSoilPercent();
    int lowBatteryThresholdMv = configGetLowBatteryMilliVolts();
    int currentBatteryMv = (int)(batteryVoltage * 1000); // Konwertuj na mV

    // --- Sprawdź poszczególne warunki alarmowe ---
    // Resetuj flagi przed sprawdzeniem
    bool currentLowWater = false;
    bool currentLowBattery = false;
    bool currentLowSoil = false;

    // 1. Niski poziom wody
    if (waterLevel <= 0) {
        currentLowWater = true;
        if (!lowWaterAlarm) { // Loguj tylko przy pierwszej detekcji
            Serial.println("[Alarm] Przyczyna: NISKI POZIOM WODY!");
        }
    }
    lowWaterAlarm = currentLowWater;

    // 2. Niska bateria (sprawdzaj tylko, jeśli odczyt > 0, aby uniknąć fałszywego alarmu przy błędzie odczytu)
    if (batteryVoltage > 0.1) { // Dodaj mały margines na błąd odczytu
         if (currentBatteryMv < lowBatteryThresholdMv) {
            currentLowBattery = true;
            if (!lowBatteryAlarm) { // Loguj tylko przy pierwszej detekcji
                Serial.printf("[Alarm] Przyczyna: NISKA BATERIA (%.2fV < %.2fV)!\n", batteryVoltage, lowBatteryThresholdMv / 1000.0);
            }
         }
    }
    lowBatteryAlarm = currentLowBattery;

    // 3. Niska wilgotność gleby (sprawdzaj tylko, jeśli odczyt >= 0)
    if (soilMoisture >= 0) {
        if (soilMoisture < lowSoilThreshold) {
            currentLowSoil = true;
            if (!lowSoilAlarm) { // Loguj tylko przy pierwszej detekcji
                 Serial.printf("[Alarm] Przyczyna: NISKA WILGOTNOŚĆ GLEBY (%d%% < %d%%)!\n", soilMoisture, lowSoilThreshold);
            }
        }
    }
    lowSoilAlarm = currentLowSoil;

    // --- Ustal ogólny stan alarmu ---
    bool previousAlarmState = isAlarmActive;
    isAlarmActive = lowWaterAlarm || lowBatteryAlarm || lowSoilAlarm;

    // --- Logika sterowania buzzerem ---
    if (isAlarmActive) {
        // Jeśli alarm właśnie się aktywował
        if (!previousAlarmState) {
            Serial.println("[Alarm] ALARM AKTYWOWANY!");
            // Umożliw natychmiastowe piknięcie po pierwszej aktywacji
            // Ustawiamy tak, aby warunek (currentTime - lastBeepTime >= BEEP_INTERVAL) był od razu prawdziwy
            lastBeepTime = millis() - BEEP_INTERVAL - 1;
        }

        // Obsługa pikania co interwał
        unsigned long currentTime = millis();
        if (currentTime - lastBeepTime >= BEEP_INTERVAL) {
            Serial.println("[Alarm] BEEP!");
            digitalWrite(buzzerPin, HIGH); // Włącz buzzer
            delay(BEEP_DURATION);          // Poczekaj BEEP_DURATION (uproszczone)
            digitalWrite(buzzerPin, LOW);  // Wyłącz buzzer
            lastBeepTime = currentTime;    // Zapisz czas TEGO pikania
        }
    } else {
        // Jeśli alarm właśnie się dezaktywował
        if (previousAlarmState) {
             Serial.println("[Alarm] Stan normalny - alarm dezaktywowany.");
             digitalWrite(buzzerPin, LOW); // Upewnij się, że buzzer jest cicho
        }
    }
}