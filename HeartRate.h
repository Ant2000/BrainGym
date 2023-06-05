#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <MAX30102_PulseOximeter.h>

#include "definations.h"

namespace HeartRate
{
    extern uint8_t heartRate;

    void setupHeartRate();
    void updateHeartRate(void *args);
}