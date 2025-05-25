#include "utils/threadsafe_serial.h"
#include <stdarg.h>

void safePrint(const char* message) {
    if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        Serial.print(message);
        xSemaphoreGive(serialMutex);
    }
}

void safePrint(const String& message) {
    if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        Serial.print(message);
        xSemaphoreGive(serialMutex);
    }
}

void safePrintln(const char* message) {
    if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        Serial.println(message);
        xSemaphoreGive(serialMutex);
    }
}

void safePrintln(const String& message) {
    if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        Serial.println(message);
        xSemaphoreGive(serialMutex);
    }
}

void safePrintf(const char* format, ...) {
    if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        char buffer[128];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        Serial.print(buffer);
        xSemaphoreGive(serialMutex);
    }
}
