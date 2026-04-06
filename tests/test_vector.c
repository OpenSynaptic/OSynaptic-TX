/*
 * test_vector.c -- Known-answer tests for OSynaptic-TX core primitives.
 *
 * Tests three areas that cannot be validated by "compile-succeeds" alone:
 *
 *   1. CRC-8/SMBUS  (poly=0x07, init=0x00)
 *   2. CRC-16/CCITT-FALSE  (poly=0x1021, init=0xFFFF)
 *   3. Base62 encoding boundary values
 *   4. Wire frame byte order (aid and timestamp big-endian fields)
 *
 * Build (native host):
 *   gcc -std=c89 -Wall -Wextra -Iinclude tests/test_vector.c \
 *       src/ostx_crc.c src/ostx_b62.c src/ostx_packet.c -o test_vector
 *   ./test_vector
 *
 * Exit 0 = all tests passed.  Any failure prints a diagnostic and exits 1.
 *
 * C89 -- no <stdbool.h>, no VLAs, no // comments, no named struct inits.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ostx_crc.h"
#include "ostx_b62.h"
#include "ostx_packet.h"
#include "ostx_config.h"

/* -------------------------------------------------------------------------
 * Minimal test framework
 * ---------------------------------------------------------------------- */

static int g_pass = 0;
static int g_fail = 0;

static void check_int(const char *label, long got, long expected)
{
    if (got == expected) {
        printf("  PASS  %s\n", label);
        ++g_pass;
    } else {
        printf("  FAIL  %s: expected 0x%lX (%ld) got 0x%lX (%ld)\n",
               label, expected, expected, got, got);
        ++g_fail;
    }
}

static void check_str(const char *label, const char *got, const char *expected)
{
    if (strcmp(got, expected) == 0) {
        printf("  PASS  %s\n", label);
        ++g_pass;
    } else {
        printf("  FAIL  %s: expected \"%s\" got \"%s\"\n",
               label, expected, got);
        ++g_fail;
    }
}

static void check_byte(const char *label, unsigned char got, unsigned char expected)
{
    if (got == expected) {
        printf("  PASS  %s\n", label);
        ++g_pass;
    } else {
        printf("  FAIL  %s: expected 0x%02X got 0x%02X\n",
               label, (unsigned)expected, (unsigned)got);
        ++g_fail;
    }
}

/* =========================================================================
 * 1.  CRC-8 / SMBUS
 *
 * Reference: CRC-8/SMBUS catalogue entry (poly=0x07, init=0x00, refin=false,
 *            refout=false, xorout=0x00).
 * Standard check value for "123456789" = 0xF4.
 * ========================================================================= */

static void test_crc8(void)
{
    /* Standard SMBUS check vector: ASCII "123456789" */
    static const ostx_u8 chk_seq[] =
        {0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39};

    /* Hand-computed: CRC-8/SMBUS of single byte 0x01.
     * crc = 0x01, shift 7 times LSB=0: 0x01->0x02->0x04->0x08->0x10->0x20->0x40->0x80
     * bit 7: 0x80 set -> crc = (0x80<<1) ^ 0x07 = 0x00 ^ 0x07 = 0x07 */
    static const ostx_u8 single_01[] = {0x01};

    /* Two-byte vector: {0x00, 0x00} -> CRC operates on two 0x00 bytes,
     * starting from init=0x00: result stays 0x00. */
    static const ostx_u8 two_zeros[] = {0x00, 0x00};

    printf("\n[CRC-8/SMBUS]\n");

    check_int("check-vector \"123456789\" == 0xF4",
              (long)ostx_crc8(chk_seq, 9, 0x07u, 0x00u), 0xF4L);

    check_int("single byte 0x01 == 0x07",
              (long)ostx_crc8(single_01, 1, 0x07u, 0x00u), 0x07L);

    check_int("two zero bytes == 0x00",
              (long)ostx_crc8(two_zeros, 2, 0x07u, 0x00u), 0x00L);

    check_int("NULL data -> 0",
              (long)ostx_crc8(NULL, 4, 0x07u, 0x00u), 0L);

    check_int("len=0 -> 0",
              (long)ostx_crc8(chk_seq, 0, 0x07u, 0x00u), 0L);
}

/* =========================================================================
 * 2.  CRC-16 / CCITT-FALSE
 *
 * Reference: CRC-16/CCITT-FALSE catalogue entry (poly=0x1021, init=0xFFFF,
 *            refin=false, refout=false, xorout=0x0000).
 * Standard check value for "123456789" = 0x29B1.
 * ========================================================================= */

