/* ostx_stream.c -- Zero-buffer streaming TX for OSynaptic-TX (C89).
 *
 * b62 encoding is inlined (no sub-call frame).
 * CRC-8 and CRC-16 are updated one byte at a time as the frame
 * is emitted; no output buffer is needed.
 */
#include "ostx_stream.h"

/* Base-62 alphabet -- in Flash/ROM, not counted against RAM. */
static const char S_ALPHA[] =
    "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

/* -----------------------------------------------------------------------
 * Single-byte CRC update helpers (static, no separate call frame on
 * most compilers when inlined by -Os).
 * ---------------------------------------------------------------------- */
static ostx_u8 s_crc8_byte(ostx_u8 crc, ostx_u8 b)
{
    int bit;
    crc = (ostx_u8)(crc ^ b);
    for (bit = 0; bit < 8; ++bit) {
        if (crc & 0x80u) crc = (ostx_u8)((crc << 1) ^ 0x07u);
        else             crc = (ostx_u8)(crc << 1);
    }
    return crc;
}

static ostx_u16 s_crc16_byte(ostx_u16 crc, ostx_u8 b)
{
    int bit;
    crc = (ostx_u16)(crc ^ ((ostx_u16)b << 8));
    for (bit = 0; bit < 8; ++bit) {
        if (crc & 0x8000u) crc = (ostx_u16)((crc << 1) ^ 0x1021u);
        else               crc = (ostx_u16)(crc << 1);
    }
    return crc;
}

/* -----------------------------------------------------------------------
 * ostx_stream_pack()
 *
 * Local variables (AVR, int = 2 bytes):
 *   ostx_u8  crc8           1 B
 *   ostx_u16 crc16          2 B
 *   ostx_u32 n              4 B
 *   int      b62_neg        2 B
 *   int      start          2 B   (P62 table index, replaces b62tmp[12])
 *   int      i              2 B
 *   int      pfx_len        2 B
 *   int      total          2 B
 *   ostx_u8  crc8           1 B
 *   ostx_u16 crc16          2 B
 *   return address + regs  ~4 B
 *   ─────────────────────────────
 *   Peak                  ~21 B   (was 29 B; b62tmp[12] gone)
 * ---------------------------------------------------------------------- */
int ostx_stream_pack(
    const OSTXStaticSensor *sensor,
    ostx_u8      tid,
    ostx_u32     ts_sec,
    ostx_i32     scaled,
    ostx_emit_fn emit,
    void        *ctx
) {
    ostx_u8  crc8;
    ostx_u16 crc16;
    ostx_u32 n;          /* b62 absolute value -- no tmp buffer needed   */
    int      b62_neg;
    int      start;      /* first P62[] index with P62[start] <= n       */
    int      i;
    int      pfx_len;
    int      total;

    /* Inline helper: emit one byte, always update crc16;
     * update crc8 only while in the body region (body_crc flag). */
#define EMIT_HDR(b)  do { ostx_u8 _b=(b); crc16=s_crc16_byte(crc16,_b); emit(_b,ctx); total++; } while(0)
#define EMIT_BODY(b) do { ostx_u8 _b=(b); crc8=s_crc8_byte(crc8,_b); crc16=s_crc16_byte(crc16,_b); emit(_b,ctx); total++; } while(0)

    if (!sensor || !emit) { return 0; }

    crc8  = 0x00u;
    crc16 = 0xFFFFu;
    total = 0;

    /* ── Header (13 bytes) ─────────────────────────────────────────── */
    /* [0] cmd -- patch tid/ts into the template on-the-fly */
    EMIT_HDR(sensor->hdr[0]);             /* cmd              */
    EMIT_HDR(sensor->hdr[1]);             /* route_count = 1  */
    EMIT_HDR(sensor->hdr[2]);             /* aid[0] BE        */
    EMIT_HDR(sensor->hdr[3]);             /* aid[1] BE        */
    EMIT_HDR(sensor->hdr[4]);             /* aid[2] BE        */
    EMIT_HDR(sensor->hdr[5]);             /* aid[3] BE        */
    EMIT_HDR(tid);                        /* tid  (runtime)   */
    EMIT_HDR(0x00u);                      /* ts[0] = 0        */
    EMIT_HDR(0x00u);                      /* ts[1] = 0        */
    EMIT_HDR((ostx_u8)((ts_sec >> 24) & 0xFFu)); /* ts[2]    */
    EMIT_HDR((ostx_u8)((ts_sec >> 16) & 0xFFu)); /* ts[3]    */
    EMIT_HDR((ostx_u8)((ts_sec >>  8) & 0xFFu)); /* ts[4]    */
    EMIT_HDR((ostx_u8)( ts_sec        & 0xFFu)); /* ts[5]    */

    /* ── Body prefix: "sid|unit|" ─────────────────────────────────── */
    pfx_len = sensor->body_pfx_len;
    for (i = 0; i < pfx_len; ++i) {
        EMIT_BODY((ostx_u8)sensor->body_pfx[i]);
    }

    /* ── Inline b62 encode + emit (MSB-first, no reversal buffer) ─── */
    /*
     * P62[i] = 62^(5-i) stored in Flash/ROM.  Max i32 needs 6 digits
     * (62^5=916132832 < 2^31-1 < 62^6), so worst-case body suffix = 7 B
     * (6 digits + optional '-').  Conservative frame cap check below.
     */
    {
        /* 62^5 .. 62^0 in descending order -- static const goes to Flash */
        static const ostx_u32 P62[6] = {
            916132832UL, 14776336UL, 238328UL, 3844UL, 62UL, 1UL
        };

        /* Worst-case frame size: 13 hdr + pfx + 7 b62 chars + 3 CRC */
        if (13 + pfx_len + 7 + 3 > OSTX_PACKET_MAX) { return 0; }

        if (scaled == 0) {
            EMIT_BODY((ostx_u8)'0');
        } else {
            b62_neg = (scaled < 0) ? 1 : 0;
            n = b62_neg ? (ostx_u32)(-(scaled + 1)) + 1u : (ostx_u32)scaled;

            if (b62_neg) { EMIT_BODY((ostx_u8)'-'); }

            /* Find highest applicable power: first P62[start] <= n */
            start = 0;
            while (start < 5 && n < P62[start]) { ++start; }

            /* Emit digits MSB-first -- no reversal, no tmp buffer */
            for (i = start; i <= 5; ++i) {
                EMIT_BODY((ostx_u8)(unsigned char)S_ALPHA[(int)(n / P62[i])]);
                n %= P62[i];
            }
        }
    }

    /* ── CRC-8 trailer ─────────────────────────────────────────────── */
    /* crc8 is final; emit it and fold into crc16 */
    crc16 = s_crc16_byte(crc16, crc8);
    emit(crc8, ctx);
    total++;

    /* ── CRC-16 trailer ────────────────────────────────────────────── */
    emit((ostx_u8)((crc16 >> 8) & 0xFFu), ctx);
    emit((ostx_u8)( crc16       & 0xFFu), ctx);
    total += 2;

#undef EMIT_HDR
#undef EMIT_BODY

    return total;
}
