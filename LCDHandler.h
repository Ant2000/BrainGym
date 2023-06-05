#pragma once

#include <esp_log.h>
#include <LiquidCrystal_I2C.h>

#include "GPIOHandler.h"
#include "definations.h"
#include "HeartRate.h"

namespace LCD
{
    extern QueueHandle_t LCDQueue;
    extern uint8_t currentScreen;
    extern bool isBlynkConnected;
    
    void setupLCD();
    void updateLCD(void *args);
} // namespace LCD
