#ifndef OSTX_SENSOR_H
#define OSTX_SENSOR_H

#include "ostx_types.h"
#include "ostx_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Pack a single sensor reading into a wire-ready OpenSynaptic packet.
 *
 * Body layout: "<sensor_id>|<unit>|<b62(scaled)>"
 * Example:     "T1|Cel|BIS"  (T1, Celsius, 21.50 = 215000/10000)
 *
 * Parameters:
 *   aid       -- 32-bit assigned device ID (from server, or fixed)
 *   tid       -- transaction/template ID (1 byte; increment each call)
 *   ts_sec    -- 32-bit current timestamp in seconds
 *   sensor_id -- null-terminated sensor name (max OSTX_ID_MAX-1 chars)
 *   unit      -- null-terminated unit string  (max OSTX_UNIT_MAX-1 chars)
 *   scaled    -- sensor value * OSTX_VALUE_SCALE, as a signed 32-bit int
 *                Example: 21.50 Celsius  -->  scaled = 215000L
 *                         1013.25 hPa   -->  scaled = 10132500L  (use scale=100)
 *   out       -- output buffer; MUST be >= OSTX_PACKET_MAX bytes
 *
 * Returns the total packet length (>0) on success, 0 on error.
 *
 * Quick start (ATmega328P, simulated read):
 *   static ostx_u8 buf[OSTX_PACKET_MAX];
 *   int len = ostx_sensor_pack(1UL, 1, uptime_sec, "T1", "Cel", 215000L, buf);
 *   if (len > 0) { uart_write(buf, len); }
 */
int ostx_sensor_pack(
    ostx_u32    aid,
    ostx_u8     tid,
    ostx_u32    ts_sec,
    const char *sensor_id,
    const char *unit,
    ostx_i32    scaled,
    ostx_u8    *out
);

#ifdef __cplusplus
}
#endif

#endif /* OSTX_SENSOR_H */
