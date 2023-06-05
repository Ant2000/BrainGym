#include "HeartRate.h"

namespace HeartRate {
uint8_t heartRate = 0;
uint32_t lastReport = 0;
PulseOximeter pox;

void onBeatDetected() {
    Serial.println("Beat!");
}

void setupHeartRate() {
    Wire.begin(MAX_SDA_PIN, MAX_SCL_PIN);
    if (!pox.begin()) {
        Serial.println("Failed to initialise heart rate sensor. Restarting in 1 second");
        delay(1000);
        esp_restart();
    } else {
        Serial.println("MAX30102 initialised");
    }
    pox.setIRLedCurrent(MAX30102_LED_CURR_7_6MA);
    pox.setOnBeatDetectedCallback(onBeatDetected);

    xTaskCreate(updateHeartRate, "Heart", 8192, NULL, 20, NULL);
}

void updateHeartRate(void *args) {
    while (1) {
        pox.update();
        if (millis() - lastReport > 1000) {
            Serial.print("Heart rate:");
            Serial.print(pox.getHeartRate());
            Serial.print("bpm / SpO2:");
            Serial.print(pox.getSpO2());
            Serial.print("%");
            lastReport = millis();
        }
    }
}
}