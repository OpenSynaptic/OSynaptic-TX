/* ostx_sensor.c -- Single-sensor packet helper for OSynaptic-TX (C89) */
#include "ostx_config.h"
#if OSTX_ENABLE_SENSOR
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
    char    aid_str[11];
    char    ts_b64[9];
    ostx_u8 body[OSTX_BODY_MAX];
    int     sid_len;
    int     unit_len;
    int     b62_len;
    int     aid_len;
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

    aid_len = ostx_u32toa(aid, aid_str, (int)sizeof(aid_str));
    if (aid_len <= 0) { return 0; }

    ostx_b64url_ts(ts_sec, ts_b64);

    /*
     * body = "{aid}.U.{ts_b64}|{sid}>U.{unit}:{b62}|"
     *
     * Matches OpenSynaptic wire format:
     *   header segment : aid.status.ts_b64
     *   sensor segment : sid>state.unit:b62
     *   Both separated and terminated by '|'.
     */
    body_len = aid_len + 3        /* ".U." */
             + 8                  /* ts_b64 (always 8 chars) */
             + 1                  /* '|' header sentinel */
             + sid_len + 3        /* sid + ">U." */
             + unit_len + 1       /* unit + ':' */
             + b62_len + 1;       /* b62 + '|' trailer */

    if (body_len > OSTX_BODY_MAX) { return 0; }

    off = 0;
    /* Header: "{aid}.U.{ts_b64}|" */
    for (i = 0; i < aid_len; ++i) { body[off++] = (ostx_u8)aid_str[i]; }
    body[off++] = (ostx_u8)'.';
    body[off++] = (ostx_u8)'U';
    body[off++] = (ostx_u8)'.';
    for (i = 0; i < 8; ++i)       { body[off++] = (ostx_u8)ts_b64[i]; }
    body[off++] = (ostx_u8)'|';

    /* Sensor: "{sid}>U.{unit}:{b62}|" */
    for (i = 0; i < sid_len;  ++i) { body[off++] = (ostx_u8)sensor_id[i]; }
    body[off++] = (ostx_u8)'>';
    body[off++] = (ostx_u8)'U';
    body[off++] = (ostx_u8)'.';
    for (i = 0; i < unit_len; ++i) { body[off++] = (ostx_u8)unit[i]; }
    body[off++] = (ostx_u8)':';
    for (i = 0; i < b62_len;  ++i) { body[off++] = (ostx_u8)b62[i]; }
    body[off++] = (ostx_u8)'|';

    return ostx_packet_build(
        (ostx_u8)OSTX_CMD_DATA_FULL,
        aid, tid, ts_sec,
        body, body_len,
        out
    );
}

#endif /* OSTX_ENABLE_SENSOR */
