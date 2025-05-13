#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include "main.h"
#include <Arduino.h>
#include <Adafruit_ADXL345_U.h>

class SensorAccelerometer {
    private:
        Adafruit_ADXL345_Unified accel;
        float accelX;
        float accelY;
        float accelZ;
        int steps;

    public:
        //SensorAccelerometer(uint8_t pin) : pin(pin), accelX(0), accelY(0), accelZ(0) {}
    
        void begin();
        void update();
    
        float getLocation() const;
        float getSteps() const;
    };
    
    #endif
    