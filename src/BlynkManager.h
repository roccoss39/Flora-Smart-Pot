#ifndef BLYNKMANAGER_H
#define BLYNKMANAGER_H

#include <stdint.h>

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

// --- Funkcje obsługi zdarzeń z Blynk (przykłady) ---
// Te funkcje zostaną automatycznie wywołane przez Blynk.run(), gdy coś się zmieni w aplikacji
// BLYNK_CONNECTED() {
//   Serial.println("Połączono z Blynk!");
//   // Synchronizuj stan przycisków/suwaków przy połączeniu
//   // Blynk.syncVirtual(VPIN_PUMP_MANUAL_BUTTON);
// }

// BLYNK_WRITE(VPIN_PUMP_MANUAL_BUTTON) { // VPIN dla przycisku manualnego
//   int value = param.asInt();
//   if (value == 1) {
//     // Uruchom pompę manualnie na domyślny czas
//     pumpControlManualTurnOn(configGetPumpRunMillis());
//   }
// }

// BLYNK_WRITE(VPIN_PUMP_DURATION_SLIDER) { // VPIN dla suwaka czasu
//    uint32_t newDuration = param.asInt();
//    Serial.printf("Otrzymano nowy czas pracy pompy: %d ms\n", newDuration);
//    // Tutaj zapisz nową wartość do Preferences (np. za pomocą funkcji z DeviceConfig)
//    // configSetPumpRunMillis(newDuration); // Trzeba by dodać taką funkcję
// }


#endif // BLYNKMANAGER_H