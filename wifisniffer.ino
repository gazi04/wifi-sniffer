#include <ESP8266WiFi.h>

extern "C" {
  #include "user_interface.h"
}

volatile uint32_t mgmtCount = 0;
volatile uint32_t ctrlCount = 0;
volatile uint32_t dataCount = 0;
volatile uint32_t unknownCount = 0;
volatile uint32_t mgmtSubCount[16] = {0};

const uint8_t CHANNEL = 6;
const uint8_t SUBTYPE_BEACON = 8;
const uint8_t SUBTYPE_PROBE_REQUEST = 4;
const uint8_t SUBTYPE_PROBE_RESPONSE = 5;

void packetSnifferCallback(uint8_t *buf, uint16_t len) {
  if (len == 12) {
    return;
  }

  uint8_t frameControlByte0 = buf[12];
  uint8_t type = (frameControlByte0 >> 2) & 0x03;
  uint8_t subtype = (frameControlByte0 >> 4) & 0x0F;

  if (type == 0) {
    mgmtCount++;
    mgmtSubCount[subtype]++;
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
  uint32_t beaconCount = mgmtSubCount[SUBTYPE_BEACON];
  uint32_t probeRequestCount = mgmtSubCount[SUBTYPE_PROBE_REQUEST];
  uint32_t probeResponseCount = mgmtSubCount[SUBTYPE_PROBE_RESPONSE];
  uint32_t other = 0;

  uint8_t mgmtSubLength = sizeof(mgmtSubCount) / sizeof(mgmtSubCount[0]);

  for (int i = 0; i < mgmtSubLength; i++) {
    uint32_t slotCount = mgmtSubCount[i];
    mgmtSubCount[i] = 0;
    if (i == SUBTYPE_BEACON || i == SUBTYPE_PROBE_REQUEST || i == SUBTYPE_PROBE_RESPONSE) {
      continue;
    }
    other += slotCount;
  }

  mgmtCount = 0;
  ctrlCount = 0;
  dataCount = 0;
  unknownCount = 0;

  Serial.print("Management: ");
  Serial.print(mgmt);
  Serial.print(" (");

  Serial.print("bcn ");
  Serial.print(beaconCount);
  Serial.print(", ");

  Serial.print("preq ");
  Serial.print(probeRequestCount);
  Serial.print(", ");

  Serial.print("presp ");
  Serial.print(probeResponseCount);
  Serial.print(", ");

  Serial.print("other ");
  Serial.print(other);

  Serial.print(")");
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
