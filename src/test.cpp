/**
 * @file test.cpp
 * @brief Plik testowy Flora Smart Pot
 *
 * Kompilowany TYLKO w środowisku lolin_d32_test (src_filter wyklucza
 * oryginalne pliki sensorów). Dzięki temu nie ma konfliktu definicji.
 *
 * Użycie:
 *   pio run -e lolin_d32_test --target upload
 *
 * Wartości testowe ustawiasz w sekcji poniżej.
 */

#ifdef TEST_MODE

#include "test.h"
#include "DeviceConfig.h"
#include <Arduino.h>
#include <cmath>

// =============================================================
//  ▼▼▼  TUTAJ USTAWIASZ WARTOŚCI TESTOWE  ▼▼▼
// =============================================================

// Wilgotność gleby [0-100 %]
// 0 = całkowicie sucha, 100 = całkowicie mokra
static int TEST_SOIL_MOISTURE = 25;

// Poziom wody w zbiorniku [0-5]
// 0 = pusty, 5 = pełny
static int TEST_WATER_LEVEL = 2;

// Napięcie baterii [V]
// Typowy zakres LiPo: 3.0 V (pusta) – 4.2 V (pełna)
static float TEST_BATTERY_VOLTAGE = 3.75f;

// Temperatura powietrza [°C]
static float TEST_TEMPERATURE = 22.5f;

// Wilgotność powietrza [%]
static float TEST_HUMIDITY = 55.0f;

// Czy symulować błąd DHT11?
// true = environmentSensorRead() zwróci false i NAN
static bool TEST_DHT_ERROR = false;

// Czy symulować aktywną pompę?
static bool TEST_PUMP_RUNNING = false;

// =============================================================
//  ▲▲▲  KONIEC SEKCJI KONFIGURACJI TESTOWEJ  ▲▲▲
// =============================================================

void testPrintConfig() {
    Serial.println(F(""));
    Serial.println(F("╔══════════════════════════════════╗"));
    Serial.println(F("║      TRYB TESTOWY AKTYWNY        ║"));
    Serial.println(F("╚══════════════════════════════════╝"));
    Serial.println(F("  Aby wyłączyć: zmień env na lolin_d32"));
    Serial.println(F(""));
    Serial.println(F("--- Wartości testowe ---"));
    Serial.printf ("  Wilgotność gleby:  %d %%\n",   TEST_SOIL_MOISTURE);
    Serial.printf ("  Poziom wody:       %d / 5\n",  TEST_WATER_LEVEL);
    Serial.printf ("  Napięcie baterii:  %.2f V\n",  TEST_BATTERY_VOLTAGE);
    Serial.printf ("  Temperatura:       %.1f C\n",  TEST_TEMPERATURE);
    Serial.printf ("  Wilgotność pow.:   %.1f %%\n", TEST_HUMIDITY);
    Serial.printf ("  Błąd DHT11:        %s\n",      TEST_DHT_ERROR    ? "TAK" : "NIE");
    Serial.printf ("  Pompa pracuje:     %s\n",      TEST_PUMP_RUNNING ? "TAK" : "NIE");
    Serial.println(F("-----------------------"));
}

// =============================================================
//  Stuby sensorów – zastępują oryginalne implementacje
//  (oryginalne pliki .cpp wykluczone przez src_filter w platformio.ini)
// =============================================================

// --- SoilSensor ---
void soilSensorSetup() {
    Serial.println(F("[TEST] soilSensorSetup() – stub"));
}
int soilSensorReadPercent() {
    Serial.printf("[TEST] soilSensorReadPercent() → %d %%\n", TEST_SOIL_MOISTURE);
    return TEST_SOIL_MOISTURE;
}

// --- WaterLevelSensor ---
void waterLevelSensorSetup() {
    Serial.println(F("[TEST] waterLevelSensorSetup() – stub"));
}
int waterLevelSensorReadLevel() {
    Serial.printf("[TEST] waterLevelSensorReadLevel() → %d\n", TEST_WATER_LEVEL);
    return TEST_WATER_LEVEL;
}

