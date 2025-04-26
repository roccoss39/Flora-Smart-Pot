#include <Arduino.h>
#include <Wire.h>              // Nadal potrzebne, jeśli planujesz inne urządzenia I2C
#include <esp_sleep.h>       // Dla Deep Sleep i Wakeup Cause
#include <cmath>             // Dla abs(), isnan(), sqrt(), atan2()

#include <WiFi.h>            // Potrzebne dla WiFi.status() itp.
#include <WiFiManager.h> 

// Nasze moduły
#include "DeviceConfig.h"
#include "secrets.h"          // Dla danych WiFi i Blynk
#include "BlynkManager.h"
#include "SoilSensor.h"
#include "WaterLevelSensor.h" // Definiuje NUM_WATER_LEVELS
#include "PumpControl.h"
#include "BatteryMonitor.h"
#include "EnvironmentSensor.h" // Dla DHT11
// #include "MotionSensor.h"   // <<--- ZAKOMENTOWANY LUB USUNIĘTY
#include "PowerManager.h"
#include "AlarmManager.h"

#include <Preferences.h> 


// Funkcja pomocnicza do drukowania przyczyny wybudzenia
void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  Serial.print("Przyczyna uruchomienia/wybudzenia: ");
  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("EXT0 (zewnętrzne przerwanie 0)"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("EXT1 (zewnętrzne przerwanie 1)"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("ULP Program"); break;
    default : Serial.printf("Reset lub Power On (przyczyna nr %d)\n", wakeup_reason); break;
  }
}

// Wbudowana dioda LED
#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

// Zmienna do śledzenia czasu ostatniej wysyłki do Blynk
unsigned long lastBlynkSend = 0;

