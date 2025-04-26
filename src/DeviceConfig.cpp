#include "DeviceConfig.h"
#include <Preferences.h>
#include <Arduino.h>

Preferences preferences;
const char* PREF_NAMESPACE = "flaura_cfg_1"; // Twoja przestrzeń nazw

#define NUM_WATER_LEVELS_CONFIG 5
#define PREF_MEASUREMENT_HOUR "meas_hour"  
#define PREF_MEASUREMENT_MINUTE "meas_min"

// --- Klucze dla Preferences ---
const char* PREF_SOIL_PIN = "soilPin";
const char* PREF_SOIL_DRY = "soilDry";
const char* PREF_SOIL_WET = "soilWet";
const char* PREF_SOIL_VCC = "soilVccPin";
const char* PREF_WL_PIN[NUM_WATER_LEVELS_CONFIG] = { "wlPin1", "wlPin2", "wlPin3", "wlPin4", "wlPin5" };
const char* PREF_PUMP_PIN = "pumpPin";
const char* PREF_PUMP_RUN_MS = "pumpMs";
const char* PREF_SOIL_THRESHOLD = "soilThresh";
const char* PREF_BAT_ADC_PIN = "batAdcPin";
const char* PREF_DHT_PIN = "dhtPin";
const char* PREF_MPU_INT_PIN = "mpuIntPin"; // Nowy klucz dla pinu INT MPU
const char* PREF_SLEEP_SEC = "sleepSec";
const char* PREF_CONT_MODE = "contMode";
const char* PREF_BUZZER_PIN = "buzzerPin";
const char* PREF_ALARM_SND_EN = "almSndEn";

// --- Domyślne wartości konfiguracji ---
// Soil Sensor - Twoje wartości, z kontrolą VCC
const uint8_t DEFAULT_SOIL_PIN = 34;
const int DEFAULT_SOIL_DRY = 2755;
const int DEFAULT_SOIL_WET = 930;
const int DEFAULT_SOIL_VCC_PIN = 26;
// Water Level Sensor - Twoja kolejność pinów
const uint8_t DEFAULT_WL_PIN[NUM_WATER_LEVELS_CONFIG] = { 19, 18, 5, 17, 16 };
// Pump
const uint8_t DEFAULT_PUMP_PIN = 25;
const uint32_t DEFAULT_PUMP_RUN_MS = 3000;
const int DEFAULT_SOIL_THRESHOLD = 30;
// Battery Monitor
const uint8_t DEFAULT_BAT_ADC_PIN = 35;
// DHT Sensor
const uint8_t DEFAULT_DHT_PIN = 4;
// MPU Sensor
const uint8_t DEFAULT_MPU_INT_PIN = 33; // Domyślny pin dla przerwania MPU
// General
const uint32_t DEFAULT_SLEEP_SECONDS = 3600; // 1 godzina
const bool DEFAULT_CONTINUOUS_MODE = true; // !! DOMYŚLNIE Deep Sleep !! Zmień na true do debugowania.
//Blynk
const char* PREF_BLYNK_INTERVAL = "blynkInt";
const uint32_t DEFAULT_BLYNK_SEND_INTERVAL_SEC = 30; // Domyślnie co 60 sekund
static uint32_t blynkSendIntervalSec;

const uint8_t DEFAULT_BUZZER_PIN = 23; // Domyślny pin dla buzzera

const bool DEFAULT_ALARM_SOUND_ENABLED = true;
const int DEFAULT_LOW_BATTERY_MV = 2900; // Np. 3.3V
const int DEFAULT_LOW_SOIL_PERCENT = 20; // Np. 20%


// Zmienne statyczne
static uint8_t soilSensorPin;
static int soilAdcDry;
static int soilAdcWet;
static int soilVccPin;
static uint8_t waterLevelPins[NUM_WATER_LEVELS_CONFIG];
static uint8_t pumpPin;
static uint32_t pumpRunMillis;
static int soilMoistureThreshold;
static uint8_t batteryAdcPin;
static uint8_t dhtPin;
static uint8_t mpuIntPin; // Zmienna dla pinu INT MPU
static uint32_t sleepDurationSeconds;
static bool continuousMode;
static uint8_t buzzerPin;
static int lowBatteryMilliVolts; // Deklaracja zmiennej dla progu baterii
static int lowSoilPercent;  
static bool alarmSoundEnabled;

