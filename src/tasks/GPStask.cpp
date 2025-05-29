/**
 * @file gpsTask.cpp
 * @brief GPS Task Implementation File
 *
 * @details This file contains the implementation of the gpsTask function for GPS location tracking.
 */

#include "tasks/gpsTask.h"
#include "SensorData.h"
#include "config.h"
#include "network/network.h"
#include "utils/threadsafe_serial.h"
#include <Arduino.h>
#include <TinyGsmClient.h>
#include <TinyGSM.h>
#include <cstring>

#define GPS_UPDATE_INTERVAL_MS 30000  // Update every 30 seconds
#define GPS_TIMEOUT_MS 120000         // 2 minute timeout for GPS fix

extern QueueHandle_t dataQueue;
extern EventGroupHandle_t networkEventGroup;
extern SemaphoreHandle_t modemMutex;
extern TinyGsm modem;

void sendGPSData(const sensor_message_t &msg)
{
    if (xQueueSend(dataQueue, &msg, pdMS_TO_TICKS(1000)) != pdPASS)
    {
        safePrintln("[GPS Task] Failed to send GPS data to queue");
    }
}

void gpsTask(void *parameter)
{
    sensor_message_t msg;
    memset(&msg, 0, sizeof(msg));

    bool gpsEnabled = false;
    float lastLat = 0.0, lastLon = 0.0;
    
    safePrintln("[GPS Task] GPS monitoring started");

    vTaskDelay(pdMS_TO_TICKS(5000));

    while (true)
    {
        if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(5000)) == pdTRUE)
        {
            if (!gpsEnabled)
            {
                safePrintln("[GPS Task] Enabling GPS...");
                if (modem.enableGPS())
                {
                    gpsEnabled = true;
                    safePrintln("[GPS Task] GPS enabled successfully");
                    vTaskDelay(pdMS_TO_TICKS(15000));
                }
                else
                {
                    safePrintln("[GPS Task] Failed to enable GPS");
                    xSemaphoreGive(modemMutex);
                    vTaskDelay(pdMS_TO_TICKS(30000));
                    continue;
                }
            }

            float lat = 0, lon = 0, speed = 0, alt = 0, accuracy = 0;
            int vsat = 0, usat = 0;
            uint8_t status = 0;

            safePrintln("[GPS Task] Requesting GPS location...");
            
            if (modem.getGPS(&status, &lat, &lon, &speed, &alt, &vsat, &usat, &accuracy
                            ))
            {
                if (lat != 0.0 && lon != 0.0 && (lat != lastLat || lon != lastLon))
                {
                    safePrintf("[GPS Task] GPS Fix - Lat: %.6f, Lon: %.6f\n", lat, lon);
                    safePrintf("[GPS Task] Speed: %.2f km/h, Altitude: %.2f m\n", speed, alt);
                    safePrintf("[GPS Task] Satellites: %d visible, %d used\n", vsat, usat);
                    safePrintf("[GPS Task] Accuracy: %.2f m\n", accuracy);
                    

                    memset(&msg, 0, sizeof(msg));
                    
                    msg.data.latitude = lat;
                    msg.data.longitude = lon;
                    msg.data.gps_speed = speed;
                    msg.data.gps_altitude = alt;
                    msg.data.gps_accuracy = accuracy;
                    msg.data.gps_satellites = vsat;
                    
                    msg.valid.latitude = 1;
                    msg.valid.longitude = 1;
                    msg.valid.gps_speed = 1;
                    msg.valid.gps_altitude = 1;
                    msg.valid.gps_accuracy = 1;
                    msg.valid.gps_satellites = 1;

                    sendGPSData(msg);
                    
                    lastLat = lat;
                    lastLon = lon;
                }
                else
                {
                    safePrintln("[GPS Task] No GPS fix or same location");
                }
            }
            else
            {
                safePrintln("[GPS Task] Failed to get GPS location");
            }

            xSemaphoreGive(modemMutex);
        }
        else
        {
            safePrintln("[GPS Task] Failed to take modem mutex");
        }

        vTaskDelay(pdMS_TO_TICKS(GPS_UPDATE_INTERVAL_MS));
    }
}