#include <ESP8266WiFi.h>

extern "C" {
  #include "user_interface.h"
}

volatile uint32_t len12Count = 0;
volatile uint32_t len128Count = 0;
volatile uint32_t otherLenCount = 0;

const uint8_t CHANNEL = 1;

void packetSnifferCallback(uint8_t *buf, uint16_t len) {
  if (len == 12) {
    len12Count++;
  }
  else if (len == 128) {
    len128Count++;
  }
  else {
    otherLenCount++;
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
  Serial.print("ch");
  Serial.print(CHANNEL);
  Serial.print(" | ");

  uint32_t count12 = len12Count;
  uint32_t count128 = len128Count;
  uint32_t countOther = otherLenCount;
  len12Count = 0;
  len128Count = 0;
  otherLenCount = 0;

  Serial.print("12: ");
  Serial.print(count12);
  Serial.print(" | ");

  Serial.print("128: ");
  Serial.print(count128);
  Serial.print(" | ");

  Serial.print("other: ");
  Serial.println(countOther);

  delay(1000);
}
