#ifndef OSTX_CRC_H
#define OSTX_CRC_H

#include "ostx_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * CRC-8 over 'len' bytes.
 * OpenSynaptic uses poly=0x07, init=0x00 (CRC-8/SMBUS).
 * Returns 0 if data is NULL or len <= 0.
 */
ostx_u8  ostx_crc8 (const ostx_u8 *data, int len, ostx_u16 poly, ostx_u8  init);

/*
 * CRC-16 over 'len' bytes.
 * OpenSynaptic uses poly=0x1021, init=0xFFFF (CRC-16/CCITT-FALSE).
 * Returns 0 if data is NULL or len <= 0.
 */
ostx_u16 ostx_crc16(const ostx_u8 *data, int len, ostx_u16 poly, ostx_u16 init);

#ifdef __cplusplus
}
#endif

#endif /* OSTX_CRC_H */
