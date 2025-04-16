#include "BatteryMonitor.h"
#include "DeviceConfig.h"
#include <Arduino.h>

static uint8_t adcPin;

// Parametry dzielnika napięcia (zgodnie ze schematem Flaura)
const float R1_VALUE = 100000.0; // Rezystor do GND
const float R4_VALUE = 33000.0;  // Rezystor do V_BAT
// Współczynnik, przez który mnożymy napięcie zmierzone na pinie ADC, aby dostać V_BAT
const float VOLTAGE_MULTIPLIER = (R1_VALUE + R4_VALUE) / R1_VALUE; // ≈ 1.33

// Napięcie referencyjne ADC i rozdzielczość (może wymagać kalibracji dla dokładności)
const float ADC_VREF = 3.3;       // Typowe napięcie referencyjne dla ESP32
const float ADC_MAX_VALUE = 4095.0; // Dla domyślnej rozdzielczości 12-bit

void batteryMonitorSetup() {
    adcPin = configGetBatteryAdcPin();
    if (adcPin != 255) {
         Serial.printf("  [Bateria] Skonfigurowano pin ADC: %d (Mnożnik napięcia: %.2f)\n", adcPin, VOLTAGE_MULTIPLIER);
         // Dla wejść analogowych zwykle nie ustawia się pinMode, ale można ustawić tłumienie/attenuation
         // np. analogSetPinAttenuation(adcPin, ADC_11db); // Dla pełnego zakresu 0-3.3V (domyślne)
    } else {
        Serial.println("  [Bateria] BŁĄD: Nie skonfigurowano poprawnie pinu ADC baterii!");
    }
}

int batteryMonitorReadRawADC() {
     if (adcPin == 255) return 0; // Zwróć 0 jeśli pin nie jest skonfigurowany
     return analogRead(adcPin);
}

float batteryMonitorReadVoltage() {
    if (adcPin == 255) return 0.0; // Zwróć 0V jeśli pin nie jest skonfigurowany

    // Odczytaj surową wartość ADC (można dodać uśrednianie dla stabilności)
    int rawAdc = analogRead(adcPin);

    // Przelicz wartość ADC na napięcie zmierzone na pinie ESP32
    // Uwaga: dla dokładnych pomiarów może być potrzebna kalibracja ADC ESP32!
    float vAdc = (float)rawAdc * (ADC_VREF / ADC_MAX_VALUE);

    // Przelicz napięcie z pinu ADC na rzeczywiste napięcie baterii używając dzielnika
    float vBat = vAdc * VOLTAGE_MULTIPLIER;

    Serial.printf("  [Bateria] Odczyt ADC=%d -> V_adc=%.2fV -> V_BAT=%.2fV\n", rawAdc, vAdc, vBat);

    return vBat;
}