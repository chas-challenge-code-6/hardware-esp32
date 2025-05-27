/**
 * @file threadsafe_serial.h
 * 
 * @brief Thread-safe Serial Communication
 * 
 */

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


/**
 * @brief Thread-safe print function
 * 
 * @tparam T 
 * @param value
 * 
 * @details This function safely prints a value to the serial port using a mutex to ensure thread safety.
 * It takes a template parameter T, allowing it to handle various data types.
 *  
 */
template<typename T>
void safePrint(const T& value) {
    if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        Serial.print(value);
        xSemaphoreGive(serialMutex);
    }
}

/**
 * @brief Thread-safe println function
 * 
 * @tparam T 
 * @param value
 * 
 * @details This function safely prints a value followed by a newline to the serial port using a mutex to ensure thread safety.
 * It takes a template parameter T, allowing it to handle various data types.
 */
template<typename T>
void safePrintln(const T& value) {
    if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        Serial.println(value);
        xSemaphoreGive(serialMutex);
    }
}

#endif