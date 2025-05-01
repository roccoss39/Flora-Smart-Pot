// Plik: PumpControl.cpp
#include "PumpControl.h"
#include "DeviceConfig.h"
#include <Arduino.h>
#include "BlynkManager.h"

// --- NOWOŚĆ: Konfiguracja LEDC (PWM) ---
const int PUMP_LEDC_CHANNEL = 0; // Wybierz kanał LEDC (0-15), upewnij się, że nie koliduje z innymi użyciami PWM
const int PUMP_LEDC_FREQ = 5000;  // Częstotliwość PWM w Hz (np. 5kHz)
const int PUMP_LEDC_RESOLUTION = 8; // Rozdzielczość PWM (8 bitów = wartości 0-255)

// Zmienne statyczne
static uint8_t pumpPin;
static bool isPumpOn = false;
static unsigned long pumpStartTime = 0;
static uint32_t pumpTargetDuration = 0;

void pumpControlSetup() {
    pumpPin = configGetPumpPin();

    if (pumpPin != 255) {
        // --- ZMIANA: Konfiguracja LEDC zamiast pinMode ---
        // Konfiguruj kanał LEDC
        ledcSetup(PUMP_LEDC_CHANNEL, PUMP_LEDC_FREQ, PUMP_LEDC_RESOLUTION);
        // Przypisz pin GPIO do skonfigurowanego kanału LEDC
        ledcAttachPin(pumpPin, PUMP_LEDC_CHANNEL);
        // Upewnij się, że pompa jest wyłączona na starcie (duty cycle = 0)
        ledcWrite(PUMP_LEDC_CHANNEL, 0);
        isPumpOn = false;

        // Logowanie informacji o konfiguracji
        Serial.printf("  [Pompa] Skonfigurowano pin sterujący PWM: %d (Kanał LEDC: %d, Freq: %d Hz, Res: %d bit)\n",
                      pumpPin, PUMP_LEDC_CHANNEL, PUMP_LEDC_FREQ, PUMP_LEDC_RESOLUTION);

        // Logowanie pozostałych parametrów (teraz pobierane z config)
        uint8_t initialDuty = configGetPumpDutyCycle();
        uint32_t initialPumpMillis = configGetPumpRunMillis();
        int initialSoilThreshold = configGetSoilThresholdPercent();
        Serial.printf("  [Pompa] Początkowa moc (Duty Cycle): %d/255, Czas: %u ms, Próg: %d%%\n", initialDuty, initialPumpMillis, initialSoilThreshold);

    } else {
        Serial.println("  [Pompa] BŁĄD: Nie skonfigurowano poprawnie pinu pompy w DeviceConfig!");
    }
}

void pumpControlActivateIfNeeded(int currentSoilMoisture, int currentWaterLevel) {
    Serial.println("--- Automatyczna kontrola pompy ---");

    // ... (Warunki sprawdzające isPumpOn, pumpPin, currentWaterLevel, currentSoilMoisture - bez zmian) ...
     if (isPumpOn) { /*...*/ return; }
     if (pumpPin == 255) { /*...*/ return; }
     if (currentWaterLevel <= 0) { /*...*/ return; }
     if (currentSoilMoisture < 0) { /*...*/ return; }

    int currentSoilThreshold = configGetSoilThresholdPercent();
    Serial.printf("  [Pompa] Wilgotność: %d%%, Aktualny próg: %d%%\n", currentSoilMoisture, currentSoilThreshold);

    if (currentSoilMoisture < currentSoilThreshold) {
        uint32_t currentPumpRunMillis = configGetPumpRunMillis();
        // --- NOWOŚĆ: Pobierz aktualną moc pompy ---
        uint8_t currentDutyCycle = configGetPumpDutyCycle();

        Serial.printf("  [Pompa] Niska wilgotność. Uruchamiam pompę automatycznie na %u ms z mocą %d/255...\n", currentPumpRunMillis, currentDutyCycle);

        // --- ZMIANA: Włącz pompę używając PWM ---
        ledcWrite(PUMP_LEDC_CHANNEL, currentDutyCycle);
        isPumpOn = true;
        pumpStartTime = millis();
        pumpTargetDuration = currentPumpRunMillis;
        blynkUpdatePumpStatus(isPumpOn);
        Serial.println("  [Pompa] Pompa uruchomiona (automat).");
    } else {
        Serial.println("  [Pompa] Wilgotność gleby OK.");
    }
     Serial.println("---------------------------------");
}

void pumpControlManualTurnOn(uint32_t durationMillis) {
     // ... (Sprawdzenie pumpPin, isPumpOn - bez zmian) ...
      if (pumpPin == 255) { /*...*/ return; }
      if (isPumpOn) { /*...*/ return; }

     // --- NOWOŚĆ: Pobierz aktualną moc pompy ---
     uint8_t currentDutyCycle = configGetPumpDutyCycle();

     Serial.printf("  [Pompa] Uruchamiam pompę manualnie na %u ms z mocą %d/255...\n", durationMillis, currentDutyCycle);

     // --- ZMIANA: Włącz pompę używając PWM ---
     ledcWrite(PUMP_LEDC_CHANNEL, currentDutyCycle);
     isPumpOn = true;
     pumpStartTime = millis();
     pumpTargetDuration = durationMillis;
     blynkUpdatePumpStatus(isPumpOn);
}

void pumpControlManualTurnOff() {
    // ... (Sprawdzenie pumpPin - bez zmian) ...
     if (pumpPin == 255) { /*...*/ return; }

    if (isPumpOn) {
        Serial.println("  [Pompa] Manualne natychmiastowe wyłączenie pompy...");
        // --- ZMIANA: Wyłącz pompę używając PWM (duty cycle = 0) ---
        ledcWrite(PUMP_LEDC_CHANNEL, 0);
        isPumpOn = false;
        pumpTargetDuration = 0;
        blynkUpdatePumpStatus(isPumpOn);
    } else {
         Serial.println("  [Pompa] Pompa już jest wyłączona.");
    }
}

bool pumpControlIsRunning() {
    return isPumpOn;
}

void pumpControlUpdate() {
    if (isPumpOn && (millis() - pumpStartTime >= pumpTargetDuration)) {
        Serial.printf("  [Pompa] Czas pracy (%u ms) upłynął. Wyłączam pompę.\n", pumpTargetDuration);
        // --- ZMIANA: Wyłącz pompę używając PWM (duty cycle = 0) ---
        ledcWrite(PUMP_LEDC_CHANNEL, 0);
        isPumpOn = false;
        pumpTargetDuration = 0;
        blynkUpdatePumpStatus(isPumpOn);
        Serial.println("  [Pompa] Pompa zatrzymana (auto-wyłączenie po czasie).");
    }
}