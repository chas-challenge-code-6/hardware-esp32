/**
 * @file accelerometerTask.cpp
 * @brief Accelerometer Task Implementation
 *
 * @details This file contains the implementation of the accelerometer task, which is responsible
 * for
 *
 */

#include "tasks/accelerometerTask.h"
#include "SensorData.h"
#include "sensors/accelerometer.h"
#include "utils/threadsafe_serial.h"
#include <Arduino.h>
#include <cstring>

#define QUEUE_SEND_TIMEOUT_MS 1000

extern QueueHandle_t dataQueue;
extern EventGroupHandle_t networkEventGroup;
extern SemaphoreHandle_t networkEventMutex;
#define NETWORK_CONNECTED_BIT BIT0

void sendAccelData(const sensor_message_t& msg)
{
    EventBits_t bits;

    if (xSemaphoreTake(networkEventMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        bits = xEventGroupGetBits(networkEventGroup);
        xSemaphoreGive(networkEventMutex);
    }
    else
    {
        safePrintln("[Accel Task] Failed to access network event group");
        return;
    }

    if (bits & NETWORK_CONNECTED_BIT)
    {
        if (xQueueSend(dataQueue, &msg, QUEUE_SEND_TIMEOUT_MS / portTICK_PERIOD_MS) != pdPASS)
        {
            safePrintln("[Accel Task] Failed to send coordinates to queue");
        }
    }
}

/**
 *
 * @brief Accelerometer task function
 *
 * @details This function initializes the accelerometer and continuously reads data from it.
 * It also handles fall detection and step counting.
 *
 * @param pvParameters
 */

void accelTask(void* pvParameters)
{
    sensor_message_t msg;
    memset(&msg, 0, sizeof(msg));

    SensorAccelerometer accel;
    bool initialized = false;

    uint32_t lastStepTime = 0;
    uint32_t lastStepSendTime = 0;
    uint32_t lastFallTime = 0;
    uint32_t now = 0;
    uint32_t totalSteps = 0;
    uint32_t lastSentSteps = 0;
    const uint32_t FIVE_MINUTES_MS = 5 * 60 * 1000;
    const uint32_t ONE_MINUTE_MS = 60 * 1000;
    const uint32_t MAX_REASONABLE_STEPS = 100000;

    // Simple step detection variables
    float lastTotal = 0.0f;
    bool wasHigh = false;

    now = millis();
    lastStepTime = now;
    lastStepSendTime = now;

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
        safePrintln("[Accel Task] Failed to initialize accelerometer, deleting task.");
        vTaskDelete(NULL);
    }

    while (true)
    {
        memset(&msg, 0, sizeof(msg));

        now = millis();

        accel.update();
        msg.data.accelPitch = accel.getPitch();
        msg.data.accelRoll = accel.getRoll();
        msg.data.accelTotal = accel.getTotal();
        msg.data.accelZ = accel.getZ();

        bool validReading = (msg.data.accelZ >= -20.0 && msg.data.accelZ <= 20.0) &&
            (msg.data.accelTotal >= 0.0 && msg.data.accelTotal <= 20.0);

        if (!validReading)
        {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        // Fall detection
        bool fallCondition =
            msg.data.accelTotal > ACC_THRESHOLD && (abs(msg.data.accelPitch) > ANGLE_THRESHOLD ||
                abs(msg.data.accelRoll) > ANGLE_THRESHOLD);

        if (fallCondition)
        {
            // Only send fall alert if enough time has passed since last fall
            if (lastFallTime == 0 || (now - lastFallTime) > ONE_MINUTE_MS)
            {
                msg.data.fall_detected = true;
                msg.valid.fall_detected = 1;
                lastFallTime = now;

                safePrintln("[Accel Task] Fall detected! Sending alert...");
                sendAccelData(msg);
            }
        }
        else
        {
            // Reset fall detection after 1 minute
            if (lastFallTime > 0 && (now - lastFallTime) > ONE_MINUTE_MS)
            {
                msg.data.fall_detected = false;
                msg.valid.fall_detected = 1;

                safePrintln("[Accel Task] Fall detection reset after 1 minute");
                sendAccelData(msg);
                lastFallTime = 0;
            }
        }

        // The simplest of step detection using total accel.
        float currentTotal = msg.data.accelTotal;
        uint32_t timeSinceLastStep = now - lastStepTime;

        if (timeSinceLastStep >= STEP_MIN_TIME_MS)
        {
            if (!wasHigh && currentTotal > STEP_THRESHOLD)
            {
                wasHigh = true;
            }
            else if (wasHigh && currentTotal < STEP_THRESHOLD)
            {
                wasHigh = false;
                totalSteps++;
                lastStepTime = now;

                safePrintf("[Accel Task] Step detected! Total: %d (accel: %.2f)\n", totalSteps, currentTotal);

                if (totalSteps > MAX_REASONABLE_STEPS)
                {
                    safePrintln("[Accel Task] Step counter corrupted, resetting...");
                    totalSteps = 0;
                }
            }
        }

        lastTotal = currentTotal;

        // Send step data every 5 minutes if changed
        if (now - lastStepSendTime > FIVE_MINUTES_MS && totalSteps != lastSentSteps)
        {
            safePrintf("[Accel Task] Sending step data: %d steps (changed from %d)\n", totalSteps,
                lastSentSteps);
            msg.data.steps = totalSteps;
            msg.valid.steps = 1;

            sendAccelData(msg);
            lastStepSendTime = now;
            lastSentSteps = totalSteps;
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}