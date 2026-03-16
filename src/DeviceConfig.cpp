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
const char* PREF_WL_GROUND_PIN = "wlGndPin";
const char* PREF_WL_THRESHOLD = "wlThresh";

const char* PREF_PUMP_PIN = "pumpPin";
const char* PREF_PUMP_RUN_MS = "pumpMs";
const char* PREF_SOIL_THRESHOLD = "soilThresh";
const char* PREF_BAT_ADC_PIN = "batAdcPin";
const char* PREF_DHT_PIN = "dhtPin";
const char* PREF_DHT_PWR_PIN = "dhtPwrPin";
const char* PREF_MPU_INT_PIN = "mpuIntPin";
const char* PREF_SLEEP_SEC = "sleepSec";
const char* PREF_CONT_MODE = "contMode";
const char* PREF_BUZZER_PIN = "buzzerPin";
const char* PREF_ALARM_SND_EN = "almSndEn";
const char* PREF_LOW_BAT_MV = "lowBatMv";
const char* PREF_LOW_SOIL_PCT = "lowSoilPct";
const char* PREF_BUTTON_PIN = "buttonPin";
const char* PREF_PUMP_DUTY = "pumpDuty";
const char* PREF_LED_PIN = "ledPin";


// Soil Sensor
const uint8_t  DEFAULT_SOIL_PIN     = 34;
const int      DEFAULT_SOIL_DRY     = 2621;
const int      DEFAULT_SOIL_WET     = 950;   // TO CALIBRATE
const int      DEFAULT_SOIL_VCC_PIN = 4;

// Water Level Sensor
const uint8_t  DEFAULT_WL_PIN[NUM_WATER_LEVELS_CONFIG] = { 33, 25, 26, 27, 14 };
const uint8_t  DEFAULT_WL_GROUND_PIN  = 32;
const uint16_t DEFAULT_WL_THRESHOLD   = 2000;

// Pump
const uint8_t  DEFAULT_PUMP_PIN       = 15;
const uint32_t DEFAULT_PUMP_RUN_MS    = 3000;
const int      DEFAULT_SOIL_THRESHOLD = 50;

// Battery Monitor
// Zmieniono z GPIO 33 → GPIO 35 (_VBAT, dedykowany pin z dzielnikiem 1:2 na LOLIN D32)
// UWAGA: odczyt ADC należy mnożyć ×2, np.:
//   int voltageMv = (adcValue * 3300 / 4095) * 2;
const uint8_t DEFAULT_BAT_ADC_PIN    = 35;
const int     DEFAULT_LOW_BATTERY_MV = 3300; // 3.3 V

// DHT Sensor
const uint8_t DEFAULT_DHT_PIN     = 16;
const uint8_t DEFAULT_DHT_PWR_PIN = 17;

// MPU Sensor
// Zmieniono z GPIO 35 → GPIO 34  (GPIO 35 oddany dla _VBAT; GPIO 34 = input-only, idealny dla INT)
const uint8_t DEFAULT_MPU_INT_PIN = 13;

// General
const uint32_t DEFAULT_SLEEP_SECONDS     = 86400; // 24 h
const bool     DEFAULT_CONTINUOUS_MODE   = true;  // !! DOMYŚLNIE Deep Sleep !! Zmień na true do debugowania

// Blynk
const char*    PREF_BLYNK_INTERVAL             = "blynkInt";
const uint32_t DEFAULT_BLYNK_SEND_INTERVAL_SEC = 30;
static uint32_t blynkSendIntervalSec;

// Buzzer
const uint8_t DEFAULT_BUZZER_PIN = 23;

// Alarm
const bool DEFAULT_ALARM_SOUND_ENABLED = true;
const int  DEFAULT_LOW_SOIL_PERCENT    = 40;

// Button
const uint8_t DEFAULT_BUTTON_PIN = 0;

// Pump duty
const uint8_t DEFAULT_PUMP_DUTY = 255; // pełna moc

