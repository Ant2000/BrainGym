#include "freertos/portmacro.h"
#include "LCDHandler.h"

namespace LCD
{
    LiquidCrystal_I2C lcd{0x27, LCD_COLUMNS, LCD_ROWS};

    QueueHandle_t LCDQueue = xQueueCreate(10, sizeof(uint8_t));
    uint8_t currentScreen = 0;
    bool isBlynkConnected = false;

    void updateScreen(uint8_t screenNum)
    {
        switch (screenNum)
        {
        case HEART_SCREEN:
            lcd.setCursor(0, 0);
            lcd.print("Heart Rate:          ");
            lcd.setCursor(0, 1);
            lcd.printf("PS BPM          ");
            break;
        case GSR_SCREEN:
            lcd.setCursor(0, 0);
            lcd.print("GSR:             ");
            lcd.setCursor(0, 1);
            lcd.printf("%d          ", GPIOHandler::GSRReadout);
            break;
        case CHEST_SCREEN:
            lcd.setCursor(0, 0);
            lcd.print("Chest Exp:        ");
            lcd.setCursor(0, 1);
            lcd.printf("%.2f cm         ", GPIOHandler::chestExpansion);
            break;
        case WIFI_SCREEN:
            lcd.setCursor(0, 0);
            lcd.print("WiFi:            ");
            lcd.setCursor(0, 1);
            if(isBlynkConnected)
                lcd.print("Connected          ");
            else
                lcd.print("Disconnected          ");
            break;
        case BATTERY_SCREEN:
            lcd.setCursor(0, 0);
            lcd.print("Battery:          ");
            lcd.setCursor(0, 1);
            lcd.printf("%.2f %          ", GPIOHandler::remainingBattery);
            break;
        default:
            break;
        }
    }

    void updateScreenValue(uint8_t screenNum)
    {
        switch (screenNum)
        {
        case HEART_SCREEN:
            lcd.setCursor(0, 1);
            lcd.printf("%d BPM          ", HeartRate::heartRate);
            break;

        case GSR_SCREEN:
            lcd.setCursor(0, 1);
            lcd.printf("%d          ", GPIOHandler::GSRReadout);
            break;

        case CHEST_SCREEN:
            lcd.setCursor(0, 1);
            lcd.printf("%.2f cm         ", GPIOHandler::chestExpansion);
            break;

        case WIFI_SCREEN:
            lcd.setCursor(0, 1);
            if(isBlynkConnected)
                lcd.print("Connected          ");
            else
                lcd.print("Disconnected          ");
            break;

        case BATTERY_SCREEN:
            lcd.setCursor(0, 1);
            lcd.printf("%.2f %%           ", GPIOHandler::remainingBattery);
            break;

        default:
            break;
        }
    }

    void setupLCD()
    {
        Wire.begin(LCD_SDA_PIN, LCD_SCL_PIN);
        lcd.init();
        lcd.backlight();
        xTaskCreate(updateLCD, "LCD Update", 2048, NULL, 12, NULL);
    }

    void updateLCD(void *args)
    {
        uint8_t requestType;
        while (1)
        {
            if (xQueueReceive(LCDQueue, &requestType, portMAX_DELAY) == pdTRUE)
            {
                if (requestType == 0)
                {
                    updateScreenValue(currentScreen);
                }
                else if (requestType == 1)
                {
                    currentScreen = (currentScreen + 1) % TOTAL_SCREENS;
                    updateScreen(currentScreen);
                }
            }
        }
    }
} // namespace LCD
