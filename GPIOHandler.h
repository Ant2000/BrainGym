#pragma once

#include <Arduino.h>
#include "definations.h"
#include "LCDHandler.h"

namespace GPIOHandler
{
    extern int16_t GSRReadout;
    extern double chestExpansion;
    extern double remainingBattery;
    extern bool resetBlynk;
    
    void setupGPIO();
    void updateLDSDefault();
    void updateGSRDefault();
    void updateGPIO(void *args);
    void interruptQueueHandler(void *args);
}