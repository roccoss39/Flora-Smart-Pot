// PumpControl.h - Niewielkie poprawki
#ifndef PUMP_CONTROL_H
#define PUMP_CONTROL_H

#include <stdint.h>

/**
 * @brief Initializes the pump control pin with PWM capabilities
 */
void pumpControlSetup();

/**
 * @brief Checks conditions and activates pump automatically if needed
 * @param currentSoilMoisture Current soil moisture (%)
 * @param currentWaterLevel Current water level (0-5)
 */
void pumpControlActivateIfNeeded(int currentSoilMoisture, int currentWaterLevel);

/**
 * @brief Manually turns on the pump for specified time (non-blocking)
 * @param durationMillis Duration in ms
 */
void pumpControlManualTurnOn(uint32_t durationMillis);

/**
 * @brief Manually turns off the pump immediately
 */
void pumpControlManualTurnOff();

/**
 * @brief Checks if pump is currently running
 * @return true if pump is running, false otherwise
 */
bool pumpControlIsRunning();

/**
 * @brief Updates pump state, should be called regularly in loop()
 */
void pumpControlUpdate();

#endif // PUMP_CONTROL_H