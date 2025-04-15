#include "PumpControl.h"
#include "DeviceConfig.h"
#include <Arduino.h>

static uint8_t pumpPin;
static uint32_t pumpRunMillisDefault;
static int soilThreshold;
static bool isPumpOn = false;

void pumpControlSetup() {
    pumpPin = configGetPumpPin();
    pumpRunMillisDefault = configGetPumpRunMillis();
    soilThreshold = configGetSoilThresholdPercent();

    if (pumpPin != 255) { // Używamy 255 jako wskaźnik błędu dla uint8_t
        pinMode(pumpPin, OUTPUT);
        digitalWrite(pumpPin, LOW);
        isPumpOn = false;
        Serial.printf("  [Pompa] Skonfigurowano pin sterujący: %d (domyślny czas: %d ms, próg: %d%%)\n", pumpPin, pumpRunMillisDefault, soilThreshold);
    } else {
        Serial.println("  [Pompa] BŁĄD: Nie skonfigurowano poprawnie pinu pompy!");
    }
}

void pumpControlActivateIfNeeded(int currentSoilMoisture, int currentWaterLevel) {
    Serial.println("--- Automatyczna kontrola pompy ---");

    if (isPumpOn) {
        Serial.println("  [Pompa] Pompa jest już włączona (manualnie?). Pomijam automatykę.");
        return;
    }
    if (pumpPin == 255) {
        Serial.println("  [Pompa] Błąd: Pin pompy nieskonfigurowany.");
        return;
    }
    if (currentWaterLevel <= 0) {
        Serial.println("  [Pompa] Poziom wody = 0. Pompa nie zostanie uruchomiona.");
        return;
    }

    Serial.printf("  [Pompa] Wilgotność: %d%%, Próg: %d%%\n", currentSoilMoisture, soilThreshold);
    if (currentSoilMoisture < soilThreshold && currentSoilMoisture >= 0) {
        Serial.printf("  [Pompa] Niska wilgotność. Uruchamiam pompę automatycznie na %d ms...\n", pumpRunMillisDefault);
        digitalWrite(pumpPin, HIGH);
        isPumpOn = true;
        delay(pumpRunMillisDefault);
        digitalWrite(pumpPin, LOW);
        isPumpOn = false;
        Serial.println("  [Pompa] Pompa zatrzymana (automat).");
    } else if (currentSoilMoisture < 0) {
         Serial.println("  [Pompa] Błąd odczytu wilgotności gleby. Pompa nieaktywna.");
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
         return;
     }
     Serial.printf("  [Pompa] Uruchamiam pompę manualnie na %d ms...\n", durationMillis);
     digitalWrite(pumpPin, HIGH);
     isPumpOn = true;
     delay(durationMillis);
     digitalWrite(pumpPin, LOW);
     isPumpOn = false;
     Serial.println("  [Pompa] Pompa zatrzymana (manual).");
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
    } else {
         Serial.println("  [Pompa] Pompa już jest wyłączona.");
    }
}

bool pumpControlIsRunning() {
    return isPumpOn;
}