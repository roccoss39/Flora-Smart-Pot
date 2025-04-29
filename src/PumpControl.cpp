#include "PumpControl.h"
#include "DeviceConfig.h" // Potrzebny do configGet...()
#include <Arduino.h>      // Potrzebny dla Serial, pinMode, digitalWrite, millis()
#include "BlynkManager.h" // Potrzebny do blynkUpdatePumpStatus()

// Zmienne statyczne (widoczne tylko w tym pliku)
static uint8_t pumpPin;                // Pin GPIO sterujący pompą (przez MOSFET/przekaźnik)
static bool isPumpOn = false;          // Aktualny stan pracy pompy
static unsigned long pumpStartTime = 0;  // Czas (wg millis()) uruchomienia pompy
static uint32_t pumpTargetDuration = 0; // Czas (w ms), przez który pompa ma pracować w bieżącym cyklu

void pumpControlSetup() {
    pumpPin = configGetPumpPin(); // Pobierz pin pompy z konfiguracji

    // Odczyt wartości w setup nie jest już potrzebny do logiki działania,
    // ale można zostawić do logowania początkowego stanu.
    uint32_t initialPumpMillis = configGetPumpRunMillis();
    int initialSoilThreshold = configGetSoilThresholdPercent();

    if (pumpPin != 255) { // Używamy 255 jako wskaźnik błędu/braku konfiguracji dla uint8_t
        pinMode(pumpPin, OUTPUT);
        digitalWrite(pumpPin, LOW); // Upewnij się, że pompa jest wyłączona na starcie
        isPumpOn = false;
        Serial.printf("  [Pompa] Skonfigurowano pin sterujący: %d (początkowy czas: %u ms, próg: %d%%)\n", pumpPin, initialPumpMillis, initialSoilThreshold);
    } else {
        Serial.println("  [Pompa] BŁĄD: Nie skonfigurowano poprawnie pinu pompy w DeviceConfig!");
    }
}

void pumpControlActivateIfNeeded(int currentSoilMoisture, int currentWaterLevel) {
    Serial.println("--- Automatyczna kontrola pompy ---");

    if (isPumpOn) {
        Serial.println("  [Pompa] Pompa jest już włączona (np. manualnie). Pomijam automatykę.");
        return; // Nie uruchamiaj automatycznie, jeśli już działa
    }
    if (pumpPin == 255) {
        Serial.println("  [Pompa] Błąd: Pin pompy nieskonfigurowany.");
        return; // Nie można sterować pompą
    }
    if (currentWaterLevel <= 0) {
        Serial.println("  [Pompa] Poziom wody = 0. Pompa nie zostanie uruchomiona (zabezpieczenie).");
        return; // Nie uruchamiaj pompy bez wody
    }
    if (currentSoilMoisture < 0) {
         Serial.println("  [Pompa] Błąd odczytu wilgotności gleby. Pompa nie zostanie uruchomiona.");
         return; // Nie uruchamiaj przy błędzie sensora
    }

    // Pobierz aktualny próg wilgotności z konfiguracji
    int currentSoilThreshold = configGetSoilThresholdPercent();
    Serial.printf("  [Pompa] Wilgotność: %d%%, Aktualny próg: %d%%\n", currentSoilMoisture, currentSoilThreshold);

    // Sprawdź warunek uruchomienia
    if (currentSoilMoisture < currentSoilThreshold) {
        // Warunek spełniony - pobierz AKTUALNY czas pracy z konfiguracji
        uint32_t currentPumpRunMillis = configGetPumpRunMillis();

        Serial.printf("  [Pompa] Niska wilgotność. Uruchamiam pompę automatycznie na %u ms...\n", currentPumpRunMillis);

        // Włącz pompę
        digitalWrite(pumpPin, HIGH);
        isPumpOn = true;
        pumpStartTime = millis();           // Zapisz czas startu
        pumpTargetDuration = currentPumpRunMillis; // Ustaw docelowy czas pracy
        blynkUpdatePumpStatus(isPumpOn); // Zaktualizuj status w Blynk
        Serial.println("  [Pompa] Pompa uruchomiona (automat).");
    } else {
        Serial.println("  [Pompa] Wilgotność gleby OK.");
    }
     Serial.println("---------------------------------");
}

void pumpControlManualTurnOn(uint32_t durationMillis) {
     if (pumpPin == 255) {
        Serial.println("  [Pompa] Błąd: Pin pompy nieskonfigurowany. Nie można włączyć manualnie.");
        return;
     }
     if (isPumpOn) {
         Serial.println("  [Pompa] Pompa już pracuje.");
         // Można rozważyć restart timera, jeśli chcemy przedłużyć działanie,
         // ale obecna logika zapobiega podwójnemu uruchomieniu.
         return;
     }

     Serial.printf("  [Pompa] Uruchamiam pompę manualnie na %u ms...\n", durationMillis);
     digitalWrite(pumpPin, HIGH);
     isPumpOn = true;
     pumpStartTime = millis();
     pumpTargetDuration = durationMillis; // Ustaw docelowy czas pracy z argumentu
     blynkUpdatePumpStatus(isPumpOn);
}

void pumpControlManualTurnOff() {
    if (pumpPin == 255) {
        Serial.println("  [Pompa] Błąd: Pin pompy nieskonfigurowany.");
        return;
     }
    if (isPumpOn) {
        Serial.println("  [Pompa] Manualne natychmiastowe wyłączenie pompy...");
        digitalWrite(pumpPin, LOW);
        isPumpOn = false;
        pumpTargetDuration = 0; // Zresetuj czas docelowy
        blynkUpdatePumpStatus(isPumpOn);
    } else {
         Serial.println("  [Pompa] Pompa już jest wyłączona.");
    }
}

bool pumpControlIsRunning() {
    return isPumpOn;
}

void pumpControlUpdate() {
    // Sprawdź, czy pompa jest włączona i czy upłynął już jej docelowy czas pracy
    if (isPumpOn && (millis() - pumpStartTime >= pumpTargetDuration)) {
        Serial.printf("  [Pompa] Czas pracy (%u ms) upłynął. Wyłączam pompę.\n", pumpTargetDuration);
        digitalWrite(pumpPin, LOW);
        isPumpOn = false;
        pumpTargetDuration = 0; // Zresetuj czas docelowy
        blynkUpdatePumpStatus(isPumpOn);
        Serial.println("  [Pompa] Pompa zatrzymana (auto-wyłączenie po czasie).");
    }
}