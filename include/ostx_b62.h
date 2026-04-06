#ifndef OSTX_B62_H
#define OSTX_B62_H

#include "ostx_types.h"
#include "ostx_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Encode a 32-bit signed integer to Base62 ASCII (TX-only, no decode).
 *
 * Alphabet: "0-9 a-z A-Z"  (same as OSynaptic-FX / OpenSynaptic Python).
 * Negative values are prefixed with '-'.
 *
 * Parameters:
 *   value   -- integer to encode
 *   out     -- output buffer; must be >= out_cap bytes
 *   out_cap -- capacity of 'out' in bytes (must be >= OSTX_B62_MAX)
 *
 * Returns 1 on success, 0 on failure (NULL pointer or buffer too small).
 */
int ostx_b62_encode(ostx_i32 value, char *out, int out_cap);

/*
 * Convert a uint32 to a decimal ASCII string.
 * Returns the number of characters written (excluding NUL), or 0 on error.
 */
int  ostx_u32toa(ostx_u32 n, char *buf, int cap);

/*
 * Encode a 32-bit Unix timestamp to the 8-character base64url string used
 * in the OpenSynaptic wire body header (6-byte big-endian, upper 2 bytes 0).
 * out must hold at least 9 bytes; out[8] is always set to '\0'.
 */
void ostx_b64url_ts(ostx_u32 ts_sec, char out[9]);

#ifdef __cplusplus
}
#endif

#endif /* OSTX_B62_H */
