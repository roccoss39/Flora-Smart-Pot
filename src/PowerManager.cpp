#include "PowerManager.h"
#include "DeviceConfig.h"
#include <Arduino.h>
#include <esp_sleep.h> // Dodane dla pewności

void powerManagerGoToDeepSleep() {
    uint32_t sleepSeconds = configGetSleepSeconds();
    uint64_t sleepDurationUs = (uint64_t)sleepSeconds * 1000000;

    Serial.printf("Przechodzę w Deep Sleep na %d sekund (%llu us)...\n", sleepSeconds, sleepDurationUs);
    Serial.flush(); // Upewnij się, że Serial został wysłany

    // Konfiguracja wybudzania timerem jest robiona tutaj,
    // ale konfigurację EXT0/EXT1 robimy w main.cpp przed wywołaniem tej funkcji
    esp_sleep_enable_timer_wakeup(sleepDurationUs);

    // Przejdź w Deep Sleep
    esp_deep_sleep_start();

    // Ten kod nigdy nie zostanie wykonany
}