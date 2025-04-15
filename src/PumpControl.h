#ifndef PUMPCONTROL_H
#define PUMPCONTROL_H

#include <stdint.h>

/**
 * @brief Inicjalizuje pin sterujący pompą.
 */
void pumpControlSetup();

/**
 * @brief Sprawdza warunki i ewentualnie aktywuje pompę automatycznie.
 * @param currentSoilMoisture Aktualna wilgotność gleby (%).
 * @param currentWaterLevel Aktualny poziom wody (0-5).
 */
void pumpControlActivateIfNeeded(int currentSoilMoisture, int currentWaterLevel);

/**
 * @brief Manualnie włącza pompę na podany czas (blokująco).
 * @param durationMillis Czas pracy w ms.
 */
void pumpControlManualTurnOn(uint32_t durationMillis);

/**
 * @brief Manualnie natychmiastowo wyłącza pompę.
 */
void pumpControlManualTurnOff();

/**
 * @brief Sprawdza, czy pompa jest aktualnie włączona.
 * @return true jeśli pompa pracuje, false w przeciwnym razie.
 */
bool pumpControlIsRunning();

#endif // PUMPCONTROL_H