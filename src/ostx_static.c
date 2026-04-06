/* ostx_static.c -- Compile-time template packer for OSynaptic-TX (C89) */
#include "ostx_config.h"
#if OSTX_ENABLE_STATIC
#include "ostx_static.h"
#include "ostx_b62.h"
#include "ostx_crc.h"

int ostx_static_pack(
    const OSTXStaticSensor *sensor,
    ostx_u8  tid,
    ostx_u32 ts_sec,
    ostx_i32 scaled,
    ostx_u8 *out
) {
    int      body_pfx_len;
    int      b62_len;
    int      body_len;
    int      frame_len;
    int      crc_off;
    int      i;
    char    *b62_slot;
    ostx_u8  crc8v;
    ostx_u16 crc16v;

    if (!sensor || !out) { return 0; }

    body_pfx_len = sensor->body_pfx_len;

    /* 1. Copy 13-byte header template into output frame. */
    for (i = 0; i < 13; ++i) {
        out[i] = sensor->hdr[i];
    }

    /* 2. Patch tid at byte [6]. */
    out[6] = tid;

    /* 3. Patch ts_sec into bytes [9..12] (big-endian; [7..8] are already 0). */
    out[9]  = (ostx_u8)((ts_sec >> 24) & 0xFFu);
    out[10] = (ostx_u8)((ts_sec >> 16) & 0xFFu);
    out[11] = (ostx_u8)((ts_sec >>  8) & 0xFFu);
    out[12] = (ostx_u8)( ts_sec        & 0xFFu);

    /* 4. Copy body prefix ("sid|unit|") directly into frame at [13..]. */
    for (i = 0; i < body_pfx_len; ++i) {
        out[13 + i] = (ostx_u8)sensor->body_pfx[i];
    }

    /* 5. Encode b62 value directly into frame (zero-copy vs body buffer). */
    b62_slot = (char *)(out + 13 + body_pfx_len);
    if (!ostx_b62_encode(scaled, b62_slot, OSTX_B62_MAX)) { return 0; }

    /* Inline strlen on the just-written b62 string (max 7 chars, fast). */
    b62_len = 0;
    while (b62_slot[b62_len]) { ++b62_len; }

    body_len  = body_pfx_len + b62_len;
    frame_len = 13 + body_len + 3;
    if (frame_len > OSTX_PACKET_MAX) { return 0; }

    crc_off = 13 + body_len;

    /* 6. CRC-8 over body bytes only (same region as ostx_packet_build). */
    crc8v        = ostx_crc8(out + 13, body_len, 0x07u, 0x00u);
    out[crc_off] = crc8v;

    /* 7. CRC-16 over all bytes preceding the CRC-16 field. */
    crc16v             = ostx_crc16(out, crc_off + 1, 0x1021u, 0xFFFFu);
    out[crc_off + 1]   = (ostx_u8)((crc16v >> 8) & 0xFFu);
    out[crc_off + 2]   = (ostx_u8)( crc16v        & 0xFFu);

    return frame_len;
}

#endif /* OSTX_ENABLE_STATIC */
