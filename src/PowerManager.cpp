#include "PowerManager.h"
#include "DeviceConfig.h" // Aby pobrać czas uśpienia
#include <Arduino.h>     // Dla esp_sleep... i Serial

void powerManagerGoToDeepSleep() {
    uint32_t sleepSeconds = configGetSleepSeconds();
    uint64_t sleepDurationUs = (uint64_t)sleepSeconds * 1000000;

    Serial.printf("Przechodzę w Deep Sleep na %d sekund (%llu us)...\n", sleepSeconds, sleepDurationUs);
    Serial.flush();

    esp_sleep_enable_timer_wakeup(sleepDurationUs);
    esp_deep_sleep_start();
}