void setup() {
    Serial.begin(115200);
    Serial.println("\n--- Flaura Smart Pot - Główny Start ---");
    print_wakeup_reason();

    Wire.begin(); // Inicjalizuj I2C raz (może się przydać w przyszłości)
    delay(100);

    configSetup(); // Wczytaj konfigurację

    // Inicjalizuj wszystkie moduły (oprócz MPU)
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH); // Zaświeć na czas setup

    soilSensorSetup();
    waterLevelSensorSetup();
    pumpControlSetup();
    batteryMonitorSetup();
    environmentSensorSetup();
    alarmManagerSetup();
    // motionSensorSetup(); // <<--- ZAKOMENTOWANE LUB USUNIĘTE

    digitalWrite(LED_BUILTIN, LOW); // Zgaś po inicjalizacji

    // --- Logika połączenia i pierwszego pomiaru ---
    bool runMeasurement = true;
    bool connectSuccess = false;

    WiFi.mode(WIFI_STA); // Ustaw tryb Station (klient WiFi)
    WiFiManager wm; // Utwórz obiekt WiFiManager


     // Ustawienia WiFiManager (opcjonalne)
    wm.setConnectTimeout(20); // Czas próby połączenia z zapisaną siecią (sekundy)
    wm.setConfigPortalTimeout(120); // Czas aktywności portalu konfiguracyjnego (sekundy)
                                    // Jeśli użytkownik nic nie zrobi, portal się zamknie.
    wm.setWiFiAutoReconnect(true); // Włącz automatyczne ponowne łączenie

    // Nazwa sieci AP, która pojawi się, gdy urządzenie nie będzie mogło się połączyć
    String apName = "Flaura-Setup-" + String((uint32_t)ESP.getEfuseMac(), HEX); // Unikalna nazwa AP

    Serial.println("Próba połączenia z zapisaną siecią WiFi...");
    // autoConnect próbuje połączyć się z zapisaną siecią.
    // Jeśli się nie uda, uruchamia portal konfiguracyjny (AP) o podanej nazwie.
    // Blokuje wykonanie, dopóki nie uzyska połączenia lub nie upłynie timeout portalu.
    if (wm.autoConnect(apName.c_str())) {
        Serial.println("\nPołączono z WiFi!");
        Serial.print("Adres IP: ");
        Serial.println(WiFi.localIP());
        connectSuccess = true;

        blynkConfigure(BLYNK_AUTH_TOKEN, BLYNK_TEMPLATE_ID, BLYNK_DEVICE_NAME);
        if (!blynkConnect()) { // Spróbuj połączyć z Blynk
             Serial.println("Nie udało się połączyć z Blynk po konfiguracji WiFi.");
             // Zastanów się, co robić dalej - np. restart?
        }
    } else {
        //Serial.println("\nNie udało się połączyć z WiFi ani skonfigurować przez portal (timeout?).");
        Serial.println("\nOSTRZEŻENIE: Nie udało się połączyć z WiFi ani skonfigurować przez portal.");
        Serial.println("Przechodzę w tryb OFFLINE.");
        connectSuccess = false;
        // Urządzenie nie ma połączenia. W zależności od logiki:
        // - Może przejść w tryb offline (jeśli ma sens)
        // - Zrestartować się, aby ponownie spróbować uruchomić portal
        // - Przejść w Deep Sleep i spróbować ponownie później
        //Serial.println("Restartuję urządzenie za 10 sekund...");
        //delay(10000);
        //ESP.restart();
    }

    // Reszta logiki setup (pomiar początkowy itp.) - uruchamiana TYLKO jeśli WiFi się połączyło (connectSuccess == true)
    if (connectSuccess) {

        // Synchronizacja czasu NTP - wymaga połączenia WiFi
        powerManagerSyncTime();

        if (runMeasurement) { // Ten if może być zbędny, jeśli zawsze mierzymy po sukcesie WiFi
            Serial.println("\n--- Pierwszy pomiar/cykl po starcie/wybudzeniu ---");
            // ... (reszta kodu pomiaru - bez zmian) ...
            digitalWrite(LED_BUILTIN, HIGH);

            int currentMoisture = soilSensorReadPercent();
            int currentWaterLevel = waterLevelSensorReadLevel();
            float currentBatteryVoltage = batteryMonitorReadVoltage();
            float currentTemperatureDHT;
            float currentHumidityDHT;
            bool dhtOk = environmentSensorRead(currentTemperatureDHT, currentHumidityDHT);

            alarmManagerUpdate(currentWaterLevel, currentBatteryVoltage, currentMoisture);

             // Sprawdzenie alarmu i ewentualna zmiana trybu na ciągły
             if (alarmManagerIsAlarmActive()) {
                  Serial.println("[SETUP] Wykryto aktywny alarm!");
                   if (!configIsContinuousMode()) {
                        Serial.println("[SETUP] Przełączam na tryb ciągły z powodu alarmu...");
                        configSetContinuousMode(true);
                   } else {
                        Serial.println("[SETUP] Już w trybie ciągłym z powodu alarmu.");
                    }
                }


            digitalWrite(LED_BUILTIN, LOW);

            // Wyświetl wyniki lokalnie
            Serial.printf("  Tryb ciągły: %s (false = Deep Sleep)\n", configIsContinuousMode() ? "TAK" : "NIE"); // Już wyświetlone wyżej
            if(currentMoisture >= 0) Serial.printf("  Wilgotność gleby: %d %%\n", currentMoisture); else Serial.println("  Wilgotność gleby: Błąd odczytu");
            Serial.printf("  Poziom wody: %d / %d\n", currentWaterLevel, NUM_WATER_LEVELS); // Zakładamy, że NUM_WATER_LEVELS jest zdefiniowane (np. w WaterLevelSensor.h)
            if(currentBatteryVoltage > 0) Serial.printf("  Napięcie baterii: %.2f V\n", currentBatteryVoltage); else Serial.println("  Napięcie baterii: Błąd odczytu / Odłączona");
            if (dhtOk && !isnan(currentTemperatureDHT) && !isnan(currentHumidityDHT)) {
                Serial.printf("  Temperatura (DHT): %.1f C\n", currentTemperatureDHT);
                Serial.printf("  Wilgotność pow. (DHT): %.1f %%\n", currentHumidityDHT);
            } else {
                 Serial.println("  Temperatura/Wilgotność (DHT): Błąd odczytu");
            }
            // Usunięto sekcję MPU
            Serial.println("-----------------------");

            // Wyślij dane do Blynk (już połączony)
            if (connectSuccess && blynkIsConnected()) { 
                 blynkSendSensorData(currentMoisture, currentWaterLevel, currentBatteryVoltage,
                                      currentTemperatureDHT, currentHumidityDHT,
                                      NAN, false, pumpControlIsRunning());
                 lastBlynkSend = millis();
            } else {
                Serial.println("Pomijam wysyłkę pierwszych danych - brak połączenia WiFi/Blynk.");
            }


            // Kontrola pompy
            pumpControlActivateIfNeeded(currentMoisture, currentWaterLevel);

        } // koniec if(runMeasurement)

    } else {
        // Ta część kodu (else dla if connectSuccess) prawdopodobnie nie zostanie osiągnięta
        // z powodu ESP.restart() w bloku obsługi błędu autoConnect.
        Serial.println("Nie udało się połączyć z WiFi. Pomijam resztę setup().");
    }


    // --- Przejście w Deep Sleep (tylko w tym trybie i po udanym setup) ---
    if (connectSuccess && !configIsContinuousMode()) { // Dodano warunek connectSuccess
        if (pumpControlIsRunning()) {
            Serial.println("Pompa pracuje - pozostaję w trybie aktywnym do zakończenia pracy pompy.");
        } else {
            digitalWrite(LED_BUILTIN, HIGH); delay(50); digitalWrite(LED_BUILTIN, LOW);
            Serial.println("Rozłączam WiFi/Blynk i konfiguruję wybudzanie...");
            blynkDisconnect();
            // WiFi jest zarządzane przez WiFiManager, explicit disconnect może nie być potrzebny
            // wifiDisconnect(); // Usuń, jeśli nie ma już tej funkcji
            powerManagerGoToDeepSleep();
        }
    } else if (connectSuccess) { // Jeśli połączono, ale jest tryb ciągły
        Serial.println("Tryb ciągły aktywny. Dalsza praca w loop().");
    }

} // Koniec setup()

