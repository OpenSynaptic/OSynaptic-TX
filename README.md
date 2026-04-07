# OSynaptic-TX

简体中文说明请见 [README.zh.md](README.zh.md)。

**TX-only OpenSynaptic packet encoder for 8-bit MCUs** — encodes sensor readings into the OpenSynaptic wire format (FULL frames) with **pure C89**, **no heap**, and a stack peak as low as **21 bytes** on AVR. Pairs directly with the [OpenSynaptic](../OpenSynaptic/README.md) server and [OSynaptic-FX](../OSynaptic-FX/README.md) gateway over any serial transport (UART / UDP / LoRa / RS-485 / CAN).

![C89](https://img.shields.io/badge/C-89-00599C?logo=c&logoColor=white)
![Version](https://img.shields.io/badge/version-1.0.0-2E8B57)
![Arduino](https://img.shields.io/badge/Arduino-Library-00979D?logo=arduino&logoColor=white)
![Status](https://img.shields.io/badge/status-stable-2E8B57)
![License](https://img.shields.io/badge/License-Apache--2.0-blue)

![AVR](https://img.shields.io/badge/AVR-supported-00599C)
![ESP32](https://img.shields.io/badge/ESP32-supported-E7352C?logo=espressif&logoColor=white)
![ESP8266](https://img.shields.io/badge/ESP8266-supported-E7352C?logo=espressif&logoColor=white)
![STM32](https://img.shields.io/badge/STM32-supported-03234B?logo=stmicroelectronics&logoColor=white)
![RP2040](https://img.shields.io/badge/RP2040-supported-B41F47?logo=raspberrypi&logoColor=white)
![Cortex-M](https://img.shields.io/badge/Cortex--M-supported-0091BD)

---

## Quick Reference Tables

Two look-up tables that are most frequently needed during development and deployment:

| Table | Description |
|-------|-------------|
| [**MCU Config Reference →**](docs/04-mcu-config-reference.md) | ~50 8-bit MCU models (AVR, PIC, 8051, STM8, HCS08, RL78, Z8) — recommended API tier, Flash/RAM budget, and `ostx_config.h` macro settings for each device |
| [**Unit Validation Table →**](docs/05-unit-validation.md) | All 80+ valid `OSTX_UNIT()` symbols across 15 sensor classes; SI prefix rules and compile-time error behavior |

> New to the library? See [Quick Start](#quick-start-code) below.
> Looking up a specific MCU or sensor unit? Use the table links above — they are the most-referenced pages in this project.

---

## Try In 30 Seconds

```
Arduino IDE → Sketch > Include Library > Add .ZIP Library → select OSynaptic-TX.zip
File > Examples > OSynaptic-TX > BasicTX → Upload
```

Open Serial Monitor at **115200 baud** — FULL packets stream out once per second.

---

## Table of Contents

- [**Quick Reference Tables**](#quick-reference-tables) — MCU config + unit validation (start here)
- [Why OSynaptic-TX](#why-osynaptic-tx)
- [Three API Tiers](#three-api-tiers)
- [Memory Usage](#memory-usage)
- [MCU Deployment Reference](#mcu-deployment-reference)
- [Transport Selection Guide](#transport-selection-guide)
- [Quick Start (Code)](#quick-start-code)
- [Wire Format](#wire-format)
- [Examples](#examples)
- [Repository Map](#repository-map)
- [CMake Build](#cmake-build)
- [Test Results](#test-results)
- [Documentation](#documentation)
- [Contributing](#contributing)
- [License](#license)

---

## Why OSynaptic-TX

- **TX-only, no decode code**: Flash cost is a fraction of a full-duplex library.
- **C89 clean**: compiles on every toolchain that targets 8-bit MCUs — avr-gcc, SDCC, IAR, MPLAB XC8.
- **No heap**: zero `malloc`/`free` calls; all buffers are stack or static.
- **Three tiers of RAM usage**: pick the API that matches your MCU's resources.
- **Spec-compatible**: packets are decoded directly by OpenSynaptic server without glue code.

---

## Three API Tiers

| API | Stack peak (AVR) | Static RAM | Flash (AVR est.) | Description |
|-----|-----------------|-----------|-----------------|-------------|
| **A** `ostx_sensor_pack()` | ~137 B | 96 B | ~600 B | Dynamic runtime strings |
| **B** `ostx_static_pack()` | ~51 B | 96 B | ~430 B | Compile-time descriptor in Flash |
| **C** `ostx_stream_pack()` | **~21 B** | **0 B** | ~760 B | Zero-buffer streaming via callback |

All three produce identical wire frames. The linker drops unused modules automatically.

---

## Memory Usage

### API C — Streaming (recommended for AVR ≤ 4 KB SRAM)

| Resource | Usage |
|----------|-------|
| Stack peak | ~21 bytes |
| Static RAM | 0 bytes |
| Flash (Uno/Nano, 1 sensor) | ~760 bytes |

### Minimum supported MCU

| Requirement | Value |
|-------------|-------|
| RAM | ≥ 128 B |
| Flash | ≥ 2 KB |
| Example | ATtiny25 / ATmega48 |

---

## MCU Deployment Reference

### 8-bit AVR family — maximum and theoretical deployment values

| MCU | Flash | RAM | UART | API tier | Max sensors per second | Notes |
|-----|-------|-----|------|----------|----------------------|-------|
| ATtiny25 | 2 KB | 128 B | SW (USI) | **C only** | 1 @ 1 Hz | Absolute minimum. No space for A/B. Requires SoftwareSerial or USI UART. |
| ATtiny45 | 4 KB | 256 B | SW (USI) | **C only** | 3 @ 1 Hz | Fits API C + 2 sensor descriptors. |
| ATtiny85 | 8 KB | 512 B | SW (USI) | C / B | 6 @ 1 Hz | Comfortable for API C; API B fits with small config. |
| ATmega48 | 4 KB | 512 B | HW UART0 | **C only** | 3 @ 1 Hz | First AVR with hardware UART; API C recommended. |
| ATmega88 | 8 KB | 1 KB | HW UART0 | C / B | 8 @ 1 Hz | |
| ATmega168 | 16 KB | 1 KB | HW UART0 | A / B / C | 10 @ 1 Hz | All three APIs fit. |
| **ATmega328P** | **32 KB** | **2 KB** | **HW UART0** | **A / B / C** | **~20 @ 1 Hz** | **Arduino Uno / Nano baseline. Recommended entry point.** |
| ATmega32U4 | 32 KB | 2.5 KB | HW UART + USB | A / B / C | ~20 @ 1 Hz | Arduino Leonardo / Micro. USB-CDC Serial available. |
| ATmega2560 | 256 KB | 8 KB | 4× HW UART | A / B / C | ~50 @ 1 Hz | Arduino Mega. Multiple independent transport channels. |
| ATmega4809 | 48 KB | 6 KB | 4× USART | A / B / C | ~40 @ 1 Hz | Arduino Nano Every (megaAVR-0). |

> **Theoretical throughput** is limited by UART baud rate, not CPU cycles. A 30-byte frame at 9600 baud takes 31 ms. At 115200 baud, the same frame takes 2.6 ms. The library's CPU contribution (~9 600 cycles on ATmega328P @ 16 MHz ≈ 0.6 ms) is negligible.

### 32-bit targets (for comparison)

| MCU | Flash | RAM | UART | Typical sustained rate | Notes |
|-----|-------|-----|------|----------------------|-------|
| STM32F030F4 | 16 KB | 4 KB | 1× USART | ~30 @ 1 Hz | Cortex-M0, smallest STM32. All APIs fit. |
| STM32F103C8 | 64 KB | 20 KB | 3× USART | ~100 @ 10 Hz | "Blue Pill". Ethernet via ENC28J60 + uIP possible. |
| ESP8266 | 1–4 MB flash | 80 KB | UART + WiFi | ~50 @ 10 Hz | Use WiFi UDP directly; built-in lwIP. |
| **ESP32** | **4 MB flash** | **520 KB** | **3× UART + WiFi** | **~200 @ 10 Hz** | **Recommended for LAN/WiFi deployments.** |
| RP2040 | 2 MB flash | 264 KB | 2× UART | ~200 @ 10 Hz | PIO UART possible. |

---

## Transport Selection Guide

Choose the transport that matches your hardware, not the protocol stack.

### UART / RS-485 (wired)
**Best for**: ATtiny, ATmega, any MCU with a hardware UART and a wired link to a gateway.

- Use `Serial.write(b)` as the emit callback (`BasicTX`, `BareMetalUARTTX`).
- For distances > 10 m, add an RS-485 driver (MAX485 / SN75176). The library produces identical bytes on both.
- A dedicated standalone TX module approach: MCU → UART → RS-485 driver → hub, no network stack required.

### WiFi UDP (ESP8266 / ESP32)
**Best for**: ESP32 / ESP8266 nodes reaching an OpenSynaptic server on a local LAN.

- Open a `UDP.beginPacket()`, emit all frame bytes via callback, call `UDP.endPacket()` — see `ESP32UdpTX`.
- UDP is preferred over TCP for sensor telemetry: no connection state, no reconnect latency, minimal overhead.
- lwIP (built into ESP-IDF and ESP8266 Arduino core) handles fragmentation automatically; no external TCP/IP stack needed.
- **uIP** (the lightweight µIP stack occasionally ported to ATmega): not recommended for sensor telemetry over WiFi. uIP requires an external SPI Ethernet IC (ENC28J60) and a >256 B packet buffer. On ATmega328P with 2 KB RAM, uIP and OSynaptic-TX together may not fit. Use a dedicated ESP8266/ESP32 module as a WiFi bridge if the MCU is AVR.

### LoRa (SX1276/SX1278)
**Best for**: outdoor sensors more than 100 m from the gateway, no WiFi infrastructure.

- Accumulate the frame in a small buffer (OSTX_PACKET_MAX ≤ 96 B), then call `LoRa.beginPacket()` / `LoRa.write()` / `LoRa.endPacket()` — see `LoRaTX`.
- **Duty cycle**: EU 868 MHz sub-band 1 (g): max 1%. At SF9/BW125, a 30-byte frame takes ~100 ms → minimum interval ~10 s.
- LoRaWAN (OTAA/ABP) vs raw LoRa: OSynaptic-TX produces binary payloads, so either works. Raw LoRa (no stack overhead) fits even in ATtiny85.

### nRF24L01+ (2.4 GHz)
**Best for**: short-range (< 100 m) wireless from an ATmega node to a Raspberry Pi or ESP32 gateway.

- Max payload per packet: 32 bytes. A minimal OSynaptic-TX frame is ~21 bytes — fits in one RF packet.
- Use a second `nrf24_emit` callback that accumulates bytes and flushes via `radio.write()` at frame end.
- No duty-cycle restrictions in most jurisdictions.

### Standalone TX module vs integrated WiFi
| | Standalone TX module | Integrated WiFi (ESP32/8266) |
|---|---|---|
| MCU complexity | Low (UART only) | Moderate (WiFi init, reconnect) |
| BOM cost | Lower (add HC-12/E32 module ~$2) | Higher (ESP32 ~$3–5) |
| Range | 100–1000 m (HC-12), 10 km (E32 LoRa) | 30–100 m (2.4 GHz WiFi) |
| Power (TX active) | ~20–40 mA (HC-12 at 100 mW) | ~170 mA (ESP32 WiFi TX) |
| Recommended for | ATtiny/ATmega field nodes | High-rate LAN telemetry |

> **Rule of thumb**: if the MCU is AVR and the server is on the same LAN, add a cheap ESP8266 bridge or E32 UART radio module rather than porting a TCP/IP stack to the AVR.

---

## Quick Start (Code)

### API C — Zero-buffer streaming (minimum RAM)

```cpp
#include <OSynaptic-TX.h>

// Sensor descriptor baked into Flash — 0 bytes of SRAM
OSTX_STATIC_DEFINE(s_temp, 0x00000001UL, "T1", "Cel");

// Emit callback — hand each byte directly to Serial
static void emit(ostx_u8 b, void *) { Serial.write(b); }

static ostx_u8 tid = 0;

void setup() { Serial.begin(115200); }

void loop() {
    // Value pre-scaled by OSTX_VALUE_SCALE (default 10000)
    // 215000 = 21.5000 °C
    ostx_stream_pack(&s_temp, tid++, millis() / 1000UL, 215000L, emit, NULL);
    delay(1000);
}
```

### API B — Static descriptor (lower stack than A)

```cpp
#include <OSynaptic-TX.h>

OSTX_STATIC_DEFINE(s_hum, 0x00000001UL, "H1", "Pct");

static ostx_u8 buf[OSTX_PACKET_MAX];
static ostx_u8 tid = 0;

void setup() { Serial.begin(115200); }

void loop() {
    int len = ostx_static_pack(&s_hum, tid++, millis() / 1000UL, 650000L, buf);
    if (len > 0) Serial.write(buf, (size_t)len);
    delay(1000);
}
```

### API A — Fully dynamic

```cpp
#include <OSynaptic-TX.h>

static ostx_u8 buf[OSTX_PACKET_MAX];
static ostx_u8 tid = 0;

void setup() { Serial.begin(115200); }

void loop() {
    int len = ostx_sensor_pack(0x00000001UL, "T1", "Cel",
                               tid++, (ostx_u32)(millis() / 1000UL),
                               215000L, buf);
    if (len > 0) Serial.write(buf, (size_t)len);
    delay(1000);
}
```

---

## Wire Format

Every frame follows the OpenSynaptic FULL packet layout (C89 big-endian):

```
[cmd:1][route:1][aid:4BE][tid:1][ts:6BE][sid|unit|b62][crc8:1][crc16:2]
```

| Field | Size | Description |
|-------|------|-------------|
| `cmd` | 1 B | `0x3F` (FULL data) |
| `route` | 1 B | routing flags |
| `aid` | 4 B | agent ID (big-endian) |
| `tid` | 1 B | transaction ID (wraps 0–255) |
| `ts` | 6 B | Unix timestamp seconds (big-endian) |
| body | variable | `sid\|unit\|b62value` |
| `crc8` | 1 B | CRC-8/SMBUS over body |
| `crc16` | 2 B | CRC-16/CCITT-FALSE over full frame |

Minimum frame size: **21 bytes** (2-char sid, 3-char unit, 1-char b62 value).

---

## Examples

| Example | API | Transport | Target |
|---------|-----|-----------|--------|
| [BasicTX](examples/BasicTX/BasicTX.ino) | C (streaming) | Serial (UART) | Any Arduino |
| [MultiSensorTX](examples/MultiSensorTX/MultiSensorTX.ino) | C (streaming) | Serial (UART) | Any Arduino, 3 sensors |
| [ESP32UdpTX](examples/ESP32UdpTX/ESP32UdpTX.ino) | C (streaming) | WiFi UDP | ESP32 / ESP8266 |
| [LoRaTX](examples/LoRaTX/LoRaTX.ino) | C (streaming) | LoRa SX1276 | Heltec / TTGO / Uno + shield |
| [BareMetalUARTTX](examples/BareMetalUARTTX/BareMetalUARTTX.ino) | C (streaming) | USART0 registers | ATmega328P (no Serial overhead) |

---

## Repository Map

```
OSynaptic-TX/
├── OSynaptic-TX.h          ← single Arduino include
├── library.properties
├── keywords.txt
├── LICENSE
├── README.md
├── CONTRIBUTING.md
├── SECURITY.md
├── docs/
│   ├── 01-deployment-guide.md  ← MCU table, transport selection, sizing
│   ├── 02-api-reference.md     ← full API documentation
│   └── 03-wire-format.md       ← byte-level wire format specification
├── src/
│   ├── ostx_config.h       ← compile-time knobs
│   ├── ostx_types.h        ← C89 portable typedefs
│   ├── ostx_crc.h/c        ← CRC-8 + CRC-16 (no lookup table)
│   ├── ostx_b62.h/c        ← Base62 encoder
│   ├── ostx_packet.h/c     ← frame builder (API A)
│   ├── ostx_sensor.h/c     ← API A: ostx_sensor_pack()
│   ├── ostx_static.h/c     ← API B: ostx_static_pack()
│   └── ostx_stream.h/c     ← API C: ostx_stream_pack()
├── include/                ← original headers (CMake build)
├── examples/
│   ├── BasicTX/            ← UART, API C (1 sensor)
│   ├── MultiSensorTX/      ← UART, API C (3 sensors)
│   ├── ESP32UdpTX/         ← WiFi UDP, API C
│   ├── LoRaTX/             ← LoRa SX1276, API C
│   ├── BareMetalUARTTX/    ← AVR registers, API C, no Serial overhead
│   └── basic_tx.c          ← native C example (all 3 APIs)
├── tests/
│   ├── test_vector.c       ← CRC / Base62 / byte-order known-answer tests (50 assertions)
│   └── README.md           ← expected output and test group descriptions
└── CMakeLists.txt
```

---

## CMake Build

Requires CMake ≥ 3.13 and a C89-capable compiler.

```powershell
cmake -B build -DCMAKE_BUILD_TYPE=MinSizeRel
cmake --build build
```

---

## Test Results

Run the built-in known-answer test suite to verify the CRC, Base62, and packet
assembly implementations on your build host:

```sh
cmake -B build -DOSTX_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

| Group | Assertions | What is verified |
|-------|-----------|------------------|
| CRC-8/SMBUS | 5 | Standard check value `0xF4` for `"123456789"`, single-byte, NULL/zero-length guard |
| CRC-16/CCITT-FALSE | 5 | Standard check value `0x29B1` for `"123456789"`, two edge bytes, NULL/zero-length guard |
| Base62 encode | 17 | Zero; alphabet boundaries; rollover (`62`→`"10"`, `3843`→`"ZZ"`); negative; `INT32_MAX`/`INT32_MIN`; buffer-overflow and `NULL` pointer guards |
| Frame byte order | 19 | `aid` big-endian at `out[2..5]`; `ts_sec` big-endian at `out[7..12]`; CRC-8 placement; CRC-16 big-endian placement |
| Frame edge cases | 4 | Zero-body, `NULL` output, negative `body_len`, `body_len > OSTX_BODY_MAX` |
| **Total** | **50** | Expected: `50 passed, 0 failed` |

See [tests/README.md](tests/README.md) for complete expected output and instructions for adding new test cases.

---

## Documentation

Full documentation is in the [`docs/`](docs/) folder:

| File | Contents |
|------|----------|
| [docs/01-deployment-guide.md](docs/01-deployment-guide.md) | MCU sizing table, transport selection, duty-cycle guidance |
| [docs/02-api-reference.md](docs/02-api-reference.md) | Complete API reference for all three tiers |
| [docs/03-wire-format.md](docs/03-wire-format.md) | Byte-level wire format, CRC algorithms, Base62 encoding |
| [docs/04-mcu-config-reference.md](docs/04-mcu-config-reference.md) | Per-MCU config tier table covering ~50 8-bit devices (AVR, PIC, 8051, STM8, HCS08, RL78, Z8) |
| [docs/05-unit-validation.md](docs/05-unit-validation.md) | Compile-time unit validation standard: `OSTX_UNIT()` macro, all valid codes, SI prefix rules, custom units |
| [tests/README.md](tests/README.md) | Test suite documentation: full expected output for all 50 assertions, CRC check-value sources, how to add new cases |

---

## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) and [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md) before opening a pull request.

---

## License

Apache License 2.0 — see [LICENSE](LICENSE).
