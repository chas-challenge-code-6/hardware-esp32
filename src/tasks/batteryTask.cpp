#include "tasks/batteryTask.h"
#include "SensorData.h"
#include "battery.h"
#include <Arduino.h>

extern QueueHandle_t dataQueue;

void batteryTask(void *parameter)
{
    BatteryMonitor battery(BOARD_BAT_ADC_PIN);
    // battery.begin();
    sensor_data_t battData = {};

    while (true)
    {
        float voltage = battery.readVoltage(); // in mV
        float percent = battery.percent();  // in %

        // Map battery percent to device_battery (uint8_t)
        battData.device_battery = static_cast<uint8_t>(percent);
        // No field for voltage in sensor_data_t, so only percent is sent

        Serial.print("[Battery Task] Voltage: ");
        Serial.print(voltage);
        Serial.print(" mV, Percent: ");
        Serial.print(percent);
        Serial.println(" %");

        if(xQueueSend(dataQueue, &battData, portMAX_DELAY) != pdPASS)
        {
            Serial.println("[Battery Task] Failed to send data to queue");
        }

        Serial.println("[Battery Task] Data sent to queue");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}