# 05 — Unit Validation & OpenSynaptic Unit Standard

This document describes the compile-time unit validation mechanism in OSynaptic-TX and the complete list of valid OpenSynaptic unit codes.

---

## 1. Why Unit Validation Matters

The OpenSynaptic wire format encodes the unit string directly into every frame body:

```
body := <sensor_id> "|" <unit> "|" <base62_value>
```

An invalid unit string (e.g. `"Celsius"` instead of `"Cel"`, or `"C"` instead of `"Cel"`) produces a frame whose `unit` field is not recognised by any OpenSynaptic-compliant receiver. The frame is syntactically valid (CRC passes) but semantically undefined — the server may reject it, log an error, or silently discard the reading.

OSynaptic-TX catches this class of error **at compile time**, before the firmware is ever flashed.

---

## 2. Mechanism: Preprocessor Token Pasting (C89)

`src/ostx_units.h` defines every valid unit as a preprocessor macro of the form:

```c
#define OSTX_UNIT_<suffix>  "<wire_code>"
```

and a single convenience macro:

```c
#define OSTX_UNIT(sym)  OSTX_UNIT_##sym
```

The `##` operator concatenates the literal token `sym` onto `OSTX_UNIT_` at preprocessing time. If the resulting macro name is not defined in `ostx_units.h`, the C preprocessor leaves it as an undefined identifier, and the compiler raises an error:

```
error: 'OSTX_UNIT_Celsius' undeclared (first use in this function)
```

Modern IDEs (VS Code + clangd, CLion, Arduino IDE 2.x) evaluate the preprocessor before displaying diagnostics, so the token is **underlined in red** as you type.

### Flow diagram

```
Source code                  Preprocessor              Compiler
──────────────────────────   ──────────────────────   ─────────────────
OSTX_UNIT(Cel)           →   OSTX_UNIT_Cel        →   "Cel"   ✓ OK
OSTX_UNIT(Celsius)       →   OSTX_UNIT_Celsius    →   undeclared  ✗ ERROR
```

### Properties

| Property | Value |
|----------|-------|
| C standard | C89 / ANSI C — no compiler extensions required |
| Runtime cost | Zero — units are string literals, resolved at compile time |
| Flash cost | Zero additional — identical to writing `"Cel"` directly |
| Toolchain support | avr-gcc, arm-none-eabi-gcc, SDCC, XC8, MSVC, Clang, GCC |
| IDE inline error | Yes, with any clangd-based LSP or IDE that parses headers |

---

## 3. Usage

### Basic usage

```c
#include <OSynaptic-TX.h>   /* already includes ostx_units.h */

/* API C — streaming */
ostx_stream_pack(&ctx, "T1", OSTX_UNIT(Cel),  2150, buf, sizeof(buf));
ostx_stream_pack(&ctx, "H1", OSTX_UNIT(pct),  6500, buf, sizeof(buf));
ostx_stream_pack(&ctx, "P1", OSTX_UNIT(Pa),   101325, buf, sizeof(buf));
```

### Direct constant (no macro indirection)

```c
/* Equivalent — use when you already know the suffix */
ostx_stream_pack(&ctx, "T1", OSTX_UNIT_Cel, 2150, buf, sizeof(buf));
```

### Storing unit in a variable

`OSTX_UNIT(sym)` **requires a literal token** — it cannot expand a variable. For runtime-selected units, store the wire string directly:

```c
const char *unit = OSTX_UNIT_Cel;   /* checked at compile time */
ostx_stream_pack(&ctx, "T1", unit, value, buf, sizeof(buf));
```

### Wrong usage that triggers the error

```c
/* All of the following fail to compile: */
OSTX_UNIT(Celsius)    /* should be Cel     */
OSTX_UNIT(celsius)    /* case-sensitive    */
OSTX_UNIT(°C)         /* non-ASCII         */
OSTX_UNIT(C)          /* ambiguous: use Cel (Celsius) or A (ampere)? */
OSTX_UNIT(percent)    /* should be pct     */
OSTX_UNIT(degreeF)    /* should be degF    */
```

---

## 4. Unit Code Reference

All codes match the OpenSynaptic UCUM unit library v1.1.0.  
The **Macro suffix** column is the token to pass to `OSTX_UNIT()`.  
The **Wire code** column is the exact string written into the frame body.

### 4.1 Length (class 0x01)

| Macro suffix | Wire code | Name | SI prefix |
|-------------|-----------|------|-----------|
| `m` | `m` | meter | ✓ |
| `in` | `in` | inch | — |
| `ft` | `ft` | foot | — |
| `nmi` | `nmi` | nautical mile | — |
| `AU` | `AU` | astronomical unit | — |
| `ang` | `ang` | angstrom | — |

### 4.2 Mass (class 0x02)

