#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>

// Perfected Pin Layout for your 30-Pin Board
#define SCK 18
#define MISO 19
#define MOSI 23
#define SS 5
#define RST 14
#define DIO0 27  // Clean, verified pin mapping!

#define BAND 433E6
unsigned long lastDebugPrint = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("\n--- LoRa Final Receiver Active ---");

  // Manual Hardware Wakeup Sequence
  pinMode(RST, OUTPUT);
  digitalWrite(RST, LOW);
  delay(20); 
  digitalWrite(RST, HIGH);
  delay(50); // Let internal crystal oscillators stabilize

  // Pass configuration to the library
  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(BAND)) {
    Serial.println("❌ Starting LoRa failed! Check connections.");
    while (1);
  }
  
  // Match the transmitter's secure configurations
  LoRa.setSyncWord(0xF3);        
  LoRa.setSpreadingFactor(10);   
  
  Serial.println("🛰️ Receiver Listening on D27 (SF10)...");
}

void loop() {
  // Check if a packet has arrived
  int packetSize = LoRa.parsePacket();
  
  if (packetSize) {
    Serial.print("📦 SUCCESS! Received Packet: '");
    while (LoRa.available()) {
      Serial.print(LoRa.readString());
    }
    Serial.print("' | RSSI: ");
    Serial.print(LoRa.packetRssi());
    Serial.println(" dBm");
  }

  // Diagnostic printout every 1.5 seconds to monitor the radio environment
 // To this updated diagnostic block:
if (millis() - lastDebugPrint > 1500) {
  lastDebugPrint = millis();
  int liveNoise = LoRa.rssi(); // ✅ Correct function for actual live background noise
  Serial.print("[Diag] Module Status: ACTIVE | Live Noise Floor: ");
  Serial.print(liveNoise);
  Serial.println(" dBm");
}
}