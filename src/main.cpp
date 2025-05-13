#include "main.h"
#include "SensorData.h"
#include "network/bluetooth.h"
#include "network/lte.h"
#include "network/wifi.h"
#include "sensors/dht22.h"
#include "sensors/mq2.h"
#include "sensors/accelerometer.h"
#include "tasks/gasTask.h"
#include "tasks/dhtTask.h"
#include "tasks/communicationTask.h"
#include "tasks/bluetoothTask.h"
#include "tasks/accelerometerTask.h"

#include <Arduino.h>
#include <DHT.h>
#include <MQUnifiedsensor.h>


SensorDHT dhtSensor(DHT_PIN);
BluetoothClient bClient;
MQ2Sensor gasSensor(MQ2_PIN);
SensorAccelerometer accel;

const char *ssid = "";
const char *password = "";
WiFiClient wifi(ssid, password);
LTEClient lte(Serial1);

QueueHandle_t dataQueue;
QueueHandle_t httpQueue;

void setup()
{
    Serial.begin(115200);

    dataQueue = xQueueCreate(10, sizeof(sensor_data_t));
    httpQueue = xQueueCreate(10, sizeof(sensor_data_t));

    xTaskCreate(accelTask, "AccelTask", 2048, &accel, 1, NULL);
    xTaskCreate(WiFiTask, "WiFi Task", 2048, &wifi, 1, NULL);
    xTaskCreate(bluetoothTask, "Bluetooth Task", 2048, &bClient, 1, NULL);
    xTaskCreate(dhtTask, "DHT Task", 2048, &dhtSensor, 1, NULL);
    xTaskCreate(gasSensorTask, "Gas Task", 2048, &gasSensor, 1, NULL);
    //xTaskCreate(communicationTask, "CommTask", 4096, &comm, 1, NULL);
    //xTaskCreate(processingTask, "Process", 2048, &process, 1, NULL);
}

void loop() {}
