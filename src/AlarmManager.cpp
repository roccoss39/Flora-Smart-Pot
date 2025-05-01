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
    bool stateChanged = false; // Flaga sygnalizująca zmianę stanu w tym wywołaniu
    if (buzzerPin == 255) { // Jeśli pin nie jest skonfigurowany, nic nie rób
        return false;;
    }

        // Wypisanie wartości argumentów
        // Serial.print("waterLevel: ");
        // Serial.println(waterLevel);
        
        // Serial.print("batteryVoltage: ");
        // Serial.println(batteryVoltage);
        
        // Serial.print("soilMoisture: ");
        // Serial.println(soilMoisture);

        // Serial.print("stan alarmu: ");
        // Serial.println(isAlarmActive);

    // Odczytaj progi alarmowe z konfiguracji
    int lowSoilThreshold = configGetLowSoilPercent();
    int lowBatteryThresholdMv = configGetLowBatteryMilliVolts();
    int currentBatteryMv = (int)(batteryVoltage * 1000);

    // Sprawdź aktualne warunki alarmowe
    bool currentLowWater = (waterLevel <= 0); // Poziom 0 oznacza brak wody
    bool currentLowBattery = (batteryVoltage > 0.1 && currentBatteryMv < lowBatteryThresholdMv); // Sprawdź tylko jeśli odczyt > 0.1V
    bool currentLowSoil = (soilMoisture >= 0 && soilMoisture < lowSoilThreshold); // Sprawdź tylko jeśli odczyt >= 0%

    // Aktualizuj flagi alarmowe i loguj tylko przy zmianie stanu na aktywny
    if (currentLowWater && !lowWaterAlarm) {
        Serial.println("[Alarm] Przyczyna: NISKI POZIOM WODY!");
    }
    lowWaterAlarm = currentLowWater;

    if (currentLowBattery && !lowBatteryAlarm) {
        Serial.printf("[Alarm] Przyczyna: NISKA BATERIA (%.2fV < %.2fV)!\n", batteryVoltage, lowBatteryThresholdMv / 1000.0);
    }
    lowBatteryAlarm = currentLowBattery;

    if (currentLowSoil && !lowSoilAlarm) {
        Serial.printf("[Alarm] Przyczyna: NISKA WILGOTNOŚĆ GLEBY (%d%% < %d%%)!\n", soilMoisture, lowSoilThreshold);
    }
    lowSoilAlarm = currentLowSoil;

    // Zaktualizuj ogólny stan alarmu
    bool previousAlarmState = isAlarmActive;
    isAlarmActive = lowWaterAlarm || lowBatteryAlarm || lowSoilAlarm;

    if (isAlarmActive != previousAlarmState) {
        stateChanged = true; // Ustaw flagę zmiany stanu
        Serial.printf("[Alarm] Wykryto zmianę stanu alarmu: %s -> %s (Sygnalizuję zmianę)\n",
                      previousAlarmState ? "AKTYWNY" : "NIEAKTYWNY",
                      isAlarmActive ? "AKTYWNY" : "NIEAKTYWNY");
    }

    // --- Logika sterowania buzzerem ---
    bool soundEnabled = configIsAlarmSoundEnabled(); // <<< NOWOŚĆ: Sprawdź, czy dźwięk jest włączony

    if (isAlarmActive) {
        unsigned long currentTime = millis();

        // Loguj aktywację alarmu (jeśli wcześniej nie był aktywny)
        if (!previousAlarmState) {
            Serial.println("[Alarm] ALARM AKTYWOWANY!");
            if (soundEnabled) {
                Serial.println("[Alarm] Dźwięk włączony - rozpoczynam pikanie.");
                lastBeepCycleTime = currentTime - BEEP_INTERVAL - 1; // Wymuś natychmiastowy start cyklu pikania
            } else {
                Serial.println("[Alarm] Dźwięk wyłączony - brak pikania.");
            }
        }

        // <<< NOWOŚĆ: Wykonuj logikę buzzera tylko jeśli dźwięk jest włączony >>>
        if (soundEnabled) {
            // Obsługa rozpoczynania nowego cyklu pikania
            if (currentTime - lastBeepCycleTime >= BEEP_INTERVAL && beepsRemaining == 0) {
                // Ustal priorytet alarmu dla liczby piknięć
                if (lowBatteryAlarm) {
                    beepCount = 3; // Najwyższy priorytet
                } else if (lowWaterAlarm) {
                    beepCount = 2;
                } else if (lowSoilAlarm) {
                    beepCount = 1;
                } else {
                    beepCount = 0; // Na wszelki wypadek
                }

                if (beepCount > 0) {
                    beepsRemaining = beepCount;
                    buzzerOn = false; // Rozpocznij od wyłączonego buzzera (pauza)
                    buzzerStateChangeTime = currentTime;
                    //Serial.printf("[Alarm] Nowy cykl pikania: %d razy.\n", beepCount);
                }
                lastBeepCycleTime = currentTime; // Zapisz czas rozpoczęcia cyklu
            }

            // Obsługa pojedynczych piknięć w cyklu
            if (beepsRemaining > 0) {
                if (!buzzerOn && (currentTime - buzzerStateChangeTime >= BEEP_PAUSE)) {
                    // Włącz buzzer (koniec pauzy)
                    digitalWrite(buzzerPin, HIGH);
                    buzzerOn = true;
                    buzzerStateChangeTime = currentTime;
                    //Serial.println("[Alarm] Buzzer ON");
                } else if (buzzerOn && (currentTime - buzzerStateChangeTime >= BEEP_DURATION)) {
                    // Wyłącz buzzer (koniec pikania)
                    digitalWrite(buzzerPin, LOW);
                    buzzerOn = false;
                    beepsRemaining--; // Zmniejsz liczbę pozostałych piknięć
                    buzzerStateChangeTime = currentTime;
                   // Serial.printf("[Alarm] Buzzer OFF, pozostało: %d\n", beepsRemaining);
                }
            }
        } else {
            // <<< NOWOŚĆ: Jeśli dźwięk jest wyłączony, upewnij się, że buzzer jest cicho >>>
            if (buzzerOn) {
                digitalWrite(buzzerPin, LOW);
                buzzerOn = false;
                beepsRemaining = 0; // Zresetuj cykl
                Serial.println("[Alarm] Dźwięk wyłączony w trakcie cyklu - wyłączam buzzer.");
            }
        }

    } else { // Jeśli alarm nie jest aktywny
        if (previousAlarmState) {
            Serial.println("[Alarm] Stan normalny - alarm dezaktywowany.");
        }
        // Upewnij się, że buzzer jest wyłączony i cykl przerwany
        if (buzzerOn || beepsRemaining > 0) {
            digitalWrite(buzzerPin, LOW);
            beepsRemaining = 0;
            buzzerOn = false;
            //Serial.println("[Alarm] Dezaktywacja - buzzer OFF.");
        }
    }
    return stateChanged; // Zwróć informację o zmianie stanu
}

// Zwraca ogólny stan alarmu (czy warunki są spełnione), niezależnie od dźwięku
bool alarmManagerIsAlarmActive() {
    return isAlarmActive;
}