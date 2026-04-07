# OSynaptic-TX

English README: [README.md](README.md)

**面向 8 位 MCU 的 OpenSynaptic 单向发送编码器**。它用 **纯 C89**、**无堆内存**、最低可达 **21 字节 AVR 栈峰值** 的实现，把传感器读数编码为 OpenSynaptic FULL 帧。它可以直接对接 [OpenSynaptic](../OpenSynaptic/README.md) 服务端，以及 [OSynaptic-FX](../OSynaptic-FX/README.md) 网关，适配 UART / UDP / LoRa / RS-485 / CAN 等任意串行传输介质。

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

## 快速参考表

开发和部署时最常查的两张表：

| 表格 | 说明 |
|---|---|
| [MCU 配置参考](docs/04-mcu-config-reference.md) | 约 50 种 8 位 MCU 的建议 API 档位、Flash/RAM 预算与 `ostx_config.h` 宏配置 |
| [单位校验表](docs/05-unit-validation.md) | 15 类传感器中 80+ 个合法 `OSTX_UNIT()` 单位符号、SI 前缀规则与编译期报错行为 |

如果你是第一次使用本库，先看下方的“快速开始”；如果你是按 MCU 或单位反查，直接看这两张表效率最高。

---

## 30 秒上手

```text
Arduino IDE → Sketch > Include Library > Add .ZIP Library → 选择 OSynaptic-TX.zip
File > Examples > OSynaptic-TX > BasicTX → Upload
```

打开 **115200 波特率** 串口监视器，即可看到 FULL 数据包每秒发送一次。

---

## 目录

