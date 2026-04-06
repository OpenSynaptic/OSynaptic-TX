/* ostx_b62.c -- Base62 encode for OSynaptic-TX (C89, encode-only) */
#include "ostx_b62.h"

/* Alphabet matches OpenSynaptic Python / OSynaptic-FX exactly. */
static const char OSTX_ALPHA[] =
    "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

int ostx_b62_encode(ostx_i32 value, char *out, int out_cap)
{
    ostx_u32 n;
    char     tmp[12]; /* log62(2^31) < 6; 12 gives ample headroom */
    int      neg;
    int      idx;
    int      w;
    int      needed;
    int      i;

    if (!out || out_cap < 2) { return 0; }

    if (value == 0) {
        out[0] = '0';
        out[1] = '\0';
        return 1;
    }

    neg = (value < 0) ? 1 : 0;
    /* Handle LONG_MIN safely: -(LONG_MIN+1)+1 avoids signed overflow. */
    if (neg) {
        n = (ostx_u32)(-(value + 1)) + 1u;
    } else {
        n = (ostx_u32)value;
    }

    /* Build digits in reverse order into tmp[]. */
    idx = 0;
    while (n > 0u && idx < (int)sizeof(tmp) - 1) {
        tmp[idx++] = OSTX_ALPHA[(int)(n % 62u)];
        n /= 62u;
    }
    if (idx == 0) { return 0; }

    /* Check capacity: digits + optional '-' + NUL */
    needed = idx + neg + 1;
    if (needed > out_cap) { return 0; }

    w = 0;
    if (neg) { out[w++] = '-'; }
    for (i = idx - 1; i >= 0; --i) { out[w++] = tmp[i]; }
    out[w] = '\0';
    return 1;
}

int ostx_u32toa(ostx_u32 n, char *buf, int cap)
{
    char tmp[11]; /* log10(2^32-1) < 10 digits */
    int  len;
    int  i;

    if (!buf || cap < 1) { return 0; }
    if (n == 0u) {
        if (cap < 2) { return 0; }
        buf[0] = '0'; buf[1] = '\0';
        return 1;
    }
    len = 0;
    while (n > 0u && len < (int)(sizeof(tmp) - 1)) {
        tmp[len++] = (char)('0' + (int)(n % 10u));
        n /= 10u;
    }
    if (len + 1 > cap) { return 0; }
    for (i = 0; i < len; ++i) { buf[i] = tmp[len - 1 - i]; }
    buf[len] = '\0';
    return len;
}

static const char OSTX_B64URL[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

void ostx_b64url_ts(ostx_u32 ts_sec, char out[9])
{
    /*
     * Pack ts_sec into 6 bytes big-endian (upper 2 bytes = 0):
     *   b[0]=0x00  b[1]=0x00  b[2]=ts>>24  b[3]=ts>>16  b[4]=ts>>8  b[5]=ts
     *
     * Base64url encode 6 bytes = 8 output chars (no padding needed;
     * 48 bits / 6 = exactly 8 base64 digits).
     *
     * Groups of 3 bytes → 4 chars:
     *   Group 1: 0x00, 0x00, b2
     *   Group 2: b3,   b4,   b5
     */
    ostx_u8 b2 = (ostx_u8)((ts_sec >> 24) & 0xFFu);
    ostx_u8 b3 = (ostx_u8)((ts_sec >> 16) & 0xFFu);
    ostx_u8 b4 = (ostx_u8)((ts_sec >>  8) & 0xFFu);
    ostx_u8 b5 = (ostx_u8)( ts_sec        & 0xFFu);

    /* Group 1 */
    out[0] = OSTX_B64URL[0];                                      /* 0x00 >> 2 */
    out[1] = OSTX_B64URL[0];                               /* (0&3)<<4|(0>>4) */
    out[2] = OSTX_B64URL[(b2 >> 6) & 0x03u];          /* (0&0xF)<<2|(b2>>6) */
    out[3] = OSTX_B64URL[ b2       & 0x3Fu];                      /* b2 & 63 */

    /* Group 2 */
    out[4] = OSTX_B64URL[(b3 >> 2)  & 0x3Fu];
    out[5] = OSTX_B64URL[((b3 & 0x03u) << 4) | ((b4 >> 4) & 0x0Fu)];
    out[6] = OSTX_B64URL[((b4 & 0x0Fu) << 2) | ((b5 >> 6) & 0x03u)];
    out[7] = OSTX_B64URL[ b5        & 0x3Fu];
    out[8] = '\0';
}
