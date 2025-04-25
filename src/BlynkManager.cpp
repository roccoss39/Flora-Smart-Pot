#include "BlynkManager.h"
#include "secrets.h" // Dla danych autoryzacyjnych
#include <Arduino.h> // Dla Serial
#include "DeviceConfig.h" // Potrzebny do setterów konfiguracji
#include "PumpControl.h"  // Potrzebny do manualnego sterowania pompą
#include <Preferences.h> 


// --- WAŻNE: Konfiguracja Wirtualnych Pinów Blynk ---
// Ustaw tutaj numery VPIN zgodne z tym, co zdefiniowałeś w szablonie Blynk!
#define BLYNK_VPIN_SOIL         V0
#define BLYNK_VPIN_WATER_LEVEL  V1
#define BLYNK_VPIN_BATTERY      V2
#define BLYNK_VPIN_TEMPERATURE  V3
#define BLYNK_VPIN_HUMIDITY     V4
#define BLYNK_VPIN_TILT_ANGLE   V5
#define BLYNK_VPIN_TILT_ALERT   V6
#define BLYNK_VPIN_PUMP_STATUS  V7
// ----------------------------------------------------
// --- VPINy do sterowania ---
#define BLYNK_VPIN_PUMP_MANUAL  V10 // Przycisk (Button Widget)
#define BLYNK_VPIN_PUMP_DURATION V11 // Suwak/Pole numeryczne (np. Slider, Step, Numeric Input)
#define BLYNK_VPIN_SOIL_THRESHOLD V12 // Suwak/Pole numeryczne

#define BLYNK_VPIN_MEASUREMENT_HOUR V20
#define BLYNK_VPIN_MEASUREMENT_MINUTE V21

#define BLYNK_VPIN_CONTINUOUS_MODE V15 

// Przekierowanie logów Blynk do Serial (opcjonalne)
#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp32.h>

// Zmienne do przechowywania danych autoryzacyjnych
static char blynkAuthToken[35] = BLYNK_AUTH_TOKEN; // Zwiększ rozmiar w razie potrzeby

// Serwer Blynk (zazwyczaj domyślny dla chmury)
const char* blynk_server = "blynk.cloud"; // Lub własny serwer
uint16_t blynk_port = 80; // Domyślny port dla TCP

// // Jeśli chcesz używać domyślnych z secrets.h i blynk.cloud:
// #include <BlynkSimpleEsp32.h>

void blynkConfigure(const char* authToken, const char* templateId, const char* deviceName) {
     // Definicje dla Blynk.begin(), jeśli używasz go zamiast config/connect
     // Na razie ta funkcja może pozostać pusta, jeśli używamy stałych z secrets.h
     // Lub można tu skopiować authToken, jeśli przekazujemy go dynamicznie
     strncpy(blynkAuthToken, authToken, sizeof(blynkAuthToken)-1);
     blynkAuthToken[sizeof(blynkAuthToken)-1] = '\0';

     // Ustawienie ID i Nazwy - potrzebne, jeśli nie używasz BLYNK_... w kodzie
     // Blynk.setTemplateId(templateId);
     // Blynk.setDeviceName(deviceName);

     // Konfiguracja połączenia (zamiast Blynk.begin)
     Blynk.config(blynkAuthToken, blynk_server, blynk_port);
}

bool blynkConnect(unsigned long timeoutMs) {
    Serial.print("Łączenie z Blynk...");
    // Blynk.connect() zwraca true, jeśli udało się połączyć w ramach timeout
    bool result = Blynk.connect(timeoutMs);
    if (result) {
        Serial.println(" Połączono!");
    } else {
        Serial.println(" Błąd połączenia (timeout)!");
    }
    return result;
}

void blynkRun() {
    // Obsługuje komunikację Blynk w tle (musi być często wywoływane)
    if (Blynk.connected()) {
        Blynk.run();
    }
    // Można dodać logikę ponownego łączenia, jeśli rozłączono
    // else {
    //    blynkConnect(); // Spróbuj połączyć ponownie (ostrożnie, może blokować)
    // }
}

bool blynkIsConnected() {
    return Blynk.connected();
}

void blynkDisconnect() {
     if (Blynk.connected()) {
        Serial.println("Rozłączanie Blynk...");
        Blynk.disconnect();
        Serial.println("Blynk rozłączony.");
     }
}

