/* ostx_packet.c -- Wire packet builder for OSynaptic-TX (C89) */
#include "ostx_packet.h"
#include "ostx_crc.h"

int ostx_packet_build(
    ostx_u8        cmd,
    ostx_u32       aid,
    ostx_u8        tid,
    ostx_u32       ts_sec,
    const ostx_u8 *body,
    int            body_len,
    ostx_u8       *out
) {
    int      frame_len;
    int      off;
    int      i;
    ostx_u8  crc8v;
    ostx_u16 crc16v;

    if (!out || body_len < 0 || body_len > OSTX_BODY_MAX) { return 0; }
    if (body_len > 0 && !body) { return 0; }

    /* Total frame = 1 cmd + 1 route + 4 aid + 1 tid + 6 ts + body + 1 crc8 + 2 crc16 */
    frame_len = 13 + body_len + 3;
    if (frame_len > OSTX_PACKET_MAX) { return 0; }

    off = 0;
    out[off++] = cmd;
    out[off++] = 1u; /* route_count is always 1 */

    /* aid: 4-byte big-endian */
    out[off++] = (ostx_u8)((aid >> 24) & 0xFFu);
    out[off++] = (ostx_u8)((aid >> 16) & 0xFFu);
    out[off++] = (ostx_u8)((aid >>  8) & 0xFFu);
    out[off++] = (ostx_u8)( aid        & 0xFFu);

    out[off++] = tid;

    /* timestamp: 6-byte big-endian; upper 2 bytes = 0, lower 4 = ts_sec */
    out[off++] = 0u;
    out[off++] = 0u;
    out[off++] = (ostx_u8)((ts_sec >> 24) & 0xFFu);
    out[off++] = (ostx_u8)((ts_sec >> 16) & 0xFFu);
    out[off++] = (ostx_u8)((ts_sec >>  8) & 0xFFu);
    out[off++] = (ostx_u8)( ts_sec        & 0xFFu);

    /* body */
    for (i = 0; i < body_len; ++i) {
        out[off++] = body[i];
    }

    /* CRC-8 over body only */
    crc8v      = ostx_crc8(body, body_len, 0x07u, 0x00u);
    out[off++] = crc8v;

    /* CRC-16 over everything preceding the CRC-16 field */
    crc16v     = ostx_crc16(out, off, 0x1021u, 0xFFFFu);
    out[off++] = (ostx_u8)((crc16v >> 8) & 0xFFu);
    out[off++] = (ostx_u8)( crc16v       & 0xFFu);

    return off;
}
