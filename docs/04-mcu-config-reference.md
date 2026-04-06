# 04 — MCU Configuration Reference

This document provides recommended `ostx_config.h` override values for every major 8-bit MCU family. Copy the relevant `-D` flags into your build system, or add `#define` overrides in a project header **before** including any OSynaptic-TX header.

---

## Configuration Tiers

Four pre-defined tiers cover the full RAM range of 8-bit MCUs:

| Tier | RAM range | `OSTX_PACKET_MAX` | `OSTX_BODY_MAX` | `OSTX_ID_MAX` | `OSTX_UNIT_MAX` | `OSTX_B62_MAX` | `OSTX_VALUE_SCALE` |
|------|-----------|------------------|----------------|--------------|----------------|---------------|-------------------|
| **Ultra** | < 256 B | `32` | `14` | `3` | `4` | `7` | `100L` |
| **Tight** | 256–512 B | `48` | `24` | `5` | `5` | `8` | `1000L` |
| **Standard** | 512 B–2 KB | `96` | `64` | `9` | `9` | `14` | `10000L` |
| **Comfort** | > 2 KB | `128` | `96` | `12` | `12` | `14` | `10000L` |

### CMake / Makefile flag sets

```cmake
# Ultra
target_compile_definitions(myapp PRIVATE
    OSTX_PACKET_MAX=32 OSTX_BODY_MAX=14 OSTX_ID_MAX=3
    OSTX_UNIT_MAX=4 OSTX_B62_MAX=7 OSTX_VALUE_SCALE=100L)

# Tight
target_compile_definitions(myapp PRIVATE
    OSTX_PACKET_MAX=48 OSTX_BODY_MAX=24 OSTX_ID_MAX=5
    OSTX_UNIT_MAX=5 OSTX_B62_MAX=8 OSTX_VALUE_SCALE=1000L)

# Standard (default — no flags needed)

# Comfort
target_compile_definitions(myapp PRIVATE
    OSTX_PACKET_MAX=128 OSTX_BODY_MAX=96 OSTX_ID_MAX=12
    OSTX_UNIT_MAX=12 OSTX_B62_MAX=14 OSTX_VALUE_SCALE=10000L)
```

> **Ultra tier note**: sensor ID max 2 chars (`"T1"`) + unit max 3 chars (`"°C"`) + b62 ≤ 6 chars = 14-byte body. Total frame ≤ 30 bytes, fits within `OSTX_PACKET_MAX=32`.

> **OSTX_VALUE_SCALE=100L** gives 2 decimal places precision (e.g. 21.50 °C → 2150). Use `10000L` when 4 decimal places are required and Flash/RAM allows.

---

## AVR ATtiny Series

Compiler: **avr-gcc** (Arduino IDE or AVR-Toolchain standalone).  
No hardware UART on classic ATtiny — use `SoftwareSerial`, `NewSoftSerial`, or the USI peripheral as a UART.

| MCU | Flash | RAM | HW UART | Recommended API | Tier | Transport | Notes |
|-----|-------|-----|---------|----------------|------|-----------|-------|
| ATtiny10 | 1 KB | 32 B | None | **Not supported** | — | — | Flash and RAM both insufficient |
| ATtiny13A | 1 KB | 64 B | None | **Not supported** | — | — | Same; too small for any tier |
| ATtiny25 | 2 KB | 128 B | USI (SW) | **C** | **Ultra** | SoftwareSerial / USI UART | API C only. Exactly fits at Ultra tier. No space for A or B. |
| ATtiny45 | 4 KB | 256 B | USI (SW) | **C** | **Ultra** | SoftwareSerial | Fits 2 sensor descriptors (27 B Flash each). |
| ATtiny85 | 8 KB | 512 B | USI (SW) | C / B | **Tight** | SoftwareSerial | Comfortable at Tight; spare capacity for application code. |
| ATtiny24 / 44 / 84 | 2 / 4 / 8 KB | 128 / 256 / 512 B | USI | Same as tiny25 / 45 / 85 | Same | SoftwareSerial | 14-pin DIP; suitable for low-cost agricultural / industrial nodes. |
| ATtiny261 / 461 / 861 | 2 / 4 / 8 KB | 128 / 256 / 512 B | USI | Same as above | Same | SoftwareSerial | Differential ADC available. |
| ATtiny1614 / 1616 | 16 KB | 2 KB | HW USART0 | A / B / C | **Standard** | UART / RS-485 | tinyAVR-1 core; hardware UART; UPDI programming. |
| ATtiny3216 / 3227 | 32 KB | 2 KB | HW USART | A / B / C | **Standard** | UART / RS-485 | Recommended for new low-cost wireless sensor node designs. |

