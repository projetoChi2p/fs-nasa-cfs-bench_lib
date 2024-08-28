
#ifndef COMMON_QMTX_H_
#define COMMON_QMTX_H_

#include "internals.h"
#include "mxm_defs.h"

#define FIXMATRIX_OVERFLOW 0x01

void QMTX_vFillRandomized (QMTX_FIX16* values, uint8_t* flags, const uint8_t r, const uint8_t c);

void QMTX_vFillZeros (QMTX_FIX16* values, uint8_t* flags, const uint8_t r, const uint8_t c);

void QMTX_vMult (const QMTX_FIX16* a_values, const uint8_t a_flags, const QMTX_FIX16* b_values, const uint8_t b_flags, QMTX_FIX16* c_values, uint8_t* c_flags, const uint8_t l, const uint8_t m, const uint8_t n);

void QMTX_vDump (const QMTX_FIX16* values, const uint8_t flags, const uint8_t r, const uint8_t c, uint8_t r_max, uint8_t c_max);

void QMTX_vInject (QMTX_FIX16* values, const uint8_t r, const uint8_t c);

void QMTX_vCopy (const QMTX_FIX16* a_values, const uint8_t a_flags, QMTX_FIX16* b_values, uint8_t *b_flags, const uint8_t r, const uint8_t c);

void QMTX_vSub (const QMTX_FIX16* a_values, const uint8_t a_flags, const QMTX_FIX16* b_values, const uint8_t b_flags, QMTX_FIX16* c_values, uint8_t* c_flags, const uint8_t r, const uint8_t c);

QMTX_FIX16 QMTX_q16Sum (const QMTX_FIX16* a_values, const uint8_t r, const uint8_t c);

uint8_t QMTX_u8LRC (QMTX_FIX16* values, const uint8_t r, const uint8_t c);

uint16_t QMTX_u16LRC (QMTX_FIX16* values, const uint8_t r, const uint8_t c);

uint8_t QMTX_u8J1708 (QMTX_FIX16* values, const uint8_t r, const uint8_t c);

uint8_t QMTX_u8Fletcher (QMTX_FIX16* values, const uint8_t r, const uint8_t c);

uint16_t QMTX_u16Fletcher (QMTX_FIX16* values, const uint8_t r, const uint8_t c);

uint8_t QMTX_u8Pearson (QMTX_FIX16* values, const uint8_t r, const uint8_t c);

uint16_t QMTX_u16Combined (QMTX_FIX16* values, const uint8_t r, const uint8_t c);

#endif /* COMMON_QMTX_H_ */
