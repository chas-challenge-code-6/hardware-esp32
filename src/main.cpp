#include "main.h"
#include "sensors/dht22.h"
#include "tasks/temp_humid.h"
#include "network/bluetooth.h"
#include "sensors/mq2.h"

#include <Arduino.h>
#include <DHT.h>

SensorDHT dhtSensor(DHT_PIN);
BluetoothClient bClient;

void setup()
{
    Serial.begin(115200);
    dhtSensor.begin();

    bClient.begin();

    //xTaskCreate(dhtTask, "DHT Task", 2048, NULL, 1, NULL);
}

void loop() {
    bClient.loop();
    delay(1000);
} // Using RTOS tasks, unused