// --- BatteryMonitor ---
void batteryMonitorSetup() {
    Serial.println(F("[TEST] batteryMonitorSetup() – stub"));
}
int batteryMonitorReadRawADC() {
    return 0;
}
float batteryMonitorReadVoltage() {
    Serial.printf("[TEST] batteryMonitorReadVoltage() → %.2f V\n", TEST_BATTERY_VOLTAGE);
    return TEST_BATTERY_VOLTAGE;
}
int batteryMonitorReadMilliVolts() {
    int mv = (int)(TEST_BATTERY_VOLTAGE * 1000.0f);
    Serial.printf("[TEST] batteryMonitorReadMilliVolts() → %d mV\n", mv);
    return mv;
}
bool batteryMonitorIsLow() {
    return TEST_BATTERY_VOLTAGE < (configGetLowBatteryMilliVolts() / 1000.0f);
}

// --- EnvironmentSensor ---
void environmentSensorSetup() {
    Serial.println(F("[TEST] environmentSensorSetup() – stub"));
}
bool environmentSensorRead(float &temperature, float &humidity) {
    if (TEST_DHT_ERROR) {
        Serial.println(F("[TEST] environmentSensorRead() → BŁĄD (symulowany)"));
        temperature = NAN;
        humidity    = NAN;
        return false;
    }
    temperature = TEST_TEMPERATURE;
    humidity    = TEST_HUMIDITY;
    Serial.printf("[TEST] environmentSensorRead() → %.1f C, %.1f %%\n",
                  temperature, humidity);
    return true;
}

// --- PumpControl ---
void pumpControlSetup() {
    Serial.println(F("[TEST] pumpControlSetup() – stub"));
}
void pumpControlUpdate() {}
bool pumpControlIsRunning() {
    return TEST_PUMP_RUNNING;
}
void pumpControlActivateIfNeeded(int soilPercent, int waterLevel) {
    if (waterLevel > 0 && soilPercent < configGetSoilThresholdPercent()) {
        Serial.println("[TEST] POMPA → GPIO15 HIGH");
        pinMode(configGetPumpPin(), OUTPUT);
        digitalWrite(configGetPumpPin(), HIGH);   // ← to zmierzysz miernikiem
        TEST_PUMP_RUNNING = true;
    } else {
        Serial.println("[TEST] POMPA → GPIO15 LOW");
        digitalWrite(configGetPumpPin(), LOW);
        TEST_PUMP_RUNNING = false;
    }
}
void pumpControlManualTurnOn(uint32_t durationMs) {
    Serial.printf("[TEST] pumpControlManualTurnOn(%d ms) – symulacja\n", durationMs);
    TEST_PUMP_RUNNING = true;
}

// =============================================================
//  Settery – zmiana wartości testowych w runtime
// =============================================================

void testSetSoilMoisture(int percent) {
    if (percent < 0)   percent = 0;
    if (percent > 100) percent = 100;
    TEST_SOIL_MOISTURE = percent;
    Serial.printf("[TEST] Ustawiono wilgotność gleby: %d %%\n", percent);
}
void testSetWaterLevel(int level) {
    if (level < 0) level = 0;
    if (level > 5) level = 5;
    TEST_WATER_LEVEL = level;
    Serial.printf("[TEST] Ustawiono poziom wody: %d\n", level);
}
void testSetBatteryVoltage(float voltage) {
    if (voltage < 0.0f) voltage = 0.0f;
    if (voltage > 4.2f) voltage = 4.2f;
    TEST_BATTERY_VOLTAGE = voltage;
    Serial.printf("[TEST] Ustawiono napięcie baterii: %.2f V\n", voltage);
}
void testSetTemperature(float temp) {
    TEST_TEMPERATURE = temp;
    Serial.printf("[TEST] Ustawiono temperaturę: %.1f C\n", temp);
}
void testSetHumidity(float hum) {
    if (hum < 0.0f)   hum = 0.0f;
    if (hum > 100.0f) hum = 100.0f;
    TEST_HUMIDITY = hum;
    Serial.printf("[TEST] Ustawiono wilgotność powietrza: %.1f %%\n", hum);
}
void testSetDhtError(bool error) {
    TEST_DHT_ERROR = error;
    Serial.printf("[TEST] Błąd DHT11: %s\n", error ? "Włączony" : "Wyłączony");
}
void testSetPumpRunning(bool running) {
    TEST_PUMP_RUNNING = running;
    Serial.printf("[TEST] Pompa: %s\n", running ? "Włączona" : "Wyłączona");
}

#endif // TEST_MODE