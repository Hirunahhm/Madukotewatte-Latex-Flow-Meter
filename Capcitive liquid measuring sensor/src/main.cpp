#include <Arduino.h>

#define TOUCH_PIN 4  // GPIO4, hardware touch channel T0

void setup() {
    Serial.begin(115200);
    delay(1000);  // let serial settle
    Serial.println("Capacitive Sensor Test");
    Serial.println("----------------------");
}

void loop() {
    uint16_t raw = touchRead(TOUCH_PIN);
    Serial.print("Touch Raw: ");
    Serial.println(raw);
    delay(200);
}