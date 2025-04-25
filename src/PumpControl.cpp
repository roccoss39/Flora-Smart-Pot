#include "PumpControl.h"
#include "DeviceConfig.h"
#include <Arduino.h>
#include "BlynkManager.h"

static uint8_t pumpPin;
static uint32_t pumpRunMillisDefault;
static int soilThreshold;
static bool isPumpOn = false;

static unsigned long pumpStartTime = 0;
static uint32_t pumpTargetDuration = 0;

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
        pumpStartTime = millis();
        pumpTargetDuration = pumpRunMillisDefault; // Używamy zdefiniowanej zmiennej pumpRunMillisDefault
        blynkUpdatePumpStatus(isPumpOn);
        Serial.println("  [Pompa] Pompa uruchomiona (automat).");
        // Usunięte natychmiastowe wyłączenie - pompa zostanie wyłączona przez pumpControlUpdate()
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
     pumpStartTime = millis();
     pumpTargetDuration = durationMillis;
     blynkUpdatePumpStatus(isPumpOn);
     //delay(durationMillis);
     //digitalWrite(pumpPin, LOW);
     //isPumpOn = false;
     //blynkUpdatePumpStatus(isPumpOn);
     //Serial.println("  [Pompa] Pompa zatrzymana (manual).");
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
        blynkUpdatePumpStatus(isPumpOn);
    } else {
         Serial.println("  [Pompa] Pompa już jest wyłączona.");
    }
}

bool pumpControlIsRunning() {
    return isPumpOn;
}

void pumpControlUpdate() {
    // Sprawdź, czy pompa powinna zostać wyłączona
    if (isPumpOn && (millis() - pumpStartTime >= pumpTargetDuration)) {
        digitalWrite(pumpPin, LOW);
        isPumpOn = false;
        blynkUpdatePumpStatus(isPumpOn);
        Serial.println("  [Pompa] Pompa zatrzymana (auto-wyłączenie po czasie).");
    }
}