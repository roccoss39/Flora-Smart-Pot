#include "DeviceConfig.h"
#include <Preferences.h>
#include <Arduino.h>

Preferences preferences;
// Twoja przestrzeń nazw:
const char* PREF_NAMESPACE = "flaura_cfg_1";

#define NUM_WATER_LEVELS_CONFIG 5 // Liczba poziomów wody do skonfigurowania

// --- Klucze dla Preferences ---
// Czujnik wilgotności
const char* PREF_SOIL_PIN = "soilPin";
const char* PREF_SOIL_DRY = "soilDry";
const char* PREF_SOIL_WET = "soilWet";
const char* PREF_SOIL_VCC = "soilVccPin";
// Czujnik poziomu wody (L1-L5)
const char* PREF_WL_PIN[NUM_WATER_LEVELS_CONFIG] = {
    "wlPin1", "wlPin2", "wlPin3", "wlPin4", "wlPin5"
};
// Pompka
const char* PREF_PUMP_PIN = "pumpPin";
const char* PREF_PUMP_RUN_MS = "pumpMs";
const char* PREF_SOIL_THRESHOLD = "soilThresh";
// Ustawienia ogólne
const char* PREF_SLEEP_SEC = "sleepSec";
const char* PREF_CONT_MODE = "contMode";

// --- Domyślne wartości konfiguracji ---
// Czujnik wilgotności - Twoje wartości
const uint8_t DEFAULT_SOIL_PIN = 34;
const int DEFAULT_SOIL_DRY = 2755;
const int DEFAULT_SOIL_WET = 930;
const int DEFAULT_SOIL_VCC_PIN = 26; // Używamy pinu 26
// Czujnik poziomu wody (L1-L5) - Twoja kolejność pinów
const uint8_t DEFAULT_WL_PIN[NUM_WATER_LEVELS_CONFIG] = {
    19, // Pin dla poziomu 1 (najniższy)
    18, // Pin dla poziomu 2
    5,  // Pin dla poziomu 3
    17, // Pin dla poziomu 4
    16  // Pin dla poziomu 5 (najwyższy)
};
// Pompka
const uint8_t DEFAULT_PUMP_PIN = 25;
const uint32_t DEFAULT_PUMP_RUN_MS = 3000; // 3 sekundy
const int DEFAULT_SOIL_THRESHOLD = 30;   // Poniżej 30%
// Ustawienia ogólne
const uint32_t DEFAULT_SLEEP_SECONDS = 3600;
const bool DEFAULT_CONTINUOUS_MODE = true; // DO TESTÓW! Zmień na false dla Deep Sleep.

// Zmienne statyczne przechowujące wczytaną konfigurację
static uint8_t soilSensorPin;
static int soilAdcDry;
static int soilAdcWet;
static int soilVccPin;
static uint8_t waterLevelPins[NUM_WATER_LEVELS_CONFIG];
static uint8_t pumpPin;
static uint32_t pumpRunMillis;
static int soilMoistureThreshold;
static uint32_t sleepDurationSeconds;
static bool continuousMode;

// Zapisuje domyślne, jeśli brakuje klucza PREF_SLEEP_SEC
// (Usuń warunek 'if', aby zawsze nadpisywać na czas testów/kalibracji)
void saveDefaultConfigurationIfNeeded() {
     if (!preferences.isKey(PREF_SLEEP_SEC)) {
        Serial.println("Zapisuję wartości domyślne z kodu...");
        preferences.putUChar(PREF_SOIL_PIN, DEFAULT_SOIL_PIN);
        preferences.putInt(PREF_SOIL_DRY, DEFAULT_SOIL_DRY);
        preferences.putInt(PREF_SOIL_WET, DEFAULT_SOIL_WET);
        preferences.putInt(PREF_SOIL_VCC, DEFAULT_SOIL_VCC_PIN);
        for (int i = 0; i < NUM_WATER_LEVELS_CONFIG; i++) {
            preferences.putUChar(PREF_WL_PIN[i], DEFAULT_WL_PIN[i]);
        }
        preferences.putUChar(PREF_PUMP_PIN, DEFAULT_PUMP_PIN);
        preferences.putUInt(PREF_PUMP_RUN_MS, DEFAULT_PUMP_RUN_MS);
        preferences.putInt(PREF_SOIL_THRESHOLD, DEFAULT_SOIL_THRESHOLD);
        preferences.putUInt(PREF_SLEEP_SEC, DEFAULT_SLEEP_SECONDS);
        preferences.putBool(PREF_CONT_MODE, DEFAULT_CONTINUOUS_MODE);
     }
}

