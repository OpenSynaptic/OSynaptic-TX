/*
 * ESP32UdpTX.ino -- OSynaptic-TX over WiFi UDP (ESP32 / ESP8266)
 * ================================================================
 * Demonstrates the streaming API (API C) with a UDP emit callback
 * instead of Serial.write. Each ostx_stream_pack() call adds bytes
 * to a UDP datagram that is flushed after the last byte is emitted.
 *
 * This is the recommended transport for ESP32 / ESP8266 nodes:
 *   - No TCP connection state overhead
 *   - Single UDP datagram per sensor frame (~21–40 bytes)
 *   - lwIP (built-in) handles fragmentation automatically
 *   - Latency < 1 ms on a local LAN
 *
 * Compatible boards: ESP32 DevKit v1, ESP32-S3, ESP8266 NodeMCU.
 *
 * Memory (ESP32):
 *   Stack peak  : ~27 B (ostx_stream_pack + emit stub)
 *   Static RAM  : ~0.3 KB (WiFiUDP object + globals; heap not used)
 *
 * Wiring: WiFi AP must be reachable; OpenSynaptic server must
 * listen on REMOTE_IP:REMOTE_PORT for UDP datagrams.
 */

#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSynaptic-TX.h>

/* ------------------------------------------------------------------
 * Network config -- edit to match your environment
 * ------------------------------------------------------------------ */
static const char * WIFI_SSID = "YourSSID";
static const char * WIFI_PASS = "YourPassword";

static const IPAddress REMOTE_IP(192, 168, 1, 100); /* OpenSynaptic server */
static const uint16_t  REMOTE_PORT = 9000u;
static const uint16_t  LOCAL_PORT  = 58925u;

static const unsigned long SEND_INTERVAL_MS    = 1000UL;
static const unsigned long WIFI_RETRY_MS       = 3000UL;

/* ------------------------------------------------------------------
 * Sensor descriptors (Flash)
 * ------------------------------------------------------------------ */
OSTX_STATIC_DEFINE(s_temp, 0x00000001UL, "T1", OSTX_UNIT(Cel));
OSTX_STATIC_DEFINE(s_hum,  0x00000001UL, "H1", OSTX_UNIT(pct));

/* ------------------------------------------------------------------
 * UDP state
 * ------------------------------------------------------------------ */
static WiFiUDP g_udp;
static bool    g_udp_ready = false;

/* emit callback -- buffer bytes inside the open UDP datagram.
   The datagram is started/flushed around each ostx_stream_pack()
   call in loop() so we never mix bytes from two different frames. */
static void udp_emit(ostx_u8 b, void * /*ctx*/)
{
    g_udp.write(b);
}

/* ------------------------------------------------------------------
 * Helpers
 * ------------------------------------------------------------------ */
static unsigned long s_last_wifi_retry = 0UL;

static bool wifi_ensure_connected()
{
    if (WiFi.status() == WL_CONNECTED) return true;

    unsigned long now = millis();
    if (now - s_last_wifi_retry < WIFI_RETRY_MS) return false;
    s_last_wifi_retry = now;

    Serial.println(F("WiFi: reconnecting..."));
    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    return false;
}

static bool udp_ensure_ready()
{
    if (g_udp_ready) return true;
    g_udp.stop();
    g_udp_ready = g_udp.begin(LOCAL_PORT);
    return g_udp_ready;
}

static ostx_u8 g_tid = 0u;
static unsigned long s_last_send = 0UL;

/* ------------------------------------------------------------------
 * setup()
 * ------------------------------------------------------------------ */
void setup()
{
    Serial.begin(115200);
    Serial.println(F("OSynaptic-TX ESP32UdpTX starting"));

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print(F("Connecting to WiFi"));
    unsigned long t0 = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t0 < 10000UL) {
        delay(250);
        Serial.print('.');
    }
    Serial.println();
    if (WiFi.status() == WL_CONNECTED) {
        Serial.print(F("IP: "));
        Serial.println(WiFi.localIP());
    }

    udp_ensure_ready();
}

/* ------------------------------------------------------------------
 * loop()
 * ------------------------------------------------------------------ */
void loop()
{
    if (!wifi_ensure_connected() || !udp_ensure_ready()) {
        delay(10);
        return;
    }

    unsigned long now = millis();
    if (now - s_last_send < SEND_INTERVAL_MS) return;
    s_last_send = now;

    ostx_u32 ts = (ostx_u32)(now / 1000UL);

    /* --- Temperature frame ---------------------------------------- */
    g_udp.beginPacket(REMOTE_IP, REMOTE_PORT);
    ostx_stream_pack(&s_temp, g_tid++, ts, 235000L /* 23.5 °C */,
                     udp_emit, NULL);
    g_udp.endPacket();

    /* --- Humidity frame ------------------------------------------- */
    g_udp.beginPacket(REMOTE_IP, REMOTE_PORT);
    ostx_stream_pack(&s_hum, g_tid++, ts, 612000L /* 61.2 %RH */,
                     udp_emit, NULL);
    g_udp.endPacket();

    Serial.print(F("Sent 2 frames, tid="));
    Serial.println(g_tid - 1);
}
