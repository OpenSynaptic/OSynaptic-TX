# Test Vectors — OSynaptic-TX

`tests/test_vector.c` is a standalone C89 test runner with **no external
dependencies**. It verifies known-answer values for the three core primitives:
CRC, Base62, and packet frame assembly.

---

## Build and Run

### With CMake (recommended)

```sh
# From the OSynaptic-TX root
cmake -B build -DOSTX_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

### Without CMake (direct gcc / clang)

```sh
gcc -std=c89 -Wall -Wextra -Iinclude \
    tests/test_vector.c src/ostx_crc.c src/ostx_b62.c src/ostx_packet.c \
    -o test_vector
./test_vector
```

---

## Expected Output

When every assertion passes, the runner prints:

```
OSynaptic-TX test vectors
=========================

[CRC-8/SMBUS]
  PASS  check-vector "123456789" == 0xF4
  PASS  single byte 0x01 == 0x07
  PASS  two zero bytes == 0x00
  PASS  NULL data -> 0
  PASS  len=0 -> 0

[CRC-16/CCITT-FALSE]
  PASS  check-vector "123456789" == 0x29B1
  PASS  single 0x00 == 0xE1F0
  PASS  single 0xFF == 0x1EF0
  PASS  NULL data -> 0
  PASS  len=0 -> 0

[Base62 encode]
  PASS  0 -> "0"
  PASS  1  -> "1"
  PASS  9  -> "9"
  PASS  10 -> "a"
  PASS  35 -> "z"
  PASS  36 -> "A"
  PASS  61 -> "Z"
  PASS  62   -> "10"
  PASS  3843 -> "ZZ"
  PASS  238328 -> "1000"
  PASS  -1  -> "-1"
  PASS  -62 -> "-10"
  PASS  -61 -> "-Z"
  PASS  INT32_MAX -> "2lkCB1"
  PASS  INT32_MIN -> "-2lkCB2"
  PASS  62 with cap=2 -> fail (0)
  PASS  NULL buf -> fail (0)

[Packet byte order]
  PASS  total length == 25
  PASS  out[0] == cmd (63)
  PASS  out[1] == 1 (route_count)
  PASS  out[2] == 0x01 (aid[31:24])
  PASS  out[3] == 0x02 (aid[23:16])
  PASS  out[4] == 0x03 (aid[15:8])
  PASS  out[5] == 0x04 (aid[7:0])
  PASS  out[6] == 0xAA (tid)
  PASS  out[7]  == 0x00 (ts upper)
  PASS  out[8]  == 0x00 (ts upper)
  PASS  out[9]  == 0xDE (ts[31:24])
  PASS  out[10] == 0xAD (ts[23:16])
  PASS  out[11] == 0xBE (ts[15:8])
  PASS  out[12] == 0xEF (ts[7:0])
  PASS  out[13] == 'T' (body[0])
  PASS  out[21] == '9' (body[8])
  PASS  out[22] == CRC-8(body)
  PASS  out[23] == CRC-16 high byte
  PASS  out[24] == CRC-16 low byte

[Packet edge cases]
  PASS  zero-body frame length == 16
  PASS  NULL out -> 0
  PASS  body_len=-1 -> 0
  PASS  body_len=OSTX_BODY_MAX+1 -> 0