---

## AVR ATmega Series

Compiler: **avr-gcc**.

| MCU | Flash | RAM | HW UART count | Recommended API | Tier | Transport | Notes |
|-----|-------|-----|--------------|----------------|------|-----------|-------|
| ATmega48P | 4 KB | 512 B | 1 | **C** | **Tight** | UART | First megaAVR with hardware UART; C-only at Tight. |
| ATmega88P | 8 KB | 1 KB | 1 | C / B | **Tight** | UART | |
| ATmega168P | 16 KB | 1 KB | 1 | A / B / C | **Standard** | UART / RS-485 | Arduino Nano (older revision). |
| **ATmega328P** | **32 KB** | **2 KB** | **1** | **A / B / C** | **Standard** | **UART / RS-485** | **Arduino Uno / Nano baseline. Most recommended entry-point MCU.** |
| ATmega32U4 | 32 KB | 2.5 KB | 1 + USB-CDC | A / B / C | **Standard** | UART + USB | Arduino Leonardo / Micro. Use UART for OSynaptic frames, USB-CDC for debug. |
| ATmega644P | 64 KB | 4 KB | 2 | A / B / C | **Comfort** | Dual UART (RS-485 + debug) | Industrial; DIP40 package. |
| **ATmega2560** | **256 KB** | **8 KB** | **4** | **A / B / C** | **Comfort** | **Multi-channel UART** | **Arduino Mega. 4× UART: RS-485 + LoRa module + debug + spare.** |
| ATmega1284P | 128 KB | 16 KB | 2 | A / B / C | **Comfort** | Dual UART | Best RAM/cost ratio in classic DIP40 AVR lineup. |

---

## megaAVR-0 / AVR-DA / AVR-DB (Modern AVR)

Compiler: **avr-gcc** with the megaAVR-0 / AVR-DA device pack (bundled in Arduino IDE 2.x or installable via `avrdude`/`avr-gcc` 12+).

| MCU | Flash | RAM | UART count | Recommended API | Tier | Notes |
|-----|-------|-----|-----------|----------------|------|-------|
| ATmega4809 | 48 KB | 6 KB | 4 | A / B / C | **Comfort** | Arduino Nano Every; UPDI; Event System. |
| AVR32DA32 | 32 KB | 4 KB | 3 | A / B / C | **Comfort** | AVR-DA core; integrated DAC and OPAMP. |
| AVR64DD32 | 64 KB | 8 KB | 4 | A / B / C | **Comfort** | Cheapest multi-UART modern AVR; recommended for all new industrial designs. |
| AVR128DB48 | 128 KB | 16 KB | 5 | A / B / C | **Comfort** | Integrated CAN transceiver (CAN-FD via USART mode); 48-pin package. |

---

## Microchip PIC Series

Compiler: **MPLAB XC8** (free Special Edition; `-O1` sufficient).  
All PIC 8-bit variants use `unsigned long` as 32-bit and `unsigned char` as 8-bit — fully compatible with `ostx_types.h`.

