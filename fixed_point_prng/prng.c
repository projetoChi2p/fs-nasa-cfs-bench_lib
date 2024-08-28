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

PRNG_stParms_t PRNG_stParms;

void PRNG_vInit(uint16_t diversification)
{
	app_assert(sizeof(uint32_t)==4);
	app_assert(sizeof(uint8_t) == 1);

#ifdef PRNG_USE_LIBC_RAND
	srand(diversification);
#else
	PRNG_stParms.octet = 0;
	//PRNG_stParms.seed = K3^diversification;
	PRNG_stParms.w = diversification;
	PRNG_stParms.z = K4;
#endif
}


PRNG_FLOAT PRNG_fNext(void) {
	return FIX16_fToFloat( PRNG_q16Next() );
}

uint32_t PRNG_u32Next(void) {
#ifdef PRNG_USE_LIBC_RAND
	app_assert(RAND_MAX == 0x7fff);

	return (uint32_t)((rand() << 30) + (rand() << 15) + rand());

#else
	PRNG_stParms.z = 36969 * ( PRNG_stParms.z & 65535 ) + ( PRNG_stParms.z >> 16 );
	PRNG_stParms.w = 18000 * ( PRNG_stParms.w & 65535 ) + ( PRNG_stParms.w >> 16 );

	return ( PRNG_stParms.z << 16 ) + PRNG_stParms.w;
#endif
}

uint8_t PRNG_u8Next(void) {
	if (PRNG_stParms.octet >= 4) {
		PRNG_stParms.octet = 0;
	}
	if (PRNG_stParms.octet == 0) {
		PRNG_stParms.octets.dw = PRNG_u32Next();
	}
	return PRNG_stParms.octets.b[PRNG_stParms.octet++];
}


QMTX_FIX16 PRNG_q16Next(void) {
	uint32_t u;
	QMTX_FIX16 q;

	u = PRNG_u32Next();
	q = (fix16_t)(((int32_t)u)>>10);

	return q;
}
