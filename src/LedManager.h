// LedManager.h
#ifndef LEDMANAGER_H
#define LEDMANAGER_H

#include <Arduino.h>

// Definicja stanów, w jakich może być dioda LED
enum LedState {
    LED_OFF,
    LED_ON,
    LED_BLINKING_SLOW, // Możemy dodać różne tryby migania
    LED_BLINKING_FAST  // Np. szybsze miganie dla alarmu
};

/**
 * @brief Inicjalizuje menedżera diody LED.
 * @param pin Numer pinu GPIO, do którego podłączona jest dioda.
 * @param ledOnState Stan (HIGH lub LOW), który zapala diodę. Domyślnie HIGH.
 */
void ledManagerSetup(uint8_t pin, uint8_t ledOnState = HIGH);

/**
 * @brief Ustawia żądany stan diody LED (stałe świecenie, zgaszenie, miganie).
 * @param state Nowy stan diody z enum LedState.
 */
void ledManagerSetState(LedState state);

/**
 * @brief Funkcja do cyklicznego wywoływania w pętli loop().
 * Obsługuje logikę migania diody, jeśli jest w odpowiednim stanie.
 */
void ledManagerUpdate();

// --- Opcjonalne funkcje pomocnicze ---
/**
 * @brief Włącza diodę na stałe (skrót do ledManagerSetState(LED_ON)).
 */
void ledManagerTurnOn();

/**
 * @brief Wyłącza diodę (skrót do ledManagerSetState(LED_OFF)).
 */
void ledManagerTurnOff();

/**
 * @brief Funkcja zwracająca aktualnie ustawiony stan diody LED.
 * @return Aktualny stan diody z enum LedState.
 */
LedState ledManagerGetCurrentState();

/**
 * @brief Inicjuje pojedyncze mignięcie diodą bez blokowania.
 * @param blinkDuration Czas trwania mignięcia w milisekundach.
 */
void ledManagerBlink(unsigned long blinkDuration = 100);

#endif // LEDMANAGER_H