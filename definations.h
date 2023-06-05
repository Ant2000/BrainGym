#pragma once

#define SELECT_BUTTON_PIN (gpio_num_t)25
#define CYCLE_BUTTON_PIN (gpio_num_t)26
#define GSR_INPUT_PIN (gpio_num_t)34
#define LDS_INPUT_PIN (gpio_num_t)35
#define VOLTAGE_INPUT_PIN (gpio_num_t)32

#define MAX_SDA_PIN (gpio_num_t)18
#define MAX_SCL_PIN (gpio_num_t)19
#define MAX_I2C_ADDRESS 0x57
#define RISING_THRESHOLD_IR 2
#define HEART_BUFFER_SIZE 12

#define LCD_SDA_PIN (gpio_num_t)23
#define LCD_SCL_PIN (gpio_num_t)22
#define LCD_I2C_ADDRESS 0x27

#define MAX_RAW_VOLTAGE 2607
#define MIN_RAW_VOLTAGE 1551

#define GSR_DEFAULT 2470

#define LCD_ROWS 2
#define LCD_COLUMNS 16

#define HEART_SCREEN 0
#define GSR_SCREEN 1
#define CHEST_SCREEN 2
#define WIFI_SCREEN 3
#define BATTERY_SCREEN 4
#define TOTAL_SCREENS 5

#define GPIO_TAG "GPIO"
#define LCD_TAG "LCD"