| Macro suffix | Wire code | Name | SI prefix |
|-------------|-----------|------|-----------|
| `g` | `g` | gram | ✓ |
| `lb` | `lb` | pound | — |
| `oz` | `oz` | ounce | — |
| `t` | `t` | metric ton | ✓ |
| `u` | `u` | unified atomic mass unit | — |

### 4.3 Time (class 0x03)

| Macro suffix | Wire code | Name | SI prefix |
|-------------|-----------|------|-----------|
| `s` | `s` | second | ✓ |
| `min` | `min` | minute | — |
| `h` | `h` | hour | — |
| `d` | `d` | day | — |
| `wk` | `wk` | week | — |
| `ann` | `ann` | year | ✓ |

### 4.4 Temperature (class 0x04)

| Macro suffix | Wire code | Name | SI prefix |
|-------------|-----------|------|-----------|
| `K` | `K` | kelvin | ✓ |
| `Cel` | `Cel` | degree Celsius | — |
| `degF` | `degF` | degree Fahrenheit | — |
| `degRe` | `degRe` | degree Réaumur | — |

> **Most common**: `OSTX_UNIT(Cel)` for temperature sensors, `OSTX_UNIT(K)` for scientific/cryogenic.

### 4.5 Electric Current (class 0x05)

| Macro suffix | Wire code | Name | SI prefix |
|-------------|-----------|------|-----------|
| `A` | `A` | ampere | ✓ |
| `Bi` | `Bi` | biot | ✓ |
| `Gau` | `Gau` | gauss unit | — |

### 4.6 Amount of Substance (class 0x06)

| Macro suffix | Wire code | Name | SI prefix |
|-------------|-----------|------|-----------|
| `mol` | `mol` | mole | ✓ |
| `eq` | `eq` | equivalents | ✓ |
| `osm` | `osm` | osmole | ✓ |
| `count` | `count` | particle count | — |

### 4.7 Luminous Intensity (class 0x07)

| Macro suffix | Wire code | Name | SI prefix |
|-------------|-----------|------|-----------|
| `cd` | `cd` | candela | ✓ |
| `cp` | `cp` | candlepower | — |
| `hk` | `hk` | hefnerkerze | — |

### 4.8 Pressure (class 0x08)

| Macro suffix | Wire code | Name | SI prefix |
|-------------|-----------|------|-----------|
| `Pa` | `Pa` | pascal | ✓ |
| `bar` | `bar` | bar | ✓ |
| `psi` | `psi` | pound-force per square inch | — |
| `atm` | `atm` | standard atmosphere | — |
| `mmHg` | `mm[Hg]` | millimeter of mercury | — |

> Note: `mmHg` maps to wire code `mm[Hg]`. The brackets are in the wire format only.

### 4.9 Frequency / Angular Rate (class 0x09)

| Macro suffix | Wire code | Name | SI prefix |
|-------------|-----------|------|-----------|
| `Hz` | `Hz` | hertz | ✓ |
| `rpm` | `rpm` | revolutions per minute | — |
| `deg_s` | `deg/s` | degrees per second | — |
| `rad_s` | `rad/s` | radians per second | — |

> Note: `/` in wire codes is replaced by `_` in macro suffixes.

### 4.10 Energy / Power (class 0x0A)

| Macro suffix | Wire code | Name | SI prefix |
|-------------|-----------|------|-----------|
| `W` | `W` | watt | ✓ |
| `J` | `J` | joule | ✓ |
| `cal` | `cal` | calorie (International Table) | ✓ |
| `hp` | `hp` | horsepower (metric) | — |

### 4.11 Electromagnetism (class 0x0B)

| Macro suffix | Wire code | Name | SI prefix |
|-------------|-----------|------|-----------|
| `V` | `V` | volt | ✓ |
| `Ohm` | `Ohm` | ohm | ✓ |
| `F` | `F` | farad | ✓ |

### 4.12 Informatics (class 0x0C)

| Macro suffix | Wire code | Name | SI prefix |
|-------------|-----------|------|-----------|
| `bit` | `bit` | bit | ✓ |
| `By` | `By` | byte | ✓ |
| `Bd` | `Bd` | baud | ✓ |

### 4.13 Humidity (class 0x0D)

| Macro suffix | Wire code | Name |
|-------------|-----------|------|
| `pct` | `%` | relative humidity percent |
| `RH` | `%` | alias — same wire code |

> Both `OSTX_UNIT(pct)` and `OSTX_UNIT(RH)` produce `"%"`.

### 4.14 Device Operations (class 0x0E)

Used in command frames (not sensor readings). The `.` in wire codes is replaced by `_` in macro suffixes.

#### Standard operations

