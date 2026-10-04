#include <ESP8266WiFi.h>

extern "C" {
  #include "user_interface.h"
}

volatile uint32_t frameCount = 0;

void packetSnifferCallback(uint8_t *buf, uint16_t len) {
  frameCount++;
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  wifi_set_opmode(STATION_MODE);
  wifi_promiscuous_enable(0);
  wifi_set_channel(1);
  wifi_set_promiscuous_rx_cb(packetSnifferCallback);
  wifi_promiscuous_enable(1);

  delay(100);
}

void loop() {
  uint32_t count = frameCount;
  frameCount = 0;
  Serial.print("Sniffed ");
  Serial.print(count);
  Serial.println(" frames");
  // Serial.println("Scanning network ....");
  // int n = WiFi.scanNetworks();
  // Serial.print(n);
  // Serial.println(" networks found");
  //
  // for (int i = 0; i < n; i++)
  // {
  //   Serial.print("SSID: ");
  //   Serial.println(WiFi.SSID(i));
  //   Serial.print("BSSID: ");
  //   Serial.println(WiFi.BSSIDstr(i));
  //   Serial.print("Channel: ");
  //   Serial.println(WiFi.channel(i));
  //   Serial.print("RSSI: ");
  //   Serial.println(WiFi.RSSI(i));
  //
  //   Serial.println("-------------------------------------");
  //   delay(500);
  // }
  // Serial.println();
  delay(1000);
}
