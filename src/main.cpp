#include <Arduino.h>
#include "MotionSensor.h" // Dołącz nagłówek naszej klasy

// --- Konfiguracja ---
const float TILT_THRESHOLD_DEGREES = 15.0; // Próg przechyłu w stopniach
const uint8_t WOM_THRESHOLD = 20; // Czułość Wake-on-Motion (0-255, niższa = czulszy, 1 LSB=4mg)
#define DEEP_SLEEP_ENABLED false  // Ustaw na true, aby włączyć usypianie po czasie
#define SLEEP_DURATION_SECONDS 15 // Czas w sekundach przed uśpieniem (jeśli DEEP_SLEEP_ENABLED)
#define MPU_INT_PIN 35 // GPIO pin ESP32 podłączony do pinu INT MPU (wybierz RTC GPIO!)

MotionSensor motionSensor; // Utwórz obiekt sensora

// Zmienna do śledzenia czasu bezczynności (braku przechyłu)
unsigned long lastOkTime = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("\nSmart Plant Monitor - Motion Sensor Test");

  // Inicjalizuj czujnik ruchu
  if (!motionSensor.begin()) {
    Serial.println("Failed to initialize Motion Sensor!");
    // Można dodać próbę ponownej inicjalizacji lub zatrzymanie
    while(1);
  }
  Serial.println("Motion Sensor Initialized.");

  // --- Konfiguracja Wybudzania ---
  if (DEEP_SLEEP_ENABLED) {
    Serial.println("Configuring Wake on Motion for Deep Sleep...");
    // Podłącz pin INT z MPU do MPU_INT_PIN na ESP32!
    if (motionSensor.setupWakeOnMotion(WOM_THRESHOLD)) {
      Serial.println("Wake on Motion configured.");
      // Skonfiguruj ESP32 do wybudzania pinem MPU_INT_PIN
      // INT jest Active High (wg setupWakeOnMotion), więc budzimy poziomem WYSOKIM (1)
      esp_sleep_enable_ext0_wakeup(GPIO_NUM_35, 1); // Użyj GPIO_NUM_XX dla swojego pinu
      Serial.print("ESP32 will wake up on HIGH signal on GPIO: ");
      Serial.println(MPU_INT_PIN);
    } else {
      Serial.println("Failed to configure Wake on Motion on MPU!");
      // Rozważ, co zrobić w tej sytuacji - np. nie usypiać
    }
  }
  lastOkTime = millis(); // Zresetuj timer bezczynności
}

void loop() {
  // Odczytaj dane z czujnika
  if (motionSensor.readData()) {
    // Dane odczytane pomyślnie

    // --- Logika Alarmu Przechyłu ---
    if (motionSensor.isTilted(TILT_THRESHOLD_DEGREES)) {
      Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
      Serial.print("ALARM: Doniczka przechylona! Pitch: ");
      Serial.print(motionSensor.getPitch());
      Serial.print(", Roll: ");
      Serial.println(motionSensor.getRoll());
      Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
      lastOkTime = millis(); // Zresetuj timer, bo coś się dzieje
    } else {
      // Stan OK - drukuj normalne dane (opcjonalnie)
      Serial.print("Acc(g): ");
      Serial.print(motionSensor.getAccX_g(), 2); Serial.print(", ");
      Serial.print(motionSensor.getAccY_g(), 2); Serial.print(", ");
      Serial.print(motionSensor.getAccZ_g(), 2);
      Serial.print(" | Gyro(dps): ");
      Serial.print(motionSensor.getGyroX_dps(), 2); Serial.print(", ");
      Serial.print(motionSensor.getGyroY_dps(), 2); Serial.print(", ");
      Serial.print(motionSensor.getGyroZ_dps(), 2);
      Serial.print(" | Pitch: "); Serial.print(motionSensor.getPitch(), 1);
      Serial.print(" | Roll: "); Serial.println(motionSensor.getRoll(), 1);

      // Sprawdź, czy czas na uśpienie (tylko jeśli włączone)
      if (DEEP_SLEEP_ENABLED && (millis() - lastOkTime > (SLEEP_DURATION_SECONDS * 1000))) {
          Serial.print("No tilt detected for ");
          Serial.print(SLEEP_DURATION_SECONDS);
          Serial.println(" seconds. Entering deep sleep. Tap pot to wake...");
          Serial.flush(); // Upewnij się, że Serial wysłał wszystko
          esp_deep_sleep_start(); // Przejdź w tryb głębokiego uśpienia
          // Kod tutaj już się nie wykona aż do następnego wybudzenia (resetu)
      }
    }

  } else {
    // Błąd odczytu danych
    Serial.println("Error reading sensor data in loop.");
  }

  delay(200); // Opóźnienie między odczytami/sprawdzeniami
}