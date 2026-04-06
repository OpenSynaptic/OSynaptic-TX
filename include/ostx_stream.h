#ifndef OSTX_STREAM_H
#define OSTX_STREAM_H

/*
 * ostx_stream.h -- Zero-buffer streaming TX for OSynaptic-TX (C89).
 *
 * Instead of assembling the entire frame in an output buffer, each
 * byte is computed and handed directly to an emit callback, so no
 * frame buffer is required at the call site.
 *
 * CRC-8 and CRC-16 are accumulated byte-by-byte as the frame is
 * emitted; no second pass over the data is needed.
 *
 * b62 encoding is inlined inside ostx_stream_pack() to avoid a
 * sub-call frame.  The only local buffer is the 12-byte b62 digit
 * reversal scratch array -- unavoidable for base-62 encoding.
 *
 * Stack peak on AVR (int = 2 bytes):
 *   ostx_stream_pack locals  ~21 B
 *   emit callback (UART ISR) ~6 B
 *   ──────────────────────────────
 *   Total                   ~27 B
 *
 * vs. ostx_static_pack:  ~51 B
 * vs. ostx_sensor_pack: ~137 B
 *
 * Wire format is byte-for-byte identical to all other TX APIs.
 *
 * Usage:
 *
 *   #include "ostx_stream.h"
 *
 *   OSTX_STATIC_DEFINE(s_temp, 0x00000001UL, "T1", "Cel")
 *
 *   static void uart_emit(ostx_u8 b, void *ctx) {
 *       while (!(UCSR0A & (1 << UDRE0)));
 *       UDR0 = b;
 *       (void)ctx;
 *   }
 *
 *   // No output buffer needed anywhere:
 *   int len = ostx_stream_pack(&s_temp, tid, ts_sec, 215000L,
 *                              uart_emit, NULL);
 */

#include "ostx_config.h"
#include "ostx_types.h"
#include "ostx_static.h"   /* OSTXStaticSensor, OSTX_STATIC_DEFINE */

/*
 * ostx_emit_fn -- user-supplied byte-sink callback.
 *
 *   byte -- the byte to transmit
 *   ctx  -- opaque pointer forwarded from ostx_stream_pack (may be NULL)
 *
 * Must not be NULL when passed to ostx_stream_pack.
 */
typedef void (*ostx_emit_fn)(ostx_u8 byte, void *ctx);

/*
 * ostx_stream_pack() -- stream one sensor packet byte by byte.
 *
 *   sensor -- compile-time descriptor created by OSTX_STATIC_DEFINE
 *   tid    -- transaction / sequence ID
 *   ts_sec -- 32-bit Unix timestamp in seconds
 *   scaled -- pre-scaled sensor value (e.g. 21.5 C * 10000 = 215000)
 *   emit   -- callback invoked once per output byte
 *   ctx    -- forwarded opaque pointer (pass NULL if unused)
 *
 * Returns total byte count emitted on success, 0 on error.
 * No output buffer is allocated or required.
 */
int ostx_stream_pack(
    const OSTXStaticSensor *sensor,
    ostx_u8      tid,
    ostx_u32     ts_sec,
    ostx_i32     scaled,
    ostx_emit_fn emit,
    void        *ctx
);

#endif /* OSTX_STREAM_H */