// Funkcja wysyłająca dane - dostosuj VPINy na górze pliku!
void blynkSendSensorData(int soil, int waterLvl, float batteryV, float temp, float humid, float tilt, bool tiltAlert, bool pumpStatus) {
    if (!Blynk.connected()) {
        Serial.println("Blynk nie połączony, pomijam wysyłanie danych.");
        return;
    }

    Serial.println("Wysyłanie danych do Blynk...");

    // Sprawdź poprawność wartości przed wysłaniem
    if (soil >= 0) Blynk.virtualWrite(BLYNK_VPIN_SOIL, soil);
    Blynk.virtualWrite(BLYNK_VPIN_WATER_LEVEL, waterLvl);
    if (batteryV > 0) Blynk.virtualWrite(BLYNK_VPIN_BATTERY, batteryV);
    if (!isnan(temp)) Blynk.virtualWrite(BLYNK_VPIN_TEMPERATURE, temp);
    if (!isnan(humid)) Blynk.virtualWrite(BLYNK_VPIN_HUMIDITY, humid);
    if (!isnan(tilt)) Blynk.virtualWrite(BLYNK_VPIN_TILT_ANGLE, tilt);
    Blynk.virtualWrite(BLYNK_VPIN_TILT_ALERT, tiltAlert ? 1 : 0);
    Blynk.virtualWrite(BLYNK_VPIN_PUMP_STATUS, pumpStatus ? 1 : 0);

    Serial.println("Dane wysłane.");
}


BLYNK_WRITE(BLYNK_VPIN_PUMP_MANUAL) {
  int value = param.asInt(); // Wartość z przycisku (zwykle 1 gdy wciśnięty)
  Serial.printf("Otrzymano komendę na V%d: %d\n", BLYNK_VPIN_PUMP_MANUAL, value);

  if (value == 1) {
    // Sprawdź poziom wody przed manualnym włączeniem!
    // Potrzebujemy dostępu do ostatniego odczytu poziomu wody.
    // To jest trudne do zrobienia tutaj w izolowanym module.
    // Lepsze rozwiązanie: Przekaż logikę do PumpControl lub zrób odczyt tutaj.
    // Na razie prostsze: ufamy użytkownikowi lub pomijamy sprawdzanie wody przy manualnym.
    Serial.println("Próba manualnego uruchomienia pompy...");
    uint32_t duration = configGetPumpRunMillis(); // Użyj aktualnie ustawionego czasu
    pumpControlManualTurnOn(duration);
    // Od razu zaktualizuj status pompy w Blynk
    Blynk.virtualWrite(BLYNK_VPIN_PUMP_STATUS, pumpControlIsRunning() ? 1 : 0);
  }
  // Zwykle przycisk nie wysyła 0 przy zwolnieniu, więc nie potrzebujemy else
}

// Handler dla zmiany czasu pracy pompy (VPIN V11)
BLYNK_WRITE(BLYNK_VPIN_PUMP_DURATION) {
  uint32_t newDurationMs = param.asInt(); // Odczytaj wartość z suwaka/pola (w ms)
  Serial.printf("Otrzymano nowy czas pracy pompy na V%d: %d ms\n", BLYNK_VPIN_PUMP_DURATION, newDurationMs);
  configSetPumpRunMillis(newDurationMs); // Użyj settera z DeviceConfig do zapisania wartości
}

// Handler dla zmiany progu wilgotności (VPIN V12)
BLYNK_WRITE(BLYNK_VPIN_SOIL_THRESHOLD) {
  int newThreshold = param.asInt(); // Odczytaj wartość z suwaka/pola (%)
  Serial.printf("Otrzymano nowy próg wilgotności na V%d: %d %%\n", BLYNK_VPIN_SOIL_THRESHOLD, newThreshold);
  configSetSoilThresholdPercent(newThreshold); // Użyj settera z DeviceConfig
}

