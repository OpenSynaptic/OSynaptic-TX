#ifndef OSTX_STATIC_H
#define OSTX_STATIC_H

/*
 * ostx_static.h -- Compile-time template variant for OSynaptic-TX (C89).
 *
 * When sensor_id and unit are fixed at compile time, this variant
 * eliminates the intermediate body[] stack buffer and sensor-string
 * validation, encoding the b62 value directly into the output frame.
 *
 * Wire format is byte-for-byte identical to ostx_sensor_pack().
 *
 * Usage (place in exactly ONE .c file per sensor):
 *
 *   #include "ostx_static.h"
 *
 *   OSTX_STATIC_DEFINE(my_t1, 0x00000001UL, "T1", "Cel")
 *
 * Then at runtime:
 *
 *   int len = ostx_static_pack(&my_t1, tid, ts_sec, scaled_val, buf);
 *
 * The macro expands to:
 *   - a static const ostx_u8[13]  (header template, in Flash/ROM)
 *   - a static const char[]       ("sid|unit|" string literal, in Flash)
 *   - an OSTXStaticSensor struct  initialised from the two above
 *
 * Stack savings vs ostx_sensor_pack():
 *   - No body[OSTX_BODY_MAX] buffer (~64 bytes)
 *   - No b62[OSTX_B62_MAX]   buffer (~14 bytes, now written directly in frame)
 *   - No strlen on sensor_id / unit
 *   Peak stack: ~24 bytes (locals only).
 */

#include "ostx_config.h"
#include "ostx_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------
 * Descriptor struct (all pointers into Flash/ROM).
 * -------------------------------------------------------------------------*/
typedef struct {
    const ostx_u8 *hdr;          /* 13-byte header template (tid,ts = 0x00) */
    const char    *body_pfx;     /* "sid|unit|"  NUL-terminated string       */
    int            body_pfx_len; /* strlen(body_pfx)                         */
} OSTXStaticSensor;

/* -------------------------------------------------------------------------
 * OSTX_STATIC_DEFINE(varname, aid32, sid_str, unit_str)
 *
 *   varname  -- C identifier for the OSTXStaticSensor variable
 *   aid32    -- 32-bit agent ID constant (e.g. 0x00000001UL)
 *   sid_str  -- sensor-id string literal  (e.g. "T1")
 *   unit_str -- unit string literal        (e.g. "Cel")
 *
 * Must be placed at file scope (not inside a function).
 * -------------------------------------------------------------------------*/
#define OSTX_STATIC_DEFINE(varname, aid32, sid_str, unit_str)              \
    static const ostx_u8 varname##__hdr[13] = {                            \
        (ostx_u8)OSTX_CMD_DATA_FULL,              /* cmd               */   \
        0x01u,                                    /* route_count = 1   */   \
        (ostx_u8)(((unsigned long)(aid32))>>24u), /* aid[0] BE         */   \
        (ostx_u8)(((unsigned long)(aid32))>>16u), /* aid[1] BE         */   \
        (ostx_u8)(((unsigned long)(aid32))>> 8u), /* aid[2] BE         */   \
        (ostx_u8)( (unsigned long)(aid32)       ),/* aid[3] BE         */   \
        0x00u,                                    /* tid  (patched)    */   \
        0x00u, 0x00u,                             /* ts[0..1] = 0      */   \
        0x00u, 0x00u, 0x00u, 0x00u               /* ts[2..5] (patched) */  \
    };                                                                       \
    static const char varname##__pfx[] = sid_str ">U." unit_str ":";         \
    const OSTXStaticSensor varname = {  /* const -> Flash/ROM on AVR     */  \
        varname##__hdr,                                                      \
        varname##__pfx,                                                      \
        (int)(sizeof(varname##__pfx) - 1)  /* excludes NUL */               \
    }

/* -------------------------------------------------------------------------
 * ostx_static_pack() -- runtime packer using a compile-time descriptor.
 *
 *   sensor    -- pointer to an OSTXStaticSensor created by OSTX_STATIC_DEFINE
 *   tid       -- transaction / sequence ID (caller increments)
 *   ts_sec    -- 32-bit Unix timestamp in seconds
 *   scaled    -- pre-scaled sensor value (e.g. 21.5 C * 10000 = 215000)
 *   out       -- caller-supplied output buffer, must be >= OSTX_PACKET_MAX
 *
 * Returns total frame length on success, 0 on error.
 *
 * Wire format (identical to ostx_sensor_pack / ostx_packet_build):
 *   [cmd:1][route:1][aid:4BE][tid:1][ts:6BE][{aid}.U.{ts_b64}|{sid}>U.{unit}:{b62}|][crc8:1][crc16:2]
 * -------------------------------------------------------------------------*/
int ostx_static_pack(
    const OSTXStaticSensor *sensor,
    ostx_u8  tid,
    ostx_u32 ts_sec,
    ostx_i32 scaled,
    ostx_u8 *out
);

#ifdef __cplusplus
}
#endif

#endif /* OSTX_STATIC_H */