// LED
// Zmieniono z GPIO 22 → GPIO 5  (GPIO 22 = I2C SCL; GPIO 5 = LED_BUILTIN na LOLIN D32)
constexpr uint8_t LED_PIN         = 5;
const uint8_t     DEFAULT_LED_PIN = LED_PIN;

// --- Zmienne statyczne ---
static uint8_t  soilSensorPin;
static int      soilAdcDry;
static int      soilAdcWet;
static int      soilVccPin;
static uint8_t  waterLevelPins[NUM_WATER_LEVELS_CONFIG];
static uint8_t  pumpPin;
static uint32_t pumpRunMillis;
static int      soilMoistureThreshold;
static uint8_t  batteryAdcPin;
static uint8_t  dhtPin;
static uint8_t  mpuIntPin;
static uint32_t sleepDurationSeconds;
static bool     continuousMode;
static uint8_t  buzzerPin;
static int      lowBatteryMilliVolts;
static int      lowSoilPercent;
static bool     alarmSoundEnabled;
static uint8_t  dhtPowerPin;
static uint8_t  buttonPin;
static uint8_t  waterLevelGroundPin;
static uint16_t waterLevelThreshold;
static uint8_t  pumpDutyCycle;
static uint8_t  ledPin;

// Zapisuje domyślne, jeśli brakuje klucza PREF_SLEEP_SEC
void saveDefaultConfigurationIfNeeded() {
     if (!preferences.isKey(PREF_SLEEP_SEC)) {
        Serial.println("Zapisuję wartości domyślne z kodu...");
        preferences.putUChar(PREF_SOIL_PIN, DEFAULT_SOIL_PIN);
        preferences.putInt(PREF_SOIL_DRY, DEFAULT_SOIL_DRY);
        preferences.putInt(PREF_SOIL_WET, DEFAULT_SOIL_WET);
        preferences.putInt(PREF_SOIL_VCC, DEFAULT_SOIL_VCC_PIN);
        preferences.putUChar(PREF_PUMP_PIN, DEFAULT_PUMP_PIN);
        preferences.putUInt(PREF_PUMP_RUN_MS, DEFAULT_PUMP_RUN_MS);
        preferences.putInt(PREF_SOIL_THRESHOLD, DEFAULT_SOIL_THRESHOLD);
        preferences.putUChar(PREF_BAT_ADC_PIN, DEFAULT_BAT_ADC_PIN);
        preferences.putUChar(PREF_DHT_PIN, DEFAULT_DHT_PIN);
        preferences.putUChar(PREF_MPU_INT_PIN, DEFAULT_MPU_INT_PIN);
        preferences.putUInt(PREF_SLEEP_SEC, DEFAULT_SLEEP_SECONDS); 
        preferences.putBool(PREF_CONT_MODE, DEFAULT_CONTINUOUS_MODE);
        preferences.putUInt(PREF_BLYNK_INTERVAL, DEFAULT_BLYNK_SEND_INTERVAL_SEC);
        preferences.putUChar(PREF_BUZZER_PIN, DEFAULT_BUZZER_PIN);
        preferences.putBool(PREF_ALARM_SND_EN, DEFAULT_ALARM_SOUND_ENABLED);
        preferences.putInt(PREF_LOW_BAT_MV, DEFAULT_LOW_BATTERY_MV);
        preferences.putInt(PREF_LOW_SOIL_PCT, DEFAULT_LOW_SOIL_PERCENT);
        preferences.putUChar(PREF_DHT_PWR_PIN, DEFAULT_DHT_PWR_PIN);
        preferences.putUChar(PREF_BUTTON_PIN, DEFAULT_BUTTON_PIN);
        preferences.putUChar(PREF_PUMP_DUTY, DEFAULT_PUMP_DUTY);
        preferences.putUChar(PREF_LED_PIN, DEFAULT_LED_PIN);

        if (!preferences.isKey(PREF_WL_GROUND_PIN)) {
            for (int i = 0; i < NUM_WATER_LEVELS_CONFIG; i++) {
                preferences.putUChar(PREF_WL_PIN[i], DEFAULT_WL_PIN[i]);
            }
            preferences.putUChar(PREF_WL_GROUND_PIN, DEFAULT_WL_GROUND_PIN);
            preferences.putUShort(PREF_WL_THRESHOLD, DEFAULT_WL_THRESHOLD);
        }
     }
}