//     esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
//     if (cause == ESP_SLEEP_WAKEUP_UNDEFINED || configIsContinuousMode()) {
//         if (wifiConnect(SECRET_SSID, SECRET_PASS)) {
//             blynkConfigure(BLYNK_AUTH_TOKEN, BLYNK_TEMPLATE_ID, BLYNK_DEVICE_NAME);
//             connectSuccess = blynkConnect(); // Spróbuj połączyć z Blynk
//         } else {
//             runMeasurement = false;
//             Serial.println("Brak WiFi, pomijam cykl pomiarowy przy starcie.");
//         }
//     } else {
//          // Wybudzono z Deep Sleep - konfiguracja Blynk wystarczy przed wysłaniem
//          blynkConfigure(BLYNK_AUTH_TOKEN, BLYNK_TEMPLATE_ID, BLYNK_DEVICE_NAME);
//          // Zakładamy, że pomiar i próba wysłania nastąpią
//     }

//     if (runMeasurement) {
//         Serial.println("\n--- Pierwszy pomiar/cykl po starcie/wybudzeniu ---");
//         Serial.printf("Tryb ciągły wczytany z pamięci: %s\n", configIsContinuousMode() ? "TAK" : "NIE");
//         digitalWrite(LED_BUILTIN, HIGH);

//         // Odczyty sensorów (bez MPU)
//         int currentMoisture = soilSensorReadPercent();
//         int currentWaterLevel = waterLevelSensorReadLevel();
//         float currentBatteryVoltage = batteryMonitorReadVoltage();
//         float currentTemperatureDHT;
//         float currentHumidityDHT;
//         bool dhtOk = environmentSensorRead(currentTemperatureDHT, currentHumidityDHT);

//         alarmManagerUpdate(currentWaterLevel, currentBatteryVoltage, currentMoisture);

//         digitalWrite(LED_BUILTIN, LOW);

//        if (alarmManagerIsAlarmActive()) {
//               Serial.println("[SETUP] Wykryto aktywny alarm!");
//                // Jeśli aktualny tryb to NIE ciągły (czyli Deep Sleep)
//               if (!configIsContinuousMode()) {
//                    Serial.println("[SETUP] Przełączam na tryb ciągły z powodu alarmu...");
//                    configSetContinuousMode(true); // Użyj settera, aby zapisać zmianę w Preferences i RAM
                 
//               } else {
//                    Serial.println("[SETUP] Już w trybie ciągłym z powodu alarmu.");
//                }
//            }
//            // --- KONIEC LOGIKI ZMIANY TRYBU ---

//         // Wyświetl wyniki lokalnie
//         Serial.println("--- Wyniki pomiarów ---");

//         Serial.printf("  Tryb ciągły: %s (false = Deep Sleep)\n", configIsContinuousMode() ? "TAK" : "NIE");
        
