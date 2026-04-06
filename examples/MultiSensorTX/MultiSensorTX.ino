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
OSTX_STATIC_DEFINE(s_temp,     0x00000001UL, "T1",  "Cel");
OSTX_STATIC_DEFINE(s_humidity, 0x00000001UL, "H1",  "Pct");
OSTX_STATIC_DEFINE(s_pressure, 0x00000001UL, "P1",  "hPa");

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
     *   23.50 °C   --> 235000L
     *   61.20 %RH  --> 612000L
     *   1013.25 hPa --> 10132500L  (scale=10000 gives 7-char b62)
     *
     * Tip: for pressure at scale 10000 the b62 value is 7 chars.
     * Reduce OSTX_VALUE_SCALE to 100 in ostx_config.h if Flash is
     * very tight; you lose two decimal places of precision.
     */
    ostx_i32 temp_scaled = 235000L;
    ostx_i32 hum_scaled  = 612000L;
    ostx_i32 pres_scaled = 10132500L;

    /* Send temperature */
    ostx_stream_pack(&s_temp,     g_tid++, ts, temp_scaled, serial_emit, NULL);

    /* Send humidity */
    ostx_stream_pack(&s_humidity, g_tid++, ts, hum_scaled,  serial_emit, NULL);

    /* Send pressure */
    ostx_stream_pack(&s_pressure, g_tid++, ts, pres_scaled, serial_emit, NULL);

    delay(1000);
}
