#include "WaterLevelSensor.h"
#include "DeviceConfig.h" // Do pobrania pinów
#include <Arduino.h>

static uint8_t levelPins[NUM_WATER_LEVELS]; // Przechowuje numery pinów dla L1-L5

void waterLevelSensorSetup() {
    Serial.print("  [Poz. Wody] Konfiguruję piny: ");
    for (int i = 0; i < NUM_WATER_LEVELS; i++) {
        levelPins[i] = configGetWaterLevelPin(i + 1); // Pobierz pin dla poziomu 1, 2, ..., 5
        uint8_t currentPin = levelPins[i];
        if (currentPin != 255) { // Sprawdź, czy pin jest poprawnie skonfigurowany (używamy 255 jako błąd uint8_t)
            pinMode(currentPin, INPUT_PULLUP); // Ustaw jako wejście z podciąganiem do VCC
            Serial.printf("L%d=%d ", i + 1, currentPin);
        } else {
            Serial.printf("L%d=BŁĄD! ", i + 1);
        }
    }
    Serial.println();
}

int waterLevelSensorReadLevel() {
    int currentLevel = 0; // Domyślnie poziom 0 (poniżej L1)
    Serial.print("  [Poz. Wody] Stany pinów (L1-L5): ");

    for (int i = 0; i < NUM_WATER_LEVELS; i++) {
        uint8_t currentPin = levelPins[i];
        if (currentPin != 255) { // Sprawdź tylko poprawnie skonfigurowane piny
            int pinState = digitalRead(currentPin);
            Serial.printf("L%d=%d(%s) ", i + 1, currentPin, (pinState == LOW ? "LOW/WODA" : "HIGH/SUCHO"));

            // Jeśli pin jest w stanie LOW, oznacza to, że woda dosięga tej sondy
            if (pinState == LOW) {
                currentLevel = i + 1; // Aktualizujemy poziom na numer tej sondy (1-5)
            }
        }
    }
    Serial.println();

    // Pętla przeszła od L1 do L5, więc `currentLevel` przechowuje numer
    // najwyższej sondy, która została zwarta do GND przez wodę.
    // Dodatkowa kontrola spójności (przykład):
    if (currentLevel > 0) {
        // Sprawdź, czy wszystkie niższe poziomy też są LOW
        for (int i = 0; i < currentLevel - 1; i++) {
             uint8_t lowerPin = levelPins[i];
             if(lowerPin != 255 && digitalRead(lowerPin) == HIGH) {
                //TODO!!!
                 Serial.printf("\n!!! BŁĄD SPÓJNOŚCI: Poziom %d jest LOW, ale poziom %d jest HIGH! Sprawdź sondy/połączenia.\n", currentLevel, i+1);
                 // Można tu dodać logikę obsługi błędu, np. zwrócić wartość specjalną (-1)
                 // return -1;
             }
        }
    }


    return currentLevel;
}