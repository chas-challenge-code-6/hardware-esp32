#include "tasks/accelerometerTask.h"
#include "SensorData.h"
#include "sensors/accelerometer.h"
#include <Arduino.h>

extern QueueHandle_t dataQueue;
extern EventGroupHandle_t networkEventGroup;
#define NETWORK_CONNECTED_BIT BIT0

void sendAccelData(sensor_message_t msg)
{
    EventBits_t bits = xEventGroupGetBits(networkEventGroup);

    if (bits & NETWORK_CONNECTED_BIT)
    {
        if (xQueueSend(dataQueue, &msg, portMAX_DELAY) != pdPASS)
        {
            Serial.println("[Accelerometer Task] Failed to send coordinates to queue");
        }
        else
        {
            Serial.println("[Accelerometer Task] Data sent to queue successfully.");
        }
    }
}

void accelTask(void *pvParameters)
{
    sensor_message_t msg = {};
    SensorAccelerometer accel;
    bool initialized = false;

    uint32_t lastStepTime = 0;
    uint32_t now = 0;

    static uint32_t lastStepSendTime = 0;
    const uint32_t TEN_MINUTES_MS = 10 * 60 * 1000;

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
        msg.data.accelPitch = accel.getPitch();
        msg.data.accelRoll = accel.getRoll();
        msg.data.accelTotal = accel.getTotal();
        msg.data.accelZ = accel.getZ();

#if DEBUG_MEH
        Serial.printf("Acc: %.2f g | Pitch: %.2f° | Roll: %.2f° | Z: %.2f\n", msg.data.accelTotal,
                      msg.data.accelPitch, msg.data.accelRoll, msg.data.accelZ);
#endif

        // Fall detection
        if (msg.data.accelTotal > ACC_THRESHOLD && (abs(msg.data.accelPitch) > ANGLE_THRESHOLD ||
                                                    abs(msg.data.accelRoll) > ANGLE_THRESHOLD))
        {
            msg.data.fall_detected = true;
            msg.valid.fall_detected = true;
            Serial.println("[Accelerometer Task] FALL DETECTED!");
            sendAccelData(msg);
            msg.data.fall_detected = false;
            msg.valid.fall_detected = false;
        }

        // Pedometer
        now = millis();
        lastStepSendTime = 0;
        if (accel.getZ() > STEP_THRESHOLD && (now - lastStepTime) > STEP_DEBOUNCE_MS)
        {
            msg.data.steps++;
            msg.valid.steps = true;
            lastStepTime = now;
            Serial.printf("[Accelerometer Task] Step detected! Total steps: %d\n", msg.data.steps);
            if (now - lastStepSendTime > TEN_MINUTES_MS) {
                sendAccelData(msg);
                lastStepSendTime = now;
            }
            msg.valid.steps = false;
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}