//         if(currentMoisture >= 0) Serial.printf("Wilgotność gleby: %d %%\n", currentMoisture); else Serial.println("Wilgotność gleby: Błąd odczytu");
//         Serial.printf("Poziom wody: %d / %d\n", currentWaterLevel, NUM_WATER_LEVELS);
//         if(currentBatteryVoltage > 0) Serial.printf("Napięcie baterii: %.2f V\n", currentBatteryVoltage); else Serial.println("Napięcie baterii: Błąd odczytu / Odłączona");
//         if (dhtOk && !isnan(currentTemperatureDHT) && !isnan(currentHumidityDHT)) {
//             Serial.printf("Temperatura (DHT11): %.1f C\n", currentTemperatureDHT);
//             Serial.printf("Wilgotność pow. (DHT11): %.1f %%\n", currentHumidityDHT);
//          } else {
//              Serial.println("Temperatura/Wilgotność (DHT11): Błąd odczytu");
//          }
//          // Usunięto sekcję MPU
//         Serial.println("-----------------------");

//         // Wyślij dane do Blynk
//         // Spróbuj połączyć WiFi/Blynk jeśli potrzeba (np. po wybudzeniu)
//         if (!wifiIsConnected()){ wifiConnect(SECRET_SSID, SECRET_PASS, 5000); }
//         if (wifiIsConnected()) {
//             powerManagerSyncTime(); // Synchronizuj czas z NTP

//             if (!blynkIsConnected()) { blynkConnect(3000); }
//             // Wyślij dane (bez MPU)
//              blynkSendSensorData(currentMoisture, currentWaterLevel, currentBatteryVoltage,
//                                   currentTemperatureDHT, currentHumidityDHT,
//                                   NAN, false, pumpControlIsRunning()); // Przekazujemy NAN dla tilt i false dla alertu
//              lastBlynkSend = millis();
//         }

//         // Kontrola pompy (bez sprawdzania przechyłu)
//         pumpControlActivateIfNeeded(currentMoisture, currentWaterLevel);

//     } // koniec if(runMeasurement)

//     // --- Przejście w Deep Sleep (tylko w tym trybie) ---
//     if (!configIsContinuousMode()) {
//     // Sprawdź, czy pompa nie pracuje
//     if (pumpControlIsRunning()) {
//         Serial.println("Pompa pracuje - pozostaję w trybie aktywnym do zakończenia pracy pompy.");
//         // Przejdź do loop() zamiast deep sleep
//     } else {
//         digitalWrite(LED_BUILTIN, HIGH); delay(50); digitalWrite(LED_BUILTIN, LOW);
//         Serial.println("Rozłączam WiFi/Blynk i konfiguruję wybudzanie...");
//         blynkDisconnect();
//         wifiDisconnect();
        
//         // Debug info - sprawdź tryb
//         Serial.print("Tryb ciągły: ");
//         Serial.println(configIsContinuousMode() ? "TAK" : "NIE");
        
//         // Go to deep sleep
//         powerManagerGoToDeepSleep(); // Konfiguruje timer i idzie spać
//     }
//   } else {
//       Serial.println("Tryb ciągły aktywny. Dalsza praca w loop().");
//   }
// } // Koniec setup()


