#include "BatteryMonitor.h"
#include "DeviceConfig.h"
#include <Arduino.h>

static uint8_t adcPin;

// =============================================================
//  Automatyczny dobór parametrów w zależności od płytki.
//  Flaga BOARD_LOLIN_D32 ustawiana jest w platformio.ini:
//    build_flags = -D BOARD_LOLIN_D32
//
//  LOLIN D32:
//    - Wbudowany dzielnik 100kΩ/100kΩ na GPIO35 (_VBAT)
//    - Mnożnik = 2.0
//    - Wymaga analogSetPinAttenuation(ADC_11db)
//
//  LOLIN32 v1.0.0 (domyślny – brak flagi):
//    - Zewnętrzny dzielnik R1=99.7kΩ (GND) / R4=33kΩ (V_BAT)
//    - Mnożnik = (R1+R4)/R1 ≈ 1.33
//    - Standardowe tłumienie ADC
// =============================================================

#ifdef BOARD_LOLIN_D32
    // ── LOLIN D32 ──────────────────────────────────────────
    // Wbudowany dzielnik 1:2 na GPIO35
    static const float VOLTAGE_MULTIPLIER = 2.0f;
    static const bool  USE_ATTENUATION    = true;
    static const char* BOARD_NAME         = "LOLIN D32 (wbudowany dzielnik 1:2)";
#else
    // ── LOLIN32 v1.0.0 ─────────────────────────────────────
    // Zewnętrzny dzielnik R1=99.7kΩ, R4=33kΩ
    static constexpr float R1 = 99700.0f;
    static constexpr float R4 = 33020.0f;
    static const float VOLTAGE_MULTIPLIER = (R1 + R4) / R1; // ≈ 1.331
    static const bool  USE_ATTENUATION    = false;
    static const char* BOARD_NAME         = "LOLIN32 v1.0.0 (zewnętrzny dzielnik R1/R4)";
#endif

// Napięcie referencyjne ADC i rozdzielczość 12-bit
// Może wymagać kalibracji per-płytka (typowo 3.1–3.3V)
static const float ADC_VREF      = 3.2f;
static const float ADC_MAX_VALUE = 4095.0f;

// Liczba próbek do uśrednienia
static const int NUM_READINGS = 10;

// =============================================================

void batteryMonitorSetup() {
    adcPin = configGetBatteryAdcPin();

    if (adcPin != 255) {
        if (USE_ATTENUATION) {
            // Potrzebne tylko na LOLIN D32 (GPIO35 input-only, pełny zakres 0–3.3V)
            analogSetPinAttenuation(adcPin, ADC_11db);
        }
        Serial.printf("  [Bateria] Płytka: %s\n", BOARD_NAME);
        Serial.printf("  [Bateria] Pin ADC: GPIO%d | Mnożnik: %.3f\n",
                      adcPin, VOLTAGE_MULTIPLIER);
    } else {
        Serial.println("  [Bateria] BŁĄD: Nieprawidłowy pin ADC baterii!");
    }
}

int batteryMonitorReadRawADC() {
    if (adcPin == 255) return 0;
    return analogRead(adcPin);
}

float batteryMonitorReadVoltage() {
    if (adcPin == 255) return 0.0f;

    // Uśrednij NUM_READINGS odczytów dla stabilności
    uint32_t total = 0;
    for (int i = 0; i < NUM_READINGS; i++) {
        total += analogRead(adcPin);
        delay(1);
    }
    int rawAdc = (int)(total / NUM_READINGS);

    // ADC → napięcie na pinie → napięcie baterii
    float vAdc = (float)rawAdc * (ADC_VREF / ADC_MAX_VALUE);
    float vBat = vAdc * VOLTAGE_MULTIPLIER;

    Serial.printf("  [Bateria] ADC=%d | V_pin=%.3fV | V_BAT=%.3fV\n",
                  rawAdc, vAdc, vBat);
    return vBat;
}

int batteryMonitorReadMilliVolts() {
    return (int)(batteryMonitorReadVoltage() * 1000.0f);
}

bool batteryMonitorIsLow() {
    int mv        = batteryMonitorReadMilliVolts();
    int threshold = configGetLowBatteryMilliVolts();
    bool low      = (mv > 0) && (mv < threshold);
    if (low) {
        Serial.printf("  [Bateria] ALARM: Niskie napięcie! %d mV < próg %d mV\n",
                      mv, threshold);
    }
    return low;
}