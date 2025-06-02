/**
 * @file processingTask.cpp
 * @brief Processing Task Implementation File
 *
 * @details This file contains the implementation of the processingTask function, which is used to
 * handle processing operations in a FreeRTOS task. The task is responsible for processing sensor
 * data and creating a JSON string for network transmission.
 *
 */

#include "tasks/processingTask.h"
#include "SensorData.h"
#include "config.h"
#include "utils/threadsafe_serial.h"
#include <Arduino.h>

#define JSON_BUFFER_SIZE 512
#define HTTP_QUEUE_SEND_TIMEOUT_MS 2000
#define HTTP_QUEUE_SEND_RETRIES 3
#define DATA_QUEUE_RECEIVE_TIMEOUT_MS 1000

extern QueueHandle_t dataQueue;
extern QueueHandle_t httpQueue;

/**
 * @brief Create a Json object
 *
 * @details This function creates a JSON string from the sensor data and stores it in the provided
 * buffer. The JSON string contains the following fields:
 * - steps
 * - humidity
 * - gas
 * - fall_detected
 * - device_battery
 * - strap_battery
 * - heart_rate
 * - noise_level
 *
 * @param data
 * @param buffer
 * @param bufferSize
 * @return true
 * @return false
 */
bool createJson(const sensor_data_t &data, char *buffer, size_t bufferSize)
{
<<<<<<< HEAD
    int len =
        snprintf(buffer, bufferSize,
                 "{\"device_id\": \"%s\", \"sensors\": { "
                 "\"steps\": %d, "
                 "\"temperature\": %.2f, "
                 "\"humidity\": %d, "
                 "\"gas\": { \"ppm\": %d }, "
                 "\"fall_detected\": %d, "
                 "\"device_battery\": %d, "
                 "\"watch_battery\": \"0\", "
                 "\"heart_rate\": %d, "
                 "\"noise_level\": %d, "
                 "\"gps\": { "
                 "\"latitude\": %.6f, "
                 "\"longitude\": %.6f, "
                 "\"speed\": %.2f, "
                 "\"altitude\": %.2f, "
                 "\"accuracy\": %.2f, "
                 "\"satellites\": %d "
                 "} } }",
                 DEVICE_ID, data.steps, data.temperature, (int)data.humidity, (int)data.gasLevel,
                 data.fall_detected, data.device_battery, data.heartRate, data.noise_level,
                 data.latitude, data.longitude, data.gps_speed, data.gps_altitude, 
                 data.gps_accuracy, data.gps_satellites);
=======
    int len = snprintf(buffer, bufferSize,
                       "{\"device_id\": \"%s\", \"sensors\": { "
                       "\"steps\": %d, "
                       "\"temperature\": %.2f, "
                       "\"humidity\": %.2f, "
                       "\"gas\": { \"ppm\": %.2f }, "
                       "\"fall_detected\": %d, "
                       "\"device_battery\": %d, "
                       "\"strap_battery\": \"0\", "
                       "\"heart_rate\": %d, "
                       "\"noise_level\": %d } }",
                       DEVICE_ID, data.steps, data.temperature, data.humidity, data.gasLevel,
                       data.fall_detected, data.device_battery, data.heartRate, data.noise_level);
>>>>>>> origin
    if (len < 0 || len >= (int)bufferSize)
    {
        safePrintln("[Proc Task] JSON creation failed or truncated.");
        return false;
    }
    return true;
}

// we send empty data in struct so validation is needed to not overwite with null
static void updateLatestData(sensor_data_t &latest, const sensor_message_t &incoming)
{
    if (incoming.valid.accelPitch)
        latest.accelPitch = incoming.data.accelPitch;
    if (incoming.valid.accelRoll)
        latest.accelRoll = incoming.data.accelRoll;
    if (incoming.valid.accelTotal)
        latest.accelTotal = incoming.data.accelTotal;
    if (incoming.valid.accelZ)
        latest.accelZ = incoming.data.accelZ;
    if (incoming.valid.device_battery)
        latest.device_battery = incoming.data.device_battery;
    if (incoming.valid.fall_detected)
        latest.fall_detected = incoming.data.fall_detected;
    if (incoming.valid.gasLevel)
        latest.gasLevel = incoming.data.gasLevel;
    if (incoming.valid.heartRate)
        latest.heartRate = incoming.data.heartRate;
    if (incoming.valid.humidity)
        latest.humidity = incoming.data.humidity;
    if (incoming.valid.noise_level)
        latest.noise_level = incoming.data.noise_level;
    if (incoming.valid.steps)
        latest.steps = incoming.data.steps;
    if (incoming.valid.temperature)
        latest.temperature = incoming.data.temperature;
    if (incoming.valid.latitude)
        latest.latitude = incoming.data.latitude;
    if (incoming.valid.longitude)
        latest.longitude = incoming.data.longitude;
    if (incoming.valid.gps_speed)
        latest.gps_speed = incoming.data.gps_speed;
    if (incoming.valid.gps_altitude)
        latest.gps_altitude = incoming.data.gps_altitude;
    if (incoming.valid.gps_accuracy)
        latest.gps_accuracy = incoming.data.gps_accuracy;
    if (incoming.valid.gps_satellites)
        latest.gps_satellites = incoming.data.gps_satellites;
}

/**
 * @brief Processing Task function
 *
 * @details This function handles processing operations in a FreeRTOS task. It reads sensor data
 * from a queue, processes it, and creates a JSON string for network transmission. The task runs in
 * an infinite loop, waiting for data to be available in the queue. When data is received, it is
 * processed and sent to the HTTP queue for transmission.
 *
 * @param pvParameters
 */
void processingTask(void *pvParameters)
{
    sensor_message_t incoming;
    sensor_data_t latestData;
    processed_data_t processedData;
    char buffer[JSON_BUFFER_SIZE];

    memset(&latestData, 0, sizeof(latestData));
    memset(&processedData, 0, sizeof(processedData));

    while (true)
    {
        if (xQueueReceive(dataQueue, &incoming, DATA_QUEUE_RECEIVE_TIMEOUT_MS / portTICK_PERIOD_MS))
        {
            updateLatestData(latestData, incoming);

            if (createJson(latestData, buffer, sizeof(buffer)))
            {
                // null terminate buffer
                buffer[sizeof(buffer) - 1] = '\0';

                // Debug print json
                safePrintln("[Proc Task] JSON to be sent to server:");
                safePrintln(buffer);
                safePrintln("----------------------------------------");

                memset(&processedData, 0, sizeof(processedData));
                strncpy(processedData.json, buffer, sizeof(processedData.json) - 1);
                processedData.json[sizeof(processedData.json) - 1] = '\0';

                int retries = 0;
                bool sent = false;
                while (retries < HTTP_QUEUE_SEND_RETRIES && !sent)
                {
                    if (xQueueSend(httpQueue, &processedData,
                                   HTTP_QUEUE_SEND_TIMEOUT_MS / portTICK_PERIOD_MS) == pdPASS)
                    {
                        sent = true;
                    }
                    else
                    {
                        retries++;
                        if (retries >= HTTP_QUEUE_SEND_RETRIES)
                        {
                            safePrintln(
                                "[Proc Task] Failed to send JSON to HTTP queue after retries.");
                        }
                    }
                }
            }
        }
        else
        {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}