| MCU | Flash | RAM | UART | Recommended API | Tier | Notes |
|-----|-------|-----|------|----------------|------|-------|
| PIC10F200 / 206 | 256–512 W | 24–32 B | None | **Not supported** | — | No UART; too small. |
| PIC12F675 | 1 KB | 64 B | None | **Not supported** | — | Same. |
| PIC12F1840 | 7 KB | 256 B | 1 EUSART | **C** | **Ultra** | Hardware UART; Flash-constrained but deployable with API C. |
| PIC16F628A | 3.5 KB | 224 B | 1 USART | **C** | **Ultra** | Classic teaching MCU; RAM very tight. |
| PIC16F877A | 14 KB | 368 B | 1 USART | **C** | **Ultra → Tight** | Common in legacy designs; Flash adequate, RAM limits tier. |
| **PIC16F18875** | **32 KB** | **2 KB** | **1 EUSART** | **A / B / C** | **Standard** | **Recommended modern PIC16 for new designs.** |
| PIC16F1619 | 14 KB | 1 KB | 1 EUSART | C / B | **Tight** | Cost-effective modern PIC16. |
| **PIC18F4520** | **32 KB** | **1.5 KB** | **1 EUSART** | **A / B / C** | **Standard** | **Industrial workhorse PIC18. Note: RAM slightly less than ATmega328P.** |
| **PIC18F45K22** | **32 KB** | **1.5 KB** | **2 EUSART** | **A / B / C** | **Standard** | **Dual UART: one for OSynaptic frames, one for debug.** |
| PIC18F87K22 | 128 KB | 4 KB | 2 EUSART | A / B / C | **Comfort** | 80-pin package; multi-sensor industrial node. |
| **PIC18F47Q10** | **128 KB** | **3.6 KB** | **4 UART** | **A / B / C** | **Comfort** | **Recommended for all new PIC18 industrial designs.** |

---

## 8051 Family

Compiler: **SDCC** (open-source, C89/C99) or **Keil C51** or **IAR 8051**.

> `int` = 16-bit, `long` = 32-bit, `char` defaults to `unsigned` in most 8051 compilers — all compatible with `ostx_types.h`.

> **Memory banking**: place `static` buffers in `__xdata` (external RAM) to preserve the 128 B internal `DATA` space for the call stack.

```c
/* SDCC xdata annotation example */
static __xdata ostx_u8 g_buf[OSTX_PACKET_MAX];
```

| MCU | Flash | Internal RAM | HW UART | Recommended API | Tier | Notes |
|-----|-------|-------------|---------|----------------|------|-------|
| 8051 original / AT89C51 | 4 KB | 128 B DATA | 1 | **C** | **Ultra** | Only 128 B internal RAM; place buffers in XDATA if available. |
| AT89C52 / AT89S52 | 8 KB | 256 B DATA | 1 | **C** | **Ultra → Tight** | 256 B internal RAM; doubles internal stack headroom vs original 8051. |
| STC89C52RC | 8 KB | 512 B (256+256) | 1 | **C** | **Tight** | Common domestic variant; internal EEPROM; SDCC supported. |
| **STC15W4K32S4** | **32 KB** | **4 KB** | **4 UART** | **A / B / C** | **Comfort** | **Most widely used enhanced 8051 in China; 4 UARTs.** |
| STC8H8K64U | 64 KB | 8 KB | 4 UART | A / B / C | **Comfort** | Latest STC series; USB direct download; recommended for all new 8051 designs. |
| Silicon Labs C8051F340 | 64 KB | 4 KB | 1 UART | A / B / C | **Comfort** | USB + UART; industrial grade. |
| Silicon Labs EFM8UB2 | 64 KB | 4 KB | 1 UART | A / B / C | **Comfort** | 48 MHz high-speed core; suited for fast-sampling sensors. |

---

## STMicroelectronics STM8 Series

Compiler: **SDCC** (recommended, free) or **IAR for STM8** or **Cosmic C** (commercial).  
SDCC STM8 C89 support is mature; `unsigned long` is 32-bit — compatible with `ostx_types.h`.

