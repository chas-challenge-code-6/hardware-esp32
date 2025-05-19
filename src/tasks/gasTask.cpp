#include "tasks/gasTask.h"
#include "SensorData.h"
#include "sensors/mq2.h"
#include <Arduino.h>

extern QueueHandle_t dataQueue;
extern EventGroupHandle_t networkEventGroup;
#define NETWORK_CONNECTED_BIT BIT0

void sendGasData(sensor_data_t &gasData)
{
    EventBits_t bits = xEventGroupGetBits(networkEventGroup);

    if (bits & NETWORK_CONNECTED_BIT)
    {
        if (xQueueSend(dataQueue, &gasData, portMAX_DELAY) != pdPASS)
        {
            Serial.println("[Gas Task] Failed to send data to queue");
        }
        else
        {
            Serial.println("[Gas Task] Data sent to queue successfully.");
        }
    }
}

void gasTask(void *parameter)
{
    sensor_data_t gasData = {};
    MQ2Sensor gasSensor(GAS_PIN);
    float oldGasPPM = NAN;
    float newGasLevel = NAN;

    while (true)
    {
        gasSensor.update();
        newGasLevel = gasSensor.getValue();

        Serial.printf("[Gas Task] Gas Concentration (PPM): %.2f\n", newGasLevel);

        // TODO: Set threshold for sending data instead, the span could be bigger
        if (isnan(oldGasPPM) || newGasLevel != oldGasPPM)
        {
            gasData.gasLevel = newGasLevel;
            sendGasData(gasData);
            oldGasPPM = newGasLevel;
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}