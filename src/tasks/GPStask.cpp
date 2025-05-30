#include "tasks/GPStask.h"
#include "config.h"
#include "network/network.h"
#include "utils/threadsafe_serial.h"
#include "SensorData.h"
#include <Arduino.h>

extern QueueHandle_t dataQueue;
extern QueueHandle_t httpQueue;
extern SemaphoreHandle_t modemMutex;


gps_location_t gpsLocation;
GPS gps;

void gpsTask(void *pvParameters)
{    
    safePrintln("[GPS Task] GPS task started");
    
    // Initial GPS setup with mutex protection
    if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(10000)) == pdTRUE)
    {
        gps.begin();
        
        if (!gps.enableGPS())
        {
            safePrintln("[GPS Task] Failed to enable GPS");
            xSemaphoreGive(modemMutex);
            vTaskDelete(NULL);
            return;
        }
        
        safePrintln("[GPS Task] GPS enabled, waiting for fix...");
        xSemaphoreGive(modemMutex);
    }
    else
    {
        safePrintln("[GPS Task] Failed to take modem mutex for GPS initialization");
        vTaskDelete(NULL);
        return;
    }
    while (true)
    {
        // Take mutex before any modem/GPS operations
        if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(5000)) == pdTRUE)
        {
            if (gps.getGPSLocation(gpsLocation))
            {
                if (gpsLocation.valid)
                {
                    safePrintf("[GPS Task] Location: Lat: %.6f, Lon: %.6f, Speed: %.2f m/s, Altitude: %.2f m, Satellites: %d\n",
                               gpsLocation.latitude, gpsLocation.longitude, gpsLocation.speed,
                               gpsLocation.altitude, gpsLocation.satellites);
                    
                    // Release mutex before queue operations (don't hold it too long)
                    xSemaphoreGive(modemMutex);
                    
                    // Prepare and send sensor message
                    sensor_message_t msg;
                    memset(&msg, 0, sizeof(msg));
                    
                    msg.data.latitude = gpsLocation.latitude;
                    msg.data.longitude = gpsLocation.longitude;
                    msg.data.gps_speed = gpsLocation.speed;
                    msg.data.gps_altitude = gpsLocation.altitude;
                    msg.data.gps_accuracy = gpsLocation.accuracy;
                    msg.data.gps_satellites = gpsLocation.satellites;
                    
                    // Mark GPS data as valid
                    msg.valid.latitude = 1;
                    msg.valid.longitude = 1;
                    msg.valid.gps_speed = 1;
                    msg.valid.gps_altitude = 1;
                    msg.valid.gps_accuracy = 1;
                    msg.valid.gps_satellites = 1;
                    
                    // Send to processing task
                    if (xQueueSend(dataQueue, &msg, pdMS_TO_TICKS(1000)) != pdPASS) 
                    {
                        safePrintln("[GPS Task] Failed to send GPS data to queue");
                    }
                    else
                    {
                        safePrintln("[GPS Task] GPS data sent to queue successfully");
                    }
                }
                else
                {
                    safePrintln("[GPS Task] Invalid GPS data received");
                    xSemaphoreGive(modemMutex);
                }
            }
            else
            {
                safePrintln("[GPS Task] Failed to get GPS location");
                xSemaphoreGive(modemMutex);
            }
        }
        else
        {
            safePrintln("[GPS Task] Failed to take modem mutex - skipping GPS read");
        }

        // Wait before next GPS reading (30 seconds is better for GPS and battery)
        vTaskDelay(pdMS_TO_TICKS(30000));
    }
}