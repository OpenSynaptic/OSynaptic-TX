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
