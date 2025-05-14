#include "tasks/accelerometerTask.h"
#include "SensorData.h"
#include "sensors/accelerometer.h"
#include <Arduino.h>

// extern SensorAccelerometer accel;
extern QueueHandle_t dataQueue;

void accelTask(void *pvParameters)
{

    SensorAccelerometer accel;
    sensor_data_t accelData = {};

    if (!accel.begin())
    {
        Serial.println("[Accelerometer Task] Failed to initialize accelerometer.");
        vTaskDelete(NULL);
    }

    while (true)
    {
        accel.update();

        // Replace this stuff with fall detection
        accelData.accelX = accel.getX();
        accelData.accelY = accel.getY();
        accelData.accelZ = accel.getZ();

        if (xQueueSend(dataQueue, &accelData, portMAX_DELAY) != pdPASS)
            Serial.println("[Accelerometer Task] Data sent to queues successfully.");
        {
            Serial.println("[Accelerometer Task] Failed to send coordinates to queue");
        }

        Serial.print("[Accelerometer Task] X: ");
        Serial.print(accelData.accelX);
        Serial.print(", Y:");
        Serial.print(accelData.accelY);
        Serial.print(", Z:");
        Serial.println(accelData.accelZ);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
