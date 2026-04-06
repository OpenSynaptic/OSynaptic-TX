#ifndef OSTX_CONFIG_H
#define OSTX_CONFIG_H

/*
 * OSynaptic-TX  --  Compile-time configuration knobs.
 *
 * Override any of these before including ostx_sensor.h, either in your
 * Makefile/CMake (-DOSTX_ID_MAX=12) or in a project-local header that
 * you include before this one.
 *
 * Defaults are conservative: suited for an ATmega328P at 16 MHz with
 * 2 KB SRAM.  All buffers are stack / BSS allocated; no heap is used.
 */

/* Maximum sensor/node ID length INCLUDING the terminating NUL byte.
 * "T1\0" needs 3, "Sensor12\0" needs 9.  Default = 9 (8 chars + NUL). */
#ifndef OSTX_ID_MAX
#  define OSTX_ID_MAX  9
#endif

/* Maximum unit string length INCLUDING the terminating NUL byte.
 * "Cel\0" = 4, "Pa\0" = 3, "umol/m2/s\0" = 10.  Default = 9. */
#ifndef OSTX_UNIT_MAX
#  define OSTX_UNIT_MAX 9
#endif

/* Maximum Base62-encoded scalar length INCLUDING the terminating NUL.
 * A 32-bit signed integer encodes to at most 6 Base62 chars + sign = 7.
 * Allow 14 for headroom.                                                   */
#ifndef OSTX_B62_MAX
#  define OSTX_B62_MAX 14
#endif

/* Maximum body byte count.  The new OpenSynaptic body layout is:
 *   "{aid}.U.{ts_b64}|{sid}>U.{unit}:{b62}|"
 * Worst case (OSTX_ID_MAX=9, OSTX_UNIT_MAX=9, OSTX_B62_MAX=14):
 *   aid(10) + ".U."(3) + ts_b64(8) + "|"(1) +
 *   sid(8) + ">U."(3) + unit(8) + ":"(1) + b62(13) + "|"(1) = 56.
 * Default = 64 for comfortable margin.                                    */
#ifndef OSTX_BODY_MAX
#  define OSTX_BODY_MAX 64
#endif

/* Maximum wire packet byte count.
 * Header (13 bytes) + OSTX_BODY_MAX + 3 CRC bytes.
 * Default = 96 (covers OSTX_BODY_MAX up to 80 bytes).                    */
#ifndef OSTX_PACKET_MAX
#  define OSTX_PACKET_MAX 96
#endif

/* Integer scale factor: real_value = scaled / OSTX_VALUE_SCALE.
 * 25.60 degrees Celsius --> scaled = 256000L (with scale 10000).
 * Use 100L on very constrained targets to reduce B62 digit count.         */
#ifndef OSTX_VALUE_SCALE
#  define OSTX_VALUE_SCALE 10000L
#endif

/* Wire command byte for a FULL data packet (OpenSynaptic protocol v1).    */
#ifndef OSTX_CMD_DATA_FULL
#  define OSTX_CMD_DATA_FULL 63
#endif

/* -------------------------------------------------------------------------
 * Convenience unit-code macros.
 * OSTX_UNIT(sym) expands to the OpenSynaptic wire unit-code string.
 * Usage:  OSTX_UNIT(Cel)  ->  "A01"                                       */
#define OSTX_UNIT_K     "A00"  /* kelvin            */
#define OSTX_UNIT_Cel   "A01"  /* degree Celsius    */
#define OSTX_UNIT_DegF  "A02"  /* degree Fahrenheit */
#define OSTX_UNIT_Pa    "900"  /* pascal            */
#define OSTX_UNIT_s     "B00"  /* second            */
#define OSTX_UNIT_min   "B01"  /* minute            */
#define OSTX_UNIT_h     "B02"  /* hour              */
#define OSTX_UNIT(sym)  OSTX_UNIT_##sym

/* -------------------------------------------------------------------------
 * Per-module enable switches.
 * Set any of these to 0 to exclude that module from compilation.
 * The linker will drop unreferenced code automatically, but explicit
 * disabling also removes the #include chain and any associated RAM.
 *
 * Dependencies:
 *   OSTX_ENABLE_STREAM requires OSTX_ENABLE_STATIC (for OSTXStaticSensor).
 *   All three modules depend on the always-on primitives:
 *   ostx_crc, ostx_b62, ostx_packet.
 *
 * Override in your Makefile/CMake:
 *   -DOSTX_ENABLE_SENSOR=0   -- exclude API A (ostx_sensor_pack)
 *   -DOSTX_ENABLE_STATIC=0   -- exclude API B (ostx_static_pack)
 *   -DOSTX_ENABLE_STREAM=0   -- exclude API C (ostx_stream_pack)
 * -------------------------------------------------------------------------*/
#ifndef OSTX_ENABLE_SENSOR
#  define OSTX_ENABLE_SENSOR 1
#endif

#ifndef OSTX_ENABLE_STATIC
#  define OSTX_ENABLE_STATIC 1
#endif

/* Enabling STREAM forces STATIC on (STREAM needs OSTXStaticSensor). */
#ifndef OSTX_ENABLE_STREAM
#  define OSTX_ENABLE_STREAM 1
#endif
#if OSTX_ENABLE_STREAM && !OSTX_ENABLE_STATIC
#  undef  OSTX_ENABLE_STATIC
#  define OSTX_ENABLE_STATIC 1
#endif

#endif /* OSTX_CONFIG_H */
