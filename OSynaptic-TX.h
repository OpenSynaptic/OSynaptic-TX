/*
 * OSynaptic-TX.h -- Top-level Arduino include for OSynaptic-TX library.
 *
 * Include this single header in your sketch:
 *
 *   #include <OSynaptic-TX.h>
 *
 * Then choose one of three APIs:
 *
 *   A) ostx_sensor_pack()   -- dynamic, runtime sensor_id/unit
 *   B) ostx_static_pack()   -- compile-time descriptor, lower stack
 *   C) ostx_stream_pack()   -- streaming, zero output buffer, min RAM
 *
 * All three produce identical wire frames compatible with OpenSynaptic.
 */

#ifndef OSTX_ARDUINO_H
#define OSTX_ARDUINO_H

#include "src/ostx_config.h"
#include "src/ostx_types.h"
#if OSTX_ENABLE_SENSOR
#  include "src/ostx_sensor.h"
#endif
#if OSTX_ENABLE_STATIC
#  include "src/ostx_static.h"
#endif
#if OSTX_ENABLE_STREAM
#  include "src/ostx_stream.h"
#endif
#include "src/ostx_units.h"

#endif /* OSTX_ARDUINO_H */
