#include <Arduino.h>
#include <HardwareSerial.h>

#define GPS_RX2 16
#define GPS_TX2 17
#define GPS_BAUD 9600

HardwareSerial GPSSerial(2);

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n--- Raw GPS NMEA passthrough ---");
  GPSSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX2, GPS_TX2);
}

void loop() {
  while (GPSSerial.available()) {
    Serial.write(GPSSerial.read());
  }
}