// Zapisuje domyślne, jeśli brakuje klucza PREF_SLEEP_SEC
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
        preferences.putUChar(PREF_BAT_ADC_PIN, DEFAULT_BAT_ADC_PIN);
        preferences.putUChar(PREF_DHT_PIN, DEFAULT_DHT_PIN);
        preferences.putUChar(PREF_MPU_INT_PIN, DEFAULT_MPU_INT_PIN); // Zapisz pin INT MPU
        preferences.putUInt(PREF_SLEEP_SEC, DEFAULT_SLEEP_SECONDS); 
        preferences.putBool(PREF_CONT_MODE, DEFAULT_CONTINUOUS_MODE); //?
        preferences.putUInt(PREF_BLYNK_INTERVAL, DEFAULT_BLYNK_SEND_INTERVAL_SEC);
        preferences.putUChar(PREF_BUZZER_PIN, DEFAULT_BUZZER_PIN);
        preferences.putBool(PREF_ALARM_SND_EN, DEFAULT_ALARM_SOUND_ENABLED);
        preferences.putInt("lowBatMv", DEFAULT_LOW_BATTERY_MV);
        preferences.putInt("lowSoilPct", DEFAULT_LOW_SOIL_PERCENT);

               // UWAGA: Te progi powinny być konfigurowalne przez Blynk w przyszłości
            //    preferences.putInt("lowBatMv", DEFAULT_LOW_BATTERY_MV);
            //    preferences.putInt("lowSoilPct", DEFAULT_LOW_SOIL_PERCENT);

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
    batteryAdcPin = preferences.getUChar(PREF_BAT_ADC_PIN, DEFAULT_BAT_ADC_PIN);
    dhtPin = preferences.getUChar(PREF_DHT_PIN, DEFAULT_DHT_PIN);
    mpuIntPin = preferences.getUChar(PREF_MPU_INT_PIN, DEFAULT_MPU_INT_PIN); // Wczytaj pin INT MPU
    sleepDurationSeconds = preferences.getUInt(PREF_SLEEP_SEC, DEFAULT_SLEEP_SECONDS);
    continuousMode = preferences.getBool(PREF_CONT_MODE, DEFAULT_CONTINUOUS_MODE);
    blynkSendIntervalSec = preferences.getUInt(PREF_BLYNK_INTERVAL, DEFAULT_BLYNK_SEND_INTERVAL_SEC);
    buzzerPin = preferences.getUChar(PREF_BUZZER_PIN, DEFAULT_BUZZER_PIN);
    alarmSoundEnabled = preferences.getBool(PREF_ALARM_SND_EN, DEFAULT_ALARM_SOUND_ENABLED);
    lowBatteryMilliVolts = preferences.getInt("lowBatMv", DEFAULT_LOW_BATTERY_MV);
    lowSoilPercent = preferences.getInt("lowSoilPct", DEFAULT_LOW_SOIL_PERCENT);

    preferences.end();

    // Wydrukuj konfigurację
    Serial.println("Wczytano konfigurację:");
    Serial.printf("  Tryb ciągły: %s (false = Deep Sleep)\n", continuousMode ? "TAK" : "NIE");
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
    Serial.printf("  Pin ADC baterii: %d\n", batteryAdcPin);
    Serial.printf("  Pin DHT11: %d\n", dhtPin);
    Serial.printf("  Pin INT MPU6500: %d\n", mpuIntPin); // Wydrukuj pin INT MPU
    Serial.printf("  Czas uśpienia: %d s\n", sleepDurationSeconds);
    Serial.printf("  Interwał wysyłania Blynk: %d s\n", blynkSendIntervalSec);
    Serial.printf("  Pin Buzzera: %d\n", buzzerPin);
    Serial.printf("  Dźwięk alarmu włączony: %s\n", alarmSoundEnabled ? "TAK" : "NIE");
    Serial.printf("  Próg alarmu niskiej baterii: %d mV\n", lowBatteryMilliVolts);
    Serial.printf("  Próg alarmu niskiej wilg. gleby: %d %%\n", lowSoilPercent);


    Serial.println("--------------------");
}

// Gettery
bool configIsContinuousMode() { return continuousMode; }
uint8_t configGetSoilPin() { return soilSensorPin; }
int configGetSoilDryADC() { return soilAdcDry; }
int configGetSoilWetADC() { return soilAdcWet; }
int configGetSoilVccPin() { return soilVccPin; }
uint32_t configGetSleepSeconds() { return sleepDurationSeconds; }
uint8_t configGetPumpPin() { return pumpPin; }
uint32_t configGetPumpRunMillis() { return pumpRunMillis; }
int configGetSoilThresholdPercent() { return soilMoistureThreshold; }
uint8_t configGetBatteryAdcPin() { return batteryAdcPin; }
uint8_t configGetDhtPin() { return dhtPin; }
uint8_t configGetMpuIntPin() { return mpuIntPin; } // Getter dla pinu INT MPU
uint32_t configGetBlynkSendIntervalSec() { return blynkSendIntervalSec; }
uint8_t configGetBuzzerPin() { return buzzerPin; }
bool configIsAlarmSoundEnabled() { return alarmSoundEnabled; }

