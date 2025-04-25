// PowerManager.cpp
#include "PowerManager.h"
#include "DeviceConfig.h"
#include <Arduino.h>
#include <esp_sleep.h>
#include <time.h>

// Konfiguracja NTP
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;  // Strefa czasowa UTC+1 (dla Polski)
const int daylightOffset_sec = 3600;  // Dodatkowa godzina dla czasu letniego

bool powerManagerSyncTime() {
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    
    // Poczekaj na synchronizację czasu
    Serial.println("Synchronizuję czas z serwerem NTP...");
    int retryCount = 0;
    const int maxRetries = 10;
    struct tm timeinfo;
    
    while (!getLocalTime(&timeinfo) && retryCount < maxRetries) {
        Serial.println("Oczekiwanie na synchronizację czasu...");
        delay(1000);
        retryCount++;
    }
    
    if (retryCount >= maxRetries) {
        Serial.println("Nie udało się zsynchronizować czasu!");
        return false;
    }
    
    powerManagerPrintCurrentTime();
    return true;
}

void powerManagerPrintCurrentTime() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Nie udało się pobrać czasu");
        return;
    }
    
    Serial.print("Aktualny czas: ");
    Serial.printf("%02d:%02d:%02d %02d/%02d/%04d\n", 
                  timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec,
                  timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
}

uint64_t powerManagerGetTimeToNextMeasurement() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Nie udało się pobrać czasu, używam domyślnego okresu sleep");
        uint32_t sleepSeconds = configGetSleepSeconds();
        return (uint64_t)sleepSeconds * 1000000ULL;
    }

    // Pobierz godzinę pomiaru z konfiguracji
    int measurementHour = configGetMeasurementHour();
    int measurementMinute = configGetMeasurementMinute();

    Serial.printf("Zaplanowany pomiar o %02d:%02d\n", measurementHour, measurementMinute);

    // Oblicz ile sekund do następnego pomiaru
    int currentHour = timeinfo.tm_hour;
    int currentMinute = timeinfo.tm_min;
    int currentSecond = timeinfo.tm_sec;

    Serial.printf("Aktualny czas: %02d:%02d:%02d\n", currentHour, currentMinute, currentSecond);

    int secondsToToday = (measurementHour - currentHour) * 3600 +
                        (measurementMinute - currentMinute) * 60 -
                        currentSecond;

    // Jeśli dzisiejszy czas już minął, ustaw na jutro
    if (secondsToToday <= 0) {
        secondsToToday += 24 * 60 * 60; // Dodaj 24h
    }

    Serial.printf("Następny pomiar za %d sekund (o %02d:%02d)\n",
                secondsToToday, measurementHour, measurementMinute);

    // Konwersja z sekund na mikrosekundy
    return (uint64_t)secondsToToday * 1000000ULL;
}

void powerManagerGoToDeepSleep() {
    // Oblicz czas do następnego pomiaru
    uint64_t sleepDurationUs = powerManagerGetTimeToNextMeasurement();

    Serial.printf("Przechodzę w Deep Sleep na %llu sekund...\n", sleepDurationUs / 1000000ULL);
    Serial.flush(); // Upewnij się, że Serial został wysłany

    // Konfiguracja wybudzania timerem
    esp_sleep_enable_timer_wakeup(sleepDurationUs);

    // Przejdź w Deep Sleep
    esp_deep_sleep_start();

    // Ten kod nigdy nie zostanie wykonany
}