static void test_crc16(void)
{
    static const ostx_u8 chk_seq[] =
        {0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39};

    /* Single zero byte: init=0xFFFF, xor 0x00 -> crc stays 0xFFFF, then
     * shift 8 times with poly 0x1021.
     * Bit-loop result: 0xE1F0  (precomputed). */
    static const ostx_u8 single_zero[] = {0x00};

    /* Single byte 0xFF: precomputed result = 0xFF00.
     * crc = 0xFFFF ^ (0xFF<<8) = 0x00FF, then 8 shifts all MSB=0:
     * 0x00FF -> 0x01FE -> ... -> 0xFF00. */
    static const ostx_u8 single_ff[] = {0xFF};

    printf("\n[CRC-16/CCITT-FALSE]\n");

    check_int("check-vector \"123456789\" == 0x29B1",
              (long)ostx_crc16(chk_seq, 9, 0x1021u, 0xFFFFu), 0x29B1L);

    check_int("single 0x00 == 0xE1F0",
              (long)ostx_crc16(single_zero, 1, 0x1021u, 0xFFFFu), 0xE1F0L);

    check_int("single 0xFF == 0xFF00",
              (long)ostx_crc16(single_ff, 1, 0x1021u, 0xFFFFu), 0xFF00L);

    check_int("NULL data -> 0",
              (long)ostx_crc16(NULL, 4, 0x1021u, 0xFFFFu), 0L);

    check_int("len=0 -> 0",
              (long)ostx_crc16(chk_seq, 0, 0x1021u, 0xFFFFu), 0L);
}

/* =========================================================================
 * 3.  Base62 encoding
 *
 * Alphabet: "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
 * Index:     0 -> '0', 9 -> '9', 10 -> 'a', 35 -> 'z', 36 -> 'A', 61 -> 'Z'
 * ========================================================================= */

static void test_b62(void)
{
    char buf[16];

    printf("\n[Base62 encode]\n");

    /* --- Zero --- */
    ostx_b62_encode(0, buf, sizeof(buf));
    check_str("0 -> \"0\"", buf, "0");

    /* --- Small positive values: alphabet boundary points --- */
    ostx_b62_encode(1,  buf, sizeof(buf)); check_str("1  -> \"1\"",  buf, "1");
    ostx_b62_encode(9,  buf, sizeof(buf)); check_str("9  -> \"9\"",  buf, "9");
    ostx_b62_encode(10, buf, sizeof(buf)); check_str("10 -> \"a\"",  buf, "a");
    ostx_b62_encode(35, buf, sizeof(buf)); check_str("35 -> \"z\"",  buf, "z");
    ostx_b62_encode(36, buf, sizeof(buf)); check_str("36 -> \"A\"",  buf, "A");
    ostx_b62_encode(61, buf, sizeof(buf)); check_str("61 -> \"Z\"",  buf, "Z");

    /* --- Base-62 rollover --- */
    ostx_b62_encode(62,  buf, sizeof(buf)); check_str("62   -> \"10\"",   buf, "10");
    ostx_b62_encode(3843, buf, sizeof(buf)); check_str("3843 -> \"ZZ\"",  buf, "ZZ");

    /* 62^3 = 238328 -> "1000" */
    ostx_b62_encode(238328, buf, sizeof(buf));
    check_str("238328 -> \"1000\"", buf, "1000");

    /* --- Negative values --- */
    ostx_b62_encode(-1,  buf, sizeof(buf)); check_str("-1  -> \"-1\"",  buf, "-1");
    ostx_b62_encode(-62, buf, sizeof(buf)); check_str("-62 -> \"-10\"", buf, "-10");
    ostx_b62_encode(-61, buf, sizeof(buf)); check_str("-61 -> \"-Z\"",  buf, "-Z");

    /* --- INT32 extremes ---
     * 2147483647 in base62 = "2lkCB1"  (verified: 2*62^5+21*62^4+20*62^3+38*62^2+37*62+1)
     * -2147483647-1 in base62 = "-2lkCB2" (|value| = 2^31 = 2147483648 = "2lkCB2") */
    ostx_b62_encode(2147483647L, buf, sizeof(buf));
    check_str("INT32_MAX -> \"2lkCB1\"", buf, "2lkCB1");

    ostx_b62_encode(-2147483647L - 1L, buf, sizeof(buf));
    check_str("INT32_MIN -> \"-2lkCB2\"", buf, "-2lkCB2");

    /* --- Buffer overflow protection ---
     * Value 62 needs 3 bytes ("10" + NUL); capacity 2 must fail. */
    {
        int ret = ostx_b62_encode(62, buf, 2);
        check_int("62 with cap=2 -> fail (0)", ret, 0L);
    }

    /* --- NULL pointer protection --- */
    {
        int ret = ostx_b62_encode(1, NULL, 16);
        check_int("NULL buf -> fail (0)", ret, 0L);
    }
}

/* =========================================================================
 * 4.  Wire frame byte order
 *
 * Verifies:
 *   a) aid   stored big-endian at offsets [2..5]
 *   b) ts    stored as 6-byte big-endian at offsets [7..12]
 *      (upper 2 bytes = 0x00, lower 4 = ts_sec big-endian)
 *   c) CRC-8 field is placed directly after body at offset 13+body_len
 *   d) CRC-16 appended as 2-byte big-endian at the final two bytes
 *   e) Returned length = 13 + body_len + 3
 * ========================================================================= */