void configSetup() {
    preferences.begin(PREF_NAMESPACE, false);
    saveDefaultConfigurationIfNeeded();

    // Wczytaj wartości
    soilSensorPin         = preferences.getUChar(PREF_SOIL_PIN,       DEFAULT_SOIL_PIN);
    soilAdcDry            = preferences.getInt  (PREF_SOIL_DRY,       DEFAULT_SOIL_DRY);
    soilAdcWet            = preferences.getInt  (PREF_SOIL_WET,       DEFAULT_SOIL_WET);
    soilVccPin            = preferences.getInt  (PREF_SOIL_VCC,       DEFAULT_SOIL_VCC_PIN);
    for (int i = 0; i < NUM_WATER_LEVELS_CONFIG; i++) {
        waterLevelPins[i] = preferences.getUChar(PREF_WL_PIN[i],      DEFAULT_WL_PIN[i]);
    }
    waterLevelGroundPin   = preferences.getUChar (PREF_WL_GROUND_PIN, DEFAULT_WL_GROUND_PIN);
    waterLevelThreshold   = preferences.getUShort(PREF_WL_THRESHOLD,  DEFAULT_WL_THRESHOLD);
    pumpPin               = preferences.getUChar (PREF_PUMP_PIN,      DEFAULT_PUMP_PIN);
    pumpRunMillis         = preferences.getUInt  (PREF_PUMP_RUN_MS,   DEFAULT_PUMP_RUN_MS);
    soilMoistureThreshold = preferences.getInt   (PREF_SOIL_THRESHOLD,DEFAULT_SOIL_THRESHOLD);
    batteryAdcPin         = preferences.getUChar (PREF_BAT_ADC_PIN,   DEFAULT_BAT_ADC_PIN);
    dhtPin                = preferences.getUChar (PREF_DHT_PIN,       DEFAULT_DHT_PIN);
    mpuIntPin             = preferences.getUChar (PREF_MPU_INT_PIN,   DEFAULT_MPU_INT_PIN);
    sleepDurationSeconds  = preferences.getUInt  (PREF_SLEEP_SEC,     DEFAULT_SLEEP_SECONDS);
    continuousMode        = preferences.getBool  (PREF_CONT_MODE,     DEFAULT_CONTINUOUS_MODE);
    blynkSendIntervalSec  = preferences.getUInt  (PREF_BLYNK_INTERVAL,DEFAULT_BLYNK_SEND_INTERVAL_SEC);
    buzzerPin             = preferences.getUChar (PREF_BUZZER_PIN,    DEFAULT_BUZZER_PIN);
    alarmSoundEnabled     = preferences.getBool  (PREF_ALARM_SND_EN,  DEFAULT_ALARM_SOUND_ENABLED);
    lowBatteryMilliVolts  = preferences.getInt   (PREF_LOW_BAT_MV,   DEFAULT_LOW_BATTERY_MV);
    lowSoilPercent        = preferences.getInt   (PREF_LOW_SOIL_PCT,  DEFAULT_LOW_SOIL_PERCENT);
    dhtPowerPin           = preferences.getUChar (PREF_DHT_PWR_PIN,   DEFAULT_DHT_PWR_PIN);
    buttonPin             = preferences.getUChar (PREF_BUTTON_PIN,    DEFAULT_BUTTON_PIN);
    pumpDutyCycle         = preferences.getUChar (PREF_PUMP_DUTY,     DEFAULT_PUMP_DUTY);
    ledPin                = preferences.getUChar (PREF_LED_PIN,       DEFAULT_LED_PIN);

    preferences.end();

    // Wydrukuj konfigurację
    Serial.println("=== Wczytano konfigurację (LOLIN D32) ===");
    Serial.printf("  Tryb ciągły:                  %s (false = Deep Sleep)\n", continuousMode ? "TAK" : "NIE");
    Serial.printf("  Pin czujnika wilg.:            %d\n", soilSensorPin);
    Serial.printf("  Kalibracja wilg.:              Sucho=%d, Mokro=%d\n", soilAdcDry, soilAdcWet);
    Serial.printf("  Pin zasilania czujnika wilg.:  %d (%s)\n", soilVccPin,
                  (soilVccPin == -1 || soilVccPin == 255 ? "nieużywany/błąd" : "używany"));
    Serial.print ("  Piny czujnika poz. wody L1-L5: ");
    for (int i = 0; i < NUM_WATER_LEVELS_CONFIG; i++) Serial.printf("%d ", waterLevelPins[i]);
    Serial.println();
    Serial.printf("  Pin masy czujnika wody:        %d\n", waterLevelGroundPin);
    Serial.printf("  Próg ADC wykrycia wody:        %u\n", waterLevelThreshold);
    Serial.printf("  Pin pompy:                     %d\n", pumpPin);
    Serial.printf("  Czas pracy pompy:              %d ms\n", pumpRunMillis);
    Serial.printf("  Próg wilgotności dla pompy:    %d %%\n", soilMoistureThreshold);
    Serial.printf("  Pin ADC baterii (_VBAT x2):    %d\n", batteryAdcPin);
    Serial.printf("  Próg niskiej baterii:          %d mV\n", lowBatteryMilliVolts);
    Serial.printf("  Pin DHT11 Data:                %d\n", dhtPin);
    Serial.printf("  Pin DHT11 Power:               %d\n", dhtPowerPin);
    Serial.printf("  Pin INT MPU6500:               %d\n", mpuIntPin);
    Serial.printf("  Czas uśpienia:                 %d s\n", sleepDurationSeconds);
    Serial.printf("  Interwał wysyłania Blynk:      %d s\n", blynkSendIntervalSec);
    Serial.printf("  Pin Buzzera:                   %d\n", buzzerPin);
    Serial.printf("  Dźwięk alarmu:                 %s\n", alarmSoundEnabled ? "Włączony" : "Wyłączony");
    Serial.printf("  Próg alarmu wilg. gleby:       %d %%\n", lowSoilPercent);
    Serial.printf("  Pin przycisku (EXT0 wake-up):  %d\n", buttonPin);
    Serial.printf("  Moc pompy (Duty Cycle):        %d/255\n", pumpDutyCycle);
    Serial.printf("  Pin LED (LED_BUILTIN D32):     %d\n", ledPin);
    Serial.println("==========================================");
}

