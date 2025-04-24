#include "WiFiManager.h"
#include <WiFi.h> // Biblioteka WiFi dla ESP32
#include <Arduino.h> // Dla Serial i millis()

bool wifiConnect(const char* ssid, const char* password, unsigned long timeoutMs) {
    Serial.printf("Łączenie z WiFi: %s\n", ssid);
    WiFi.mode(WIFI_STA); // Tryb stacji (klient)
    WiFi.begin(ssid, password);

    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        if (millis() - startTime > timeoutMs) {
            Serial.println("\nBŁĄD: Nie udało się połączyć z WiFi (timeout)!");
            WiFi.disconnect(); // Rozłącz, jeśli próba się nie powiodła
            return false;
        }
    }

    Serial.println("\nPołączono z WiFi!");
    Serial.print("Adres IP: ");
    Serial.println(WiFi.localIP());
    return true;
}

bool wifiIsConnected() {
    return (WiFi.status() == WL_CONNECTED);
}

void wifiDisconnect() {
    if (wifiIsConnected()) {
        Serial.println("Rozłączanie WiFi...");
        WiFi.disconnect(true); // true = wyłącz też tryb WiFi
        // WiFi.mode(WIFI_OFF); // Opcjonalnie, aby oszczędzać energię
        Serial.println("WiFi rozłączone.");
    }
}