| MCU | Flash | RAM | UART count | Recommended API | Tier | Notes |
|-----|-------|-----|-----------|----------------|------|-------|
| STM8S003F3P6 | 8 KB | 1 KB | 1 UART | **C** | **Tight** | SOT23-6 smallest package; ~¥1; entry-level choice. |
| **STM8S103F3P6** | **8 KB** | **1 KB** | **1 UART** | **C / B** | **Tight** | **Most common STM8; DIP20; recommended TX-only node MCU.** |
| STM8S105C6T6 | 32 KB | 2 KB | 2 UART | A / B / C | **Standard** | Larger Flash / RAM; TQFP48 package. |
| STM8S207MB | 128 KB | 6 KB | 3 UART | A / B / C | **Comfort** | STM8 flagship; CAN + UART. |
| STM8L151G4 | 16 KB | 1 KB | 1 USART | C / B | **Tight** | Low-power series; RTC + 32 kHz oscillator; ideal for sleep-wake sensor nodes. |
| STM8L152C6 | 32 KB | 2 KB | 2 USART | A / B / C | **Standard** | Low-power + larger Flash. |

---

## NXP (Freescale) HCS08 Series

Compiler: **CodeWarrior for HCS08** (free Special Edition) or **IAR**. C89 supported.

| MCU | Flash | RAM | UART | Recommended API | Tier | Notes |
|-----|-------|-----|------|----------------|------|-------|
| MC9S08QG8 | 8 KB | 512 B | 1 SCI | **C** | **Tight** | Entry-level HCS08; QFN16 package. |
| MC9S08AC128 | 128 KB | 4 KB | 2 SCI | A / B / C | **Comfort** | Industrial control; dual UART. |
| MC9S08GT60 | 60 KB | 4 KB | 1 SCI | A / B / C | **Comfort** | Automotive grade. |

---

## Renesas RL78/G Series

> RL78 uses a 16-bit instruction set but is architecturally comparable to 8-bit MCUs in RAM constraints and is commonly deployed alongside them.  
> Compiler: **KPIT GNURL78** (open-source) or **Renesas CC-RL**. C89 supported.  
> `ostx_u16` maps to `unsigned short` (16-bit) — correct.

| MCU | Flash | RAM | UART count | Recommended API | Tier | Notes |
|-----|-------|-----|-----------|----------------|------|-------|
| RL78/G13 R5F100 | 4–512 KB | 0.5–32 KB | 3 UART/CSI | A / B / C | Standard – Comfort | Ultra-low power; factory automation. Range covers multiple sub-variants. |
| RL78/G14 R5F104 | 64 KB | 4 KB | 3 | A / B / C | **Comfort** | Recommended for industrial sensor nodes in Renesas ecosystems. |

---

## Zilog Z8 Encore!

Compiler: **ZDS-II** (Zilog official, free). C89 supported.

| MCU | Flash | RAM | UART | Recommended API | Tier |
|-----|-------|-----|------|----------------|------|
| Z8F0423 | 4 KB | 237 B | 0 | **C** | **Ultra** |
| Z8F1621 | 16 KB | 1 KB | 2 | C / B | **Standard** |
| Z8F4821 | 48 KB | 2 KB | 2 | A / B / C | **Standard** |

---

## Quick-Select Summary

| Use case | Recommended MCU | Tier | API |
|----------|----------------|------|-----|
| Lowest-cost wired sensor node | STM8S003F3P6 or ATmega328P | Tight / Standard | C |
| Domestic industrial (8051 ecosystem) | STC15W4K32S4 or STC8H8K64U | Comfort | A / B / C |
| Low-power battery node | STM8L152C6 or ATtiny1614 | Tight / Standard | C |
| Multi-sensor industrial node | ATmega2560 or AVR64DD32 | Comfort | A / B / C |
| Existing PIC ecosystem | PIC18F47Q10 | Comfort | A / B / C |
| AVR + WiFi bridge (ESP-01 AT) | ATmega328P + ESP-01 | Standard | C |
| Native WiFi UDP | ESP32 / ESP8266 *(32-bit)* | — | C |

---

## API Selection Rule

```
RAM < 256 B  → API C only, Ultra tier
RAM 256–512 B → API C preferred (B if Flash ≥ 8 KB), Tight tier
RAM 512 B–2 KB → API C / B recommended, Standard tier
RAM > 2 KB   → API A / B / C freely, Comfort tier
```
