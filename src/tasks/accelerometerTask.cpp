/**
 * @file accelerometerTask.cpp
 * @brief Accelerometer Task Implementation
 * 
 * @details This file contains the implementation of the accelerometer task, which is responsible for
 * 
 */

#include "tasks/accelerometerTask.h"
#include "SensorData.h"
#include "sensors/accelerometer.h"
#include <Arduino.h>


extern QueueHandle_t dataQueue;

/**
 * 
 * @brief Accelerometer task function
 * 
 * @details This function initializes the accelerometer and continuously reads data from it.
 * It also handles fall detection and step counting.
 * 
 * @param pvParameters 
 */

void accelTask(void *pvParameters)
{
    sensor_data_t accelData = {};
    SensorAccelerometer accel;
    bool initialized = false;

    uint32_t lastStepTime = 0;
    uint32_t now = 0;

    for (size_t i = 0; i < 3; i++)
    {
        if (accel.begin())
        {
            initialized = true;
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    if (!initialized)
    {
        Serial.println("[Accelerometer Task] Failed to initialize accelerometer, deleting task.");
        vTaskDelete(NULL);
    }

    while (true)
    {
        accel.update();

        accelData.accelZ = accel.getZ();
        accelData.accelTotal = accel.getTotal();
        accelData.accelPitch = accel.getPitch();
        accelData.accelRoll = accel.getRoll();

        Serial.printf("Acc: %.2f g | Pitch: %.2f° | Roll: %.2f° | Z: %.2f\n", accelData.accelTotal,
                      accelData.accelPitch, accelData.accelRoll, accelData.accelZ);

        // Fall detection
        if (accelData.accelTotal > ACC_THRESHOLD && (abs(accelData.accelPitch) > ANGLE_THRESHOLD ||
                                                     abs(accelData.accelRoll) > ANGLE_THRESHOLD))
        {
            Serial.println("[Accelerometer Task] FALL DETECTED!");
        }

        // Pedometer
        now = millis();
        if (accel.getZ() > STEP_THRESHOLD && (now - lastStepTime) > STEP_DEBOUNCE_MS)
        {
            accelData.steps++;
            lastStepTime = now;
            Serial.printf("[Accelerometer Task] Step detected! Total steps: %d\n", accelData.steps);
        }

        // TODO: Skriv logik för att bara skicka vidare data om något detekteras.
        // if (xQueueSend(dataQueue, &accelData, portMAX_DELAY) != pdPASS)
        //     Serial.println("[Accelerometer Task] Data sent to queues successfully.");
        // {
        //     Serial.println("[Accelerometer Task] Failed to send coordinates to queue");
        // }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}