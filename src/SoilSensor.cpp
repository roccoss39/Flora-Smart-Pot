#include "SoilSensor.h"
#include "DeviceConfig.h" // Aby uzyskać dostęp do konfiguracji
#include <Arduino.h>

static uint8_t sensorPin;
static int adcDry;
static int adcWet;
static int vccPin; // Przechowuje pin VCC (-1 jeśli nieużywany)

void soilSensorSetup() {
    // Pobierz konfigurację z modułu DeviceConfig
    sensorPin = configGetSoilPin();
    adcDry = configGetSoilDryADC();
    adcWet = configGetSoilWetADC();
    vccPin = configGetSoilVccPin();

    // Skonfiguruj pin VCC, jeśli jest używany
    if (vccPin != -1) {
        pinMode(vccPin, OUTPUT);
        digitalWrite(vccPin, LOW); // Domyślnie wyłączony
        Serial.printf("  [Wilgotność] Skonfigurowano pin VCC: %d\n", vccPin);
    }
     Serial.printf("  [Wilgotność] Skonfigurowano pin ADC: %d (Kalibracja: Sucho=%d, Mokro=%d)\n", sensorPin, adcDry, adcWet);
}

int soilSensorReadPercent() {
    int sensorValue = 0;
    int moisturePercent = 0;

    // Włącz zasilanie, jeśli VCC Pin jest skonfigurowany
    if (vccPin != -1) {
        digitalWrite(vccPin, HIGH);
        delay(500); // Czas na stabilizację
    }

    // Uśrednianie odczytów
    long totalValue = 0;
    int samples = 5;
    for (int i = 0; i < samples; i++) {
        totalValue += analogRead(sensorPin);
        delay(50);
    }
    sensorValue = totalValue / samples;

    // Wyłącz zasilanie, jeśli VCC Pin jest skonfigurowany
    if (vccPin != -1) {
        digitalWrite(vccPin, LOW);
    }

    Serial.printf("  [Wilgotność] Surowy odczyt ADC (Pin %d): %d\n", sensorPin, sensorValue);

    // Przeliczanie na procenty (logika jak poprzednio, ale używa zmiennych lokalnych)
    if (adcDry > adcWet) {
        int constrainedValue = constrain(sensorValue, adcWet, adcDry);
        moisturePercent = map(constrainedValue, adcDry, adcWet, 0, 100);
    } else {
        int constrainedValue = constrain(sensorValue, adcDry, adcWet);
        moisturePercent = map(constrainedValue, adcDry, adcWet, 0, 100);
    }

    return moisturePercent;
}