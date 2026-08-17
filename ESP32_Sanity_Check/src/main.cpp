#include <Arduino.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

constexpr int LED_PIN = LED_BUILTIN;
constexpr int TRIG_PIN = 5;
constexpr int ECHO_PIN = 18;

constexpr unsigned long PULSE_TIMEOUT_US = 30000UL;

float readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration = pulseIn(ECHO_PIN, HIGH, PULSE_TIMEOUT_US);
  if (duration == 0) {
    return -1.0f;
  }

  return duration * 0.0343f / 2.0f;
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  digitalWrite(TRIG_PIN, LOW);

  Serial.println("HC-SR04 board check started");
  Serial.println("Wiring note: ESP32 echo pin must be level-shifted to 3.3V");
}

void loop() {
  float distanceCm = readDistanceCm();

  if (distanceCm < 0.0f) {
    Serial.println("No echo detected");
    digitalWrite(LED_PIN, HIGH);
  } else {
    Serial.print("Distance: ");
    Serial.print(distanceCm, 1);
    Serial.println(" cm");
    digitalWrite(LED_PIN, LOW);
    delay(100);
    digitalWrite(LED_PIN, HIGH);
  }

  delay(500);
}

