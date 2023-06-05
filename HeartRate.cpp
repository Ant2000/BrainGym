#include "HeartRate.h"

namespace HeartRate
{
    uint8_t heartRate = 0;

    uint32_t currentIR = 0;
    uint32_t previousIR = 0;

    uint8_t risingCountIR = 0;
    bool riseIR = false;

    uint32_t previousBeat = 0;
    uint8_t bpmArray[HEART_BUFFER_SIZE];

    MAX30105 particleSensor;

    void setupHeartRate()
    {
        Wire1.begin(MAX_SDA_PIN, MAX_SCL_PIN, I2C_SPEED_FAST);
        if (!particleSensor.begin(Wire1, I2C_SPEED_FAST)) // Use default I2C port, 400kHz speed
        {
            Serial.println("MAX30105 was not found. Please check wiring/power. ");
            while (1)
                vTaskDelay(10000 / portTICK_RATE_MS);
        }

        uint8_t ledBrightness = 0x80; // Options: 0=Off to 255=50mA
        uint8_t sampleAverage = 8;    // Options: 1, 2, 4, 8, 16, 32
        uint8_t ledMode = 2;          // Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
        int sampleRate = 100;         // Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
        int pulseWidth = 411;         // Options: 69, 118, 215, 411
        int adcRange = 4096;          // Options: 2048, 4096, 8192, 16384
        particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);

        xTaskCreate(updateHeartRate, "Heart Rate Update", 2048, NULL, 10, NULL);
    }

    void updateHeartRate(void *args)
    {
        while (1)
        {
            currentIR = particleSensor.getIR();
            if (currentIR > previousIR)
            {
                if (++risingCountIR > RISING_THRESHOLD_IR)
                    riseIR = true;
            }
            else if (currentIR < previousIR)
            {
                if (risingCountIR > 0)
                    risingCountIR--;

                if (riseIR)
                {
                    riseIR = false;

                    uint32_t timeBetweenBeats = millis() - previousBeat;
                    for (int i = 1; i < HEART_BUFFER_SIZE; i++)
                    {
                        bpmArray[i - 1] = bpmArray[i];
                    }
                    bpmArray[HEART_BUFFER_SIZE - 1] = (uint8_t)((60000L) / timeBetweenBeats);

                    uint16_t bpmSum = bpmArray[0];
                    uint8_t previousBPM = bpmArray[0];
                    uint8_t bpmCount = 1;
                    for (int i = 1; i < HEART_BUFFER_SIZE; i++)
                    {
                        if (abs(previousBPM - bpmArray[i]) > previousBPM / 2 && previousBPM != 0)
                            continue;
                        if (bpmArray[i] == 0)
                            continue;
                        bpmSum += bpmArray[i];
                        bpmCount++;
                        previousBPM = bpmArray[i];
                    }
                    heartRate = (uint8_t)(bpmSum / bpmCount);
                    previousBeat = millis();
                    risingCountIR = 0;
                }
            }
            previousIR = currentIR;
        }
    }
}