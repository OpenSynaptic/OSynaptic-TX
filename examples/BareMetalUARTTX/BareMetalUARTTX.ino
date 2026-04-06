/*
 * BareMetalUARTTX.ino -- OSynaptic-TX on ATmega328P without Arduino runtime
 * ==========================================================================
 * Shows ostx_stream_pack() driving a bare-metal UART transmitter directly
 * via hardware registers. No Serial / Stream overhead. Purpose:
 *
 *   - Demonstrate the absolute minimum integration: 7 statements total
 *   - Confirm the library is usable in strict C99/C89 embedded contexts
 *   - Provide a template for IAR, Microchip XC8, SDCC targets
 *
 * The sketch is STILL uploaded via the Arduino IDE, but the emit callback
 * accesses USART0 hardware registers directly. On non-AVR boards, replace
 * the uart_emit() body with your UART TDR write sequence.
 *
 * Compatible boards: Arduino Uno, Nano, Pro Mini (ATmega328P @ 5 V/16 MHz).
 * Not directly portable to ESP32/STM32 without editing uart_emit().
 *
 * Memory (ATmega328P):
 *   Stack peak  : ~27 B
 *   Static RAM  : 0 B
 *   Flash total : ~780 B (ostx_stream + min startup)
 *
 * Compare with typical Arduino Serial sketch:
 *   Serial.begin(9600) alone pulls in ~400 B of Stream/Print overhead.
 */

#include <avr/io.h>
#include <OSynaptic-TX.h>

/* ------------------------------------------------------------------
 * ATmega328P USART0 bare-metal initialisation at 9600 baud / 16 MHz
 * UBRR = F_CPU / (16 * baud) - 1 = 16000000 / (16 * 9600) - 1 = 103
 * ------------------------------------------------------------------ */
static void uart_init(void)
{
    UBRR0H = 0;
    UBRR0L = 103u;                  /* 9600 baud @ 16 MHz              */
    UCSR0B = (1u << TXEN0);         /* enable TX only                  */
    UCSR0C = (1u << UCSZ01) | (1u << UCSZ00); /* 8-N-1               */
}

/* ------------------------------------------------------------------
 * Bare-metal byte emitter -- blocks until UDR0 is ready, then writes.
 * ------------------------------------------------------------------ */
static void uart_emit(ostx_u8 b, void * /*ctx*/)
{
    while (!(UCSR0A & (1u << UDRE0))) { ; } /* wait for UDRE (Data Reg Empty) */
    UDR0 = b;
}

/* ------------------------------------------------------------------
 * Sensor descriptor in Flash
 * ------------------------------------------------------------------ */
OSTX_STATIC_DEFINE(s_temp, 0x00000001UL, "T1", "Cel");

static ostx_u8  g_tid     = 0u;
static ostx_u32 g_ts_sec  = 0u;   /* simple free-running second counter */

/* ------------------------------------------------------------------
 * setup() -- replaces Arduino init path for UART
 * ------------------------------------------------------------------ */
void setup()
{
    uart_init();
}

/* ------------------------------------------------------------------
 * loop() -- send one frame per second using a millis()-based timer.
 * millis() is still functional because Arduino init() runs before
 * setup(); only Serial is bypassed.
 * ------------------------------------------------------------------ */
static unsigned long s_last_ms = 0UL;

void loop()
{
    unsigned long now = millis();
    if (now - s_last_ms < 1000UL) return;
    s_last_ms = now;

    g_ts_sec = (ostx_u32)(now / 1000UL);

    /*
     * Replace 235000L with your ADC conversion result × 10000.
     * Example for a 10-bit ADC on a 10 kΩ NTC thermistor:
     *   float voltage = adc_val * (5.0f / 1023.0f);
     *   float temp_c  = (voltage - 2.5f) / 0.01f;  // sensor-specific
     *   ostx_i32 scaled = (ostx_i32)(temp_c * 10000.0f);
     */
    ostx_stream_pack(&s_temp, g_tid++, g_ts_sec, 235000L, uart_emit, NULL);
}
