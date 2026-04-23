#include "BackendTasks.h"
#include "DeviceConfig.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "secrets.h" // <--- Tutaj płytka bierze prawdziwe adresy i tokeny!

// Funkcje zewnętrzne do obsługi pompy
extern void pumpControlManualTurnOn(uint32_t durationMs);

// Zmienne do stoperów i komend
static int g_lastCommandId = 0;

static unsigned long g_lastCommandCheckTime = 0;
const unsigned long COMMAND_CHECK_INTERVAL_MS = 2000; // Komendy co 2 sekundy

void backendTasksSetup() {
    // Pobieramy ID komendy z Flash przy starcie
    g_lastCommandId = configGetLastCommandId();
    Serial.printf("[Backend] System start. Ostatnie ID komendy z Flash: %d\n", g_lastCommandId);
}

void fetchAndApplyConfiguration() {
    // --- NOWY STOPER (np. co 15 sekund) ---
    static unsigned long lastConfigCheckTime = 0;
    const unsigned long CONFIG_CHECK_INTERVAL_MS = 2000;

    // Omijamy stoper, jeśli to pierwsze uruchomienie (lastConfigCheckTime == 0)
    if (lastConfigCheckTime != 0 && (millis() - lastConfigCheckTime < CONFIG_CHECK_INTERVAL_MS)) {
        return; // Za wcześnie, wracamy do loop!
    }
    lastConfigCheckTime = millis();
    // --------------------------------------

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
            // Zakomentuj tego printa, jeśli nie chcesz by co 15 sekund pisał w logach, że "sprawdza"
            // Serial.println("[Backend] Sprawdzam aktualizacje konfiguracji z aplikacji...");

            // 1. Tryb ciągły
            if (doc.containsKey("continuousMode")) {
                bool serverVal = doc["continuousMode"].as<bool>();
               // Serial.printf("[Debug] Pobrałem z serwera continuousMode: %s\n", serverVal ? "TAK" : "NIE");
                if (serverVal != configIsContinuousMode()) {
                    configSetContinuousMode(serverVal);
                    Serial.printf("[Backend] -> Zmieniono Tryb Ciągły na: %s\n", serverVal ? "TAK" : "NIE");
                }
            }

            // 2. Czas pracy pompy 
            if (doc.containsKey("pumpRunMillis")) {
                int serverVal = doc["pumpRunMillis"].as<int>();
                if (serverVal != configGetPumpRunMillis()) {
                    configSetPumpRunMillis(serverVal);
                    Serial.printf("[Backend] -> Zmieniono Czas Pompy na: %d ms\n", serverVal);
                }
            }

            // 3. Próg alarmu wilgotności gleby (%)
            if (doc.containsKey("lowSoilPercent")) {
                int serverVal = doc["lowSoilPercent"].as<int>();
                if (serverVal != configGetLowSoilPercent()) {
                    configSetLowSoilPercent(serverVal);
                    Serial.printf("[Backend] -> Zmieniono Próg Alarmu Gleby na: %d %%\n", serverVal);
                }
            }

            // 4. Próg alarmu baterii (mV)
            if (doc.containsKey("lowBatteryMilliVolts")) {
                int serverVal = doc["lowBatteryMilliVolts"].as<int>();
                if (serverVal != configGetLowBatteryMilliVolts()) {
                    configSetLowBatteryMilliVolts(serverVal);
                    Serial.printf("[Backend] -> Zmieniono Próg Alarmu Baterii na: %d mV\n", serverVal);
                }
            }

            // 5. Dźwięk alarmu
            if (doc.containsKey("alarmSoundEnabled")) {
                bool serverVal = doc["alarmSoundEnabled"].as<bool>();
                if (serverVal != configIsAlarmSoundEnabled()) {
                    configSetAlarmSoundEnabled(serverVal);
                    Serial.printf("[Backend] -> Zmieniono Dźwięk Alarmu na: %s\n", serverVal ? "Włączony" : "Wyłączony");
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