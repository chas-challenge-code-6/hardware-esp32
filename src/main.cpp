#include <Arduino.h>
#include <DHT.h>

#define RED_PIN 16
#define GREEN_PIN 15
#define BLUE_PIN 7

#define DHTPIN 20
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

void setup()
{
    Serial.begin(115200);

    pinMode(RED_PIN, OUTPUT);
    pinMode(GREEN_PIN, OUTPUT);
    pinMode(BLUE_PIN, OUTPUT);

    dht.begin();

    analogWrite(RED_PIN, 255);
    analogWrite(GREEN_PIN, 0);
    analogWrite(BLUE_PIN, 0);
}

void loop()
{
    float temperature = dht.readTemperature();

    if (isnan(temperature))
    {
        Serial.println("Failed to read from DHT sensor");
        return;
    }

    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println("C");

    delay(1000);
}
