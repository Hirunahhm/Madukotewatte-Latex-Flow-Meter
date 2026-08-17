/*
 * HC-SR04 example sketch
 *
 * https://create.arduino.cc/projecthub/Isaac100/getting-started-with-the-hc-sr04-ultrasonic-sensor-036380
 *
 * by Isaac100
 */

const int trigPin = 9;
const int echoPin = 10;

const float radius = 4.25; // cm 
const float height = 15.0; // cm

float duration, distance, liquidVolumeCM3, liquidVolumeL;

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.begin(9600);
}

void loop() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = (duration*.0343)/2;

  liquidVolumeCM3 = 3.1416 * radius * radius * (height - distance);
  liquidVolumeL = liquidVolumeCM3/1000; // 1 cm³ = 1 ml
  Serial.print("Distance: ");
  Serial.println(distance);
  Serial.print("Liquid Volume (cm³): ");
  Serial.println(liquidVolumeCM3);
  Serial.print("Liquid Volume (L): ");
  Serial.println(liquidVolumeL);
  delay(1000);
}