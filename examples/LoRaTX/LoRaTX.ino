/*
 * LoRaTX.ino -- OSynaptic-TX over LoRa (SX1276/SX1278 via LoRa.h)
 * =================================================================
 * Demonstrates collecting each frame byte into a small buffer, then
 * sending the complete datagram as a single LoRa packet.
 *
 * This is the recommended pattern for LoRa:
 *   - LoRa modules expect a complete payload passed to beginPacket/write/endPacket
 *   - Minimum frame is ~21-40 bytes -- comfortably inside LoRa 255-byte limit
 *   - At SF7 / BW125 / CR4/5: a 30-byte payload takes ~15 ms air time (EU 868 MHz)
 *   - At SF12 / BW125 / CR4/8: same payload takes ~2.7 s -- plan send intervals carefully
 *
 * Library required: "LoRa" by Sandeep Mistry
 *   Arduino IDE → Sketch > Include Library > Manage Libraries → search "LoRa"
 *
 * Compatible boards:
 *   - Heltec LoRa 32 (ESP32 + SX1276)
 *   - TTGO LoRa32 (ESP32 + SX1276)
 *   - Arduino Uno + LoRa shield (SX1276)
 *   - Any AVR/ESP32 board with SX1276/SX1278 on SPI
 *
 * Wiring (typical Heltec LoRa 32 v2):
 *   SCK  = GPIO5   MISO = GPIO19   MOSI = GPIO27
 *   SS   = GPIO18  RST  = GPIO14   DIO0 = GPIO26
 *
 * Memory (ATmega328P, API C):
 *   Stack peak  : ~27 B (ostx_stream_pack + lora_emit stub)
 *   Static RAM  : OSTX_PACKET_MAX (96 B default, for frame accumulation)
 *
 * NOTE: Unlike the UART/UDP emit callbacks that accept a byte at a time,
 * LoRa.write() in packet mode is fine byte-by-byte after beginPacket().
 * The frame_buf below is optional -- kept here to show the pattern for
 * transports that need a contiguous buffer before transmission.
 */

#include <SPI.h>
#include <LoRa.h>
#include <OSynaptic-TX.h>

/* ------------------------------------------------------------------
 * LoRa pin mapping -- uncomment / adjust for your board
 * ------------------------------------------------------------------ */
/* Heltec LoRa 32 v2 */
#define LORA_SCK  5
#define LORA_MISO 19
#define LORA_MOSI 27
#define LORA_SS   18
#define LORA_RST  14
#define LORA_DIO0 26

/* Arduino Uno + LoRa shield (uncomment and comment the Heltec block):
#define LORA_SCK  13
#define LORA_MISO 12
#define LORA_MOSI 11
#define LORA_SS   10
#define LORA_RST  9
#define LORA_DIO0 2
*/

/* LoRa frequency -- match your region's ISM band */
static const long LORA_FREQ = 915E6;  /* 915 MHz (Americas). Use 868E6 for EU. */

/* ------------------------------------------------------------------
 * Sensor descriptor (Flash)
 * ------------------------------------------------------------------ */
OSTX_STATIC_DEFINE(s_temp, 0x00000001UL, "T1", "Cel");

/* ------------------------------------------------------------------
 * Streaming emit -- accumulate into a static buffer, then LoRa sends
 * the entire frame atomically in loop().
 * ------------------------------------------------------------------ */
static ostx_u8 s_frame_buf[OSTX_PACKET_MAX];
static int     s_frame_len = 0;

static void lora_buf_emit(ostx_u8 b, void * /*ctx*/)
{
    if (s_frame_len < OSTX_PACKET_MAX) {
        s_frame_buf[s_frame_len++] = b;
    }
}

static ostx_u8 g_tid = 0u;

/* ------------------------------------------------------------------
 * setup()
 * ------------------------------------------------------------------ */
void setup()
{
    Serial.begin(115200);
    while (!Serial) { ; }

    /* LoRa SPI pins */
    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
    LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

    if (!LoRa.begin(LORA_FREQ)) {
        Serial.println(F("LoRa init failed! Check wiring."));
        while (true) { ; }
    }

    /*
     * Spreading factor / bandwidth / coding rate trade-off:
     *
     *   SF  BW(kHz) CR   Air time (30B)  Range (LoS)  Use case
     *   7   125     4/5  ~15 ms          ~2 km        Fast, low power
     *   9   125     4/5  ~100 ms         ~5 km        Balance
     *   12  125     4/8  ~2.7 s          ~15 km       Max range (duty cycle!)
     */
    LoRa.setSpreadingFactor(9);
    LoRa.setSignalBandwidth(125E3);
    LoRa.setCodingRate4(5);

    Serial.println(F("OSynaptic-TX LoRaTX ready"));
}

/* ------------------------------------------------------------------
 * loop() -- send one temperature reading every 10 seconds.
 *
 * IMPORTANT: Respect local duty cycle regulations.
 *   EU 868 MHz sub-band 1 (g): max 1% duty cycle.
 *   A 30-byte payload at SF9/BW125 takes ~100 ms, so min interval = 10 s.
 * ------------------------------------------------------------------ */
void loop()
{
    ostx_u32 ts = (ostx_u32)(millis() / 1000UL);

    /* Build frame into buffer */
    s_frame_len = 0;
    int len = ostx_stream_pack(&s_temp, g_tid++, ts, 235000L /* 23.5 °C */,
                               lora_buf_emit, NULL);

    if (len > 0 && s_frame_len == len) {
        LoRa.beginPacket();
        LoRa.write(s_frame_buf, (size_t)s_frame_len);
        LoRa.endPacket();

        Serial.print(F("LoRa sent "));
        Serial.print(s_frame_len);
        Serial.println(F(" bytes"));
    }

    /* 10-second interval -- adjust based on SF and duty cycle constraints */
    delay(10000);
}