void configSetup() {
    preferences.begin(PREF_NAMESPACE, false);
    saveDefaultConfigurationIfNeeded();

    // Wczytaj wartości
    soilSensorPin = preferences.getUChar(PREF_SOIL_PIN, DEFAULT_SOIL_PIN);
    soilAdcDry = preferences.getInt(PREF_SOIL_DRY, DEFAULT_SOIL_DRY);
    soilAdcWet = preferences.getInt(PREF_SOIL_WET, DEFAULT_SOIL_WET);
    soilVccPin = preferences.getInt(PREF_SOIL_VCC, DEFAULT_SOIL_VCC_PIN);
    for (int i = 0; i < NUM_WATER_LEVELS_CONFIG; i++) {
        waterLevelPins[i] = preferences.getUChar(PREF_WL_PIN[i], DEFAULT_WL_PIN[i]);
    }
    pumpPin = preferences.getUChar(PREF_PUMP_PIN, DEFAULT_PUMP_PIN);
    pumpRunMillis = preferences.getUInt(PREF_PUMP_RUN_MS, DEFAULT_PUMP_RUN_MS);
    soilMoistureThreshold = preferences.getInt(PREF_SOIL_THRESHOLD, DEFAULT_SOIL_THRESHOLD);
    sleepDurationSeconds = preferences.getUInt(PREF_SLEEP_SEC, DEFAULT_SLEEP_SECONDS);
    continuousMode = preferences.getBool(PREF_CONT_MODE, DEFAULT_CONTINUOUS_MODE);

    preferences.end();

    // Wydrukuj konfigurację
    Serial.println("Wczytano konfigurację:");
    Serial.printf("  Tryb ciągły: %s (Pamiętaj zmienić na 'false' dla Deep Sleep!)\n", continuousMode ? "TAK" : "NIE");
    Serial.printf("  Pin czujnika wilg.: %d\n", soilSensorPin);
    Serial.printf("  Kalibracja wilg.: Sucho=%d, Mokro=%d\n", soilAdcDry, soilAdcWet);
    Serial.printf("  Pin zasilania czujnika wilg.: %d (%s)\n", soilVccPin, (soilVccPin == -1 || soilVccPin == 255 ? "nieużywany/błąd" : "używany"));
    Serial.print("  Piny czujnika poz. wody (L1-L5): ");
    for (int i = 0; i < NUM_WATER_LEVELS_CONFIG; i++) {
        Serial.printf("%d ", waterLevelPins[i]);
    }
    Serial.println();
    Serial.printf("  Pin pompy: %d\n", pumpPin);
    Serial.printf("  Czas pracy pompy: %d ms\n", pumpRunMillis);
    Serial.printf("  Próg wilgotności dla pompy: %d %%\n", soilMoistureThreshold);
    Serial.printf("  Czas uśpienia: %d s\n", sleepDurationSeconds);
    Serial.println("--------------------");

    // Konfiguracja pinu VCC dla soil (jeśli używany)
    if (soilVccPin != -1 && soilVccPin != 255) {
        pinMode(soilVccPin, OUTPUT);
        digitalWrite(soilVccPin, LOW);
    }
}

// Gettery
bool configIsContinuousMode() { return continuousMode; }
uint8_t configGetSoilPin() { return soilSensorPin; }
int configGetSoilDryADC() { return soilAdcDry; }
int configGetSoilWetADC() { return soilAdcWet; }
int configGetSoilVccPin() { return soilVccPin; } // Zwraca int, -1 jeśli nie zdefiniowano
uint32_t configGetSleepSeconds() { return sleepDurationSeconds; }
uint8_t configGetPumpPin() { return pumpPin; }
uint32_t configGetPumpRunMillis() { return pumpRunMillis; }
int configGetSoilThresholdPercent() { return soilMoistureThreshold; }

uint8_t configGetWaterLevelPin(int level) {
    if (level >= 1 && level <= NUM_WATER_LEVELS_CONFIG) {
        return waterLevelPins[level - 1];
    }
    return 255; // Zwróć nieprawidłowy numer pinu (uint8_t max)
}