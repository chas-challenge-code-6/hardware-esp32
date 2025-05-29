#include "network/gps.h"
#include "network/network.h"
#include "config.h"
#include "utilities.h"
#include "utils/threadsafe_serial.h"
#include <Arduino.h>
#include <TinyGSM.h>
#include <WiFi.h>

extern TinyGsm modem;

GPS::GPS() : lastGPSUpdate(0) {
    // Constructor initializes GPS location to default values
    lastGPSLocation = {0, 0, 0, 0, 0, 0, false, 0};
}

void GPS::begin() {
    safePrintln("[Network] GPS hardware initialized");
}

bool GPS::enableGPS() {
    safePrintln("[Network] Enabling GPS...");
    if (modem.isEnableGPS()) {
        safePrintln("[Network] GPS is already enabled");
        return true;
    }
    else {
        safePrintln("[Network] GPS is not enabled, attempting to enable...");
        modem.enableGPS(MODEM_GPS_ENABLE_GPIO, MODEM_GPS_ENABLE_LEVEL);
    }
    return true;
}

bool GPS::disableGPS() {
    safePrintln("[Network] Disabling GPS...");
    
    if (!modem.disableGPS()) {
        safePrintln("[Network] Failed to disable GPS");
        return false;
    }
    
    safePrintln("[Network] GPS disabled successfully");
    return true;
}

bool GPS::isGPSEnabled() {
    return modem.isEnableGPS();  // This should work now
}


bool GPS::getGPSLocation(gps_location_t& location) {
    float lat, lon, speed, alt, accuracy;
    int vsat, usat;
    int year, month, day, hour, minute, second;
    uint8_t status;
    
    if (modem.getGPS(&status, &lat, &lon, &speed, &alt, &vsat, &usat, &accuracy,
                     &year, &month, &day, &hour, &minute, &second)) {
        
        location.latitude = lat;
        location.longitude = lon;
        location.speed = speed;
        location.altitude = alt;
        location.accuracy = accuracy;
        location.satellites = vsat;
        location.valid = (lat != 0.0 && lon != 0.0);
        location.timestamp = millis();
        
        lastGPSLocation = location;
        lastGPSUpdate = millis();
        
        return location.valid;
    }
    
    return false;
}

bool GPS::waitForGPSFix(unsigned long timeoutMs) {
    safePrintln("[Network] Waiting for GPS fix...");
    unsigned long startTime = millis();
    gps_location_t location;
    
    while ((millis() - startTime) < timeoutMs) {
        if (getGPSLocation(location) && location.valid) {
            safePrintf("[Network] GPS fix obtained in %lu ms\n", millis() - startTime);
            return true;
        }
        
        vTaskDelay(pdMS_TO_TICKS(2000)); // Check every 2 seconds
    }
    
    safePrintln("[Network] Timeout waiting for GPS fix");
    return false;
}