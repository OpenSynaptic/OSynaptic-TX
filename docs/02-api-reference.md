# 02 — API Reference

Complete reference for all three OSynaptic-TX encoding APIs. All APIs produce byte-for-byte identical wire frames.

---

## Compile-time Configuration (`ostx_config.h`)

Override any constant before including any OSynaptic-TX header, either via your build system (`-DOSTX_ID_MAX=12`) or a project-local header.

| Macro | Default | Description |
|-------|---------|-------------|
| `OSTX_ID_MAX` | `9` | Max sensor ID length **including NUL byte**. `"T1\0"` = 3. |
| `OSTX_UNIT_MAX` | `9` | Max unit string length **including NUL byte**. `"Cel\0"` = 4. |
| `OSTX_B62_MAX` | `14` | Max Base62 encoded scalar length **including NUL**. 32-bit signed int encodes to ≤ 7 chars. |
| `OSTX_BODY_MAX` | `64` | Max body byte count = `ID + '|' + unit + '|' + b62`. |
| `OSTX_PACKET_MAX` | `96` | Max wire packet byte count = 13 (header) + body + 3 (CRC). |
| `OSTX_VALUE_SCALE` | `10000L` | Integer scale factor. `21.5 °C → 215000L`. Use `100L` to reduce b62 length. |
| `OSTX_CMD_DATA_FULL` | `63` | Wire command byte for FULL data packets (OpenSynaptic protocol v1). Do not change. |

---

## Portable Types (`ostx_types.h`)

| Type | C89 definition | Width |
|------|---------------|-------|
| `ostx_u8` | `unsigned char` | 8-bit |
| `ostx_u16` | `unsigned short` (or `unsigned int` on 8-bit) | 16-bit |
| `ostx_u32` | `unsigned long` | 32-bit |
| `ostx_i32` | `signed long` | 32-bit signed |

---

## API A — Dynamic (`ostx_sensor.h` / `ostx_sensor.c`)

### `ostx_sensor_pack()`

```c
int ostx_sensor_pack(
    ostx_u32    aid,
    const char *sensor_id,
    const char *unit,
    ostx_u8     tid,
    ostx_u32    ts_sec,
    ostx_i32    scaled_value,
    ostx_u8    *out
);
```

**Parameters**:

| Parameter | Description |
|-----------|-------------|
| `aid` | 32-bit agent ID (big-endian in wire frame). Must match the ID registered on the OpenSynaptic server. |
| `sensor_id` | Null-terminated sensor identifier string. Max length `OSTX_ID_MAX - 1` characters. |
| `unit` | Null-terminated unit string. Max length `OSTX_UNIT_MAX - 1` characters. |
| `tid` | Transaction ID (0–255). Caller is responsible for incrementing. Wraps on overflow. |
| `ts_sec` | Unix timestamp (seconds since epoch). On MCUs without RTC, use `millis() / 1000`. |
| `scaled_value` | Sensor value pre-multiplied by `OSTX_VALUE_SCALE`. E.g. 21.5 °C → `215000L`. |
| `out` | Output buffer. Must be at least `OSTX_PACKET_MAX` bytes. |

**Returns**: frame length in bytes (> 0) on success; negative error code on failure.

**Error codes**:

| Code | Meaning |
|------|---------|
| `-1` | `sensor_id` is NULL or empty |
| `-2` | `sensor_id` exceeds `OSTX_ID_MAX - 1` |
| `-3` | `unit` is NULL or empty |
| `-4` | `unit` exceeds `OSTX_UNIT_MAX - 1` |
| `-5` | `out` is NULL |
| `-6` | Body too large for `OSTX_BODY_MAX` |

**Stack usage (AVR)**: ~137 B (includes `body[OSTX_BODY_MAX]` = 64 B and `b62[OSTX_B62_MAX]` = 14 B).

**Static RAM**: 96 B scratchpad (shared with API B, allocated once as static BSS).

**Example**:
```c
ostx_u8 buf[OSTX_PACKET_MAX];
int len = ostx_sensor_pack(0x00000001UL, "T1", "Cel",
                           tid++, ts_sec, 215000L, buf);
if (len > 0) uart_send(buf, len);
```

---

## API B — Static (`ostx_static.h` / `ostx_static.c`)

### `OSTX_STATIC_DEFINE(varname, aid32, sid_str, unit_str)`

Declare a compile-time sensor descriptor. Place at **file scope** (not inside a function).

```c
OSTX_STATIC_DEFINE(s_temp, 0x00000001UL, "T1", "Cel");
```

**Expands to**:
- `static const ostx_u8 s_temp__hdr[13]` — 13-byte header template with AID pre-encoded. Lives in `.rodata` / Flash on AVR Harvard architectures.
- `static const char s_temp__pfx[]` — `"T1|Cel|"` zero-terminated prefix string. Flash.
- `const OSTXStaticSensor s_temp` — struct referencing the two above. Flash.

**Flash cost per descriptor**: ~27 bytes (13 header + prefix string length + struct pointers).

**RAM cost**: 0 bytes (all fields are `const`).

### `ostx_static_pack()`

```c
int ostx_static_pack(
    const OSTXStaticSensor *sensor,
    ostx_u8                 tid,
    ostx_u32                ts_sec,
    ostx_i32                scaled_value,
    ostx_u8                *out
);
```

**Parameters**:

| Parameter | Description |
|-----------|-------------|
| `sensor` | Pointer to descriptor created by `OSTX_STATIC_DEFINE`. |
| `tid` | Transaction ID (0–255). |
| `ts_sec` | Unix timestamp seconds. |
| `scaled_value` | Pre-scaled sensor value. |
| `out` | Output buffer, ≥ `OSTX_PACKET_MAX` bytes. |

