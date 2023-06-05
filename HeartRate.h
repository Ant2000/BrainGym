#pragma once

#include <Arduino.h>
#include <Wire.h>

#include "definations.h"

namespace HeartRate
{
    extern uint8_t heartRate;

    void setupHeartRate();
    void updateHeartRate(void *args);
}