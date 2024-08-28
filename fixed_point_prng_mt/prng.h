
#ifndef COMMON_PRNG_H_
#define COMMON_PRNG_H_

#include "internals.h"

typedef struct PRNG_stParms
{
    union {
        uint32_t dw;
        uint8_t  b[4];
    } octets;
    int octet;

#ifndef PRNG_USE_LIBC_RAND
    //int seed;
    uint32_t w;
    uint32_t z;
#endif
} PRNG_stParms_t;

void PRNG_vInit(PRNG_stParms_t* parms, uint16_t diversification);

PRNG_FLOAT PRNG_fNext(PRNG_stParms_t* parms);

uint32_t PRNG_u32Next(PRNG_stParms_t* parms);

uint8_t PRNG_u8Next(PRNG_stParms_t* parms);

QMTX_FIX16 PRNG_q16Next(PRNG_stParms_t* parms);

#endif /* COMMON_PRNG_H_ */
