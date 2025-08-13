// PowerManager.h
#ifndef POWERMANAGER_H
#define POWERMANAGER_H

#include <stdint.h>

/**
 * @brief Przechodzi w tryb głębokiego snu na czas określony w konfiguracji
 */
void powerManagerGoToDeepSleep();

/**
 * @brief Synchronizuje czas z serwerem NTP
 * @return true jeśli synchronizacja się powiodła, false w przeciwnym razie
 */
bool powerManagerSyncTime();

/**
 * @brief Oblicza czas do następnego zaplanowanego pomiaru
 * @return Czas w mikrosekundach do następnego pomiaru
 */
uint64_t powerManagerGetTimeToNextMeasurement();

/**
 * @brief Wyświetla aktualny czas systemowy
 */
void powerManagerPrintCurrentTime();

void powerManagerConfigureButtonWakeup();

#endif // POWERMANAGER_H