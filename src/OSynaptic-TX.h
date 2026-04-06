/*
 * src/OSynaptic-TX.h -- Arduino entry-point for the OSynaptic-TX library.
 *
 * When Arduino IDE/CLI installs a src-layout library it adds src/ to the
 * compiler include path.  This file lives in src/ so that
 *
 *   #include <OSynaptic-TX.h>
 *
 * resolves correctly in Arduino sketches.  All includes below are relative
 * to src/ and do NOT use the "src/" prefix.
 *
 * For native CMake builds the include/ directory is already on the path;
 * the root-level OSynaptic-TX.h (which uses "src/" prefixes) continues to
 * work for any non-Arduino consumers that include from the repo root.
 */

#ifndef OSTX_ARDUINO_H
#define OSTX_ARDUINO_H

#include "ostx_config.h"
#include "ostx_types.h"
#include "ostx_sensor.h"
#include "ostx_static.h"
#include "ostx_stream.h"
#include "ostx_units.h"

#endif /* OSTX_ARDUINO_H */
