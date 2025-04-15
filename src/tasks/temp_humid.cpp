#include "tasks/temp_humid.h"
#include "sensors/dht22.h"
#include <Arduino.h>

extern SensorDHT dhtSensor;

void dhtTask(void *parameter)
{
    while (true)
    {
        dhtSensor.update();
        Serial.print("Humdidity: ");
        Serial.println(dhtSensor.getHumdity());
        Serial.print("Temperature: ");
        Serial.println(dhtSensor.getTemperature());
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}