void loop() {
    static int lastMoisture = -1;
    static int lastWaterLevel = -1;
    static float lastBatteryVoltage = -1.0;

    // --- Logika WiFi w loop() - JUŻ NIEPOTRZEBNA ---
    // Biblioteka WiFiManager sama dba o utrzymanie połączenia (jeśli setWiFiAutoReconnect(true))
    // Usuwamy ręczne próby ponownego łączenia WiFi.
    // Sprawdzamy tylko połączenie Blynk.

    // Obsługa Blynk (jeśli WiFi jest połączone - co powinno być zapewnione przez WiFiManager)
    if (WiFi.status() == WL_CONNECTED) { // Sprawdź stan połączenia WiFi
       if (!blynkIsConnected()) {
          // Jeśli Blynk się rozłączył, pomimo aktywnego WiFi, spróbuj połączyć ponownie
          static unsigned long lastBlynkTry = 0;
          // Nie próbuj zbyt często, żeby nie blokować pętli
          if (millis() - lastBlynkTry > 15000) { // Np. co 15 sekund
              Serial.println("[Loop] Blynk rozłączony, próba połączenia...");
              blynkConnect(1000); // Krótki timeout
              lastBlynkTry = millis();
          }
       }
       // Zawsze uruchamiaj Blynk.run(), jeśli jest połączenie WiFi
       // (Blynk.run() wewnętrznie sprawdza, czy jest połączony z serwerem)
       blynkRun();
    } else {
        // Jeśli WiFi nie jest połączone (WiFiManager mógł stracić połączenie i próbuje się połączyć)
        // Można dodać log, ale generalnie czekamy, aż WiFiManager przywróci połączenie.
        // Serial.println("[Loop] Oczekuję na połączenie WiFi...");
        // delay(1000); // Unikaj blokowania pętli na długo
    }


    // Reszta pętli loop() - uruchamiana niezależnie od stanu Blynk, ale wymaga działającego urządzenia
    pumpControlUpdate(); // Aktualizacja stanu pompy

    // --- Pomiar i wysyłka w trybie ciągłym ---
    if (configIsContinuousMode()) {
        uint32_t interval = configGetBlynkSendIntervalSec() * 1000;
        if (interval == 0) interval = 60000; // Domyślny interwał, jeśli 0

        if (millis() - lastBlynkSend > interval) {
             // Wykonuj pomiar i wysyłkę TYLKO jeśli WiFi jest połączone
             if (WiFi.status() == WL_CONNECTED) {
                 Serial.println("\n--- Cykliczny pomiar i wysyłka danych (tryb ciągły) ---");
                // ... (kod pomiaru sensorów - bez zmian) ...
                 digitalWrite(LED_BUILTIN, HIGH);
                 int currentMoisture = soilSensorReadPercent();
                 int currentWaterLevel = waterLevelSensorReadLevel();
                 float currentBatteryVoltage = batteryMonitorReadVoltage();
                 float currentTemperatureDHT;
                 float currentHumidityDHT;
                 bool dhtOk = environmentSensorRead(currentTemperatureDHT, currentHumidityDHT);
                 digitalWrite(LED_BUILTIN, LOW);

                 // Aktualizuj ostatnie wartości do użycia przez alarmManagerUpdate na końcu pętli
                 lastMoisture = currentMoisture;
                 lastWaterLevel = currentWaterLevel;
                 lastBatteryVoltage = currentBatteryVoltage;


                // ... (kod wyświetlania lokalnego - bez zmian) ...

                 // Wyślij dane do Blynk (sprawdź ponownie, czy jest połączony)
                 if (blynkIsConnected()) {
                     blynkSendSensorData(currentMoisture, currentWaterLevel, currentBatteryVoltage,
                                          currentTemperatureDHT, currentHumidityDHT,
                                          NAN, false, pumpControlIsRunning());
                     lastBlynkSend = millis();
                 } else {
                      Serial.println("Pomijam wysyłkę - brak połączenia z Blynk.");
                 }

                // Kontrola pompy
                pumpControlActivateIfNeeded(currentMoisture, currentWaterLevel);

             } else {
                 // Jeśli WiFi nie działa, nie wykonuj cyklicznego pomiaru i wysyłki
                 //Serial.println("[Loop] WiFi rozłączone, pomijam cykliczny pomiar.");
                 // Zresetuj timer, aby spróbować ponownie po odzyskaniu połączenia
                 lastBlynkSend = millis();
             }

        } // koniec if (millis() - lastBlynkSend > interval)

    } else { // Tryb Deep Sleep
        // W trybie Deep Sleep, pętla loop() jest aktywna tylko, gdy pompa pracuje.
        // Po zakończeniu pracy pompy, setup() przejdzie w Deep Sleep.
        if (!pumpControlIsRunning()) {
             // Ta sytuacja nie powinna wystąpić, jeśli logika w setup() jest poprawna,
             // ale na wszelki wypadek dodajemy logikę uśpienia.
             Serial.println("[Loop] Pompa zakończyła pracę w trybie Deep Sleep (nieoczekiwane?), przechodzę do uśpienia...");
             digitalWrite(LED_BUILTIN, HIGH); delay(50); digitalWrite(LED_BUILTIN, LOW);
             blynkDisconnect();
             // WiFi jest zarządzane przez WiFiManager
             powerManagerGoToDeepSleep();
        } else {
            // Pompa nadal pracuje, nic nie rób poza obsługą Blynk.run() na początku pętli
             //Serial.println("[Loop] Pompa pracuje w trybie Deep Sleep...");
             // delay(500); // Można dodać małe opóźnienie
        }
    }

    // Aktualizacja stanu alarmu na podstawie ostatnich poprawnych odczytów
    // Wykonuj nawet jeśli WiFi/Blynk nie działa, aby alarm dźwiękowy (jeśli włączony) działał
    alarmManagerUpdate(lastWaterLevel, lastBatteryVoltage, lastMoisture);

} // Koniec loop()