// =============================================================
//  Gettery
// =============================================================
uint8_t  configGetLedPin()                          { return ledPin; }
uint8_t  configGetPumpDutyCycle()                   { return pumpDutyCycle; }
bool     configIsContinuousMode()                   { return continuousMode; }
uint8_t  configGetSoilPin()                         { return soilSensorPin; }
int      configGetSoilDryADC()                      { return soilAdcDry; }
int      configGetSoilWetADC()                      { return soilAdcWet; }
int      configGetSoilVccPin()                      { return soilVccPin; }
uint32_t configGetSleepSeconds()                    { return sleepDurationSeconds; }
uint8_t  configGetPumpPin()                         { return pumpPin; }
uint32_t configGetPumpRunMillis()                   { return pumpRunMillis; }
int      configGetSoilThresholdPercent()            { return soilMoistureThreshold; }
uint8_t  configGetBatteryAdcPin()                   { return batteryAdcPin; }
uint8_t  configGetDhtPin()                          { return dhtPin; }
uint8_t  configGetMpuIntPin()                       { return mpuIntPin; }
uint32_t configGetBlynkSendIntervalSec()            { return blynkSendIntervalSec; }
uint8_t  configGetBuzzerPin()                       { return buzzerPin; }
bool     configIsAlarmSoundEnabled()                { return alarmSoundEnabled; }
uint8_t  configGetDhtPowerPin()                     { return dhtPowerPin; }
uint8_t  configGetButtonPin()                       { return buttonPin; }
int      configGetLowBatteryMilliVolts()            { return lowBatteryMilliVolts; }
int      configGetLowSoilPercent()                  { return lowSoilPercent; }
uint8_t  configGetWaterLevelPin(int level)          { return (level >= 1 && level <= NUM_WATER_LEVELS_CONFIG) ? waterLevelPins[level - 1] : 255; }
uint8_t  configGetWaterLevelGroundPin()             { return waterLevelGroundPin; }
uint16_t configGetWaterLevelThreshold()             { return waterLevelThreshold; }

