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
 * Convert an unsigned 32-bit integer to a NUL-terminated decimal string.
 *
 * Returns the number of characters written (excluding NUL), 0 on error
 * (cap too small or NULL pointer).
 */
int ostx_u32toa(ostx_u32 n, char *buf, int cap);

/*
 * Encode a 32-bit Unix timestamp as an 8-character base64url string.
 *
 * The timestamp is treated as the lower 32 bits of a 48-bit big-endian
 * word (upper 16 bits = 0), matching the OpenSynaptic ts_b64 wire field.
 * out must have capacity for at least 9 bytes (8 chars + NUL).
 */
void ostx_b64url_ts(ostx_u32 ts_sec, char out[9]);

#ifdef __cplusplus
}
#endif

#endif /* OSTX_B62_H */
