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
    safePrintln("[GPS] GPS hardware initialized");
}

bool GPS::enableGPS() {
    safePrintln("[GPS] Enabling GPS...");
    
    // Check if GPS is already enabled
    if (modem.isEnableGPS()) {
        safePrintln("[GPS] GPS is already enabled");
        return true;
    }
    
    safePrintln("[GPS] GPS is not enabled, attempting to enable...");
#ifdef BOARD_POWERON_PIN
    pinMode(BOARD_POWERON_PIN, OUTPUT);
    digitalWrite(BOARD_POWERON_PIN, HIGH);
    safePrintln("[Network] Modem power enabled");
#endif

    pinMode(BOARD_PWRKEY_PIN, OUTPUT);
    digitalWrite(BOARD_PWRKEY_PIN, LOW);
    vTaskDelay(pdMS_TO_TICKS(100));
    digitalWrite(BOARD_PWRKEY_PIN, HIGH);
    vTaskDelay(pdMS_TO_TICKS(1000));
    digitalWrite(BOARD_PWRKEY_PIN, LOW);
    safePrintln("[Network] Modem power key sequence completed");
    
    // Try to enable GPS
    if (modem.enableGPS(MODEM_GPS_ENABLE_GPIO, MODEM_GPS_ENABLE_LEVEL)) {
        safePrintln("[GPS] GPS enabled successfully");
        
        // Wait a bit for GPS to initialize
        vTaskDelay(pdMS_TO_TICKS(2000));
        
        // Verify GPS is now enabled
        if (modem.isEnableGPS()) {
            safePrintln("[GPS] GPS enable confirmed");
            return true;
        } else {
            safePrintln("[GPS] GPS enable failed - not responding");
            return false;
        }
    } else {
        safePrintln("[GPS] Failed to enable GPS");
        return false;
    }
}

bool GPS::disableGPS() {
    safePrintln("[GPS] Disabling GPS...");
    
    if (modem.disableGPS()) {
        safePrintln("[GPS] GPS disabled successfully");
        return true;
    } else {
        safePrintln("[GPS] Failed to disable GPS");
        return false;
    }
}

bool GPS::isGPSEnabled() {
    return modem.isEnableGPS();
}

bool GPS::getGPSLocation(gps_location_t& location) {
    // Check if GPS is enabled first
    if (!isGPSEnabled()) {
        safePrintln("[GPS] GPS not enabled");
        return false;
    }
    
    float lat, lon, speed, alt, accuracy;
    int vsat, usat;
    int year, month, day, hour, minute, second;
    uint8_t status;
    
    // Get GPS data from modem
    if (modem.getGPS(&status, &lat, &lon, &speed, &alt, &vsat, &usat, &accuracy,
                     &year, &month, &day, &hour, &minute, &second)) {
        
        // Fill location structure
        location.latitude = lat;
        location.longitude = lon;
        location.speed = speed;
        location.altitude = alt;
        location.accuracy = accuracy;
        location.satellites = vsat;
        
        // Validate GPS fix - check both coordinates and status
        location.valid = (lat != 0.0 && lon != 0.0 && status > 0);
        location.timestamp = millis();
        
        // Cache the location
        lastGPSLocation = location;
        lastGPSUpdate = millis();
        
        // Debug output
        if (location.valid) {
            safePrintf("[GPS] Valid fix: %.6f, %.6f (status: %d, sats: %d)\n", 
                      lat, lon, status, vsat);
        } else {
            safePrintf("[GPS] Invalid fix: %.6f, %.6f (status: %d, sats: %d)\n", 
                      lat, lon, status, vsat);
        }
        
        return location.valid;
    } else {
        safePrintln("[GPS] Failed to get GPS data from modem");
        return false;
    }
}

bool GPS::getLastLocation(gps_location_t& location) {
    if (lastGPSUpdate > 0 && lastGPSLocation.valid) {
        location = lastGPSLocation;
        return true;
    }
    return false;
}

bool GPS::waitForGPSFix(unsigned long timeoutMs) {
    safePrintln("[GPS] Waiting for GPS fix...");
    unsigned long startTime = millis();
    gps_location_t location;
    
    while ((millis() - startTime) < timeoutMs) {
        if (getGPSLocation(location) && location.valid) {
            safePrintf("[GPS] GPS fix obtained in %lu ms\n", millis() - startTime);
            return true;
        }
        
        // Print progress every 10 seconds
        unsigned long elapsed = millis() - startTime;
        if (elapsed % 10000 == 0 && elapsed > 0) {
            safePrintf("[GPS] Still waiting for fix... %lu/%lu ms\n", elapsed, timeoutMs);
        }
        
        vTaskDelay(pdMS_TO_TICKS(2000)); // Check every 2 seconds
    }
    
    safePrintln("[GPS] Timeout waiting for GPS fix");
    return false;
}

// Helper function to print GPS status
void GPS::printStatus() {
    safePrintln("=== GPS Status ===");
    safePrintf("GPS Enabled: %s\n", isGPSEnabled() ? "Yes" : "No");
    
    if (lastGPSUpdate > 0) {
        safePrintf("Last update: %lu ms ago\n", millis() - lastGPSUpdate);
        safePrintf("Last position: %.6f, %.6f\n", 
                  lastGPSLocation.latitude, lastGPSLocation.longitude);
        safePrintf("Valid: %s, Satellites: %d\n", 
                  lastGPSLocation.valid ? "Yes" : "No", lastGPSLocation.satellites);
    } else {
        safePrintln("No GPS data available");
    }
    safePrintln("==================");
}