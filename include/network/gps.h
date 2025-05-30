#ifndef GPS_H
#define GPS_H

#include "utilities.h"
#include <Arduino.h>
#include <TinyGsmClient.h>

// GPS Location Structure
typedef struct {
    float latitude;
    float longitude;
    float speed;
    float altitude;
    float accuracy;
    int satellites;
    bool valid;
    unsigned long timestamp;
} gps_location_t;

class GPS
{
public:
    GPS();
    void begin();

    bool enableGPS();
    bool disableGPS();
    bool isGPSEnabled();
    bool getGPSLocation(gps_location_t& location);
    bool waitForGPSFix(unsigned long timeoutMs = 120000);
    bool getLastLocation(gps_location_t& location);
    void printStatus();

private:

    
    gps_location_t lastGPSLocation;
    unsigned long lastGPSUpdate;

};

#endif
