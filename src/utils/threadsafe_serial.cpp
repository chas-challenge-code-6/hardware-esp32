/**
 * @file threadsafe_serial.cpp
 * 
 * @details This file implements thread-safe serial printing functions using FreeRTOS semaphores.
 * 
 */

#include "utils/threadsafe_serial.h"
#include <stdarg.h>


/**
 * @brief Thread-safe print functions for serial output
 * 
 * @param message 
 */
void safePrint(const char *message)
{
    if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(1000)) == pdTRUE)
    {
        Serial.print(message);
        xSemaphoreGive(serialMutex);
    }
}


/**
 * @brief Thread-safe print functions for serial output
 * 
 * @param message 
 */
void safePrint(const String &message)
{
    if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(1000)) == pdTRUE)
    {
        Serial.print(message);
        xSemaphoreGive(serialMutex);
    }
}


/**
 * @brief Thread-safe println functions for serial output
 * 
 * @param message 
 */
void safePrintln(const char *message)
{
    if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(1000)) == pdTRUE)
    {
        Serial.println(message);
        xSemaphoreGive(serialMutex);
    }
}


/**
 * @brief Thread-safe println functions for serial output
 * 
 * @param message 
 */
void safePrintln(const String &message)
{
    if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(1000)) == pdTRUE)
    {
        Serial.println(message);
        xSemaphoreGive(serialMutex);
    }
}


/**
 * @brief Thread-safe printf function for serial output
 * 
 * @param format 
 * @param ... 
 */
void safePrintf(const char *format, ...)
{
    if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(1000)) == pdTRUE)
    {
        char buffer[128];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        Serial.print(buffer);
        xSemaphoreGive(serialMutex);
    }
}