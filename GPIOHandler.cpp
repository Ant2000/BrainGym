#include "esp32-hal-gpio.h"
#include "GPIOHandler.h"

namespace GPIOHandler {
int16_t GSRReadout = 0;
double chestExpansion = 0.00;
double remainingBattery = 0.00;
bool resetBlynk = false;

uint16_t GSRDefault = GSR_DEFAULT;
uint16_t LDSDefault = 4095;

const double batteryConst = 100.00 / (MAX_RAW_VOLTAGE - MIN_RAW_VOLTAGE);

QueueHandle_t interruptQueue = xQueueCreate(1, sizeof(gpio_num_t));
;

void getGSRDefault() {
    ESP_LOGI(GPIO_TAG, "Calculating initial GRS values");
    uint32_t GSRInput = 0;
    for (int i = 0; i < 10000; i++)
        GSRInput += analogRead(GSR_INPUT_PIN);
    GSRDefault = (uint32_t)((double)GSRInput * 0.0001);
}

double getRemainingBattery(uint16_t batIn) {
    if (batIn > MAX_RAW_VOLTAGE) {
        return 100.0;
    }
    if (batIn < MIN_RAW_VOLTAGE) {
        return 0.0;
    }
    double temp = batteryConst * ((double)batIn - (double)MIN_RAW_VOLTAGE);
    ESP_LOGI(GPIO_TAG, "%d %f", batIn, temp);
    return temp;
}

void updateLDSDefault() {
    LDSDefault = analogRead(LDS_INPUT_PIN);
}

void updateGSRDefault() {
    getGSRDefault();
}



void setupGPIO() {
    pinMode(SELECT_BUTTON_PIN, INPUT_PULLUP);
    pinMode(CYCLE_BUTTON_PIN, INPUT_PULLUP);
    pinMode(GSR_INPUT_PIN, INPUT);
    pinMode(LDS_INPUT_PIN, INPUT);
    pinMode(VOLTAGE_INPUT_PIN, INPUT);

    xTaskCreatePinnedToCore(updateGPIO, "updateGPIO", 4096, NULL, 10, NULL, 0);
    xTaskCreatePinnedToCore(interruptQueueHandler, "interruptQueueHandler", 4096, NULL, 10, NULL, 0);
}

void updateGPIO(void *args) {
    while (1) {
        GSRReadout = analogRead(GSR_INPUT_PIN) - GSRDefault;
        chestExpansion = 15.0 - ((double)(analogRead(LDS_INPUT_PIN) + (4095 - LDSDefault)) * 0.003663003663003663);
        remainingBattery = getRemainingBattery(analogRead(VOLTAGE_INPUT_PIN));

        ESP_LOGD(GPIO_TAG, "GSR: %d, LDS: %f, Battery: %f", GSRReadout, chestExpansion, remainingBattery);
        uint8_t temp = 0;
        xQueueSend(LCD::LCDQueue, &temp, 0);

        if (digitalRead(SELECT_BUTTON_PIN) == LOW) {
            gpio_num_t pin = SELECT_BUTTON_PIN;
            xQueueSend(interruptQueue, &pin, 0);
        }
        else if (digitalRead(CYCLE_BUTTON_PIN) == LOW) {
            gpio_num_t pin = CYCLE_BUTTON_PIN;
            xQueueSend(interruptQueue, &pin, 0);
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void interruptQueueHandler(void *args) {
    gpio_num_t pin;
    uint8_t temp = 1;
    xQueueSend(LCD::LCDQueue, &temp, 0);
    while (1) {
        if (xQueueReceive(interruptQueue, &pin, portMAX_DELAY) == pdTRUE) {
            detachInterrupt(pin);
            vTaskDelay(100 / portTICK_PERIOD_MS);

            if (pin == SELECT_BUTTON_PIN) {
                ESP_LOGI(GPIO_TAG, "Select button pressed");
                if (LCD::currentScreen == CHEST_SCREEN) {
                    updateLDSDefault();
                } else if (LCD::currentScreen == GSR_SCREEN) {
                    updateGSRDefault();
                } else if (LCD::currentScreen == WIFI_SCREEN) {
                    resetBlynk = true;
                }
            } else if (pin == CYCLE_BUTTON_PIN) {
                ESP_LOGI(GPIO_TAG, "Cycle button pressed");
                xQueueSend(LCD::LCDQueue, &temp, 0);
            }
            vTaskDelay(400 / portTICK_PERIOD_MS);
            xQueueReset(interruptQueue);
        }
    }
}
}