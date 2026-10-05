#include <ESP8266WiFi.h>

extern "C" {
  #include "user_interface.h"
}

volatile uint32_t mgmtCount = 0;
volatile uint32_t ctrlCount = 0;
volatile uint32_t dataCount = 0;
volatile uint32_t unknownCount = 0;

const uint8_t CHANNEL = 11;

void packetSnifferCallback(uint8_t *buf, uint16_t len) {
  if (len == 12) {
    return;
  }

  uint8_t frameControlByte0 = buf[12];
  uint8_t type = (frameControlByte0 >> 2) & 0x03;
  uint8_t subtype = (frameControlByte0 >> 4) & 0x0F;

  if (type == 0) {
    mgmtCount++;
  }
  else if (type == 1) {
    ctrlCount++;
  }
  else if (type == 2) {
    dataCount++;
  }
  else {
    unknownCount++;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  wifi_promiscuous_enable(0);
  wifi_set_channel(CHANNEL);
  wifi_set_promiscuous_rx_cb(packetSnifferCallback);
  wifi_promiscuous_enable(1);

  delay(100);
}

void loop() {
  Serial.print("ch ");
  Serial.print(CHANNEL);
  Serial.print(" | ");

  uint32_t mgmt = mgmtCount;
  uint32_t ctrl = ctrlCount;
  uint32_t data = dataCount;
  uint32_t unknown = unknownCount;
  mgmtCount = 0;
  ctrlCount = 0;
  dataCount = 0;
  unknownCount = 0;

  Serial.print("Management: ");
  Serial.print(mgmt);
  Serial.print(" | ");

  Serial.print("Control: ");
  Serial.print(ctrl);
  Serial.print(" | ");

  Serial.print("Data: ");
  Serial.print(data);
  Serial.print(" | ");

  Serial.print("Unknown: ");
  Serial.print(unknown);
  Serial.println();

  delay(1000);
}
