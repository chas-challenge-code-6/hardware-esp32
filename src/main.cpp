#include "main.h"
#include "sensors/dht22.h"
#include "tasks/temp_humid.h"

#include <Arduino.h>
#include <DHT.h>

SensorDHT dhtSensor(DHT_PIN);

void setup()
{
    Serial.begin(115200);
    dhtSensor.begin();

    xTaskCreate(dhtTask, "DHT Task", 2048, NULL, 1, NULL);
}

void loop() {} // Using RTOS tasks, unused
