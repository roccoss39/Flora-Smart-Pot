// AlarmManager.h
#ifndef ALARM_MANAGER_H
#define ALARM_MANAGER_H

/**
 * @brief Initializes the alarm module (configures buzzer pin)
 */
void alarmManagerSetup();

/**
 * @brief Updates alarm state and controls buzzer based on readings.
 * Should be called regularly in loop or after sensor reads.
 * @param waterLevel Current water level reading (0-5)
 * @param batteryVoltage Current battery voltage (V)
 * @param soilMoisture Current soil moisture (%)
 * @return true if alarm state changed, false otherwise
 */
bool alarmManagerUpdate(int waterLevel, float batteryVoltage, int soilMoisture);

/**
 * @brief Returns current alarm state regardless of sound setting
 * @return true if any alarm condition is met, false otherwise
 */
bool alarmManagerIsAlarmActive();

#endif // ALARM_MANAGER_H