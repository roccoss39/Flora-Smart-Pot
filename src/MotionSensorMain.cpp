// #include <Arduino.h>
// #include "MotionSensor.h" // Dołącz nagłówek naszej klasy

// // --- Konfiguracja ---
// #define SDA_PIN 21 // Domyślny SDA dla ESP32
// #define SCL_PIN 22 // Domyślny SCL dla ESP32

// const float TILT_THRESHOLD_DEGREES = 15.0;
// const uint8_t WOM_THRESHOLD = 1;     // Czułość WoM (0-255, niższa=czulszy) - może wymagać dostrojenia!
// #define DEEP_SLEEP_ENABLED true       // ******** Włączone dla testu! ********
// #define SLEEP_DURATION_SECONDS 30     // Krótszy czas do uśpienia dla testów
// #define MPU_INT_PIN 35                // GPIO ESP32 podłączony do INT MPU (musi być RTC!)

// MotionSensor motionSensor;
// unsigned long lastOkTime = 0;
// bool firstLoopAfterWake = false; // Flaga do obsługi pierwszego cyklu po wybudzeniu

// void setup() {
//   Serial.begin(115200);
//   while (!Serial);
//   delay(100); // Krótka pauza dla stabilizacji Serial
//   Serial.println("\n\n--- Smart Plant Monitor ---");

//   // Sprawdź przyczynę wybudzenia
//   esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
//   Serial.print("Wakeup Cause: ");
//   switch(wakeup_reason) {
//     case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("External signal using RTC_IO (MPU INT)"); firstLoopAfterWake = true; break;
//     case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Timer"); break;
//     // Dodaj inne przyczyny jeśli potrzebujesz
//     default : Serial.println("Power ON or other reset"); break;
//   }

//   // Inicjalizuj czujnik ruchu - funkcja begin() ustawi PWR_MGMT_1 = 0 (normalny tryb)
//   if (!motionSensor.begin(SDA_PIN, SCL_PIN)) { // Przekazujemy piny
//     Serial.println("FATAL: Failed to initialize Motion Sensor!");
//     while(1);
//   }
//   Serial.println("Motion Sensor Initialized (Normal Mode).");

//   // Skonfiguruj Wake on Motion (tylko jeśli Deep Sleep włączony)
//   if (DEEP_SLEEP_ENABLED) {
//     Serial.println("Configuring Wake on Motion settings...");
//     // Podłącz pin INT z MPU do MPU_INT_PIN na ESP32!
//     if (motionSensor.setupWakeOnMotion(WOM_THRESHOLD)) {
//       Serial.println("Wake on Motion configured on MPU.");
//       // Skonfiguruj ESP32 do wybudzania pinem MPU_INT_PIN (Active High)
//       if (esp_sleep_enable_ext0_wakeup((gpio_num_t)MPU_INT_PIN, 1) == ESP_OK) { // Rzutowanie na gpio_num_t
//           Serial.print("ESP32 will wake up on HIGH signal on GPIO: ");
//           Serial.println(MPU_INT_PIN);
//       } else {
//           Serial.println("Error configuring ESP32 EXT0 wakeup!");
//       }
//     } else {
//       Serial.println("Failed to configure Wake on Motion on MPU!");
//     }
//   }
//   lastOkTime = millis(); // Zresetuj timer bezczynności
//   Serial.println("-----------------------------");
// }

// void loop() {

//   // Opcjonalnie: zignoruj pierwszy odczyt po wybudzeniu, aby dać czas na stabilizację
//   if(firstLoopAfterWake) {
//     Serial.println("First loop after wake-up, allowing sensor to stabilize...");
//     delay(500); // Poczekaj chwilę
//     firstLoopAfterWake = false; // Zresetuj flagę
//     // Przeczytaj dane raz, aby wyczyścić ewentualne stare flagi przerwań w MPU?
//     motionSensor.readData();
//     lastOkTime = millis(); // Zresetuj timer, aby od razu nie usnął
//     return; // Przejdź do następnej iteracji pętli
//   }


//   // Odczytaj dane z czujnika
//   if (motionSensor.readData()) {
//     // Dane odczytane pomyślnie

//     // --- Logika Alarmu Przechyłu ---
//     if (motionSensor.isTilted(TILT_THRESHOLD_DEGREES)) {
//       Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
//       Serial.print("ALARM: Doniczka przechylona! Pitch: ");
//       Serial.print(motionSensor.getPitch());
//       Serial.print(", Roll: ");
//       Serial.println(motionSensor.getRoll());
//       Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
//       lastOkTime = millis(); // Reset timera, bo wykryto alarm/ruch
//     } else {
//       // Stan OK - drukuj normalne dane
//       Serial.print("Acc(g): ");
//       Serial.print(motionSensor.getAccX_g(), 2); Serial.print(", ");
//       Serial.print(motionSensor.getAccY_g(), 2); Serial.print(", ");
//       Serial.print(motionSensor.getAccZ_g(), 2);
//       Serial.print(" | Gyro(dps): ");
//       Serial.print(motionSensor.getGyroX_dps(), 2); Serial.print(", ");
//       Serial.print(motionSensor.getGyroY_dps(), 2); Serial.print(", ");
//       Serial.print(motionSensor.getGyroZ_dps(), 2);
//       Serial.print(" | Pitch: "); Serial.print(motionSensor.getPitch(), 1);
//       Serial.print(" | Roll: "); Serial.println(motionSensor.getRoll(), 1);

//       // --- Logika Usypiania (tylko jeśli włączone) ---
//       if (DEEP_SLEEP_ENABLED && (millis() - lastOkTime > (SLEEP_DURATION_SECONDS * 1000))) {
//           Serial.print("No significant tilt detected for ");
//           Serial.print(SLEEP_DURATION_SECONDS);
//           Serial.println(" seconds.");

//           // *** Krok 1: Przełącz MPU w tryb niskiego poboru mocy (Cycle Mode) ***
//           if (!motionSensor.enterCycleMode()) {
//               Serial.println("Error putting MPU into cycle mode! Aborting sleep.");
//           } else {
//               // *** Krok 2: Przejdź w tryb głębokiego uśpienia ESP32 ***
//               Serial.println("MPU in Cycle Mode. Entering ESP32 deep sleep. Tap pot to wake...");
//               Serial.flush(); // Upewnij się, że Serial wysłał wszystko
//               esp_deep_sleep_start(); // Uśpij ESP32
//               // Kod tutaj już się nie wykona aż do następnego wybudzenia (resetu)
//           }
//           lastOkTime = millis(); // Zresetuj timer na wszelki wypadek (gdyby sleep się nie udał)
//       }
//     }

//   } else {
//     // Błąd odczytu danych
//     Serial.println("Error reading sensor data in loop.");
//     lastOkTime = millis(); // Zresetuj timer, żeby nie usnął od razu po błędzie
//   }

//   delay(200); // Opóźnienie między odczytami/sprawdzeniami
// }