**Returns**: frame length on success; -1 if `sensor` or `out` is NULL.

**Stack usage (AVR)**: ~51 B.

**Example**:
```c
OSTX_STATIC_DEFINE(s_temp, 0x00000001UL, "T1", "Cel");

ostx_u8 buf[OSTX_PACKET_MAX];
int len = ostx_static_pack(&s_temp, tid++, ts_sec, 215000L, buf);
if (len > 0) uart_send(buf, len);
```

---

## API C — Streaming (`ostx_stream.h` / `ostx_stream.c`)

### `ostx_emit_fn` callback type

```c
typedef void (*ostx_emit_fn)(ostx_u8 byte, void *ctx);
```

User-supplied callback that receives one byte at a time. Called synchronously inside `ostx_stream_pack()`. Must not be NULL.

`ctx` is the opaque pointer forwarded unchanged from `ostx_stream_pack()`. Pass NULL if not needed.

**Typical implementations**:

```c
/* Arduino UART */
static void serial_emit(ostx_u8 b, void *ctx) { Serial.write(b); }

/* AVR bare-metal UART (blocking) */
static void uart_emit(ostx_u8 b, void *ctx) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = b;
}

/* Buffer accumulator (for LoRa, nRF24, etc.) */
static uint8_t g_buf[OSTX_PACKET_MAX];
static int     g_len = 0;
static void buf_emit(ostx_u8 b, void *ctx) {
    if (g_len < OSTX_PACKET_MAX) g_buf[g_len++] = b;
}
```

### `ostx_stream_pack()`

```c
int ostx_stream_pack(
    const OSTXStaticSensor *sensor,
    ostx_u8                 tid,
    ostx_u32                ts_sec,
    ostx_i32                scaled_value,
    ostx_emit_fn            emit,
    void                   *ctx
);
```

**Parameters**:

| Parameter | Description |
|-----------|-------------|
| `sensor` | Pointer to descriptor created by `OSTX_STATIC_DEFINE`. |
| `tid` | Transaction ID (0–255). |
| `ts_sec` | Unix timestamp seconds. |
| `scaled_value` | Pre-scaled sensor value. |
| `emit` | Byte-sink callback. Must not be NULL. |
| `ctx` | Opaque pointer forwarded to every `emit()` call. May be NULL. |

**Returns**: total bytes emitted (> 0) on success; -1 if `sensor` or `emit` is NULL.

**Stack usage (AVR)**: ~21 B (function locals only — no body buffer, no b62 scratch).

**Static RAM**: 0 B.

**How it works internally**:
1. Emits header bytes (cmd, route, aid, tid, ts) one by one, accumulating CRC-8 over the body range and CRC-16 over all.
2. Emits the `body_pfx` string (`"sid|unit|"`) byte by byte.
3. Converts `scaled_value` to Base62 using a `static const ostx_u32 P62[6]` powers-of-62 table (MSB-first, no reversal buffer).
4. Emits CRC-8 (1 byte) then CRC-16 (2 bytes big-endian).

---

## Shared: `OSTXStaticSensor` struct

```c
typedef struct {
    const ostx_u8 *hdr;          /* 13-byte header template */
    const char    *body_pfx;     /* "sid|unit|" NUL-terminated */
    int            body_pfx_len; /* strlen(body_pfx) */
} OSTXStaticSensor;
```

Always created via `OSTX_STATIC_DEFINE` — do not initialise manually.

---

## CRC functions (`ostx_crc.h`)

These are low-level helpers used internally. Available to callers for custom framing or verification.

```c
/* CRC-8/SMBUS: poly=0x07, init=0x00 */
ostx_u8  ostx_crc8_update(ostx_u8 crc, ostx_u8 byte);

/* CRC-16/CCITT-FALSE: poly=0x1021, init=0xFFFF */
ostx_u16 ostx_crc16_update(ostx_u16 crc, ostx_u8 byte);
```

Both are bit-loop implementations (no lookup table). Each call processes one byte.

**Usage**:
```c
ostx_u8  crc8  = 0x00u;
ostx_u16 crc16 = 0xFFFFu;
for (i = 0; i < frame_len - 2; i++) {
    crc16 = ostx_crc16_update(crc16, frame[i]);
}
/* CRC-16 covers all bytes except the final 2 */
```

---

## Base62 encoder (`ostx_b62.h`)

Available for custom use (e.g. encoding device IDs, not just sensor values).

```c
/* Encode a signed 32-bit integer into Base62.
 * out must be at least OSTX_B62_MAX bytes.
 * Returns the number of characters written (excluding NUL), or -1 on error.
 * Negative values are prefixed with '-'. */
int ostx_b62_encode(ostx_i32 value, char *out);
```

Base62 alphabet: `0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ`

Scale example:
- 21.50 °C → `215000L` → `"OhBu"` (4 chars)
- -5.25 °C → `-52500L` → `"-Dcm"` (4 chars + sign)

---

## Value Scaling Reference

| Real value | Unit | `OSTX_VALUE_SCALE=10000` | b62 chars | `OSTX_VALUE_SCALE=100` | b62 chars |
|------------|------|--------------------------|-----------|------------------------|-----------|
| 21.5 °C | Cel | 215000 | 4 | 2150 | 3 |
| -5.25 °C | Cel | -52500 | 4+sign | -525 | 3+sign |
| 61.2 %RH | Pct | 612000 | 4 | 6120 | 3 |
| 1013.25 hPa | hPa | 10132500 | 5 | 101325 | 4 |
| 0.001 V | V | 10 | 1 | 0 → clipped | — |

> **Tip**: choose `OSTX_VALUE_SCALE=100` on tight ATtiny targets to reduce body length by 1–2 bytes; you lose two decimal places of resolution.
