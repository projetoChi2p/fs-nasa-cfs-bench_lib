
#ifndef COMMON_PRNG_H_
#define COMMON_PRNG_H_

#include "internals.h"

void PRNG_vInit(uint16_t diversification);

PRNG_FLOAT PRNG_fNext(void);

uint32_t PRNG_u32Next(void);

uint8_t PRNG_u8Next(void);

QMTX_FIX16 PRNG_q16Next(void);

#endif /* COMMON_PRNG_H_ */
