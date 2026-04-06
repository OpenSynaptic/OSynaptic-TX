/* ostx_crc.c -- CRC-8 and CRC-16 for OSynaptic-TX (C89) */
#include "ostx_crc.h"

ostx_u8 ostx_crc8(const ostx_u8 *data, int len, ostx_u16 poly, ostx_u8 init)
{
    ostx_u8 crc;
    int     i;
    int     bit;

    if (!data || len <= 0) { return 0; }

    crc = init;
    for (i = 0; i < len; ++i) {
        crc = (ostx_u8)(crc ^ data[i]);
        for (bit = 0; bit < 8; ++bit) {
            if (crc & 0x80u) {
                crc = (ostx_u8)((crc << 1) ^ (ostx_u8)poly);
            } else {
                crc = (ostx_u8)(crc << 1);
            }
        }
    }
    return crc;
}

ostx_u16 ostx_crc16(const ostx_u8 *data, int len, ostx_u16 poly, ostx_u16 init)
{
    ostx_u16 crc;
    int      i;
    int      bit;

    if (!data || len <= 0) { return 0; }

    crc = init;
    for (i = 0; i < len; ++i) {
        crc = (ostx_u16)(crc ^ ((ostx_u16)data[i] << 8));
        for (bit = 0; bit < 8; ++bit) {
            if (crc & 0x8000u) {
                crc = (ostx_u16)((crc << 1) ^ poly);
            } else {
                crc = (ostx_u16)(crc << 1);
            }
        }
    }
    return crc;
}
