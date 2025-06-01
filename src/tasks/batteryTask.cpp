/**
 * @file batteryTask.cpp
 * @brief Battery Task Implementation File
 *
 * @details This file contains the implementation of the batteryTask function, which is used to
 * handle battery monitoring operations in a FreeRTOS task. The task is responsible for reading
 * battery voltage and percentage data from the Battery class and sending it to a queue for
 * processing.
 *
 */

#include "tasks/batteryTask.h"
#include "SensorData.h"
#include "battery.h"
#include "config.h"
#include "utils/threadsafe_serial.h"
#include <Arduino.h>
#include <cstring>

#define QUEUE_SEND_TIMEOUT_MS 1000

extern QueueHandle_t dataQueue;
extern EventGroupHandle_t networkEventGroup;
extern SemaphoreHandle_t networkEventMutex;
#define NETWORK_CONNECTED_BIT BIT0

void sendBatteryData(const sensor_message_t& msg)
{
    EventBits_t bits;

    if (xSemaphoreTake(networkEventMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        bits = xEventGroupGetBits(networkEventGroup);
        xSemaphoreGive(networkEventMutex);
    }
    else
    {
        safePrintln("[Battery Task] Failed to access network event group");
        return;
    }

    if (bits & NETWORK_CONNECTED_BIT)
    {
        if (xQueueSend(dataQueue, &msg, QUEUE_SEND_TIMEOUT_MS / portTICK_PERIOD_MS) != pdPASS)
        {
            safePrintln("[Battery Task] Failed to send data to queue");
        }
    }
}

/**
 * @brief Battery Task function
 *
 * @details This function handles battery monitoring operations in a FreeRTOS task. It reads battery
 * voltage and percentage data from the Battery class and sends it to a queue for processing. The
 * task runs in an infinite loop, monitoring battery status and sending updates when the battery
 * level changes.
 *
 * @param parameter Task parameter (unused)
 */
void batteryTask(void* parameter)
{
    sensor_message_t msg;
    memset(&msg, 0, sizeof(msg));

    Battery battery;
    battery.begin();

    int oldBatteryPercent = -1;
    int newBatteryPercent = 0;
    float voltage = 0.0;

    safePrintln("[Battery Task] Battery monitoring started");

    while (true)
    {
        memset(&msg, 0, sizeof(msg));

        voltage = battery.readVoltage();
        newBatteryPercent = battery.percent();

        uint32_t voltage_mv = voltage * 1000;

        //voltage can't be read when USB is connected, just hack around it for now
        if (voltage_mv > 100)
        {
            if (voltage_mv < LOW_VOLTAGE_LEVEL)
            {
                safePrintf("[Battery Task] CRITICAL: Low battery voltage detected: %lu mV - System should enter deep sleep\n", voltage_mv);
            }
            else if (voltage_mv < WARN_VOLTAGE_LEVEL)
            {
                safePrintf("[Battery Task] WARNING: Low battery voltage: %lu mV\n", voltage_mv);
            }
        }


        // Send data when battery percentage changes by 1% or more
        if (oldBatteryPercent == -1 || abs(newBatteryPercent - oldBatteryPercent) >= 1)
        {
            msg.data.device_battery = newBatteryPercent;
            msg.valid.device_battery = 1;

            sendBatteryData(msg);

            safePrintf("[Battery Task] Voltage: %.2f V (%lu mV), Percent: %d %%\n", voltage, voltage_mv, newBatteryPercent);

            oldBatteryPercent = newBatteryPercent;
            battery.setRGB(newBatteryPercent);
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}