#include "BackendTasks.h"
#include "DeviceConfig.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "secrets.h"

// Zewnętrzne funkcje do obsługi pompy
extern void pumpControlManualTurnOn(uint32_t durationMs);

// Zmienne do stoperów i komend
static int g_lastCommandId = 0;
static unsigned long g_lastCommandCheckTime = 0;
const unsigned long COMMAND_CHECK_INTERVAL_MS = 2000; // Sprawdzanie komend co 2s

void backendTasksSetup() {
    g_lastCommandId = configGetLastCommandId();
    Serial.printf("[Backend] System start. Ostatnie ID komendy z Flash: %d\n", g_lastCommandId);
}

void fetchAndApplyConfiguration() {
    // --- STOPER (Pobieranie konfiguracji co 2 sekundy) ---
    static unsigned long lastConfigCheckTime = 0;
    const unsigned long CONFIG_CHECK_INTERVAL_MS = 2000; 

    if (lastConfigCheckTime != 0 && (millis() - lastConfigCheckTime < CONFIG_CHECK_INTERVAL_MS)) {
        return; // Za wcześnie, wracamy do pętli
    }
    lastConfigCheckTime = millis();
    // -----------------------------------------------------

    if (WiFi.status() != WL_CONNECTED) return;

    HTTPClient http;
    String url = String(FLORA_BACKEND_BASE_URL) + "/api/flora/" + FLORA_BACKEND_DEVICE_ID + "/config";
    
    http.begin(url);
    http.setTimeout(2000);
    http.addHeader("Authorization", String("Bearer ") + FLORA_BACKEND_TOKEN);

    int httpCode = http.GET();
    if (httpCode == 200) {
        String payload = http.getString();
        DynamicJsonDocument doc(2048); 
        DeserializationError error = deserializeJson(doc, payload);

        if (!error) {
            // 1. Tryb ciągły
            if (doc.containsKey("continuousMode")) {
                bool serverVal = doc["continuousMode"].as<bool>();
                if (serverVal != configIsContinuousMode()) {
                    configSetContinuousMode(serverVal);
                    Serial.printf("[Backend] -> Zmieniono Tryb Ciągły na: %s\n", serverVal ? "TAK" : "NIE");
                }
            }

            // 2. Czas pracy pompy 
            if (doc.containsKey("pumpDurationMs")) {
                uint32_t serverVal = doc["pumpDurationMs"].as<uint32_t>();
                if (serverVal != configGetPumpRunMillis()) {
                    configSetPumpRunMillis(serverVal);
                    Serial.printf("[Backend] -> Zmieniono Czas Pracy Pompy na: %lu ms\n", serverVal);
                }
            }

            // 3. Próg wilgotności dla uruchomienia pompy
            if (doc.containsKey("soilThresholdPercent")) {
                int serverVal = doc["soilThresholdPercent"].as<int>();
                if (serverVal != configGetSoilThresholdPercent()) {
                    configSetSoilThresholdPercent(serverVal);
                    Serial.printf("[Backend] -> Zmieniono Próg Podlewania na: %d %%\n", serverVal);
                }
            }

            // 4. Próg alarmu niskiej baterii
            if (doc.containsKey("lowBatteryMilliVolts")) {
                int serverVal = doc["lowBatteryMilliVolts"].as<int>();
                if (serverVal != configGetLowBatteryMilliVolts()) {
                    configSetLowBatteryMilliVolts(serverVal);
                    Serial.printf("[Backend] -> Zmieniono Próg Baterii na: %d mV\n", serverVal);
                }
            }

            // 5. Próg alarmu suchej gleby
            if (doc.containsKey("lowSoilPercent")) {
                int serverVal = doc["lowSoilPercent"].as<int>();
                if (serverVal != configGetLowSoilPercent()) {
                    configSetLowSoilPercent(serverVal);
                    Serial.printf("[Backend] -> Zmieniono Próg Alarmu Gleby na: %d %%\n", serverVal);
                }
            }

            // 6. Próg detekcji wody w zbiorniku
            if (doc.containsKey("waterLevelThreshold")) {
                uint16_t serverVal = doc["waterLevelThreshold"].as<uint16_t>();
                if (serverVal != configGetWaterLevelThreshold()) {
                    configSetWaterLevelThreshold(serverVal);
                    Serial.printf("[Backend] -> Zmieniono Próg Wykrycia Wody na: %u ADC\n", serverVal);
                }
            }

            // 7. Dźwięk alarmu
            if (doc.containsKey("alarmSoundEnabled")) {
                bool serverVal = doc["alarmSoundEnabled"].as<bool>();
                if (serverVal != configIsAlarmSoundEnabled()) {
                    configSetAlarmSoundEnabled(serverVal);
                    Serial.printf("[Backend] -> Zmieniono Dźwięk Alarmu na: %s\n", serverVal ? "Włączony" : "Wyłączony");
                }
            }

            // 8. Kalibracja czujnika wilgotności - SUCHO
            if (doc.containsKey("soilDryAdc")) {
                int serverVal = doc["soilDryAdc"].as<int>();
                if (serverVal != configGetSoilDryADC()) {
                    configSetSoilDryADC(serverVal);
                    Serial.printf("[Backend] -> Zmieniono Kalibrację SUCHO na: %d ADC\n", serverVal);
                }
            }

            // 9. Kalibracja czujnika wilgotności - MOKRO
            if (doc.containsKey("soilWetAdc")) {
                int serverVal = doc["soilWetAdc"].as<int>();
                if (serverVal != configGetSoilWetADC()) {
                    configSetSoilWetADC(serverVal);
                    Serial.printf("[Backend] -> Zmieniono Kalibrację MOKRO na: %d ADC\n", serverVal);
                }
            }

            // 10. Moc pompy (Z procentów 0-100 na PWM 0-255)
            if (doc.containsKey("pumpPowerPercent")) {
                int serverVal = doc["pumpPowerPercent"].as<int>();
                uint8_t dutyCycle = (uint8_t)((serverVal * 255) / 100);
                if (dutyCycle != configGetPumpDutyCycle()) {
                    configSetPumpDutyCycle(dutyCycle);
                    Serial.printf("[Backend] -> Zmieniono Moc Pompy na: %d%% (Duty: %d)\n", serverVal, dutyCycle);
                }
            }

            // 11 i 12. Godzina i Minuta pomiaru (Używamy jednej łączonej funkcji settera)
            if (doc.containsKey("measurementHour") && doc.containsKey("measurementMinute")) {
                int serverHour = doc["measurementHour"].as<int>();
                int serverMinute = doc["measurementMinute"].as<int>();
                
                if (serverHour != configGetMeasurementHour() || serverMinute != configGetMeasurementMinute()) {
                    if (configSetMeasurementTime(serverHour, serverMinute)) {
                        Serial.printf("[Backend] -> Zmieniono Czas Pomiaru na: %02d:%02d\n", serverHour, serverMinute);
                    }
                }
            }

        } else {
            Serial.printf("[Backend] Błąd parsowania konfiguracji JSON: %s\n", error.c_str());
        }
    }
    http.end();
}

