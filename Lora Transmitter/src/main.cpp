#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <MFRC522.h>
#include <TinyGPS++.h>

// ---------- Shared VSPI bus (LoRa + RFID) ----------
#define SCK   18
#define MISO  19
#define MOSI  23

// ---------- LoRa SX1278 ----------
#define LORA_SS   5
#define LORA_RST  14
#define LORA_DIO0 26
#define BAND      433E6

#define LORA_SF       10
#define LORA_BW       125E3
#define LORA_CR       5
#define LORA_SYNCWORD 0xF3
#define LORA_PREAMBLE 8
#define REG_OCP       0x0B

// ---------- RC522 RFID ----------
#define RFID_SS   27
#define RFID_RST  4

// ---------- GPS (UART2) ----------
#define GPS_RX2  16   // ESP32 RX2 <- GPS TX
#define GPS_TX2  17   // ESP32 TX2 -> GPS RX
#define GPS_BAUD 9600 // check your module — most NEO-M8N ship at 9600

// ---------- OLED (SH1106, I2C) ----------
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 22, 21);

MFRC522 rfid(RFID_SS, RFID_RST);
HardwareSerial GPSSerial(2);
TinyGPSPlus gps;

int packetCounter = 0;
String lastTagUID = "NONE";
unsigned long lastTx = 0;
const unsigned long TX_INTERVAL = 2000;

void writeReg(uint8_t addr, uint8_t val) {
  SPI.beginTransaction(SPISettings(8E6, MSBFIRST, SPI_MODE0));
  digitalWrite(LORA_SS, LOW);
  SPI.transfer(addr | 0x80);
  SPI.transfer(val);
  digitalWrite(LORA_SS, HIGH);
  SPI.endTransaction();
}

String readRfidUID() {
  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  return uid;
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("[ESTATE TX] Booting...");

  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(10, 25, "Booting...");
  u8g2.sendBuffer();

  // Deselect both SPI devices before the bus comes up
  pinMode(LORA_SS, OUTPUT);
  pinMode(RFID_SS, OUTPUT);
  digitalWrite(LORA_SS, HIGH);
  digitalWrite(RFID_SS, HIGH);

  // No SS param here — two devices share this bus, each library
  // manages its own CS pin instead of relying on one hardware SS
  SPI.begin(SCK, MISO, MOSI);

  // ---- LoRa init ----
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  pinMode(LORA_RST, OUTPUT);
  digitalWrite(LORA_RST, LOW);
  delay(20);
  digitalWrite(LORA_RST, HIGH);
  delay(50);

  if (!LoRa.begin(BAND)) {
    Serial.println("[FATAL] LoRa init failed. Check wiring.");
    while (1) delay(1000);
  }

  LoRa.setTxPower(14, PA_OUTPUT_PA_BOOST_PIN);
  writeReg(REG_OCP, 0x00);          // disable OCP directly — library's setOCP(0) underflows
  LoRa.setSignalBandwidth(LORA_BW);
  LoRa.setSpreadingFactor(LORA_SF);
  LoRa.setCodingRate4(LORA_CR);
  LoRa.setSyncWord(LORA_SYNCWORD);
  LoRa.setPreambleLength(LORA_PREAMBLE);
  LoRa.enableCrc();                 // remember: add this to the receiver too

  Serial.println("[OK] LoRa armed. 14dBm SF10 BW125 433MHz, CRC on");

  // ---- RFID init (shares the SPI bus with LoRa) ----
  rfid.PCD_Init();
  Serial.println("[OK] RFID reader ready");

  // ---- GPS init ----
  GPSSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX2, GPS_TX2);
  Serial.println("[OK] GPS serial ready");

  Serial.println("[ESTATE TX] All peripherals initialized.");
}

void loop() {
  // Keep the GPS parser fed every loop — never block on it
  while (GPSSerial.available()) {
    gps.encode(GPSSerial.read());
  }

  // Non-blocking tag check — only does anything when a card is present
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    lastTagUID = readRfidUID();
    Serial.printf("[RFID] Tag detected: %s\n", lastTagUID.c_str());
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }

  // Transmit + refresh OLED on a fixed interval
  if (millis() - lastTx >= TX_INTERVAL) {
    lastTx = millis();

    String gpsStr = gps.location.isValid()
      ? String(gps.location.lat(), 5) + "," + String(gps.location.lng(), 5)
      : "NOFIX";

    String payload = "PKT:" + String(packetCounter) +
                      ",GPS:" + gpsStr +
                      ",TAG:" + lastTagUID;

    LoRa.beginPacket();
    LoRa.print(payload);
    LoRa.endPacket();

    Serial.printf("[TX %d] \"%s\" sent.\n", packetCounter, payload.c_str());

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_7x14_tf);
    u8g2.drawStr(0, 15, "ESTATE TX NODE");
    u8g2.drawHLine(0, 18, 128);
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 30, gps.location.isValid() ? "GPS: fix" : "GPS: no fix");
    u8g2.setCursor(0, 42);
    u8g2.print("Tag: ");
    u8g2.print(lastTagUID);
    u8g2.setCursor(0, 54);
    u8g2.print("Sent: ");
    u8g2.print(packetCounter);
    u8g2.sendBuffer();

    packetCounter++;
  }
}