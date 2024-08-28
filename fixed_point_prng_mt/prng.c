#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <math.h>

#include "prng.h"
#include "fix16.h"

//#define PRNG_USE_LIBC_RAND

#ifndef PRNG_USE_LIBC_RAND

#define K3 1325
#define K4 3125
#define K5 (1<<16)
#define K6 32768.0f
#define K7 16384.0f

#endif


void PRNG_vInit(PRNG_stParms_t* parms, uint16_t diversification)
{
	app_assert(sizeof(uint32_t)==4);
	app_assert(sizeof(uint8_t) == 1);

#ifdef PRNG_USE_LIBC_RAND
	srand(diversification);
#else
	parms->octet = 0;
	//PRNG_stParms.seed = K3^diversification;
	parms->w = diversification;
	parms->z = K4;
#endif
}


PRNG_FLOAT PRNG_fNext(PRNG_stParms_t* parms) {
	return FIX16_fToFloat( PRNG_q16Next(parms) );
}

uint32_t PRNG_u32Next(PRNG_stParms_t* parms) {
#ifdef PRNG_USE_LIBC_RAND
	app_assert(RAND_MAX == 0x7fff);

	return (uint32_t)((rand() << 30) + (rand() << 15) + rand());

#else
	parms->z = 36969 * ( parms->z & 65535 ) + ( parms->z >> 16 );
	parms->w = 18000 * ( parms->w & 65535 ) + ( parms->w >> 16 );

	return ( parms->z << 16 ) + parms->w;
#endif
}

uint8_t PRNG_u8Next(PRNG_stParms_t* parms) {
	if (parms->octet >= 4) {
	    parms->octet = 0;
	}
	if (parms->octet == 0) {
	    parms->octets.dw = PRNG_u32Next(parms);
	}
	return parms->octets.b[parms->octet++];
}


QMTX_FIX16 PRNG_q16Next(PRNG_stParms_t* parms) {
	uint32_t u;
	QMTX_FIX16 q;

	u = PRNG_u32Next(parms);
	q = (fix16_t)(((int32_t)u)>>10);

	return q;
}