- [为什么选择 OSynaptic-TX](#为什么选择-osynaptic-tx)
- [三层 API 档位](#三层-api-档位)
- [内存占用](#内存占用)
- [MCU 部署参考](#mcu-部署参考)
- [传输选择指南](#传输选择指南)
- [快速开始（代码）](#快速开始代码)
- [线协议格式](#线协议格式)
- [示例](#示例)
- [仓库结构](#仓库结构)
- [CMake 构建](#cmake-构建)
- [测试结果](#测试结果)
- [文档](#文档)
- [贡献](#贡献)
- [许可证](#许可证)

---

## 为什么选择 OSynaptic-TX

- **只发不收**：不带解码路径，Flash 成本远低于全双工方案。
- **C89 干净**：可在 avr-gcc、SDCC、IAR、MPLAB XC8 等 8 位 MCU 工具链上编译。
- **无堆内存**：不调用 `malloc` / `free`，全部使用栈或静态缓冲。
- **三档 RAM 占用**：可以根据 MCU 资源选择不同 API。
- **协议兼容**：生成的帧能被 OpenSynaptic 服务端直接解码。

---

## 三层 API 档位

| API | AVR 栈峰值 | 静态 RAM | AVR 预估 Flash | 说明 |
|---|---|---|---|---|
| A `ostx_sensor_pack()` | 约 137 B | 96 B | 约 600 B | 运行时动态字符串 |
| B `ostx_static_pack()` | 约 51 B | 96 B | 约 430 B | 编译期描述符放在 Flash |
| C `ostx_stream_pack()` | **约 21 B** | **0 B** | 约 760 B | 通过回调流式输出，零缓冲 |

三种 API 生成的线协议完全一致，链接器会自动裁掉未用模块。

---

## 内存占用

### API C（推荐给 AVR ≤ 4 KB SRAM）

| 资源 | 占用 |
|---|---|
| 栈峰值 | 约 21 B |
| 静态 RAM | 0 B |
| Flash（Uno/Nano，1 传感器） | 约 760 B |

### 最低支持 MCU

| 要求 | 数值 |
|---|---|
| RAM | ≥ 128 B |
| Flash | ≥ 2 KB |
| 示例 | ATtiny25 / ATmega48 |

---

## MCU 部署参考

### 8 位 AVR 家族

| MCU | Flash | RAM | UART | API 档位 | 典型能力 | 说明 |
|---|---|---|---|---|---|---|
| ATtiny25 | 2 KB | 128 B | USI/SW | **仅 C** | 1 传感器 @ 1 Hz | 最小可用配置 |
| ATtiny45 | 4 KB | 256 B | USI/SW | **仅 C** | 3 传感器 @ 1 Hz | 适合极简节点 |
| ATtiny85 | 8 KB | 512 B | USI/SW | C / B | 6 传感器 @ 1 Hz | C 最稳妥 |
| ATmega48 | 4 KB | 512 B | HW UART0 | **仅 C** | 3 传感器 @ 1 Hz | 首个带硬件 UART 的最小档 |
| ATmega168 | 16 KB | 1 KB | HW UART0 | A / B / C | 10 传感器 @ 1 Hz | 三档均可用 |
| **ATmega328P** | **32 KB** | **2 KB** | **HW UART0** | **A / B / C** | **约 20 @ 1 Hz** | **Arduino Uno / Nano 基线平台** |
| ATmega2560 | 256 KB | 8 KB | 4× HW UART | A / B / C | 约 50 @ 1 Hz | 多串口场景理想 |

### 32 位平台（对照）

| MCU | Flash / RAM | 典型速率 | 说明 |
|---|---|---|---|
| STM32F030F4 | 16 KB / 4 KB | 约 30 @ 1 Hz | Cortex-M0 |
| STM32F103C8 | 64 KB / 20 KB | 约 100 @ 10 Hz | Blue Pill |
| ESP8266 | 1–4 MB / 80 KB | 约 50 @ 10 Hz | 适合 WiFi UDP |
| **ESP32** | **4 MB / 520 KB** | **约 200 @ 10 Hz** | **推荐用于局域网 / WiFi 发送节点** |
| RP2040 | 2 MB / 264 KB | 约 200 @ 10 Hz | 双核可扩展 |

---

## 传输选择指南

### UART / RS-485

适合 ATtiny、ATmega 或任何带硬件 UART 的 MCU。直接把 `Serial.write()` 作为 emit 回调即可；距离较远时可加 MAX485 / SN75176 之类的 RS-485 收发器。

### WiFi UDP（ESP8266 / ESP32）

适合局域网传感器遥测。推荐 `UDP.beginPacket()` + emit callback + `UDP.endPacket()` 的最小路径。相比 TCP，UDP 无连接状态、延迟低、实现轻。

### LoRa（SX1276 / SX1278）

适合户外、长距离、无 WiFi 基础设施的节点。通常先把帧写入小缓冲区，再调用 `LoRa.write()` 发送。

### nRF24L01+

适合短距无线（< 100 m）和轻量低功耗链路。OSynaptic-TX 的最小帧通常在 32 字节以内，可以装进单个射频包。

---

## 快速开始（代码）

### API C：零缓冲流式发送

```cpp
#include <OSynaptic-TX.h>

OSTX_STATIC_DEFINE(s_temp, 0x00000001UL, "T1", "Cel");

static void emit(ostx_u8 b, void *) { Serial.write(b); }
static ostx_u8 tid = 0;

void setup() { Serial.begin(115200); }

void loop() {
    ostx_stream_pack(&s_temp, tid++, millis() / 1000UL, 215000L, emit, NULL);
    delay(1000);
}
```

### API B：静态描述符

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

### API A：全动态参数

```cpp
#include <OSynaptic-TX.h>

static ostx_u8 buf[OSTX_PACKET_MAX];
static ostx_u8 tid = 0;

void setup() { Serial.begin(115200); }

void loop() {
    int len = ostx_sensor_pack(0x00000001UL, "T1", "Cel", tid++, (ostx_u32)(millis() / 1000UL), 215000L, buf);
    if (len > 0) Serial.write(buf, (size_t)len);
    delay(1000);
}
```

---

## 线协议格式

所有帧都遵循 OpenSynaptic FULL 包格式：

```text
[cmd:1][route:1][aid:4BE][tid:1][ts:6BE][sid|unit|b62][crc8:1][crc16:2]
```

| 字段 | 大小 | 说明 |
|---|---|---|
| `cmd` | 1 B | `0x3F`，即 FULL 数据帧 |
| `route` | 1 B | 路由标志 |
| `aid` | 4 B | agent ID，大端 |
| `tid` | 1 B | 事务 ID |
| `ts` | 6 B | Unix 时间戳秒，大端 |
| body | 可变 | `sid|unit|b62value` |
| `crc8` | 1 B | body 的 CRC-8/SMBUS |
| `crc16` | 2 B | 整帧 CRC-16/CCITT-FALSE |

最小帧长约为 **21 字节**。

---

## 示例

| 示例 | API | 传输 | 目标 |
|---|---|---|---|
| [BasicTX](examples/BasicTX/BasicTX.ino) | C | Serial (UART) | 任意 Arduino |
| [MultiSensorTX](examples/MultiSensorTX/MultiSensorTX.ino) | C | Serial (UART) | 任意 Arduino，3 传感器 |
| [ESP32UdpTX](examples/ESP32UdpTX/ESP32UdpTX.ino) | C | WiFi UDP | ESP32 / ESP8266 |
| [LoRaTX](examples/LoRaTX/LoRaTX.ino) | C | LoRa SX1276 | Heltec / TTGO / Uno + shield |
| [BareMetalUARTTX](examples/BareMetalUARTTX/BareMetalUARTTX.ino) | C | USART0 寄存器 | ATmega328P |

---

## 仓库结构

```text
OSynaptic-TX/
├── OSynaptic-TX.h
├── library.properties
├── docs/
│   ├── 01-deployment-guide.md
│   ├── 02-api-reference.md
│   └── 03-wire-format.md
├── src/
│   ├── ostx_config.h
│   ├── ostx_types.h
│   ├── ostx_crc.h/c
│   ├── ostx_b62.h/c
│   ├── ostx_packet.h/c
│   ├── ostx_sensor.h/c
│   ├── ostx_static.h/c
│   └── ostx_stream.h/c
├── include/
├── examples/
├── tests/
└── CMakeLists.txt
```

---

## CMake 构建

需要 CMake ≥ 3.13 和支持 C89 的编译器：

```powershell
cmake -B build -DCMAKE_BUILD_TYPE=MinSizeRel
cmake --build build
```

---

## 测试结果

运行内置测试：

```sh
cmake -B build -DOSTX_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

测试覆盖：CRC-8、CRC-16、Base62 编码、帧字节序、边界条件，总计 **50 条断言**，预期结果为 **50 passed, 0 failed**。

---

## 文档

完整文档位于 [docs/](docs/)：

| 文件 | 内容 |
|---|---|
| [docs/01-deployment-guide.md](docs/01-deployment-guide.md) | MCU 容量表、传输选择、占空比建议 |
| [docs/02-api-reference.md](docs/02-api-reference.md) | 三档 API 的完整说明 |
| [docs/03-wire-format.md](docs/03-wire-format.md) | 线协议格式、CRC、Base62 |
| [docs/04-mcu-config-reference.md](docs/04-mcu-config-reference.md) | 按 MCU 的配置档位与宏推荐 |
| [docs/05-unit-validation.md](docs/05-unit-validation.md) | 单位合法性、SI 前缀与 `OSTX_UNIT()` 规则 |

---

## 贡献

提交 PR 前请阅读 [CONTRIBUTING.md](CONTRIBUTING.md) 和 [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md)。

---

## 许可证

Apache License 2.0。详见 [LICENSE](LICENSE)。