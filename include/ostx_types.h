#ifndef OSTX_TYPES_H
#define OSTX_TYPES_H

/*
 * OSynaptic-TX  --  C89-compatible portable integer types.
 *
 * <stdint.h> is C99; we derive equivalent types from first principles.
 *
 * Assumptions (valid for avr-gcc, arm-none-eabi-gcc, sdcc, xc8):
 *   unsigned char   == 8 bits
 *   unsigned short  == 16 bits
 *   unsigned long   == 32 bits
 *
 * If your toolchain differs, override these typedefs in a project header
 * that is included before ostx_types.h.
 */

#ifndef OSTX_U8_DEFINED
typedef unsigned char   ostx_u8;
typedef unsigned short  ostx_u16;
typedef unsigned long   ostx_u32;
typedef signed   char   ostx_i8;
typedef signed   short  ostx_i16;
typedef signed   long   ostx_i32;
#define OSTX_U8_DEFINED 1
#endif

#ifndef NULL
#  include <stddef.h>
#endif

#endif /* OSTX_TYPES_H */