void fetchAndExecuteCommands(int currentWaterLevel) {
    static bool firstCheckDone = false; 

    if (!firstCheckDone || (millis() - g_lastCommandCheckTime >= COMMAND_CHECK_INTERVAL_MS)) {
        g_lastCommandCheckTime = millis();
        firstCheckDone = true;
    } else {
        return; 
    }

    if (WiFi.status() != WL_CONNECTED) return; 

    HTTPClient http;
    String url = String(FLORA_BACKEND_BASE_URL) + "/api/flora/" + FLORA_BACKEND_DEVICE_ID + "/commands?after_id=" + String(g_lastCommandId);
    
    http.begin(url);
    http.setTimeout(2000); 
    http.addHeader("Authorization", String("Bearer ") + FLORA_BACKEND_TOKEN);

    int httpCode = http.GET();
    if (httpCode >= 200 && httpCode < 300) {
        String payload = http.getString();
        DynamicJsonDocument doc(4096);
        DeserializationError error = deserializeJson(doc, payload);

        if (!error) {
            JsonArray items = doc["items"];
            bool pumpTriggeredInThisBatch = false; // Tarcza antyspamowa!

            for (JsonObject item : items) {
                int currentId = item["id"].as<int>();
                String type = item["type"].as<String>();

                if (type == "pump") {
                    int durationMs = item["payload"]["durationMs"].as<int>();
                    
                    if (currentWaterLevel <= 0) {
                        Serial.println("[Backend] Odrzucono komendę z aplikacji - BRAK WODY!");
                    } else if (!pumpTriggeredInThisBatch) {
                        Serial.printf("[Backend] Wykryto komendę PUMP! Czas: %d ms\n", durationMs);
                        pumpControlManualTurnOn(durationMs);
                        pumpTriggeredInThisBatch = true; 
                    } else {
                        Serial.printf("[Backend] Zignorowano powieloną komendę PUMP (ID: %d) - antyspam!\n", currentId);
                    }
                }

                if (currentId > g_lastCommandId) {
                    g_lastCommandId = currentId;
                    configSetLastCommandId(g_lastCommandId); 
                }
            }
        }
    }
    http.end();
}