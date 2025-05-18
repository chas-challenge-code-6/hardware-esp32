#include "tasks/accelerometerTask.h"
#include "SensorData.h"
#include "sensors/accelerometer.h"
#include <Arduino.h>

extern QueueHandle_t dataQueue;

void accelTask(void *pvParameters)
{
    sensor_data_t accelData = {};
    SensorAccelerometer accel;
    bool initialized = false;

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

        accelData.accelTotal = accel.getTotal();
        accelData.accelPitch = accel.getPitch();
        accelData.accelRoll = accel.getRoll();

        Serial.printf("Acc: %.2f g | Pitch: %.2f° | Roll: %.2f°\n", accelData.accelTotal,
                      accelData.accelPitch, accelData.accelRoll);

        // Fall detection
        if (accelData.accelTotal > ACC_THRESHOLD && (abs(accelData.accelPitch) > ANGLE_THRESHOLD ||
                                                     abs(accelData.accelRoll) > ANGLE_THRESHOLD))
        {
            Serial.println("[Accelerometer Task] FALL DETECTED!");
        }

        // Pedometer
        // if (xQueueSend(dataQueue, &accelData, portMAX_DELAY) != pdPASS)
        //     Serial.println("[Accelerometer Task] Data sent to queues successfully.");
        // {
        //     Serial.println("[Accelerometer Task] Failed to send coordinates to queue");
        // }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
