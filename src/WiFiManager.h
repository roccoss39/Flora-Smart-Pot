#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <stdint.h>

/**
 * @brief Łączy z siecią WiFi używając podanych danych.
 * Próbuje się połączyć przez określony czas.
 * @param ssid Nazwa sieci WiFi.
 * @param password Hasło do sieci WiFi.
 * @param timeoutMs Czas oczekiwania na połączenie w milisekundach.
 * @return true jeśli połączenie się powiodło, false w przeciwnym razie.
 */
bool wifiConnect(const char* ssid, const char* password, unsigned long timeoutMs = 15000);

/**
 * @brief Sprawdza, czy ESP32 jest aktualnie połączone z WiFi.
 * @return true jeśli jest połączenie, false w przeciwnym razie.
 */
bool wifiIsConnected();

/**
 * @brief Rozłącza sieć WiFi.
 */
void wifiDisconnect();


#endif // WIFIMANAGER_H