static void test_packet_byteorder(void)
{
    ostx_u8  out[OSTX_PACKET_MAX];
    int      len;
    ostx_u8  crc8_check;
    ostx_u16 crc16_check;

    /* Use a non-trivial body so CRC fields are meaningful. */
    static const ostx_u8 body[]  = {'T','1','|','C','e','l','|','1','5'};
    int                  body_len = 9;

    /* Chosen values with distinct bytes to catch endian swaps. */
    ostx_u32 aid    = 0x01020304u;
    ostx_u8  tid    = 0xAAu;
    ostx_u32 ts_sec = 0xDEADBEEFu;
    ostx_u8  cmd    = 63u; /* OSTX_CMD_DATA_FULL */

    int      expected_len = 13 + body_len + 3; /* 25 */

    printf("\n[Packet byte order]\n");

    memset(out, 0xCC, sizeof(out)); /* poison */
    len = ostx_packet_build(cmd, aid, tid, ts_sec, body, body_len, out);

    /* Length */
    check_int("total length == 25", len, (long)expected_len);

    /* cmd byte */
    check_byte("out[0] == cmd (63)", out[0], 63u);

    /* route_count */
    check_byte("out[1] == 1 (route_count)", out[1], 1u);

    /* aid big-endian */
    check_byte("out[2] == 0x01 (aid[31:24])", out[2], 0x01u);
    check_byte("out[3] == 0x02 (aid[23:16])", out[3], 0x02u);
    check_byte("out[4] == 0x03 (aid[15:8])",  out[4], 0x03u);
    check_byte("out[5] == 0x04 (aid[7:0])",   out[5], 0x04u);

    /* tid */
    check_byte("out[6] == 0xAA (tid)", out[6], 0xAAu);

    /* timestamp big-endian: upper 2 bytes = 0, lower 4 = ts_sec */
    check_byte("out[7]  == 0x00 (ts upper)", out[7],  0x00u);
    check_byte("out[8]  == 0x00 (ts upper)", out[8],  0x00u);
    check_byte("out[9]  == 0xDE (ts[31:24])", out[9],  0xDEu);
    check_byte("out[10] == 0xAD (ts[23:16])", out[10], 0xADu);
    check_byte("out[11] == 0xBE (ts[15:8])",  out[11], 0xBEu);
    check_byte("out[12] == 0xEF (ts[7:0])",   out[12], 0xEFu);

    /* body content */
    check_byte("out[13] == 'T' (body[0])", out[13], (ostx_u8)'T');
    check_byte("out[21] == '5' (body[8])", out[21], (ostx_u8)'5');

    /* CRC-8 at offset 13+body_len = 22 */
    crc8_check = ostx_crc8(body, body_len, 0x07u, 0x00u);
    check_byte("out[22] == CRC-8(body)", out[22], crc8_check);

    /* CRC-16 at offsets 23..24 (big-endian) */
    crc16_check = ostx_crc16(out, 23, 0x1021u, 0xFFFFu);
    check_byte("out[23] == CRC-16 high byte",
               out[23], (ostx_u8)((crc16_check >> 8) & 0xFFu));
    check_byte("out[24] == CRC-16 low byte",
               out[24], (ostx_u8)(crc16_check & 0xFFu));
}

/* =========================================================================
 * 5.  Packet edge cases
 * ========================================================================= */

static void test_packet_edge(void)
{
    ostx_u8 out[OSTX_PACKET_MAX];
    int     len;

    printf("\n[Packet edge cases]\n");

    /* Zero-length body is valid; frame = 13 header + 0 body + 3 CRC = 16 bytes */
    len = ostx_packet_build(63u, 0u, 0u, 0u, NULL, 0, out);
    check_int("zero-body frame length == 16", len, 16L);

    /* NULL output buffer must return 0 */
    len = ostx_packet_build(63u, 0u, 0u, 0u, NULL, 0, NULL);
    check_int("NULL out -> 0", len, 0L);

    /* body_len < 0 must return 0 */
    len = ostx_packet_build(63u, 0u, 0u, 0u, NULL, -1, out);
    check_int("body_len=-1 -> 0", len, 0L);

    /* body_len > OSTX_BODY_MAX must return 0 */
    len = ostx_packet_build(63u, 0u, 0u, 0u, out, OSTX_BODY_MAX + 1, out);
    check_int("body_len=OSTX_BODY_MAX+1 -> 0", len, 0L);
}

/* =========================================================================
 * Entry point
 * ========================================================================= */

int main(void)
{
    printf("OSynaptic-TX test vectors\n");
    printf("=========================\n");

    test_crc8();
    test_crc16();
    test_b62();
    test_packet_byteorder();
    test_packet_edge();

    printf("\n=========================\n");
    printf("Results: %d passed, %d failed\n", g_pass, g_fail);

    return (g_fail == 0) ? 0 : 1;
}
