#include "tasks/accelerometerTask.h"
#include "sensors/accelerometer.h"
#include "SensorData.h"
#include <Arduino.h>

//extern Accelerometer accelSensor;
extern QueueHandle_t dataQueue;

void accelTask(void *pvParameters)
{
    sensor_data_t accelData = {};

    while (true)
    {
        //accelData.accelX = accelSensor.getAccelX();
        //accelData.accelY = accelSensor.getAccelY();
        //accelData.accelZ = accelSensor.getAccelZ();

        if (xQueueSend(dataQueue, &accelData, pdMS_TO_TICKS(100)) != pdPASS) {
            Serial.println("[Bluetooth Task] Failed to send coordinates to queue");
        }
        else {
            Serial.print("[Bluetooth Task] Coordinates: ");
            Serial.println(accelData.accelX);
            Serial.print(", ");
            Serial.print(accelData.accelY);
            Serial.print(", ");
            Serial.println(accelData.accelZ);
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); // Run every 1 second
    }
}