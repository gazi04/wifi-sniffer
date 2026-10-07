# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

ESP8266 Wi-Fi packet sniffer. The ESP8266 runs in promiscuous mode, parses 802.11 headers in C++ (Arduino framework), and streams results over USB serial to a PC, which handles logging and visualization. Full design: `docs/wifi_packet_sniffer_project.md`.

**This is a learning project.** Read `docs/ROADMAP.md` first. It has the staged plan, the current stage, and the concepts already covered. Act as a mentor: explain new concepts from zero, give hints and reviews rather than finished code, and update `ROADMAP.md` when a stage or lesson finishes.

## Build / flash / monitor

The toolchain is `arduino-cli` with core `esp8266:esp8266` 3.1.2 installed. Target board is a NodeMCU or Wemos D1 Mini; use FQBN `esp8266:esp8266:nodemcuv2` or `esp8266:esp8266:d1_mini` to match the hardware.

```sh
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 .
arduino-cli upload  --fqbn esp8266:esp8266:nodemcuv2 -p /dev/ttyUSB0 .
arduino-cli monitor -p /dev/ttyUSB0 -c baudrate=115200   # match Serial.begin() baud
```

There are no tests or linter.

## ESP8266 constraints the code must respect

- **SDK API:** add `extern "C" { #include "user_interface.h" }`. Setup order: `wifi_set_opmode(STATION_MODE)`, then `wifi_promiscuous_enable(0)`, then `wifi_set_promiscuous_rx_cb(cb)`, then `wifi_promiscuous_enable(1)`. Change channels with `wifi_set_channel()`. The radio hears only one channel at a time, so hopping needs a timer.
- **Callback buffers are truncated, and the layout depends on `len`:**
  - `len == 128`: management frame, 112 bytes of payload.
  - `len % 10 == 0`: data frame, first 36 bytes of the header only.
  - `len == 12`: `RxControl` only, no 802.11 header.
  - Always branch on `len` before parsing.
- **Keep the callback minimal.** Don't call `Serial.print` in it. Copy a small struct into a ring buffer and drain it in `loop()`. Serial bandwidth is the bottleneck, so prefer a compact or binary output format at a high baud rate, and count dropped frames.
- **No unaligned multi-byte loads.** Casting `buf+off` to `uint16_t*` or `uint32_t*` crashes with Exception 9. Read fields with `memcpy` or byte by byte. Frame Control is little-endian:
  - byte0: version bits 0–1, type bits 2–3, subtype bits 4–7.
  - byte1: ToDS bit 0, FromDS bit 1.
- **Address meaning depends on ToDS/FromDS** (00: DA,SA,BSSID; 10: BSSID,SA,DA; 01: DA,BSSID,SA; 11: RA,TA,DA,SA). ACK and CTS frames carry only Addr1. The ESP8266 rarely delivers control frames at all.
- 2.4 GHz only (802.11b/g/n).
