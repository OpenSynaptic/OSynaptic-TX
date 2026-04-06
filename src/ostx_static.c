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
    char     aid_str[11];
    char     ts_b64[9];
    int      aid_len;
    int      b62_len;
    int      body_len;
    int      frame_len;
    int      crc_off;
    int      body_off;
    int      i;
    char    *b62_slot;
    ostx_u8  crc8v;
    ostx_u16 crc16v;
    ostx_u32 aid;

    if (!sensor || !out) { return 0; }

    /* 1. Copy 13-byte binary header template into output frame. */
    for (i = 0; i < 13; ++i) {
        out[i] = sensor->hdr[i];
    }

    /* 2. Patch tid at byte [6]. */
    out[6] = tid;

    /* 3. Patch ts_sec into bytes [9..12]. */
    out[9]  = (ostx_u8)((ts_sec >> 24) & 0xFFu);
    out[10] = (ostx_u8)((ts_sec >> 16) & 0xFFu);
    out[11] = (ostx_u8)((ts_sec >>  8) & 0xFFu);
    out[12] = (ostx_u8)( ts_sec        & 0xFFu);

    /* 4. Derive AID decimal string from binary header bytes [2..5]. */
    aid = ((ostx_u32)sensor->hdr[2] << 24)
        | ((ostx_u32)sensor->hdr[3] << 16)
        | ((ostx_u32)sensor->hdr[4] <<  8)
        |  (ostx_u32)sensor->hdr[5];
    aid_len = ostx_u32toa(aid, aid_str, (int)sizeof(aid_str));
    if (aid_len <= 0) { return 0; }

    /* 5. Encode timestamp as 8-char base64url string. */
    ostx_b64url_ts(ts_sec, ts_b64);

    /*
     * 6. Write body text directly into frame at offset 13:
     *      "{aid}.U.{ts_b64}|{body_pfx}{b62}|"
     *    body_pfx = "sid>U.unit:"  (baked at compile time)
     */
    body_off = 13;

    /* Header segment: "{aid}.U.{ts_b64}|" */
    for (i = 0; i < aid_len; ++i) { out[body_off++] = (ostx_u8)aid_str[i]; }
    out[body_off++] = (ostx_u8)'.';
    out[body_off++] = (ostx_u8)'U';
    out[body_off++] = (ostx_u8)'.';
    for (i = 0; i < 8; ++i)       { out[body_off++] = (ostx_u8)ts_b64[i]; }
    out[body_off++] = (ostx_u8)'|';

    /* Sensor segment prefix: e.g. "T1>U.A01:" */
    for (i = 0; i < sensor->body_pfx_len; ++i) {
        out[body_off++] = (ostx_u8)sensor->body_pfx[i];
    }

    /* 7. Encode b62 value directly into frame (zero-copy). */
    b62_slot = (char *)(out + body_off);
    if (!ostx_b62_encode(scaled, b62_slot, OSTX_B62_MAX)) { return 0; }
    b62_len = 0;
    while (b62_slot[b62_len]) { ++b62_len; }
    body_off += b62_len;

    /* Trailing sensor '|' */
    out[body_off++] = (ostx_u8)'|';

    body_len  = body_off - 13;
    frame_len = body_off + 3; /* body + crc8(1) + crc16(2) */
    if (frame_len > OSTX_PACKET_MAX) { return 0; }

    crc_off = body_off;

    /* 8. CRC-8 over body bytes only. */
    crc8v        = ostx_crc8(out + 13, body_len, 0x07u, 0x00u);
    out[crc_off] = crc8v;

    /* 9. CRC-16 over everything preceding the CRC-16 field. */
    crc16v           = ostx_crc16(out, crc_off + 1, 0x1021u, 0xFFFFu);
    out[crc_off + 1] = (ostx_u8)((crc16v >> 8) & 0xFFu);
    out[crc_off + 2] = (ostx_u8)( crc16v        & 0xFFu);

    return frame_len;
}

#endif /* OSTX_ENABLE_STATIC */
