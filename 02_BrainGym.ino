#define BLYNK_PRINT Serial

#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <WiFiManager.h>
#include <FS.h>
#include <SPIFFS.h>

#include "./definations.h"
#include "./GPIOHandler.h"
#include "./LCDHandler.h"
#include "./HeartRate.h"

char blynk_token[34] = "DEFAULT";

StaticJsonDocument<200> doc;
bool saveNewConfiguration = false;

WiFiManagerParameter custom_blynk_token("blynk", "blynk token", blynk_token, 34);
WiFiManager wifiManager;

void inline saveConfigCallback()
{
    saveNewConfiguration = true;
}

/**
 * @brief Connect to WiFi and Blynk
 * 
 * @details Get Blynk token from SPIFFS if it exists, if not, get it from user whrough WiFi Manager.
 * If WiFi or Blynk connection fails, reset WiFi Manager settings and create AP
 * 
 */
void connectToBlynk()
{
    if (SPIFFS.begin(true))
    {
        if (SPIFFS.exists("/config.json"))
        {
            File configFile = SPIFFS.open("/config.json", FILE_READ);
            if (configFile)
            {
                size_t size = configFile.size();
                char buffer[size + 5];
                int i = 0;

                while (configFile.available())
                {
                    buffer[i] = configFile.read();
                    i++;
                }
                buffer[i] = '\0';
                Serial.println(buffer);

                DeserializationError error = deserializeJson(doc, buffer);
                if (error)
                {
                    Serial.print(F("deserializeJson() failed: "));
                    Serial.println(error.f_str());
                }
                else
                {
                    Serial.println("Parsed json");
                    strcpy(blynk_token, doc["blynk_token"]);
                    ESP_LOGI("MAIN", "%s", blynk_token);
                }
                configFile.close();
            }
        }
    }
    else
    {
        Serial.println("Failed to mount SPIFFS");
        esp_restart();
    }

    if (!wifiManager.autoConnect("BrainGym AP"))
    {
        Serial.println("failed to connect and hit timeout");
        delay(3000);
        esp_restart();
    }

    Serial.println("Connected");

    if (saveNewConfiguration)
    {
        Serial.println("Saving config");
        strcpy(blynk_token, custom_blynk_token.getValue());
        doc["blynk_token"] = blynk_token;

        if (SPIFFS.exists("/config.json"))
            SPIFFS.remove("/config.json");

        File configFile = SPIFFS.open("/config.json", FILE_WRITE);
        if (!configFile)
        {
            Serial.println("failed to open config file for writing");
        }
        serializeJson(doc, configFile);
        configFile.close();
    }

    uint32_t timeout = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
        if (millis() - timeout > 6000)
        {
            Serial.println(" ");
            break;
        }
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        ;
        Serial.print("Connected to WiFi with IP: ");
        Serial.println(WiFi.localIP());

        Blynk.config(blynk_token);
        bool result = Blynk.connect();

        if (result != true)
        {
            Serial.println("BLYNK Connection Fail");
            Serial.println(blynk_token);
            wifiManager.resetSettings();
        }
        else
        {
            Serial.println("BLYNK Connected");
            LCD::isBlynkConnected = true;
        }
    }
    else if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi Connection Fail");
    }
}

void blynkConnectionTask(void *args)
{
    while (1)
    {
        if (Blynk.connected())
        {
            LCD::isBlynkConnected = true;
            Blynk.virtualWrite(V0, GPIOHandler::remainingBattery);
            Blynk.virtualWrite(V1, HeartRate::heartRate);
            Blynk.virtualWrite(V2, GPIOHandler::GSRReadout);
            Blynk.virtualWrite(V3, GPIOHandler::chestExpansion);
        }
        else
        {
            LCD::isBlynkConnected = false;
        }

        if(GPIOHandler::resetBlynk)
        {
            Blynk.disconnect();
            WiFi.disconnect();
            wifiManager.resetSettings();
            LCD::isBlynkConnected = false;
            GPIOHandler::resetBlynk = false;
            connectToBlynk();
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void setup()
{
    Serial.begin(115200);
    wifiManager.setSaveConfigCallback(saveConfigCallback);
    wifiManager.addParameter(&custom_blynk_token);
    GPIOHandler::setupGPIO();
    LCD::setupLCD();
    HeartRate::setupHeartRate();
    connectToBlynk();
    xTaskCreate(blynkConnectionTask, "blynkConnectionTask", 10000, NULL, 10, NULL);
}

void loop()
{
    Blynk.run();
}