// =============================================================
//  Settery
// =============================================================

void configSetPumpDutyCycle(uint8_t duty) {
    if (pumpDutyCycle != duty) {
        pumpDutyCycle = duty;
        preferences.begin(PREF_NAMESPACE, false);
        preferences.putUChar(PREF_PUMP_DUTY, pumpDutyCycle);
        preferences.end();
        Serial.printf("[Config] Zapisano nową moc pompy (Duty Cycle): %d/255\n", pumpDutyCycle);
    }
}

void configSetWaterLevelGroundPin(uint8_t pin) {
    if (waterLevelGroundPin != pin) {
        waterLevelGroundPin = pin;
        preferences.begin(PREF_NAMESPACE, false);
        preferences.putUChar(PREF_WL_GROUND_PIN, pin);
        preferences.end();
        Serial.printf("[Config] Zapisano nowy pin sondy odniesienia: %d\n", pin);
    }
}

void configSetWaterLevelThreshold(uint16_t threshold) {
    if (threshold > 4095) threshold = 4095;
    if (waterLevelThreshold != threshold) {
        waterLevelThreshold = threshold;
        preferences.begin(PREF_NAMESPACE, false);
        preferences.putUShort(PREF_WL_THRESHOLD, threshold);
        preferences.end();
        Serial.printf("[Config] Zapisano nowy próg detekcji wody: %u\n", threshold);
    }
}

void configSetPumpRunMillis(uint32_t durationMs) {
    if (durationMs > 30000) {
        durationMs = 30000;
        Serial.println("[Config] Ostrzeżenie: Czas pracy pompy ograniczony do 30000 ms.");
    }
    if (durationMs < 500) {
        durationMs = 500;
        Serial.println("[Config] Ostrzeżenie: Minimalny czas pracy pompy to 500 ms.");
    }
    if (pumpRunMillis != durationMs) {
        pumpRunMillis = durationMs;
        preferences.begin(PREF_NAMESPACE, false);
        preferences.putUInt(PREF_PUMP_RUN_MS, pumpRunMillis);
        preferences.end();
        Serial.printf("[Config] Zapisano nowy czas pracy pompy: %d ms\n", pumpRunMillis);
    }
}

void configSetSoilThresholdPercent(int threshold) {
    if (threshold < 0)   threshold = 0;
    if (threshold > 100) threshold = 100;
    if (soilMoistureThreshold != threshold) {
        soilMoistureThreshold = threshold;
        preferences.begin(PREF_NAMESPACE, false);
        preferences.putInt(PREF_SOIL_THRESHOLD, soilMoistureThreshold);
        preferences.end();
        Serial.printf("[Config] Zapisano nowy próg wilgotności dla pompy: %d %%\n", soilMoistureThreshold);
    }
}

int configGetMeasurementHour() {
    preferences.begin(PREF_NAMESPACE, true);
    int hour = preferences.getInt(PREF_MEASUREMENT_HOUR, 8);
    preferences.end();
    return hour;
}

int configGetMeasurementMinute() {
    preferences.begin(PREF_NAMESPACE, true);
    int minute = preferences.getInt(PREF_MEASUREMENT_MINUTE, 0);
    preferences.end();
    return minute;
}

bool configSetMeasurementTime(int hour, int minute) {
    if (hour >= 0 && hour < 24 && minute >= 0 && minute < 60) {
        preferences.begin(PREF_NAMESPACE, false);
        preferences.putInt(PREF_MEASUREMENT_HOUR, hour);
        preferences.putInt(PREF_MEASUREMENT_MINUTE, minute);
        preferences.end();
        return true;
    }
    return false;
}

