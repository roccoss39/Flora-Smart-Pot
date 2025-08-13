// LedManager.cpp
#include "LedManager.h"

// --- Zmienne statyczne (prywatne dla tego pliku) ---
static uint8_t _ledPin = -1;         // Przechowuje numer pinu LED
static uint8_t _ledOnState = HIGH;   // Stan zapalający diodę
static uint8_t _ledOffState = LOW;   // Stan gaszący diodę
static LedState _currentState = LED_OFF; // Aktualnie ustawiony stan (logiczny)

static unsigned long _lastBlinkTime = 0; // Czas ostatniej zmiany stanu przy miganiu
static bool _isBlinkingLedOn = false;   // Aktualny fizyczny stan LED podczas migania

// Zmienne do kontroli pojedynczego mignięcia
static bool _isSingleBlinkActive = false;
static unsigned long _singleBlinkStartTime = 0;
static unsigned long _singleBlinkDuration = 0;
static LedState _stateBeforeBlink = LED_OFF;

// Interwały migania (w milisekundach)
const unsigned long BLINK_INTERVAL_SLOW = 750;
const unsigned long BLINK_INTERVAL_FAST = 250;

// --- Implementacje funkcji ---

void ledManagerSetup(uint8_t pin, uint8_t ledOnState) {
    _ledPin = pin;
    _ledOnState = ledOnState;
    _ledOffState = (_ledOnState == HIGH) ? LOW : HIGH; // Wyznacz stan przeciwny

    if (_ledPin != -1) {
        pinMode(_ledPin, OUTPUT);
        digitalWrite(_ledPin, _ledOffState); // Początkowo zgaś diodę
        _currentState = LED_OFF;
        Serial.printf("[LedMgr] Skonfigurowano pin %d (ON=%s)\n", _ledPin, (_ledOnState == HIGH ? "HIGH" : "LOW"));
    } else {
        Serial.println("[LedMgr] Błąd: Nieprawidłowy numer pinu.");
    }
}

void ledManagerSetState(LedState newState) {
    if (_ledPin == -1) return; // Nie rób nic, jeśli pin nie jest skonfigurowany

    // Jeśli stan się nie zmienia, nie rób nic
    if (newState == _currentState) {
        return;
    }

    _currentState = newState;
    Serial.printf("[LedMgr] Zmiana stanu na: %d\n", _currentState); // Debug

    // Natychmiast ustaw stan fizyczny dla ON/OFF
    switch (_currentState) {
        case LED_ON:
            digitalWrite(_ledPin, _ledOnState);
            break;
        case LED_OFF:
            digitalWrite(_ledPin, _ledOffState);
            break;
        case LED_BLINKING_SLOW:
        case LED_BLINKING_FAST:
            // Przy przejściu na miganie, zresetuj stan migania
            _isBlinkingLedOn = false; // Zacznij od zgaszonej
            digitalWrite(_ledPin, _ledOffState);
            _lastBlinkTime = millis(); // Zresetuj czasomierz migania
            break;
    }
}

void ledManagerUpdate() {
    if (_ledPin == -1) return;

    unsigned long currentMillis = millis();
    
    // Sprawdź, czy trwa pojedyncze mignięcie
    if (_isSingleBlinkActive) {
        if (currentMillis - _singleBlinkStartTime >= _singleBlinkDuration) {
            // Koniec mignięcia - przywróć poprzedni stan
            _isSingleBlinkActive = false;
            ledManagerSetState(_stateBeforeBlink);
            Serial.println("[LedMgr] Pojedyncze mignięcie zakończone");
        }
        return; // Podczas pojedynczego mignięcia nie wykonuj zwykłej logiki migania
    }
    
    unsigned long blinkInterval = 0;

    // Sprawdź, czy jesteśmy w stanie cyklicznego migania
    if (_currentState == LED_BLINKING_SLOW) {
        blinkInterval = BLINK_INTERVAL_SLOW;
    } else if (_currentState == LED_BLINKING_FAST) {
        blinkInterval = BLINK_INTERVAL_FAST;
    } else {
        return; // Nie jesteśmy w stanie migania, wyjdź
    }

    // Logika migania
    if (currentMillis - _lastBlinkTime >= blinkInterval) {
        _lastBlinkTime = currentMillis;
        _isBlinkingLedOn = !_isBlinkingLedOn; // Zmień stan
        digitalWrite(_ledPin, _isBlinkingLedOn ? _ledOnState : _ledOffState);
        //Serial.printf("[LedMgr] Blink! Stan: %d\n", _isBlinkingLedOn); // Debug
    }
}

// --- Implementacje funkcji pomocniczych ---
void ledManagerTurnOn() {
    ledManagerSetState(LED_ON);
}

void ledManagerTurnOff() {
    ledManagerSetState(LED_OFF);
}

LedState ledManagerGetCurrentState() {
    return _currentState;
}

void ledManagerBlink(unsigned long blinkDuration) {
    if (_ledPin == -1) return;
    
    // Zapisz poprzedni stan przed mignięciem
    _stateBeforeBlink = _currentState;
    
    // Ustaw parametry pojedynczego mignięcia
    _isSingleBlinkActive = true;
    _singleBlinkStartTime = millis();
    _singleBlinkDuration = blinkDuration;
    
    // Włącz diodę na czas mignięcia
    digitalWrite(_ledPin, _ledOnState);
    
    Serial.printf("[LedMgr] Rozpoczęto pojedyncze mignięcie (czas: %lums)\n", blinkDuration);
}