// Handler wywoływany po połączeniu z Blynk
BLYNK_CONNECTED() {
    Serial.println("Połączono z serwerem Blynk. Synchronizuję ustawienia...");
    // Istniejące synchronizacje...
    Blynk.syncVirtual(BLYNK_VPIN_PUMP_DURATION);
    Blynk.syncVirtual(BLYNK_VPIN_SOIL_THRESHOLD);
    Blynk.syncVirtual(BLYNK_VPIN_MEASUREMENT_HOUR);
    Blynk.syncVirtual(BLYNK_VPIN_MEASUREMENT_MINUTE);
    Blynk.syncVirtual(BLYNK_VPIN_CONTINUOUS_MODE);
  
    // Istniejące aktualizacje VPIN...
    Blynk.virtualWrite(BLYNK_VPIN_MEASUREMENT_HOUR, configGetMeasurementHour());
    Blynk.virtualWrite(BLYNK_VPIN_MEASUREMENT_MINUTE, configGetMeasurementMinute());
    Blynk.virtualWrite(BLYNK_VPIN_PUMP_STATUS, pumpControlIsRunning() ? 1 : 0);
  
    // --- Nowy kod synchronizacji trybu ---
    // Preferences prefs;
    // // Tylko do odczytu tym razem
    // if (prefs.begin("flaura_cfg_1", true)) { // true = tryb Read-Only
    //     bool currentMode = prefs.getBool("contMode", configIsContinuousMode()); // Odczytaj z prefs, użyj bieżącego jako fallback
    //     prefs.end();
    //     Serial.printf("Synchronizacja statusu V%d: Tryb ciągły = %s\n", BLYNK_VPIN_CONTINUOUS_MODE, currentMode ? "TAK" : "NIE");
    //    // Blynk.virtualWrite(BLYNK_VPIN_CONTINUOUS_MODE, currentMode ? 1 : 0); // Ustaw stan przełącznika w apce
    // } else {
    //      Serial.println("Błąd otwarcia Preferences do odczytu stanu trybu!");
    // }
    // --- Koniec nowego kodu ---
  }

void blynkUpdatePumpStatus(bool isRunning) {
    Blynk.virtualWrite(BLYNK_VPIN_PUMP_STATUS, isRunning ? 1 : 0);
}

// Dodaj handlery dla tych pinów
BLYNK_WRITE(BLYNK_VPIN_MEASUREMENT_HOUR) {
    int newHour = param.asInt();
    Serial.printf("Otrzymano nową godzinę pomiaru: %d\n", newHour);
    int currentMinute = configGetMeasurementMinute();
    configSetMeasurementTime(newHour, currentMinute);
  }
  
  BLYNK_WRITE(BLYNK_VPIN_MEASUREMENT_MINUTE) {
    int newMinute = param.asInt();
    Serial.printf("Otrzymano nową minutę pomiaru: %d\n", newMinute);
    int currentHour = configGetMeasurementHour();
    configSetMeasurementTime(currentHour, newMinute);
  }


  // Handler dla przełącznika trybu ciągłego (VPIN V15)
BLYNK_WRITE(BLYNK_VPIN_CONTINUOUS_MODE) {
    bool isContinuous = param.asInt() == 1; // 1 = Włączony (Ciągły), 0 = Wyłączony (Deep Sleep)
    Serial.printf("Otrzymano komendę zmiany trybu na V%d: %s\n",
                  BLYNK_VPIN_CONTINUOUS_MODE, isContinuous ? "Ciągły (TRUE)" : "Deep Sleep (FALSE)");
        
    configSetContinuousMode(isContinuous);
    // Preferences prefs;
    // // Użyj tej samej przestrzeni nazw co w DeviceConfig!
    // if (prefs.begin("flaura_cfg_1", false)) { // false = tryb Read/Write
    //     prefs.putBool("contMode", isContinuous); // Zapisz nową wartość do Preferences
    //     prefs.end();
    //     Serial.println("Zapisano nowe ustawienie trybu w Preferences.");
  
    //     // Opcjonalnie: Zresetuj urządzenie, aby zmiana trybu zadziałała natychmiast
    //     // Serial.println("Resetuję urządzenie, aby zastosować zmianę trybu...");
    //     // delay(1000); // Daj czas na wysłanie logów
    //     // ESP.restart();
    //     // Uwaga: Restart może być problematyczny, jeśli np. pompa pracuje.
    //     // Bez restartu zmiana trybu zostanie uwzględniona przy następnym uruchomieniu/wybudzeniu.
  
    // } else {
    //     Serial.println("Błąd otwarcia Preferences do zapisu!");
    // }
  }
