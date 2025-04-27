#include "EnvironmentSensor.h"
#include "DeviceConfig.h"
#include <Arduino.h>
#include <DHT.h> // Dołączona biblioteka DHT

// Definiujemy typ czujnika
#define DHTTYPE DHT11

// Czas stabilizacji DHT w ms po włączeniu zasilania
// Eksperymentuj z tą wartością. Zacznij od 1000ms (1 sekunda).
const unsigned long DHT_STABILIZATION_DELAY_MS = 1000;

// Zmienne globalne/statyczne dla modułu
static uint8_t dhtDataPin = 255;  // Pin danych
static uint8_t dhtPowerPin = 255; // <<< NOWOŚĆ: Pin zasilania >>>
static DHT *dht_sensor = nullptr; // Wskaźnik na obiekt DHT
static bool isDhtInitialized = false; // Flaga - czy ostatni odczyt był udany

void environmentSensorSetup() {
    dhtDataPin = configGetDhtPin();     // Pobierz pin danych z konfiguracji
    dhtPowerPin = configGetDhtPowerPin(); // <<< NOWOŚĆ: Pobierz pin zasilania >>>

    Serial.printf("  [DHT11] Konfiguruję pin DATA: %d\n", dhtDataPin);

    // Skonfiguruj pin zasilania jako WYJŚCIE i ustaw stan NISKI (wyłączony)
    if (dhtPowerPin != 255) {
        Serial.printf("  [DHT11] Konfiguruję pin ZASILANIA: %d\n", dhtPowerPin);
        pinMode(dhtPowerPin, OUTPUT);
        digitalWrite(dhtPowerPin, LOW); // Wyłącz zasilanie na starcie
    } else {
        Serial.println("  [DHT11] OSTRZEŻENIE: Pin zasilania DHT nie jest skonfigurowany! Sensor nie będzie działał.");
        // Nie inicjalizujemy sensora, jeśli nie ma jak go zasilić
        isDhtInitialized = false;
        // Upewnij się, że wskaźnik jest nullptr, jeśli byłby ustawiony wcześniej
        if (dht_sensor != nullptr) {
            delete dht_sensor;
            dht_sensor = nullptr;
        }
        return; // Zakończ setup dla DHT
    }

    // Sprawdź, czy pin danych jest poprawny
    if (dhtDataPin == 255) {
         Serial.println("  [DHT11] BŁĄD: Nie skonfigurowano poprawnie pinu DATA DHT11 w DeviceConfig!");
         isDhtInitialized = false;
         // Wyłącz zasilanie, jeśli zostało włączone testowo
         digitalWrite(dhtPowerPin, LOW);
         return;
    }

    // Utworzenie obiektu DHT - robimy to teraz w setup, ale begin() będzie w read()
    // Sprawdź, czy już nie istnieje
    if (dht_sensor == nullptr) {
         dht_sensor = new DHT(dhtDataPin, DHTTYPE);
         if(dht_sensor == nullptr) {
             Serial.println("  [DHT11] BŁĄD: Nie udało się alokować pamięci dla obiektu DHT!");
             isDhtInitialized = false;
             digitalWrite(dhtPowerPin, LOW); // Wyłącz zasilanie
             return;
         }
         Serial.println("  [DHT11] Obiekt DHT utworzony.");
    }

    // Nie wywołujemy begin() ani nie robimy testowego odczytu tutaj, bo sensor jest wyłączony
    isDhtInitialized = false; // Inicjalizacja zostanie potwierdzona przy pierwszym udanym odczycie
    Serial.println("  [DHT11] Konfiguracja pinów zakończona. Sensor gotowy do użycia (zasilanie wyłączone).");
}

bool environmentSensorRead(float &temperature, float &humidity) {
    // Sprawdź, czy piny są poprawnie skonfigurowane i obiekt istnieje
    if (dhtPowerPin == 255 || dhtDataPin == 255 || dht_sensor == nullptr) {
        if(dht_sensor == nullptr && dhtPowerPin != 255 && dhtDataPin != 255) {
             // Spróbuj utworzyć obiekt ponownie, jeśli go nie ma, a piny są ok
             dht_sensor = new DHT(dhtDataPin, DHTTYPE);
             if(dht_sensor == nullptr) {
                Serial.println("  [DHT11 Read] BŁĄD: Alokacja pamięci dla DHT nieudana.");
                temperature = NAN; humidity = NAN; return false;
             }
        } else {
            Serial.println("  [DHT11 Read] BŁĄD: Sensor nie jest gotowy (brak obiektu lub błędna konfiguracja pinów).");
            temperature = NAN; humidity = NAN; return false;
        }
    }

    bool readOk = false; // Flaga sukcesu odczytu

    // --- Sekwencja odczytu z przełączaniem zasilania ---
    // 1. Włącz zasilanie
    digitalWrite(dhtPowerPin, HIGH);
    // Serial.println("  [DHT11 Read] Zasilanie ON");

    // 2. Poczekaj na stabilizację
    // Serial.printf("  [DHT11 Read] Czekam %lu ms na stabilizację...\n", DHT_STABILIZATION_DELAY_MS);
    delay(DHT_STABILIZATION_DELAY_MS);

    // 3. Zainicjuj komunikację (begin() jest wymagane po włączeniu zasilania)
    dht_sensor->begin();
    // Można dodać krótki delay po begin, jeśli wymagane
    // delay(50);

    // 4. Odczytaj wartości
    // Serial.println("  [DHT11 Read] Próba odczytu...");
    humidity = dht_sensor->readHumidity();
    temperature = dht_sensor->readTemperature(); // Domyślnie w stopniach Celsjusza

    // 5. Wyłącz zasilanie (zrób to od razu po odczycie)
    digitalWrite(dhtPowerPin, LOW);
    // Serial.println("  [DHT11 Read] Zasilanie OFF");
    // --- Koniec sekwencji ---

    // 6. Sprawdź wynik odczytu
    if (isnan(humidity) || isnan(temperature)) {
        Serial.println("  [DHT11 Read] BŁĄD: Odczyt z czujnika DHT11 nieudany!");
        temperature = NAN; // Upewnij się, że zwracasz NAN w razie błędu
        humidity = NAN;
        isDhtInitialized = false; // Ostatni odczyt nieudany
        readOk = false;
    } else {
        Serial.printf("  [DHT11 Read] Odczytano: Temp=%.1f°C, Wilg=%.1f%%\n", temperature, humidity);
        isDhtInitialized = true; // Ostatni odczyt udany
        readOk = true;
    }

    return readOk; // Zwróć status odczytu
}