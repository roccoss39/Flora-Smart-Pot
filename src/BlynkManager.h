#ifndef BLYNKMANAGER_H
#define BLYNKMANAGER_H

#include <stdint.h>
#include "secrets.h" // <-- WAŻNE: Dołącz NAJPIERW definicje/secrety
#include <BlynkSimpleEsp32.h> 
/**
 * @brief Konfiguruje dane autoryzacyjne Blynk.
 * Należy wywołać przed blynkConnect().
 * @param authToken Token autoryzacyjny urządzenia Blynk.
 * @param templateId ID szablonu Blynk.
 * @param deviceName Nazwa urządzenia Blynk.
 */
void blynkConfigure(const char* authToken, const char* templateId, const char* deviceName);

/**
 * @brief Łączy z serwerem Blynk.
 * Wymaga aktywnego połączenia WiFi.
 * @param timeoutMs Czas oczekiwania na połączenie w milisekundach.
 * @return true jeśli połączenie się powiodło, false w przeciwnym razie.
 */
bool blynkConnect(unsigned long timeoutMs = 5000);

/**
 * @brief Główna funkcja obsługi Blynk.
 * Musi być wywoływana regularnie w pętli loop(), gdy jest połączenie.
 */
void blynkRun();

/**
 * @brief Sprawdza, czy jest aktywne połączenie z serwerem Blynk.
 * @return true jeśli połączono, false w przeciwnym razie.
 */
bool blynkIsConnected();

/**
 * @brief Rozłącza z serwerem Blynk.
 * Dobrze jest wywołać przed pójściem w Deep Sleep.
 */
void blynkDisconnect();

/**
 * @brief Wysyła odczytane wartości sensorów do odpowiednich wirtualnych pinów Blynk.
 * Należy dostosować numery VPIN wewnątrz funkcji w pliku .cpp!
 */
void blynkSendSensorData(int soil, int waterLvl, float batteryV, float temp, float humid, float tilt, bool tiltAlert, bool pumpStatus);

void blynkUpdatePumpStatus(bool isRunning);

void blynkUpdateAlarmSoundEnableWidget(bool enabled);


#endif // BLYNKMANAGER_H