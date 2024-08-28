
#ifndef _internals_h_
#define _internals_h_

#include <stdint.h>
#include <stddef.h>
#ifndef __LPC11UXX__
#include <assert.h>
#endif

#if defined(__LPC11UXX__)|defined(CORTEX_M0)
	#define app_assert(x)
#else
	#define app_assert(x) assert(x)
#endif

#if defined(__LPC11UXX__)
#define app_printf(x,...)
#else
#define app_printf(x,...) printf(x, ##__VA_ARGS__)
#endif


#if defined(__GNUC__)
#define UNUSED_PARAMETER(a) (void)(a)
#endif

//#define FIXMATH_NO_OVERFLOW

#define FMTX_FLOAT float
#define PRNG_FLOAT FMTX_FLOAT

typedef int32_t fix16_t;

#define QMTX_FIX16 fix16_t

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#endif /* #ifndef _internals_h_ */