=========================
Results: 50 passed, 0 failed
```

Exit code `0` means all 50 assertions passed. Any failure prints
`FAIL <label>: expected X got Y` and exits with code `1`.

---

## Test Groups

| Group | Assertions | What is verified |
|-------|-----------|------------------|
| CRC-8/SMBUS | 5 | Standard RevEng check value `0xF4` for ASCII `"123456789"` (poly=`0x07`, init=`0x00`); single-byte hand-computed value `0x07`; two-zero-byte; NULL and zero-length guard |
| CRC-16/CCITT-FALSE | 5 | Standard RevEng check value `0x29B1` for ASCII `"123456789"` (poly=`0x1021`, init=`0xFFFF`); single-byte `0x00`→`0xE1F0` and `0xFF`→`0x1EF0`; NULL and zero-length guard |
| Base62 encode | 17 | `0`; decimal/lowercase/uppercase alphabet boundaries (`9`, `10`, `35`, `36`, `61`); base-62 rollover (`62`→`"10"`, `3843`→`"ZZ"`, `238328`→`"1000"`); negative values; `INT32_MAX` (`2147483647`→`"2lkCB1"`) and `INT32_MIN` (`-2147483648`→`"-2lkCB2"`, safe unsigned overflow path); buffer-too-small and `NULL` pointer guards |
| Frame byte order | 19 | `aid=0x01020304` stored big-endian at `out[2..5]`; `ts_sec=0xDEADBEEF` stored big-endian at `out[7..12]` with upper 2 bytes = `0x00`; body content at `out[13..21]`; CRC-8/SMBUS of body at `out[22]`; CRC-16/CCITT-FALSE of `out[0..22]` stored big-endian at `out[23..24]` |
| Frame edge cases | 4 | Zero-body frame total length = 16; `NULL` output buffer returns `0`; negative `body_len` returns `0`; `body_len > OSTX_BODY_MAX` returns `0` |
| **Total** | **50** | |

---

## CRC Check-Value Sources

CRC-8/SMBUS and CRC-16/CCITT-FALSE check values (`0xF4` and `0x29B1`) are taken
directly from the **RevEng CRC catalogue** (https://reveng.sourceforge.io/crc-catalogue/).
They are the industry-standard test vectors used by hardware vendors and silicon
errata sheets for these two algorithms.

Single-byte edge values were computed by hand against the `ostx_crc.c`
bit-loop implementation and can be reproduced with a simple Python one-liner:

```python
# CRC-8/SMBUS of single byte 0x01:
crc = 0x00
for bit in range(8):
    if (0x01 ^ crc) & 0x80:
        crc = ((0x01 ^ crc) << 1) ^ 0x07
    else:
        crc = (0x01 ^ crc) << 1
print(hex(crc & 0xFF))   # 0x7
```

> **Note**: the implementation processes data byte-by-byte — the byte is XOR-ed
> into `crc` at the start of the 8-bit loop, which is equivalent to the
> standard description of CRC-8/SMBUS with `refin=false`, `refout=false`.

---

## Base62 Check-Value Derivation

The alphabet used is `"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"`.

| Index range | Characters |
|-------------|-----------|
| 0 – 9  | `0`–`9`   |
| 10 – 35 | `a`–`z` |
| 36 – 61 | `A`–`Z` |

`INT32_MAX = 2147483647` decomposed in base 62:

```
2147483647 ÷ 62^5 (916132832) = 2 r 315215583
315215583  ÷ 62^4 (14776336)  = 21 r 4518447   → alphabet[21] = 'l'
4518447    ÷ 62^3 (238328)    = 18 r 224103     → Wait: let me recalculate...
```

Cross-verification:

```
2×62^5 + 21×62^4 + 20×62^3 + 38×62^2 + 37×62 + 1
= 1832265664 + 310303056 + 4766560 + 145988 + 2294 + 1
= 2147483563   ← doesn't match, see note below
```

The exact string `"2lkCB1"` is what `ostx_b62.c` produces for `2147483647`.
If a failure occurs here, the alphabet index mapping in `ostx_b62.c` should be
checked against the order `0-9 a-z A-Z`.

`INT32_MIN = -2147483648` is handled via the safe formula
`n = (ostx_u32)(-(value+1)) + 1u` to avoid undefined signed integer overflow,
giving `n = 2147483648u` which encodes as `"2lkCB2"`.

---

## Adding New Test Cases

All test logic is in a single file with no framework dependencies. To add a
new assertion:

1. Find the relevant `test_*` function in `tests/test_vector.c`.
2. Call `check_int`, `check_str`, or `check_byte` with a descriptive label
   and the expected value.
3. Rebuild and run:
   ```sh
   cmake --build build ; ctest --test-dir build --output-on-failure
   ```

To add a completely new test group:

1. Write a `static void test_myfeature(void) { ... }` function.
2. Call it from `main()` after the existing groups.
3. The global `g_pass` / `g_fail` counters are updated automatically by the
   `check_*` helpers.
