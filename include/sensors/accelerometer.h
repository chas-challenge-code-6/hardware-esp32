#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include "main.h"
#include <Arduino.h>

class SensorAccelerometer {
    private:
        //Accelerometer accelerometer;
        float accelX;
        float accelY;
        float accelZ;

    public:
        //SensorAccelerometer(uint8_t pin) : pin(pin), accelX(0), accelY(0), accelZ(0) {}
    
        void begin();
        void update();
    
        float getLocation() const;
        float getSteps() const;
    };
    
    #endif
    