uint8_t configGetWaterLevelPin(int level) {
    if (level >= 1 && level <= NUM_WATER_LEVELS_CONFIG) {
        return waterLevelPins[level - 1];
    }
    return 255;
}

// --- IMPLEMENTACJA NOWYCH SETTERÓW ---

/**
 * @brief Ustawia i zapisuje nowy czas pracy pompy.
 */
void configSetPumpRunMillis(uint32_t durationMs) {
    // Walidacja wartości (przykładowa - max 30 sekund)
    if (durationMs > 30000) {
        durationMs = 30000;
        Serial.println("Ostrzeżenie: Czas pracy pompy ograniczony do 30000 ms.");
    }
    if (durationMs < 500) { // Minimalny czas
         durationMs = 500;
         Serial.println("Ostrzeżenie: Minimalny czas pracy pompy to 500 ms.");
    }

    if (pumpRunMillis != durationMs) { // Zapisz tylko jeśli wartość się zmieniła
        pumpRunMillis = durationMs; // Zaktualizuj wartość w pamięci RAM
        preferences.begin(PREF_NAMESPACE, false); // Otwórz R/W
        preferences.putUInt(PREF_PUMP_RUN_MS, pumpRunMillis); // Zapisz nową wartość do Flash
        preferences.end(); // Zamknij
        Serial.printf("Zapisano nowy czas pracy pompy: %d ms\n", pumpRunMillis);
    }
}
/**
 * @brief Ustawia i zapisuje nowy próg wilgotności gleby.
 */
void configSetSoilThresholdPercent(int threshold) {
    // Walidacja wartości (0-100 %)
    if (threshold < 0) threshold = 0;
    if (threshold > 100) threshold = 100;

    if (soilMoistureThreshold != threshold) { // Zapisz tylko jeśli wartość się zmieniła
        soilMoistureThreshold = threshold; // Zaktualizuj wartość w RAM
        preferences.begin(PREF_NAMESPACE, false); // Otwórz R/W
        preferences.putInt(PREF_SOIL_THRESHOLD, soilMoistureThreshold); // Zapisz do Flash
        preferences.end(); // Zamknij
        Serial.printf("Zapisano nowy próg wilgotności dla pompy: %d %%\n", soilMoistureThreshold);
    }
}


int configGetMeasurementHour() {
    preferences.begin(PREF_NAMESPACE, true); // Read-only mode
    int hour = preferences.getInt(PREF_MEASUREMENT_HOUR, 8); // Default 8:00
    preferences.end();
    return hour;
}

int configGetMeasurementMinute() {
    preferences.begin(PREF_NAMESPACE, true); // Read-only mode
    int minute = preferences.getInt(PREF_MEASUREMENT_MINUTE, 0);
    preferences.end();
    return minute;
}

bool configSetMeasurementTime(int hour, int minute) {
    if (hour >= 0 && hour < 24 && minute >= 0 && minute < 60) {
        preferences.begin(PREF_NAMESPACE, false); // Read-write mode
        preferences.putInt(PREF_MEASUREMENT_HOUR, hour);
        preferences.putInt(PREF_MEASUREMENT_MINUTE, minute);
        preferences.end();
        return true;
    }
    return false;
}

void configSetContinuousMode(bool enabled)
{
    continuousMode = enabled;
    preferences.begin(PREF_NAMESPACE, false); // Read-write mode
    preferences.putBool(PREF_CONT_MODE, continuousMode);
    preferences.end();
}

int configGetLowBatteryMilliVolts() {
    // Zwraca wartość zmiennej statycznej przechowującej próg
    // Upewnij się, że zmienna 'lowBatteryMilliVolts' jest zadeklarowana statycznie wyżej w tym pliku
    return lowBatteryMilliVolts;
}

int configGetLowSoilPercent() {
    // Zwraca wartość zmiennej statycznej przechowującej próg
    // Upewnij się, że zmienna 'lowSoilPercent' jest zadeklarowana statycznie wyżej w tym pliku
    return lowSoilPercent;
}

void configSetAlarmSoundEnabled(bool enabled) {
    if (alarmSoundEnabled != enabled) {
        alarmSoundEnabled = enabled;
        preferences.begin(PREF_NAMESPACE, false);
        preferences.putBool(PREF_ALARM_SND_EN, alarmSoundEnabled);
        preferences.end();
        Serial.printf("Zapisano nowy stan dźwięku alarmu: %s\n", alarmSoundEnabled ? "Włączony" : "Wyłączony");
    }
}