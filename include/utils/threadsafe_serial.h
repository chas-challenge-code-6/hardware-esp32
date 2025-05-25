#ifndef THREADSAFE_SERIAL_H
#define THREADSAFE_SERIAL_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

extern SemaphoreHandle_t serialMutex;

void safePrint(const char* message);
void safePrint(const String& message);
void safePrintln(const char* message);
void safePrintln(const String& message);
void safePrintf(const char* format, ...);

template<typename T>
void safePrint(const T& value) {
    if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        Serial.print(value);
        xSemaphoreGive(serialMutex);
    }
}

template<typename T>
void safePrintln(const T& value) {
    if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        Serial.println(value);
        xSemaphoreGive(serialMutex);
    }
}

#endif