void configSetContinuousMode(bool enabled) {
    continuousMode = enabled;
    preferences.begin(PREF_NAMESPACE, false);
    preferences.putBool(PREF_CONT_MODE, continuousMode);
    preferences.end();
}

void configSetAlarmSoundEnabled(bool enabled) {
    if (alarmSoundEnabled != enabled) {
        alarmSoundEnabled = enabled;
        preferences.begin(PREF_NAMESPACE, false);
        preferences.putBool(PREF_ALARM_SND_EN, alarmSoundEnabled);
        preferences.end();
        Serial.printf("[Config] Zapisano nowy stan dźwięku alarmu: %s\n", alarmSoundEnabled ? "Włączony" : "Wyłączony");
    }
}

void configSetLowBatteryMilliVolts(int mv) {
    if (mv < 2500) mv = 2500;
    if (mv > 4200) mv = 4200;
    if (lowBatteryMilliVolts != mv) {
        lowBatteryMilliVolts = mv;
        preferences.begin(PREF_NAMESPACE, false);
        preferences.putInt(PREF_LOW_BAT_MV, lowBatteryMilliVolts);
        preferences.end();
        Serial.printf("[Config] Zapisano nowy próg alarmu baterii: %d mV\n", lowBatteryMilliVolts);
    }
}

void configSetLowSoilPercent(int percent) {
    if (percent < 0)   percent = 0;
    if (percent > 100) percent = 100;
    if (lowSoilPercent != percent) {
        lowSoilPercent = percent;
        preferences.begin(PREF_NAMESPACE, false);
        preferences.putInt(PREF_LOW_SOIL_PCT, lowSoilPercent);
        preferences.end();
        Serial.printf("[Config] Zapisano nowy próg alarmu wilgotności gleby: %d %%\n", lowSoilPercent);
    }
}

void configSetSoilDryADC(int value) {
    if (value < 0)    value = 0;
    if (value > 4095) value = 4095;
    if (soilAdcDry != value) {
        soilAdcDry = value;
        preferences.begin(PREF_NAMESPACE, false);
        preferences.putInt(PREF_SOIL_DRY, soilAdcDry);
        preferences.end();
        Serial.printf("[Config] Zapisano nową wartość kalibracji ADC 'sucho': %d\n", soilAdcDry);
    }
}

void configSetSoilWetADC(int value) {
    if (value < 0)    value = 0;
    if (value > 4095) value = 4095;
    if (value >= soilAdcDry) {
        Serial.printf("[Config] Ostrzeżenie: Wartość ADC 'mokro' (%d) >= wartości 'sucho' (%d). Sprawdź kalibrację.\n", value, soilAdcDry);
    }
    if (soilAdcWet != value) {
        soilAdcWet = value;
        preferences.begin(PREF_NAMESPACE, false);
        preferences.putInt(PREF_SOIL_WET, soilAdcWet);
        preferences.end();
        Serial.printf("[Config] Zapisano nową wartość kalibracji ADC 'mokro': %d\n", soilAdcWet);
    }
}

void clearPreferencesData(const char* namespaceToClear) {
    Preferences preferences;
    Serial.printf("Próba wyczyszczenia przestrzeni nazw Preferences: '%s'\n", namespaceToClear);
    if (preferences.begin(namespaceToClear, false)) {
        Serial.println("  Przestrzeń nazw otwarta...");
        if (preferences.clear()) {
            Serial.println("  Sukces! Wszystkie dane w tej przestrzeni nazw zostały usunięte.");
        } else {
            Serial.println("  BŁĄD: Nie udało się wyczyścić danych.");
        }
        preferences.end();
        Serial.println("  Przestrzeń nazw zamknięta.");
    } else {
        Serial.printf("  BŁĄD: Nie udało się otworzyć przestrzeni nazw '%s' do zapisu.\n", namespaceToClear);
    }
}