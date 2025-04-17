#include "EnvironmentSensor.h"
#include "DeviceConfig.h"
#include <Arduino.h>
#include <DHT.h> // Dołączona biblioteka DHT

// Definiujemy typ czujnika
#define DHTTYPE DHT11

// Zmienna globalna przechowująca skonfigurowany pin
static uint8_t dhtPin = 255; // Inicjalizujemy nieprawidłowym pinem

// Globalny wskaźnik na obiekt DHT - utworzymy go w setup()
DHT *dht_sensor = nullptr;

// Flaga oznaczająca, czy inicjalizacja się powiodła
bool isDhtInitialized = false;

void environmentSensorSetup() {
    dhtPin = configGetDhtPin(); // Pobierz pin z konfiguracji
    if (dhtPin != 255) {
        Serial.printf("  [DHT11] Konfiguruję pin: %d\n", dhtPin);

        // Utwórz obiekt DHT dynamicznie, gdy znamy pin
        // Sprawdź, czy już nie istnieje (na wypadek wielokrotnego wywołania setup)
        if (dht_sensor == nullptr) {
             dht_sensor = new DHT(dhtPin, DHTTYPE); // Tworzymy obiekt
             // Sprawdź, czy alokacja pamięci się powiodła
             if(dht_sensor == nullptr) {
                 Serial.println("  [DHT11] BŁĄD: Nie udało się alokować pamięci dla obiektu DHT!");
                 isDhtInitialized = false;
                 return; // Wyjdź z funkcji, jeśli nie udało się utworzyć obiektu
             }
        }

        // Zainicjuj komunikację z czujnikiem
        dht_sensor->begin();

        // DHT potrzebuje trochę czasu po begin() na ustabilizowanie
        Serial.println("  [DHT11] Czekam na stabilizację czujnika...");
        delay(2000); // Odczekaj 2 sekundy

        // Wykonaj próbny odczyt, aby sprawdzić, czy komunikacja działa
        float initialHumidity = dht_sensor->readHumidity();
        if (!isnan(initialHumidity)) { // isnan() sprawdza, czy wartość to "Not a Number" (błąd)
            Serial.println("  [DHT11] Inicjalizacja i pierwszy odczyt DHT11 OK.");
            isDhtInitialized = true; // Ustaw flagę sukcesu
        } else {
            Serial.println("  [DHT11] OSTRZEŻENIE: Inicjalizacja DHT11 nieudana lub błąd pierwszego odczytu. Sprawdź połączenia i rezystor pull-up!");
            isDhtInitialized = false;
             // Posprzątaj, jeśli inicjalizacja się nie powiodła
             delete dht_sensor;
             dht_sensor = nullptr;
        }
    } else {
         Serial.println("  [DHT11] BŁĄD: Nie skonfigurowano poprawnie pinu DHT11 w DeviceConfig!");
         isDhtInitialized = false;
    }
}

bool environmentSensorRead(float &temperature, float &humidity) {
    // Sprawdź, czy czujnik został poprawnie zainicjowany
    if (!isDhtInitialized || dht_sensor == nullptr) {
        Serial.println("  [DHT11] BŁĄD: Czujnik nie został zainicjowany poprawnie.");
        temperature = NAN; // Zwróć wartości błędne
        humidity = NAN;
        return false; // Zwróć informację o błędzie
    }

    // Odczytaj wilgotność i temperaturę
    // Biblioteka Adafruit DHT sama dba o wymagane przerwy między odczytami (>1s dla DHT11)
    humidity = dht_sensor->readHumidity();
    temperature = dht_sensor->readTemperature(); // Domyślnie w stopniach Celsjusza

    // Sprawdź, czy odczyty się powiodły (nie są NaN)
    if (isnan(humidity) || isnan(temperature)) {
        Serial.println("  [DHT11] BŁĄD: Odczyt z czujnika DHT11 nieudany!");
        return false; // Zwróć błąd
    } else {
        Serial.printf("  [DHT11] Odczytano: Temp=%.1f°C, Wilg=%.1f%%\n", temperature, humidity);
        return true; // Zwróć sukces
    }
}