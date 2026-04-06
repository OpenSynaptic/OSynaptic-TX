# 01 — Deployment Guide

This document covers hardware selection, memory sizing, transport selection, and duty-cycle considerations for deploying OSynaptic-TX on 8-bit MCUs.

---

## 1. Minimum Hardware Requirements

| Resource | Absolute Minimum | Recommended Minimum |
|----------|-----------------|-------------------|
| Flash | 2 KB (API C only) | 8 KB (all APIs) |
| RAM | 128 B (API C, 1 sensor) | 512 B (API B/C, 3 sensors) |
| UART | SW UART (USI, SoftwareSerial) | HW UART |
| Clock | Any (no timing dependency in library) | 8–16 MHz for reliable baud rates |

---

## 2. AVR Family Deployment Table

### 8-bit AVR — maximum and theoretical deployment values

| MCU | Flash | RAM | UART type | Max concurrent sensors | Max sustained rate | Recommended API |
|-----|-------|-----|-----------|----------------------|-------------------|----------------|
| ATtiny25 | 2 KB | 128 B | USI/SW only | 1 | 1 Hz | **C** |
| ATtiny45 | 4 KB | 256 B | USI/SW only | 2 | 2 Hz | **C** |
| ATtiny85 | 8 KB | 512 B | USI/SW only | 4 | 4 Hz | C / B |
| ATmega48 | 4 KB | 512 B | HW UART0 | 2 | 2 Hz | **C** |
| ATmega88 | 8 KB | 1 KB | HW UART0 | 4 | 5 Hz | C / B |
| ATmega168 | 16 KB | 1 KB | HW UART0 | 8 | 10 Hz | A / B / C |
| **ATmega328P** | **32 KB** | **2 KB** | **HW UART0** | **16** | **~20 Hz** | **A / B / C** |
| ATmega32U4 | 32 KB | 2.5 KB | HW UART + USB | 16 | ~20 Hz | A / B / C |
| ATmega2560 | 256 KB | 8 KB | 4× HW UART | 50 | ~50 Hz | A / B / C |
| ATmega4809 | 48 KB | 6 KB | 4× USART | 30 | ~40 Hz | A / B / C |

> **Note on "Max concurrent sensors"**: each OSTX_STATIC_DEFINE descriptor occupies ~27 bytes in Flash (header template + prefix string). Static RAM impact is 0 B for API C, 96 B scratchpad for API A/B (shared, not per-sensor).

> **Note on "Max sustained rate"**: limited by UART baud rate, not CPU. At 9 600 baud, a 30-byte frame takes 31 ms → max throughput ≈ 32 frames/s. At 115 200 baud: 2.6 ms per frame → ~380 frames/s. The library's CPU time per frame is ~0.6 ms at 16 MHz (negligible).

### Rate formula

$$
\text{rate}_{\max} = \frac{\text{baud}}{10 \times \text{frame\_bytes}}\ \text{frames/s}
$$

(10 bits per UART byte: 1 start + 8 data + 1 stop, no parity)

For a 30-byte frame at 9 600 baud:

$$
\text{rate}_{\max} = \frac{9600}{10 \times 30} = 32\ \text{frames/s}
$$

---

## 3. 32-bit Targets

| MCU | Flash | RAM | Recommended API | Notes |
|-----|-------|-----|----------------|-------|
| STM32F030F4 | 16 KB | 4 KB | A / B / C | Cortex-M0, all APIs fit. |
| STM32F103C8 | 64 KB | 20 KB | A / B / C | "Blue Pill". Ethernet via ENC28J60 + uIP possible (see §4.4). |
| ESP8266 | 1–4 MB | 80 KB | C (WiFi UDP) | Built-in lwIP. No external stack needed. |
| ESP32 | 4 MB | 520 KB | C (WiFi UDP) | Preferred for LAN telemetry. |
| RP2040 | 2 MB | 264 KB | A / B / C | PIO can implement custom UART framing. |

---

## 4. Transport Selection

### 4.1 UART / RS-485 (wired, recommended for AVR)

Use when:
- MCU has hardware UART and is within cable reach of the gateway
- Maximum simplicity required

Emit callback:
```c
/* Arduino */
static void emit(ostx_u8 b, void *ctx) { Serial.write(b); }

/* AVR bare metal */
static void emit(ostx_u8 b, void *ctx) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = b;
}
```

For distances > 10 m: add a MAX485 / SN75176 RS-485 driver. The library output is identical; only the physical layer changes.

### 4.2 WiFi UDP (ESP32 / ESP8266)

Use when:
- MCU is ESP32 or ESP8266
- Server is on a local LAN

```cpp
g_udp.beginPacket(REMOTE_IP, REMOTE_PORT);
ostx_stream_pack(&s_sensor, tid++, ts, value, udp_emit, NULL);
g_udp.endPacket();
```

**Why UDP over TCP for sensor telemetry**:
- No connection state → survives WiFi reconnects without code changes
- No TCP ACK round-trip → lower latency
- Single datagram per frame → atomic delivery or drop (no partial frames)
- OpenSynaptic server deduplicates via `tid` — a dropped UDP packet is recoverable

### 4.3 LoRa (SX1276/SX1278)

Use when:
- Node is outdoors > 100 m from gateway
- Low data rate is acceptable (< 1 frame per 10 s in EU)

