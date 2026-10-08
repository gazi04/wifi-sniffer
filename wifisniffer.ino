#include <ESP8266WiFi.h>

extern "C" {
  #include "user_interface.h"
}

volatile uint32_t mgmtCount = 0;
volatile uint32_t ctrlCount = 0;
volatile uint32_t dataCount = 0;
volatile uint32_t unknownCount = 0;

volatile uint32_t mgmtSubCount[16] = {0};
volatile uint32_t controlSubCount[16] = {0};

uint8_t lastBeaconBssid[6];

const uint8_t CHANNEL = 11;

const uint8_t SUBTYPE_MGMT_BEACON = 8;
const uint8_t SUBTYPE_MGMT_PROBE_REQUEST = 4;
const uint8_t SUBTYPE_MGMT_PROBE_RESPONSE = 5;

const uint8_t SUBTYPE_CTRL_BLOCK_ACK_REQUEST = 8;
const uint8_t SUBTYPE_CTRL_BLOCK_ACK = 9;
const uint8_t SUBTYPE_CTRL_PS_POLL = 10; 
const uint8_t SUBTYPE_CTRL_RTS = 11;
const uint8_t SUBTYPE_CTRL_CTS = 12;
const uint8_t SUBTYPE_CTRL_ACK = 13;

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

    if (subtype == SUBTYPE_MGMT_BEACON) {
      memcpy(lastBeaconBssid, &buf[28], sizeof(lastBeaconBssid));
    }
  }
  else if (type == 1) {
    ctrlCount++;
    controlSubCount[subtype]++;
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
  Serial.print("Channel: ");
  Serial.println(CHANNEL);

  uint32_t mgmt = mgmtCount;
  uint32_t ctrl = ctrlCount;
  uint32_t data = dataCount;
  uint32_t unknown = unknownCount;

  uint32_t beaconCount = mgmtSubCount[SUBTYPE_MGMT_BEACON];
  uint32_t probeRequestCount = mgmtSubCount[SUBTYPE_MGMT_PROBE_REQUEST];
  uint32_t probeResponseCount = mgmtSubCount[SUBTYPE_MGMT_PROBE_RESPONSE];
  uint32_t otherMgmtCount = 0;
  uint8_t mgmtSubLength = sizeof(mgmtSubCount) / sizeof(mgmtSubCount[0]);

  for (int i = 0; i < mgmtSubLength; i++) {
    uint32_t slotCount = mgmtSubCount[i];
    mgmtSubCount[i] = 0;
    if (i == SUBTYPE_MGMT_BEACON || i == SUBTYPE_MGMT_PROBE_REQUEST || i == SUBTYPE_MGMT_PROBE_RESPONSE) {
      continue;
    }
    otherMgmtCount += slotCount;
  }

  uint32_t blockAckRequestCount = controlSubCount[SUBTYPE_CTRL_BLOCK_ACK_REQUEST];
  uint32_t blockAckCount = controlSubCount[SUBTYPE_CTRL_BLOCK_ACK];
  uint32_t psPollCount = controlSubCount[SUBTYPE_CTRL_PS_POLL];
  uint32_t rtsCount = controlSubCount[SUBTYPE_CTRL_RTS];
  uint32_t ctsCount = controlSubCount[SUBTYPE_CTRL_CTS];
  uint32_t ackCount = controlSubCount[SUBTYPE_CTRL_ACK];
  uint32_t otherControlCount = 0;
  uint8_t controlSubLength = sizeof(controlSubCount) / sizeof(controlSubCount[0]);

  for (int i = 0; i < controlSubLength; i++) {
    uint32_t slotCount = controlSubCount[i];
    controlSubCount[i] = 0;
    if (i == SUBTYPE_CTRL_BLOCK_ACK_REQUEST || i == SUBTYPE_CTRL_BLOCK_ACK || i == SUBTYPE_CTRL_PS_POLL || i == SUBTYPE_CTRL_RTS || i == SUBTYPE_CTRL_CTS || i == SUBTYPE_CTRL_ACK) {
      continue;
    }
    otherControlCount += slotCount;
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
  Serial.print(otherMgmtCount);
  Serial.print(")");
  Serial.println();

  Serial.print("Control: ");
  Serial.print(ctrl);
  Serial.print(" (");

  Serial.print("bar ");
  Serial.print(blockAckRequestCount);
  Serial.print(", ");

  Serial.print("ba ");
  Serial.print(blockAckCount);
  Serial.print(", ");

  Serial.print("ps-poll ");
  Serial.print(psPollCount);
  Serial.print(", ");

  Serial.print("rts ");
  Serial.print(rtsCount);
  Serial.print(", ");

  Serial.print("cts ");
  Serial.print(ctsCount);
  Serial.print(", ");

  Serial.print("ack ");
  Serial.print(ackCount);
  Serial.print(", ");

  Serial.print("other ");
  Serial.print(otherControlCount);
  Serial.print(")");
  Serial.println();

  Serial.print("Data: ");
  Serial.print(data);
  Serial.println();

  Serial.print("Unknown: ");
  Serial.print(unknown);
  Serial.println();

  Serial.print("Last beacon from: ");
  uint8_t lastBeaconBssidCopied[6];
  memcpy(lastBeaconBssidCopied, lastBeaconBssid, sizeof(lastBeaconBssid));

  uint8_t beaconLength = sizeof(lastBeaconBssidCopied) / sizeof(lastBeaconBssidCopied[0]);

  for (int i = 0; i < beaconLength; i++) {
    Serial.printf("%02x", lastBeaconBssidCopied[i]);
    if (i != beaconLength - 1) {
      Serial.print(":");
    }
    lastBeaconBssid[i] = 0;
  }

  Serial.println();
  Serial.println("----------------------------------");

  delay(1000);
}
