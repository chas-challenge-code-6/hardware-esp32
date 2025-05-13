#include "network/wifi.h"
#include "SensorData.h"
#include <Arduino.h>

extern QueueHandle_t httpQueue;

void WiFiTask(void* pvParameters) {
    WiFiClient* wifiClient = static_cast<WiFiClient*>(pvParameters);
    sensor_data_t outgoingData;
    while (true) 
    {
        if (!wifiClient->isConnected()) 
        {
            wifiClient->begin();
        }

        if(xQueueReceive(httpQueue, &outgoingData, portMAX_DELAY) == pdPASS) 
        {
            Serial.println("[Communication Task] Sending data over WiFi...");
            Serial.print("[Communication Task] Data: ");
            Serial.print("Temperature: ");
            Serial.print(outgoingData.temperature);
            Serial.print(" Humidity: ");
            Serial.print(outgoingData.humidity);
            Serial.print(" Gas: ");
            Serial.print(outgoingData.gasLevel);
            Serial.print(" Acceleration: ");
            Serial.print(outgoingData.accelX);
            Serial.print(" ");
            Serial.print(outgoingData.accelY);
            Serial.print(" ");
            Serial.print(outgoingData.accelZ);
        }

        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
