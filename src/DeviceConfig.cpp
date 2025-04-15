#include "DeviceConfig.h"
#include <Preferences.h>
#include <Arduino.h>

Preferences preferences;
// Używamy Twojej przestrzeni nazw:
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
// Ustawienia ogólne
const char* PREF_SLEEP_SEC = "sleepSec";
const char* PREF_CONT_MODE = "contMode";

// --- Domyślne wartości konfiguracji ---
// Czujnik wilgotności - Twoje wartości po kalibracji
const uint8_t DEFAULT_SOIL_PIN = 34;
const int DEFAULT_SOIL_DRY = 2755;     // Twoja wartość "sucho"
const int DEFAULT_SOIL_WET = 930;      // Twoja wartość "mokro"
const int DEFAULT_SOIL_VCC_PIN = 26;   // Twój wybór pinu VCC
// Czujnik poziomu wody (L1-L5) - piny sugerowane dla Keyestudio ESP32
const uint8_t DEFAULT_WL_PIN[NUM_WATER_LEVELS_CONFIG] = {
    19, // Pin dla poziomu 1 (najniższy) - BYŁO: 16
    18, // Pin dla poziomu 2 - BYŁO: 17
    5,  // Pin dla poziomu 3 - BEZ ZMIAN
    17, // Pin dla poziomu 4 - BYŁO: 18
    16  // Pin dla poziomu 5 (najwyższy) - BYŁO: 19
};
// Ustawienia ogólne
const uint32_t DEFAULT_SLEEP_SECONDS = 3600; // Domyślnie 1 godzina
const bool DEFAULT_CONTINUOUS_MODE = true; // !! USTAWIONE NA true ZGODNIE Z TWOIM KODEM (DO TESTÓW) !! Zmień na false dla Deep Sleep.

// Zmienne statyczne przechowujące wczytaną konfigurację
static uint8_t soilSensorPin;
static int soilAdcDry;
static int soilAdcWet;
static int soilVccPin;
static uint8_t waterLevelPins[NUM_WATER_LEVELS_CONFIG];
static uint32_t sleepDurationSeconds;
static bool continuousMode;

// Prywatna funkcja pomocnicza - zapisuje domyślne, jeśli brakuje klucza
// UWAGA: Na czas dalszych zmian/testów możesz usunąć warunek 'if', aby zawsze nadpisywać ustawienia przy starcie.
void saveDefaultConfigurationIfNeeded() {
     if (!preferences.isKey(PREF_SLEEP_SEC)) { // Sprawdza tylko jeden klucz
        Serial.println("Pierwsze uruchomienie lub brak klucza konfiguracji. Zapisuję wartości domyślne...");
        // Soil Sensor
        preferences.putUChar(PREF_SOIL_PIN, DEFAULT_SOIL_PIN);
        preferences.putInt(PREF_SOIL_DRY, DEFAULT_SOIL_DRY); // Zapisze Twoje wartości
        preferences.putInt(PREF_SOIL_WET, DEFAULT_SOIL_WET); // Zapisze Twoje wartości
        preferences.putInt(PREF_SOIL_VCC, DEFAULT_SOIL_VCC_PIN); // Zapisze Twój wybór
        // Water Level Sensor
        for (int i = 0; i < NUM_WATER_LEVELS_CONFIG; i++) {
            preferences.putUChar(PREF_WL_PIN[i], DEFAULT_WL_PIN[i]);
        }
        // General
        preferences.putUInt(PREF_SLEEP_SEC, DEFAULT_SLEEP_SECONDS);
        preferences.putBool(PREF_CONT_MODE, DEFAULT_CONTINUOUS_MODE); // Zapisze 'true'
     }
}

void configSetup() {
    preferences.begin(PREF_NAMESPACE, false); // Używa Twojej przestrzeni nazw
    saveDefaultConfigurationIfNeeded();

    // Wczytaj wartości do zmiennych statycznych
    // Soil
    soilSensorPin = preferences.getUChar(PREF_SOIL_PIN, DEFAULT_SOIL_PIN);
    soilAdcDry = preferences.getInt(PREF_SOIL_DRY, DEFAULT_SOIL_DRY);
    soilAdcWet = preferences.getInt(PREF_SOIL_WET, DEFAULT_SOIL_WET);
    soilVccPin = preferences.getInt(PREF_SOIL_VCC, DEFAULT_SOIL_VCC_PIN);
    // Water Level
    for (int i = 0; i < NUM_WATER_LEVELS_CONFIG; i++) {
        waterLevelPins[i] = preferences.getUChar(PREF_WL_PIN[i], DEFAULT_WL_PIN[i]);
    }
    // General
    sleepDurationSeconds = preferences.getUInt(PREF_SLEEP_SEC, DEFAULT_SLEEP_SECONDS);
    continuousMode = preferences.getBool(PREF_CONT_MODE, DEFAULT_CONTINUOUS_MODE);

    preferences.end();

    // Wydrukuj wczytaną konfigurację
    Serial.println("Wczytano konfigurację:");
    Serial.printf("  Tryb ciągły: %s (Pamiętaj zmienić na 'false' dla Deep Sleep!)\n", continuousMode ? "TAK" : "NIE");
    Serial.printf("  Pin czujnika wilg.: %d\n", soilSensorPin);
    Serial.printf("  Kalibracja wilg.: Sucho=%d, Mokro=%d\n", soilAdcDry, soilAdcWet); // Wyświetli Twoje wartości
    Serial.printf("  Pin zasilania czujnika wilg.: %d (%s)\n", soilVccPin, (soilVccPin == -1 || soilVccPin == 255 ? "nieużywany/błąd" : "używany")); // Wyświetli Twój wybór
    Serial.print("  Piny czujnika poz. wody (L1-L5): ");
    for (int i = 0; i < NUM_WATER_LEVELS_CONFIG; i++) {
        Serial.printf("%d ", waterLevelPins[i]);
    }
    Serial.println();
    Serial.printf("  Czas uśpienia: %d s\n", sleepDurationSeconds);
    Serial.println("--------------------");

    // Konfiguracja pinu zasilania czujnika wilgotności, jeśli jest używany
    // (Dodano sprawdzanie 255 jako błędu dla uint8_t)
    if (soilVccPin != -1 && soilVccPin != 255) {
        pinMode(soilVccPin, OUTPUT);
        digitalWrite(soilVccPin, LOW);
    }
}

// Implementacje funkcji dostępowych (getterów) - bez zmian
bool configIsContinuousMode() { return continuousMode; }
uint8_t configGetSoilPin() { return soilSensorPin; }
int configGetSoilDryADC() { return soilAdcDry; }
int configGetSoilWetADC() { return soilAdcWet; }
int configGetSoilVccPin() { return soilVccPin; }
uint32_t configGetSleepSeconds() { return sleepDurationSeconds; }

uint8_t configGetWaterLevelPin(int level) {
    if (level >= 1 && level <= NUM_WATER_LEVELS_CONFIG) {
        return waterLevelPins[level - 1];
    }
    return 255; // Zwróć nieprawidłowy numer pinu (uint8_t) w razie błędu
}