| Macro suffix | Wire code | Description |
|-------------|-----------|-------------|
| `cmd` | `cmd` | Generic raw command |
| `pow_on` | `pow.on` | Power on |
| `pow_off` | `pow.off` | Power off |
| `set_val` | `set.val` | Write parameter value |
| `get_val` | `get.val` | Read parameter value |
| `get_st` | `get.st` | Query device status |
| `rst` | `rst` | Reset / reboot |

#### Motion control

| Macro suffix | Wire code | Description |
|-------------|-----------|-------------|
| `mv_up` | `mv.up` | Move up |
| `mv_dn` | `mv.dn` | Move down |
| `mv_lt` | `mv.lt` | Move left |
| `mv_rt` | `mv.rt` | Move right |
| `mv_fw` | `mv.fw` | Move forward |
| `mv_bk` | `mv.bk` | Move backward |
| `stp` | `stp` | Stop |
| `stp_e` | `stp.e` | Emergency stop |
| `mv_to` | `mv.to` | Move to absolute position (requires value) |
| `mv_by` | `mv.by` | Move by relative offset (requires value) |
| `rot_cw` | `rot.cw` | Rotate clockwise |
| `rot_cc` | `rot.cc` | Rotate counter-clockwise |

#### Custom slots

26 custom command slots (`cmdA`–`cmdZ`) and 26 mode switch slots (`modeA`–`modeZ`) are reserved for application-specific use. Their wire codes match the suffix exactly.

---

## 5. Non-identifier Character Mapping Rules

| Character in wire code | Macro suffix replacement | Example |
|------------------------|--------------------------|---------|
| `/` | `_` | `deg/s` → `deg_s` |
| `[` `]` | removed | `mm[Hg]` → `mmHg` |
| `.` | `_` | `pow.on` → `pow_on` |
| `%` | renamed to `pct` / `RH` | `%` → `pct` |

---

## 6. SI Prefix Handling

Some units support SI prefixes (marked ✓ in the tables above). OSynaptic-TX does **not** apply SI prefixes automatically. If you need `mA` (milliamperes), transmit the value scaled to base units and use the base unit code:

```c
/* Current sensor reading: 4.5 mA */
/* Scale to base unit (A): 4.5 mA = 0.0045 A */
/* With OSTX_VALUE_SCALE = 10000L: 0.0045 × 10000 = 45 */
ostx_stream_pack(&ctx, "I1", OSTX_UNIT(A), 45L, buf, sizeof(buf));
```

The server reconstructs `0.0045 A` from the Base62 value and the scale factor embedded in the session registration. Alternatively, use `OSTX_VALUE_SCALE = 1000000L` if milliampere precision is required in the value field.

---

## 7. Custom Units — Limitations

### What happens on a standard OpenSynaptic server

When the server receives a frame whose `unit` field is not in its registered unit library, the following occurs:

| Field | Server behaviour |
|-------|-----------------|
| Raw value | Stored as-is in the raw frame log |
| Displayed value | **N/A** or `--` — cannot convert without a known scale factor |
| Physical quantity | Unknown — chart axis label undefined |
| Alarm rules | Cannot be bound — threshold unit mismatch |
| Historical plot | Not drawable — Y-axis unit undefined |

The frame itself is **not rejected** — the CRC is valid and the frame is structurally correct. However, the reading is **semantically dead** at the application layer on any server that has not been explicitly extended to support the custom code.

### When custom units are acceptable

| Scenario | Safe to use custom unit? |
|----------|--------------------------|
| Standard OpenSynaptic server, unmodified | **No** — value shows as N/A |
| Private deployment with server-side unit registry extended | Yes, if unit is registered before data arrives |
| Local logging only (no server decode) | Yes — raw frames are preserved |
| Protocol testing / development | Yes — with awareness of the limitation |

### Correct approach for non-standard measurements

**Option A — Map to the nearest standard unit.**  
Soil volumetric water content is dimensionless (volume/volume); use `%` (`OSTX_UNIT(pct)`) and document the semantic in the sensor ID naming convention (e.g. sensor ID `"SWC1"`).

**Option B — Request addition to the OpenSynaptic unit library.**  
Submit a unit definition to the OpenSynaptic protocol repository. Once merged, the unit will have an official `class_id` and `tid`, and all conforming servers will recognise it automatically.

**Option C — Server-side custom unit registration (advanced).**  
If you operate a private OpenSynaptic server and have extended the unit registry, you may define a project-local macro to keep the compile-time validation benefit:

```c
/* myproject_units.h — only valid with a custom-configured server */
#include <OSynaptic-TX.h>

#define OSTX_UNIT_VWC   "VWC"   /* registered in private server unit DB */
```

```c
ostx_stream_pack(&ctx, "SW1", OSTX_UNIT(VWC), value, buf, sizeof(buf));
```

> **Do not ship firmware using custom unit codes against a public or unmodified OpenSynaptic server.** The data will be stored but will display as N/A and cannot be used in rules, dashboards, or exports.
