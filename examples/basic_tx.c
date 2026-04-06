/*
 * OSynaptic-TX  --  Basic TX Example
 * ====================================
 * Target: Any 8-bit MCU (e.g. ATmega328P @ 16 MHz, PIC16F, STM8, ...)
 * Toolchain: avr-gcc, sdcc, xc8, arm-none-eabi-gcc  (C89 mode)
 *
 * This example demonstrates two equivalent APIs:
 *
 *   A) ostx_sensor_pack()  -- dynamic: runtime sensor_id/unit, ~137 B stack (AVR).
 *   B) ostx_static_pack()  -- compile-time descriptor,   ~51 B stack (AVR).
 *   C) ostx_stream_pack()  -- byte-by-byte streaming,    ~29 B stack (AVR).
 *                             NO output buffer needed at the call site.
 *
 * All three produce byte-for-byte identical frames on the wire.
 *
 * Build (avr-gcc, ATmega328P):
 *   avr-gcc -std=c89 -mmcu=atmega328p -Os -Wall \
 *     -I../include \
 *     ../src/ostx_crc.c ../src/ostx_b62.c \
 *     ../src/ostx_packet.c ../src/ostx_sensor.c \
 *     ../src/ostx_static.c ../src/ostx_stream.c \
 *     basic_tx.c -o basic_tx.elf
 */

#include "../include/ostx_config.h"
#include "../include/ostx_types.h"
#include "../include/ostx_sensor.h"
#include "../include/ostx_static.h"
#include "../include/ostx_stream.h"

/* ----------------------------------------------------------------------- */
/* Platform stub -- replace with your UART write routine                   */
/* ----------------------------------------------------------------------- */
static void uart_send_byte(ostx_u8 b)
{
    /* AVR example:
     *   while (!(UCSR0A & (1 << UDRE0)));
     *   UDR0 = b;
     */
    (void)b;
}

static void uart_send(const ostx_u8 *buf, int len)
{
    int i;
    for (i = 0; i < len; ++i) {
        uart_send_byte(buf[i]);
    }
}

/* ----------------------------------------------------------------------- */
/* Platform stub -- replace with your clock/timer read                     */
/* ----------------------------------------------------------------------- */
static ostx_u32 get_time_sec(void)
{
    /* Return seconds since boot or Unix epoch.
     * AVR example using Timer2 overflow counter (approximate):
     *   extern volatile ostx_u32 g_seconds;
     *   return g_seconds;
     */
    return 0UL;
}

/* ----------------------------------------------------------------------- */
/* Compile-time sensor descriptor (Flash/ROM, not RAM)                     */
/*                                                                         */
/* OSTX_STATIC_DEFINE(varname, aid, sensor_id_str, unit_str)               */
/* Sensor_id and unit are baked into the template at compile time.         */
/* To change them, recompile.  AID must match device registration.        */
/* ----------------------------------------------------------------------- */
OSTX_STATIC_DEFINE(s_temp, 0x00000001UL, "T1", "Cel");

/* ----------------------------------------------------------------------- */
/* Streaming emit callback (Option C) -- write directly to UART, no buf   */
/* ----------------------------------------------------------------------- */
static void stream_emit(ostx_u8 b, void *ctx)
{
    /* AVR example -- direct UART register write, no buffer:
     *   while (!(UCSR0A & (1 << UDRE0)));
     *   UDR0 = b;
     */
    uart_send_byte(b);
    (void)ctx;
}

/* ----------------------------------------------------------------------- */
/* Static output buffer -- used by options A and B only                    */
/* ----------------------------------------------------------------------- */
static ostx_u8 g_tx_buf[OSTX_PACKET_MAX];

int main(void)
{
    ostx_u32 aid    = 1UL;
    ostx_u8  tid    = 1u;
    ostx_i32 scaled;
    int      len;

    /* Simulated ADC: 21.50 Celsius  (10000 scale) */
    scaled = 215000L;

    /* ------------------------------------------------------------------
     * Option A: dynamic API -- sensor_id / unit chosen at runtime.
     * Peak stack ~80 bytes.  Use when IDs may differ between calls.
     * ------------------------------------------------------------------ */
    len = ostx_sensor_pack(
        aid, tid, get_time_sec(),
        "T1",   /* sensor_id */
        "Cel",  /* unit      */
        scaled,
        g_tx_buf
    );
    if (len > 0) {
        uart_send(g_tx_buf, len);
    }

    /* ------------------------------------------------------------------
     * Option B: compile-time template -- same wire output as option A.
     * Peak stack (AVR) ~51 bytes.  Needs g_tx_buf[OSTX_PACKET_MAX].
     * ------------------------------------------------------------------ */
    len = ostx_static_pack(
        &s_temp,
        tid, get_time_sec(),
        scaled,
        g_tx_buf
    );
    if (len > 0) {
        uart_send(g_tx_buf, len);
    }

    /* ------------------------------------------------------------------
     * Option C: streaming -- NO output buffer.  Each byte is handed
     * directly to stream_emit() as it is computed.
     * Peak stack (AVR) ~29 bytes.  Minimum RAM variant.
     * ------------------------------------------------------------------ */
    ostx_stream_pack(
        &s_temp,
        tid, get_time_sec(),
        scaled,
        stream_emit, NULL
    );

    return 0;
}
