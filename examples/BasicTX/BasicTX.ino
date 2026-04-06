/*
 * BasicTX.ino -- OSynaptic-TX basic example for Arduino
 * ========================================================
 * Demonstrates Option C (streaming API): no output buffer needed,
 * minimum RAM ~21 bytes stack peak on AVR.
 *
 * Compatible boards: Arduino Uno, Nano, Mega, ESP8266, ESP32, STM32, etc.
 *
 * Wiring: TX data sent over Serial (UART0).
 *
 * OpenSynaptic server / OSynaptic-RX on the receiving end will decode
 * the frame automatically.
 */

#include <OSynaptic-TX.h>

/* ------------------------------------------------------------------
 * Compile-time sensor descriptor -- baked into Flash, not RAM.
 * Change "T1" to your sensor ID and OSTX_UNIT(Cel) to your unit.
 * AID (agent ID) must match what the OpenSynaptic server assigned.
 * ------------------------------------------------------------------ */
OSTX_STATIC_DEFINE(s_temp, 0x00000001UL, "T1", OSTX_UNIT(Cel));

/* ------------------------------------------------------------------
 * Streaming emit callback -- hands each byte directly to Serial.
 * No output buffer allocated anywhere.
 * ------------------------------------------------------------------ */
static void serial_emit(ostx_u8 b, void * /*ctx*/)
{
    Serial.write(b);
}

/* ------------------------------------------------------------------
 * Globals
 * ------------------------------------------------------------------ */
static ostx_u8 g_tid = 0u;

/* ------------------------------------------------------------------
 * setup()
 * ------------------------------------------------------------------ */
void setup()
{
    Serial.begin(115200);
    /* Wait for Serial on boards that need it (Leonardo, Micro, etc.) */
    while (!Serial) { ; }
}

/* ------------------------------------------------------------------
 * loop() -- send one temperature reading every second
 * ------------------------------------------------------------------ */
void loop()
{
    /*
     * Replace this with your actual sensor read.
     * Value must be pre-scaled: multiply by OSTX_VALUE_SCALE (default 10000).
     * Example: 21.50 °C  →  215000L
     *          -5.25 °C  →  -52500L
     */
    ostx_i32 scaled = 215000L; /* 21.50 °C */

    ostx_u32 ts_sec = (ostx_u32)(millis() / 1000UL);

    int len = ostx_stream_pack(
        &s_temp,
        g_tid++,
        ts_sec,
        scaled,
        serial_emit,
        NULL
    );

    /* len > 0 means frame was emitted successfully */
    (void)len;

    delay(1000);
}
