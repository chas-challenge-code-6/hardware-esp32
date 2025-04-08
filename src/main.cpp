#include <Arduino.h>

void setup() {
  // Initialize serial communication at 115200 baud
  Serial.begin(115200);
  // Add your setup code here
}

void loop() {
  // Add your main code here, to run repeatedly
  Serial.println("Hello, Alles!");
  delay(1000); // Wait for 1 second
}