Frame accumulation pattern:
```c
static ostx_u8  s_buf[OSTX_PACKET_MAX];
static int      s_len = 0;

static void lora_emit(ostx_u8 b, void *ctx) {
    if (s_len < OSTX_PACKET_MAX) s_buf[s_len++] = b;
}

/* In main loop: */
s_len = 0;
ostx_stream_pack(&s_sensor, tid++, ts, value, lora_emit, NULL);
LoRa.beginPacket();
LoRa.write(s_buf, s_len);
LoRa.endPacket();
```

**Duty cycle table (EU 868 MHz, 30-byte frame)**:

| SF | BW (kHz) | CR | Air time | Min interval (1% duty) | Range (LoS) |
|----|----------|----|----------|----------------------|-------------|
| 7  | 125      | 4/5 | ~15 ms  | 1.5 s                | ~2 km       |
| 9  | 125      | 4/5 | ~100 ms | 10 s                 | ~5 km       |
| 12 | 125      | 4/8 | ~2.7 s  | 270 s                | ~15 km      |

### 4.4 Ethernet + uIP (STM32 + ENC28J60)

Use when:
- Wired Ethernet is required and the MCU is STM32 or higher
- lwIP is too large for available RAM

**uIP vs lwIP decision matrix**:

| Factor | uIP | lwIP |
|--------|-----|------|
| RAM footprint | ~1 KB | ~10–30 KB |
| UDP support | Yes (UIP_UDP=1) | Yes |
| TCP reliability | Limited (no retransmit buffering) | Full |
| ESP32 / ESP8266 | Not applicable — use built-in lwIP | Built-in |
| ATmega328P | Theoretically possible but leaves < 1 KB RAM | — |
| STM32F103 (20 KB RAM) | Fits comfortably | Fits with tuning |
| Recommendation | Only if RAM < 8 KB and Ethernet mandatory | Prefer for STM32 |

**For ATmega328P**: avoid uIP. Use a dedicated ESP8266 or HC-12 module as a WiFi/RF bridge. The ATmega sends UART bytes, the bridge forwards them as UDP or serial.

### 4.5 Standalone TX module vs integrated (decision guide)

```
Is the MCU AVR (ATtiny/ATmega)?
├── Yes
│   ├── Does it have WiFi? No → Use UART → RS-485 or nRF24L01+ or HC-12 bridge
│   └── Does it have Ethernet? No → Same as above
└── No (ESP32/ESP8266/STM32)
    ├── Built-in WiFi? Yes → WiFi UDP (see §4.2)
    ├── Ethernet IC available? Yes → lwIP + UDP (see §4.4)
    └── Long range required? → LoRa (see §4.3)
```

---

## 5. Flash Footprint by Configuration

### With default config (OSTX_PACKET_MAX=96, OSTX_BODY_MAX=64)

| Module | Text (x86-64 measured) | AVR est. (×0.65) |
|--------|----------------------|-----------------|
| ostx_crc.c | 128 B | ~83 B |
| ostx_b62.c | 192+64 B | ~125+42 B |
| ostx_packet.c | 192 B | ~125 B |
| ostx_sensor.c | 352 B | ~229 B |
| ostx_static.c | 208 B | ~135 B |
| ostx_stream.c | 1008+96 B | ~655+62 B |

API combinations (linker drops unused modules):

| API used | Modules linked | Total Flash (AVR est.) |
|----------|---------------|----------------------|
| A only | crc + b62 + packet + sensor | ~562 B |
| B only | crc + b62 + static | ~385 B |
| C only | stream (self-contained) | ~717 B |
| A + B | crc + b62 + packet + sensor + static | ~697 B |
| All three | all 6 modules | ~1.45 KB |

### Reducing Flash further

1. **Lower OSTX_VALUE_SCALE to 100**: reduces Base62 digit count by ~2, saving ~30 B in `ostx_stream.c` loop iterations (Flash unchanged, but output frame is 2 bytes shorter).
2. **Shorten sensor IDs**: `"T"` instead of `"T1"` saves 1 byte per descriptor in Flash.
3. **-Os / MinSizeRel**: avr-gcc typically produces 20–30% smaller code than -O0.
4. **LTO (Link-Time Optimisation)**: `-flto` can eliminate inlining overhead; saves ~5–10% additional Flash on AVR.

---

## 6. Power Budget (AVR @ 3.3 V)

| State | Current | Notes |
|-------|---------|-------|
| ATmega328P active @ 8 MHz | ~4 mA | During pack + UART TX |
| ATmega328P power-down sleep | 0.1 µA | Between readings; use `sleep_mode()` |
| HC-12 UART radio TX (100 mW) | ~100 mA | Dominant; burst per frame only |
| nRF24L01+ TX (+0 dBm) | ~11 mA | Burst < 1 ms per frame |
| SX1276 TX (LoRa, +17 dBm) | ~90 mA | Burst duration = SF-dependent air time |

On a 1 000 mAh LiPo at 1 Hz send rate with HC-12:

$$
I_{\text{avg}} = I_{\text{sleep}} + \text{duty} \times (I_{\text{active}} + I_{\text{radio}})
\approx 0 + \frac{31\,\text{ms}}{1000\,\text{ms}} \times 104\,\text{mA} \approx 3.2\,\text{mA}
$$

→ ~312 hours ≈ 13 days on a single charge.
