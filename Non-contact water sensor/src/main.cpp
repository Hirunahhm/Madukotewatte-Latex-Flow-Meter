#include <Arduino.h>

// We are reading the scaled-down sensor signal on GPIO 4
#define SENSOR_PIN 4

void setup() {
  // Initialize Serial Monitor at 115200 baud rate
  Serial.begin(115200);
  
  // Set the sensor pin as an input
  pinMode(SENSOR_PIN, INPUT);
  
  Serial.println("--- XKC-Y25-V Sensor Test Initialized ---");
}

void loop() {
  // Read the digital state of the sensor (HIGH or LOW)
  int sensorState = digitalRead(SENSOR_PIN);
  
  if (sensorState == HIGH) {
    Serial.println("💧 Liquid Detected! (Sensor Outputting HIGH)");
  } else {
    Serial.println("❌ No Liquid Detected. (Sensor Outputting LOW)");
  }
  
  // Check the status every 300 milliseconds
  delay(300);
}