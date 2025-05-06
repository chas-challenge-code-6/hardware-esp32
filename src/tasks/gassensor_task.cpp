#include "tasks/gas_sensor.h"
#include "sensors/mq2.h"
#include <Arduino.h>

extern MQ2Sensor gasSensor;

void MQ2Task(void *parameter)
{
    while (true)
    {
        gasSensor.update();
        Serial.print("Gas Concentration (PPM): ");
        Serial.println(gasSensor.getPPM());
        vTaskDelay(pdMS_TO_TICKS(2000)); 
    }
}