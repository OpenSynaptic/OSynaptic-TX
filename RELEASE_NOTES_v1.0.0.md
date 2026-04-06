# OSynaptic-TX v1.0.0 — First Stable Release

We are pleased to announce the **first stable release** of OSynaptic-TX — a TX-only, pure C89 packet encoder for 8-bit MCUs that encodes sensor readings into OpenSynaptic FULL wire frames and streams them to the [OpenSynaptic](https://github.com/OpenSynaptic) server over any serial transport (UART / UDP / LoRa / RS-485 / CAN).

---

## What Is OSynaptic-TX?

OSynaptic-TX is the minimal-footprint MCU-side transmitter of the OpenSynaptic sensor telemetry stack. A single `#include <OSynaptic-TX.h>` gives your sketch:

- **Three API tiers** — choose the right tradeoff between Flash, RAM, and code complexity.
- **Pure C89** — compiles clean on avr-gcc, SDCC, IAR, MPLAB XC8, and every host toolchain with `-std=c89 -pedantic -Wall -Wextra -Werror`.
- **No heap** — zero `malloc`/`free` calls; stack peak as low as **21 bytes** on AVR.
- **Spec-compatible wire format** — packets decoded directly by OpenSynaptic server and OSynaptic-FX gateway without custom glue.

---

## Install

**Arduino IDE:** `Sketch > Include Library > Add .ZIP Library…` → select `OSynaptic-TX.zip`

**Manual:** copy the `OSynaptic-TX` folder into `Documents/Arduino/libraries/`

**CMake (native build):**
```powershell
cmake -B build -DCMAKE_BUILD_TYPE=MinSizeRel
cmake --build build
```

---

## Three API Tiers

### API A — Dynamic (`ostx_sensor_pack`)

Runtime sensor ID and unit strings. Use when sensor configuration is not known at compile time.

```c
int len = ostx_sensor_pack(0x00000001UL, "T1", "Cel",
                           tid++, ts_sec, 215000L, buf);
```

- Stack peak: ~137 B (AVR)
- Flash: ~600 B (AVR)

### API B — Static (`ostx_static_pack` + `OSTX_STATIC_DEFINE`)

Sensor descriptor baked into Flash at compile time; only `tid`, `ts`, and the encoded value are written at runtime.

```c
OSTX_STATIC_DEFINE(s_temp, 0x00000001UL, "T1", "Cel");
int len = ostx_static_pack(&s_temp, tid++, ts_sec, 215000L, buf);
```

- Stack peak: ~51 B (AVR)
- Flash: ~430 B (AVR)

### API C — Streaming (`ostx_stream_pack`)

Zero output buffer — each byte is handed to a user-supplied emit callback as it is produced.

```c
OSTX_STATIC_DEFINE(s_temp, 0x00000001UL, "T1", "Cel");
ostx_stream_pack(&s_temp, tid++, ts_sec, 215000L, serial_emit, NULL);
```

- Stack peak: ~21 B (AVR) ← minimum possible under protocol constraints
- Static RAM: **0 B**
- Flash: ~760 B (AVR)

---

## Wire Format

```
[cmd:1][route:1][aid:4BE][tid:1][ts:6BE][sid|unit|b62][crc8:1][crc16:2]
```

- CRC-8/SMBUS over body
- CRC-16/CCITT-FALSE over full frame
- Base62 value encoding with scale factor 10000 (configurable)

---

## Supported Architectures

All architectures with a C89-capable compiler and ≥ 128 B RAM / ≥ 2 KB Flash.

Tested: AVR (Uno, Nano, Mega, ATtiny), ESP8266, ESP32, STM32, RP2040, Cortex-M, x86-64.

---

## Known Limitations

- TX only — no receive or decode path. Use [OSynaptic-RX](../OSynaptic-RX) on the receiver side.
- No encryption at the library level — encrypt at the transport layer if confidentiality is required.
- Single sensor per packet — multi-sensor nodes should call the pack function once per sensor and concatenate or interleave at the transport level.

---

## Changelog

### v1.0.0 (2026-04-06)

- Initial release
- Three API tiers: A (dynamic), B (static), C (streaming)
- CRC-8/SMBUS + CRC-16/CCITT-FALSE
- Base62 encoder with P62 powers-of-62 table (no reversal buffer)
- Arduino library structure (`src/` flat layout, `OSynaptic-TX.h` wrapper)
- CMake build verified on x86-64 MinSizeRel (9/9 targets, zero warnings)
- `BasicTX.ino` example (API C with `Serial.write` emit callback)
