#include "WaterLevelSensor.h"
#include "DeviceConfig.h"  // do pobrania pinów
#include <Arduino.h>

static uint8_t levelPins[NUM_WATER_LEVELS];       // piny sond poziomów L1–L5
static uint8_t groundPin;                         // wspólna sonda (analogowa)
static uint16_t sensorThreshold;                  // próg ADC
static const uint16_t sensorWaitingTime = 10;     // ms, do ustabilizowania napięcia

void waterLevelSensorSetup() {
    Serial.print("  [Poz. Wody] Konfiguruję piny: ");
    // Pobieramy piny poziomów
    for (int i = 0; i < NUM_WATER_LEVELS; i++) {
        levelPins[i] = configGetWaterLevelPin(i + 1);
        Serial.printf("L%d=%d ", i + 1, levelPins[i]);
    }
    // Pobieramy wspólną sondę ADC
    groundPin = configGetWaterLevelGroundPin();
    Serial.printf("GND_SONDA=%d\n", groundPin);
    
    // Konfigurujemy: sondy poziomów jako INPUT (stan wysoki/niski ustawiany doraźnie)
    for (int i = 0; i < NUM_WATER_LEVELS; i++) {
        if (levelPins[i] != 255) {
            pinMode(levelPins[i], INPUT);
        }
    }
    // Sonda odniesienia jako wejście analogowe
    pinMode(groundPin, INPUT);
}

int waterLevelSensorReadLevel() {
    uint8_t resultLevel = 0;
    sensorThreshold = configGetWaterLevelThreshold();

    // Iterujemy od ostatniego indeksu (wysoki pin) do 0
    for (int8_t idx = NUM_WATER_LEVELS - 1; idx >= 0; --idx) {
        uint8_t pin = configGetWaterLevelPin(idx + 1);
        if (pin == 255) continue;

        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH);
        delay(sensorWaitingTime);

        uint16_t adc = analogRead(configGetWaterLevelGroundPin());

        pinMode(pin, INPUT);
        Serial.printf("[Poz%d pin=%d] ADC=%u\n", idx+1, pin, adc);

        if (adc > sensorThreshold) {
            resultLevel = idx + 1;
            break;
        }
    }

    Serial.printf(">> Poziom wody: %d\n", resultLevel);
    return resultLevel;
}

