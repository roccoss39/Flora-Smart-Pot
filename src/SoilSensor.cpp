#include "SoilSensor.h"
#include "DeviceConfig.h" // Aby uzyskać dostęp do konfiguracji
#include <Arduino.h>

static uint8_t sensorPin;
static int adcDry;
static int adcWet;
static int vccPin; // Przechowuje pin VCC (-1 jeśli nieużywany)

void soilSensorSetup() {
    // Pobierz konfigurację z modułu DeviceConfig
    sensorPin = configGetSoilPin();
    adcDry = configGetSoilDryADC();
    adcWet = configGetSoilWetADC();
    vccPin = configGetSoilVccPin();

    // Skonfiguruj pin VCC, jeśli jest używany
    if (vccPin != -1) {
        pinMode(vccPin, OUTPUT);
        digitalWrite(vccPin, LOW); // Domyślnie wyłączony
        Serial.printf("  [Wilgotność] Skonfigurowano pin VCC: %d\n", vccPin);
    }
     Serial.printf("  [Wilgotność] Skonfigurowano pin ADC: %d (Kalibracja: Sucho=%d, Mokro=%d)\n", sensorPin, adcDry, adcWet);
}

int soilSensorReadPercent() {
    int sensorValue = 0;
    int moisturePercent = -1; // Domyślnie błąd (-1), aby wskazać problem z odczytem lub kalibracją

    // Włącz zasilanie, jeśli VCC Pin jest skonfigurowany
    if (vccPin != -1) {
        digitalWrite(vccPin, HIGH);
        delay(500); // Czas na stabilizację
    }

    // Uśrednianie odczytów
    long totalValue = 0;
    int samples = 5; // Możesz dostosować liczbę próbek
    for (int i = 0; i < samples; i++) {
        totalValue += analogRead(sensorPin);
        delay(50); // Krótka przerwa między odczytami
    }
    sensorValue = totalValue / samples;

    // Wyłącz zasilanie, jeśli VCC Pin jest skonfigurowany
    if (vccPin != -1) {
        digitalWrite(vccPin, LOW);
    }

    Serial.printf("  [Wilgotność] Surowy odczyt ADC (Pin %d): %d\n", sensorPin, sensorValue);

    // --- POPRAWIONA LOGIKA PRZELICZANIA ---
    // Pobierz AKTUALNE wartości kalibracyjne z konfiguracji ZA KAŻDYM RAZEM
    int currentAdcDry = configGetSoilDryADC();
    int currentAdcWet = configGetSoilWetADC();

    // Dodano logowanie używanych wartości kalibracyjnych dla diagnostyki
    Serial.printf("  [Wilgotność] Używam kalibracji: Sucho=%d, Mokro=%d\n", currentAdcDry, currentAdcWet);

    // Sprawdź poprawność kalibracji (standardowy czujnik pojemnościowy: mokro < sucho)
    // Twoje logi pokazują ostrzeżenie "Wartość ADC 'mokro' >= wartości 'sucho'",
    // co jest ODWROTNIE niż dla typowych czujników pojemnościowych.
    // Poniższy kod zakłada standardowe zachowanie (MOKRO < SUCHO).
    // Jeśli twój czujnik działa inaczej, musisz dostosować logikę.
    if (currentAdcWet >= currentAdcDry) {
        Serial.printf("  [Wilgotność] BŁĄD KALIBRACJI: Wartość ADC 'mokro' (%d) >= 'sucho' (%d). Ustaw poprawnie w Blynk! (Mokro powinno dać NIŻSZY odczyt ADC).\n", currentAdcWet, currentAdcDry);
        moisturePercent = -1; // Zwróć błąd
    } else {
        // Wartości kalibracyjne są w oczekiwanej kolejności (mokro < sucho)
        // Ogranicz odczyt do zakresu kalibracji, aby uniknąć wartości <0% lub >100%
        int constrainedValue = constrain(sensorValue, currentAdcWet, currentAdcDry);

        // Mapuj wartość:
        //  - Wartość równa currentAdcWet (najbardziej mokro) -> 100%
        //  - Wartość równa currentAdcDry (najbardziej sucho) -> 0%
        moisturePercent = map(constrainedValue, currentAdcWet, currentAdcDry, 100, 0);
    }

    // Możesz dodać logowanie obliczonego procentu dla pewności
    Serial.printf("  [Wilgotność] Obliczony procent: %d%%\n", moisturePercent);

    return moisturePercent;
}