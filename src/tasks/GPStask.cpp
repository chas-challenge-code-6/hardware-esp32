#include "tasks/GPStask.h"
#include "config.h"
#include "network/network.h"
#include "utils/threadsafe_serial.h"
#include <Arduino.h>

gps_location_t gpsLocation;
GPS gps;

void gpsTask(void *pvParameters)
{    
    gps.begin();
    
    if (!gps.enableGPS())
    {
        safePrintln("[GPS Task] Failed to enable GPS");
        vTaskDelete(NULL);
        return;
    }

    safePrintln("[GPS Task] GPS enabled, waiting for fix...");

    while (true)
    {
        if (gps.getGPSLocation(gpsLocation))
        {
            if (gpsLocation.valid)
            {
                safePrintf("[GPS Task] Location: Lat: %.6f, Lon: %.6f, Speed: %.2f m/s, Altitude: %.2f m, Satellites: %d\n",
                           gpsLocation.latitude, gpsLocation.longitude, gpsLocation.speed,
                           gpsLocation.altitude, gpsLocation.satellites);
            }
            else
            {
                safePrintln("[GPS Task] Invalid GPS data received");
            }
        }
        else
        {
            safePrintln("[GPS Task] Failed to get GPS location");
        }

        vTaskDelay(pdMS_TO_TICKS(5000)); // Wait for 5 seconds before next update
    }
}