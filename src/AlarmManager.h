#ifndef ALARMMANAGER_H
#define ALARMMANAGER_H

/**
 * @brief Inicjalizuje moduł alarmu (konfiguruje pin buzzera).
 */
void alarmManagerSetup();

/**
 * @brief Aktualizuje stan alarmu i steruje buzzerem na podstawie odczytów.
 * Należy wywoływać regularnie w pętli lub po odczycie sensorów.
 * @param waterLevel Aktualny odczytany poziom wody (0-5).
 * @param batteryVoltage Aktualne napięcie baterii (V).
 * @param soilMoisture Aktualna wilgotność gleby (%).
 */
bool alarmManagerUpdate(int waterLevel, float batteryVoltage, int soilMoisture);

bool alarmManagerIsAlarmActive(); // <-- Dodaj tę deklarację

#endif // ALARMMANAGER_H