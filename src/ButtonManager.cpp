
#include "ButtonManager.h"
#include "DeviceConfig.h" // Dla configGetButtonPin() i configIsContinuousMode()
#include <Arduino.h>      // Dla pinMode, digitalRead, millis()

// Zmienne statyczne (widoczne tylko w tym pliku)
static uint8_t buttonPin = 255;            // Numer pinu GPIO przycisku
static int buttonState = HIGH;             // Aktualny stabilny stan przycisku (HIGH = nie naciśnięty)
static int lastButtonState = HIGH;         // Poprzedni odczytany stan (do debouncingu)
static unsigned long lastDebounceTime = 0; // Czas ostatniej zmiany stanu
const unsigned long debounceDelay = 50;   // Czas stabilizacji (ms)

void buttonSetup() {
    buttonPin = configGetButtonPin(); // Pobierz pin z konfiguracji
    if (buttonPin != 255) {
        // Ustaw jako wejście z podciąganiem do VCC.
        // Odczyt LOW będzie oznaczał naciśnięcie.
        pinMode(buttonPin, INPUT_PULLUP);
        Serial.printf("  [Button] Skonfigurowano pin przycisku %d jako INPUT_PULLUP.\n", buttonPin);
    } else {
        Serial.println("  [Button] OSTRZEŻENIE: Pin przycisku nieskonfigurowany!");
    }
    // Ustaw stan początkowy
    buttonState = (buttonPin != 255) ? digitalRead(buttonPin) : HIGH;
    lastButtonState = buttonState;
}

bool buttonWasPressed() {
    // Jeśli pin nie jest skonfigurowany lub nie jesteśmy w trybie ciągłym, nic nie rób
    if (buttonPin == 255 || !configIsContinuousMode()) {
        return false;
    }
    bool pressedEvent = false; // Flaga sygnalizująca wykrycie naciśnięcia
    int reading = digitalRead(buttonPin); // Odczytaj aktualny stan pinu

    // Jeśli stan się zmienił w stosunku do poprzedniego *odczytu* - zresetuj timer debouncingu
    if (reading != lastButtonState) {
        lastDebounceTime = millis();
    }

    // Jeśli od ostatniej zmiany minął czas stabilizacji
    if ((millis() - lastDebounceTime) > debounceDelay) {
        // Jeśli obecny odczyt różni się od *ostatniego stabilnego* stanu
        if (reading != buttonState) {
            buttonState = reading; // Zapisz nowy stabilny stan

            // Jeśli nowym stabilnym stanem jest LOW (przycisk właśnie został naciśnięty)
            if (buttonState == LOW) {
                pressedEvent = true; // Zasygnalizuj zdarzenie naciśnięcia
                Serial.println("\n[Main] Wykryto sygnał naciśnięcia przycisku - Wywołuję ręczny pomiar!");
            } else {
                // Przycisk został właśnie puszczony (przejście z LOW na HIGH)
                 Serial.println("[Button] Wykryto puszczenie przycisku.");
            }
        }
    }

    lastButtonState = reading; // Zapisz bieżący odczyt do następnej iteracji
    return pressedEvent; // Zwróć true tylko w momencie wykrycia naciśnięcia
}