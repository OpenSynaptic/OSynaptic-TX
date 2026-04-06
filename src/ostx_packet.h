#ifndef OSTX_PACKET_H
#define OSTX_PACKET_H

#include "ostx_types.h"
#include "ostx_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Build a full OpenSynaptic wire packet into 'out'.
 *
 * Wire format (identical to OSynaptic-FX / OpenSynaptic Python):
 *   [0]      cmd          (1 byte)
 *   [1]      route_count  (1 byte, fixed = 1)
 *   [2..5]   source_aid   (big-endian u32)
 *   [6]      tid          (1 byte, transaction/template ID)
 *   [7..12]  timestamp    (48-bit big-endian; bytes 7-8 = 0, bytes 9-12 = ts_sec)
 *   [13..]   body         (body_len bytes)
 *   [-3]     CRC-8 of body (poly=0x07, init=0x00)
 *   [-2..-1] CRC-16 of all preceding bytes (poly=0x1021, init=0xFFFF)
 *
 * Parameters:
 *   cmd      -- command byte (use OSTX_CMD_DATA_FULL = 63)
 *   aid      -- 32-bit assigned device ID
 *   tid      -- 1-byte template/transaction ID (wraps 0-255)
 *   ts_sec   -- 32-bit timestamp in seconds (Unix epoch, or MCU uptime)
 *   body     -- payload bytes; may be NULL if body_len == 0
 *   body_len -- number of body bytes (0-OSTX_BODY_MAX)
 *   out      -- output buffer; must be >= OSTX_PACKET_MAX bytes
 *
 * Returns total packet length (>0) on success; 0 on error.
 */
int ostx_packet_build(
    ostx_u8        cmd,
    ostx_u32       aid,
    ostx_u8        tid,
    ostx_u32       ts_sec,
    const ostx_u8 *body,
    int            body_len,
    ostx_u8       *out
);

#ifdef __cplusplus
}
#endif

#endif /* OSTX_PACKET_H */
