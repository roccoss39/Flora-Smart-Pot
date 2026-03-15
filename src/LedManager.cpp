// LedManager.cpp
#include "LedManager.h"
#include <Arduino.h>

// --- Zmienne statyczne (prywatne dla tego pliku) ---
static int8_t  _ledPin        = -1;    // Numer pinu LED (-1 = niezainicjalizowany)
static uint8_t _ledOnState    = HIGH;  // Stan zapalający diodę
static uint8_t _ledOffState   = LOW;   // Stan gaszący diodę
static LedState _currentState = LED_OFF; // Aktualnie ustawiony stan (logiczny)

// Zmienne do cyklicznego migania
static unsigned long _lastBlinkTime  = 0;
static bool          _isBlinkingLedOn = false;

// Zmienne do kontroli pojedynczego mignięcia
static bool          _isSingleBlinkActive   = false;
static unsigned long _singleBlinkStartTime  = 0;
static unsigned long _singleBlinkDuration   = 0;
static LedState      _stateBeforeBlink      = LED_OFF;

// Interwały migania (ms)
const unsigned long BLINK_INTERVAL_SLOW = 750;
const unsigned long BLINK_INTERVAL_FAST = 250;

// =============================================================
//  Setup
// =============================================================

void ledManagerSetup(uint8_t pin, uint8_t ledOnState) {
    _ledPin     = (int8_t)pin;
    _ledOnState = ledOnState;
    _ledOffState = (_ledOnState == HIGH) ? LOW : HIGH;

    if (_ledPin != -1) {
        pinMode(_ledPin, OUTPUT);
        digitalWrite(_ledPin, _ledOffState); // Zgaś diodę na starcie
        _currentState = LED_OFF;
        Serial.printf("[LedMgr] Skonfigurowano pin %d (ON=%s)\n",
                      _ledPin, (_ledOnState == HIGH ? "HIGH" : "LOW"));
    } else {
        Serial.println("[LedMgr] Błąd: Nieprawidłowy numer pinu.");
    }
}

// =============================================================
//  Ustawianie stanu
// =============================================================

void ledManagerSetState(LedState newState) {
    if (_ledPin == -1) return;

    // Anuluj ewentualne pojedyncze mignięcie – jawne ustawienie stanu ma priorytet
    _isSingleBlinkActive = false;

    if (newState == _currentState) return; // Brak zmiany – nic nie rób

    _currentState = newState;
    Serial.printf("[LedMgr] Zmiana stanu na: %d\n", _currentState);

    switch (_currentState) {
        case LED_ON:
            digitalWrite(_ledPin, _ledOnState);
            break;

        case LED_OFF:
            digitalWrite(_ledPin, _ledOffState);
            break;

        case LED_BLINKING_SLOW:
        case LED_BLINKING_FAST:
            // Zacznij od zgaszonej i zresetuj timer
            _isBlinkingLedOn = false;
            digitalWrite(_ledPin, _ledOffState);
            _lastBlinkTime = millis();
            break;
    }
}

// =============================================================
//  Funkcje pomocnicze
// =============================================================

void ledManagerTurnOn() {
    ledManagerSetState(LED_ON);
}

void ledManagerTurnOff() {
    ledManagerSetState(LED_OFF);
}

LedState ledManagerGetCurrentState() {
    return _currentState;
}

// Pojedyncze mignięcie przez zadany czas (ms), potem przywrócenie poprzedniego stanu
void ledManagerBlink(unsigned long blinkDuration) {
    if (_ledPin == -1) return;

    _stateBeforeBlink      = _currentState;
    _isSingleBlinkActive   = true;
    _singleBlinkStartTime  = millis();
    _singleBlinkDuration   = blinkDuration;

    digitalWrite(_ledPin, _ledOnState);
    Serial.printf("[LedMgr] Pojedyncze mignięcie przez %lu ms\n", blinkDuration);
}

// =============================================================
//  Update – wywołuj w każdej iteracji loop()
// =============================================================

void ledManagerUpdate() {
    if (_ledPin == -1) return;

    unsigned long now = millis();

    // --- Obsługa pojedynczego mignięcia ---
    if (_isSingleBlinkActive) {
        if (now - _singleBlinkStartTime >= _singleBlinkDuration) {
            _isSingleBlinkActive = false;
            Serial.println("[LedMgr] Pojedyncze mignięcie zakończone, przywracam stan.");
            ledManagerSetState(_stateBeforeBlink);
        }
        return; // Podczas single blink nie wykonuj cyklicznego migania
    }

    // --- Obsługa cyklicznego migania ---
    unsigned long interval = 0;
    if      (_currentState == LED_BLINKING_SLOW) interval = BLINK_INTERVAL_SLOW;
    else if (_currentState == LED_BLINKING_FAST) interval = BLINK_INTERVAL_FAST;
    else return; // Stan ON/OFF – nic do roboty w update

    if (now - _lastBlinkTime >= interval) {
        _lastBlinkTime    = now;
        _isBlinkingLedOn  = !_isBlinkingLedOn;
        digitalWrite(_ledPin, _isBlinkingLedOn ? _ledOnState : _ledOffState);
    }
}