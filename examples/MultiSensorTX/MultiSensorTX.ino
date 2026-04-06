/*
 * MultiSensorTX.ino -- OSynaptic-TX multi-sensor example (API C)
 * ================================================================
 * Demonstrates sending multiple sensor readings in sequence using
 * the streaming API (API C). Each sensor gets its own compile-time
 * descriptor; all three share a single tid counter that increments
 * per sensor so the server sees distinct transaction IDs.
 *
 * Compatible boards: Arduino Uno, Nano, Mega, ESP8266, ESP32, etc.
 *
 * Memory (AVR ATmega328P):
 *   Stack peak  : ~27 B (ostx_stream_pack ~21 B + emit stub ~6 B)
 *   Static RAM  : 0 B   (all descriptors in Flash)
 *   Flash total : ~760 B (ostx_stream module) + sketch overhead
 *
 * Wiring: packets emitted over Serial at 115200 baud.
 */

#include <OSynaptic-TX.h>

/* ------------------------------------------------------------------
 * Compile-time sensor descriptors (Flash, 0 SRAM each).
 * All three sensors belong to agent ID 0x00000001.
 * Change sensor IDs / units / AID to match your server config.
 * ------------------------------------------------------------------ */
OSTX_STATIC_DEFINE(s_temp,     0x00000001UL, "T1",  OSTX_UNIT(Cel));
OSTX_STATIC_DEFINE(s_humidity, 0x00000001UL, "H1",  OSTX_UNIT(pct));
OSTX_STATIC_DEFINE(s_pressure, 0x00000001UL, "P1",  OSTX_UNIT(Pa));

/* ------------------------------------------------------------------
 * Streaming emit callback -- each byte goes directly to Serial.
 * ------------------------------------------------------------------ */
static void serial_emit(ostx_u8 b, void * /*ctx*/)
{
    Serial.write(b);
}

/* tid counter -- shared across all sensors so the server sees a
   monotonically incrementing sequence regardless of which sensor
   is being sent.                                                     */
static ostx_u8 g_tid = 0u;

/* ------------------------------------------------------------------
 * setup()
 * ------------------------------------------------------------------ */
void setup()
{
    Serial.begin(115200);
    while (!Serial) { ; }
    Serial.println(F("OSynaptic-TX MultiSensorTX ready"));
}

/* ------------------------------------------------------------------
 * loop() -- send all three sensors once per second
 * ------------------------------------------------------------------ */
void loop()
{
    ostx_u32 ts = (ostx_u32)(millis() / 1000UL);

    /*
     * Replace these with real ADC / I2C / SPI reads.
     * Values are pre-scaled by OSTX_VALUE_SCALE (default 10000):
     *   23.50 °C   --> 235000L   (unit wire code: A01)
     *   61.20 %RH  --> 612000L   (unit wire code: C00)
     *   101325 Pa  --> 1013250000L (unit wire code: 900)
     *
     * Note: pressure is now in Pa (not hPa). 1013.25 hPa = 101325 Pa.
     * Reduce OSTX_VALUE_SCALE to 100 in ostx_config.h and use 10132500L
     * if you want to keep hPa-scale values with less precision.
     */
    ostx_i32 temp_scaled = 235000L;    /* 23.50 °C  (A01, scale=10000) */
    ostx_i32 hum_scaled  = 612000L;    /* 61.20 %RH (C00, scale=10000) */
    ostx_i32 pres_scaled = 1013250000L; /* 101325 Pa (900, scale=10000) */

    /* Send temperature */
    ostx_stream_pack(&s_temp,     g_tid++, ts, temp_scaled, serial_emit, NULL);

    /* Send humidity */
    ostx_stream_pack(&s_humidity, g_tid++, ts, hum_scaled,  serial_emit, NULL);

    /* Send pressure */
    ostx_stream_pack(&s_pressure, g_tid++, ts, pres_scaled, serial_emit, NULL);

    delay(1000);
}
