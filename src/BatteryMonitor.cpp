#include "BatteryMonitor.h"
#include "DeviceConfig.h"
#include <Arduino.h>

static uint8_t adcPin;

// =============================================================
//  LOLIN D32 – wbudowany dzielnik napięcia na GPIO35 (_VBAT)
//  Płytka ma na PCB dwa rezystory 100kΩ tworzące dzielnik 1:2.
//  NIE dodawaj zewnętrznych rezystorów – są już na płytce!
//
//  Schemat wbudowanego dzielnika:
//    LiPo (+) ──── 100kΩ ──── GPIO35 ──── 100kΩ ──── GND
//
//  Mnożnik = (100k + 100k) / 100k = 2.0
//  Przykład: ADC czyta 2.05V → V_BAT = 2.05 × 2.0 = 4.10V
// =============================================================
const float VOLTAGE_MULTIPLIER = 2.0f;

// Napięcie referencyjne ADC ESP32 i rozdzielczość 12-bit
// ADC_VREF może wymagać kalibracji per-płytka (typowo 3.1–3.3V)
const float ADC_VREF      = 3.2f;
const float ADC_MAX_VALUE = 4095.0f;

// Liczba próbek do uśrednienia odczytu ADC
const int NUM_READINGS = 10;

void batteryMonitorSetup() {
    adcPin = configGetBatteryAdcPin(); // powinno zwrócić 35

    if (adcPin != 255) {
        // GPIO35 jest input-only – nie wywołujemy pinMode
        // Ustaw tłumienie ADC na pełny zakres 0–3.3V
        analogSetPinAttenuation(adcPin, ADC_11db);
        Serial.printf("  [Bateria] Pin ADC: GPIO%d | Dzielnik LOLIN D32 (wbudowany 1:2) | Mnożnik: %.1f\n",
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
    int rawAdc = total / NUM_READINGS;

    // Przelicz ADC → napięcie na pinie → napięcie baterii
    float vAdc = (float)rawAdc * (ADC_VREF / ADC_MAX_VALUE);
    float vBat = vAdc * VOLTAGE_MULTIPLIER;

    Serial.printf("  [Bateria] ADC=%d | V_pin=%.3fV | V_BAT=%.3fV\n", rawAdc, vAdc, vBat);

    return vBat;
}

int batteryMonitorReadMilliVolts() {
    return (int)(batteryMonitorReadVoltage() * 1000.0f);
}

bool batteryMonitorIsLow() {
    int mv = batteryMonitorReadMilliVolts();
    int threshold = configGetLowBatteryMilliVolts();
    bool low = (mv > 0) && (mv < threshold);
    if (low) {
        Serial.printf("  [Bateria] ALARM: Niskie napięcie! %d mV < próg %d mV\n", mv, threshold);
    }
    return low;
}