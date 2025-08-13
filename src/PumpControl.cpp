// PumpControl.cpp
#include "PumpControl.h"
#include "DeviceConfig.h"
#include <Arduino.h>
#include "BlynkManager.h"

// LEDC (PWM) configuration
const int PUMP_LEDC_CHANNEL = 0;    // LEDC channel (0-15)
const int PUMP_LEDC_FREQ = 5000;    // PWM frequency in Hz (5kHz)
const int PUMP_LEDC_RESOLUTION = 8; // PWM resolution (8 bits = values 0-255)

// Static variables
static uint8_t pumpPin;
static bool isPumpOn = false;
static unsigned long pumpStartTime = 0;
static uint32_t pumpTargetDuration = 0;

void pumpControlSetup() {
    pumpPin = configGetPumpPin();

    if (pumpPin != 255) {
        // Configure LEDC channel
        ledcSetup(PUMP_LEDC_CHANNEL, PUMP_LEDC_FREQ, PUMP_LEDC_RESOLUTION);
        // Assign GPIO pin to configured LEDC channel
        ledcAttachPin(pumpPin, PUMP_LEDC_CHANNEL);
        // Ensure pump is off at startup (duty cycle = 0)
        ledcWrite(PUMP_LEDC_CHANNEL, 0);
        isPumpOn = false;

        // Log configuration info
        Serial.printf("  [Pump] Configured PWM control pin: %d (LEDC Channel: %d, Freq: %d Hz, Res: %d bit)\n",
                      pumpPin, PUMP_LEDC_CHANNEL, PUMP_LEDC_FREQ, PUMP_LEDC_RESOLUTION);

        // Log other parameters
        uint8_t initialDuty = configGetPumpDutyCycle();
        uint32_t initialPumpMillis = configGetPumpRunMillis();
        int initialSoilThreshold = configGetSoilThresholdPercent();
        Serial.printf("  [Pump] Initial power (Duty Cycle): %d/255, Duration: %u ms, Threshold: %d%%\n", 
                      initialDuty, initialPumpMillis, initialSoilThreshold);

    } else {
        Serial.println("  [Pump] ERROR: Pump pin not properly configured in DeviceConfig!");
    }
}

void pumpControlActivateIfNeeded(int currentSoilMoisture, int currentWaterLevel) {
    Serial.println("--- Automatic pump control ---");

    // Early returns for invalid conditions
    if (isPumpOn) {
        Serial.println("  [Pump] Pump already running.");
        return;
    }
    
    if (pumpPin == 255) {
        Serial.println("  [Pump] ERROR: Pump pin not configured.");
        return;
    }
    
    if (currentWaterLevel <= 0) {
        Serial.println("  [Pump] ERROR: No water detected, cannot run pump.");
        return;
    }
    
    if (currentSoilMoisture < 0) {
        Serial.println("  [Pump] ERROR: Invalid soil moisture reading.");
        return;
    }

    int currentSoilThreshold = configGetSoilThresholdPercent();
    Serial.printf("  [Pump] Moisture: %d%%, Current threshold: %d%%\n", 
                 currentSoilMoisture, currentSoilThreshold);

    if (currentSoilMoisture < currentSoilThreshold) {
        uint32_t currentPumpRunMillis = configGetPumpRunMillis();
        uint8_t currentDutyCycle = configGetPumpDutyCycle();

        Serial.printf("  [Pump] Low moisture. Starting pump automatically for %u ms at power %d/255...\n", 
                     currentPumpRunMillis, currentDutyCycle);

        // Turn on pump using PWM
        ledcWrite(PUMP_LEDC_CHANNEL, currentDutyCycle);
        isPumpOn = true;
        pumpStartTime = millis();
        pumpTargetDuration = currentPumpRunMillis;
        blynkUpdatePumpStatus(isPumpOn);
        Serial.println("  [Pump] Pump started (auto).");
    } else {
        Serial.println("  [Pump] Soil moisture OK.");
    }
    Serial.println("---------------------------------");
}

void pumpControlManualTurnOn(uint32_t durationMillis) {
    // Check valid conditions
    if (pumpPin == 255) {
        Serial.println("  [Pump] ERROR: Pump pin not configured.");
        return;
    }
    
    if (isPumpOn) {
        Serial.println("  [Pump] Pump already running.");
        return;
    }

    uint8_t currentDutyCycle = configGetPumpDutyCycle();
    Serial.printf("  [Pump] Starting pump manually for %u ms at power %d/255...\n", 
                 durationMillis, currentDutyCycle);

    // Turn on pump using PWM
    ledcWrite(PUMP_LEDC_CHANNEL, currentDutyCycle);
    isPumpOn = true;
    pumpStartTime = millis();
    pumpTargetDuration = durationMillis;
    blynkUpdatePumpStatus(isPumpOn);
}

void pumpControlManualTurnOff() {
    if (pumpPin == 255) {
        Serial.println("  [Pump] ERROR: Pump pin not configured.");
        return;
    }

    if (isPumpOn) {
        Serial.println("  [Pump] Manual immediate pump shutdown...");
        // Turn off pump using PWM (duty cycle = 0)
        ledcWrite(PUMP_LEDC_CHANNEL, 0);
        isPumpOn = false;
        pumpTargetDuration = 0;
        blynkUpdatePumpStatus(isPumpOn);
    } else {
        Serial.println("  [Pump] Pump is already off.");
    }
}

bool pumpControlIsRunning() {
    return isPumpOn;
}

void pumpControlUpdate() {
    if (isPumpOn && (millis() - pumpStartTime >= pumpTargetDuration)) {
        Serial.printf("  [Pump] Run time (%u ms) elapsed. Turning off pump.\n", pumpTargetDuration);
        // Turn off pump using PWM (duty cycle = 0)
        ledcWrite(PUMP_LEDC_CHANNEL, 0);
        isPumpOn = false;
        pumpTargetDuration = 0;
        blynkUpdatePumpStatus(isPumpOn);
        Serial.println("  [Pump] Pump stopped (auto-off after timeout).");
    }
}