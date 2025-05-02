#ifndef BUTTONMANAGER_H
#define BUTTONMANAGER_H

/**
 * @brief Inicjalizuje moduł przycisku.
 * Konfiguruje pin GPIO przycisku jako wejście z podciąganiem (pull-up).
 * Należy wywołać raz w funkcji setup().
 */
void buttonSetup();

/**
 * @brief Obsługuje logikę przycisku w pętli głównej.
 * Odczytuje stan przycisku, wykonuje debouncing.
 * Sprawdza, czy urządzenie jest w trybie ciągłym.
 * Należy wywoływać regularnie w funkcji loop().
 *
 * @return true jeśli wykryto właśnie potwierdzone naciśnięcie przycisku (zbocze opadające)
 * w trybie ciągłym, false w przeciwnym razie.
 */
bool buttonWasPressed(); // Zmieniona nazwa funkcji dla jasności

#endif // BUTTONMANAGER_H