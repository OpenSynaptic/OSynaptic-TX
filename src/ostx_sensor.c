/* ostx_sensor.c -- Single-sensor packet helper for OSynaptic-TX (C89) */
#include "ostx_sensor.h"
#include "ostx_b62.h"
#include "ostx_packet.h"

#include <string.h>

int ostx_sensor_pack(
    ostx_u32    aid,
    ostx_u8     tid,
    ostx_u32    ts_sec,
    const char *sensor_id,
    const char *unit,
    ostx_i32    scaled,
    ostx_u8    *out
) {
    char    b62[OSTX_B62_MAX];
    ostx_u8 body[OSTX_BODY_MAX];
    int     sid_len;
    int     unit_len;
    int     b62_len;
    int     body_len;
    int     i;
    int     off;

    if (!sensor_id || !unit || !out) { return 0; }

    sid_len  = (int)strlen(sensor_id);
    unit_len = (int)strlen(unit);

    if (sid_len  <= 0 || sid_len  >= OSTX_ID_MAX)   { return 0; }
    if (unit_len <= 0 || unit_len >= OSTX_UNIT_MAX)  { return 0; }

    if (!ostx_b62_encode(scaled, b62, OSTX_B62_MAX)) { return 0; }
    b62_len = (int)strlen(b62);

    /* body = <sid>|<unit>|<b62> */
    body_len = sid_len + 1 + unit_len + 1 + b62_len;
    if (body_len > OSTX_BODY_MAX) { return 0; }

    off = 0;
    for (i = 0; i < sid_len;  ++i) { body[off++] = (ostx_u8)sensor_id[i]; }
    body[off++] = (ostx_u8)'|';
    for (i = 0; i < unit_len; ++i) { body[off++] = (ostx_u8)unit[i]; }
    body[off++] = (ostx_u8)'|';
    for (i = 0; i < b62_len;  ++i) { body[off++] = (ostx_u8)b62[i]; }

    return ostx_packet_build(
        (ostx_u8)OSTX_CMD_DATA_FULL,
        aid, tid, ts_sec,
        body, body_len,
        out
    );
}
