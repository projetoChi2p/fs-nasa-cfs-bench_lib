
#ifndef COMMON_FIX16_H_
#define COMMON_FIX16_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/***************************************************************************
 */

#include "internals.h"

/***************************************************************************
 */

#ifdef QMTX_FIX16_IS_FLOAT

#define FIX16_CONST(A) ((fix16_t)(A))

#else /* QMTX_FIX16_IS_FLOAT */

#define FIX16_ONE      ((fix16_t)0x00010000)
#define FIX16_MAXIMUM  ((fix16_t)0x7FFFFFFF)
#define FIX16_MINIMUM  ((fix16_t)0x80000000)
#define FIX16_OVERFLOW ((fix16_t)0x80000000)

#define FIX16_CONST(A) (((A) >= 0) ? ((fix16_t)((A)*(FIX16_ONE)+0.5)) : ((fix16_t)((A)*(FIX16_ONE)-0.5)))

#endif /* !QMTX_FIX16_IS_FLOAT */


/***************************************************************************
 * Public (extern) functions prototypes
 */

/** Initializes the module.
 */
void FIX16_vInit(void);

/** Terminate the module.
 */
void FIX16_vTerm(void);


/****************************************************************************
 * Convert a floating point to fraction representation.
 */
fix16_t FIX16_qFromFloat(const float a);


/****************************************************************************
 */
float FIX16_fToFloat(const fix16_t);

#ifdef QMTX_FIX16_IS_FLOAT

/****************************************************************************
 */
static inline fix16_t FIX16_qAdd(const fix16_t a, const fix16_t b)
{
	return a + b;
}

static inline fix16_t FIX16_qMul(const fix16_t a, const fix16_t b)
{
	return a * b;
}

#else /* QMTX_FIX16_IS_FLOAT */


// See also https://code.google.com/archive/p/libfixmath/
// See also https://github.com/Yveaux/Arduino_fixpt

/****************************************************************************
 */
static inline fix16_t FIX16_qAdd(const fix16_t a, const fix16_t b)
{
	uint32_t _a = a, _b = b;
	uint32_t sum = _a + _b;

	if (!((_a ^ _b) & 0x80000000) && ((_a ^ sum) & 0x80000000)) {
		return FIX16_OVERFLOW;
	}

	return sum;
}

/****************************************************************************
 */
static inline fix16_t FIX16_qSub(const fix16_t a, const fix16_t b)
{
	uint32_t _a = a, _b = b;
	uint32_t diff = _a - _b;

	if (((_a ^ _b) & 0x80000000) && ((_a ^ diff) & 0x80000000))
		return FIX16_OVERFLOW;

	return diff;
}

/****************************************************************************
 */
static inline fix16_t FIX16_qMul(const fix16_t inArg0, const fix16_t inArg1)
{
	int32_t A = (inArg0 >> 16), C = (inArg1 >> 16);
	uint32_t B = (inArg0 & 0xFFFF), D = (inArg1 & 0xFFFF);

	int32_t AC = A*C;
	int32_t AD_CB = A*D + C*B;
	uint32_t BD = B*D;

	int32_t product_hi = AC + (AD_CB >> 16);

	uint32_t ad_cb_temp = AD_CB << 16;
	uint32_t product_lo = BD + ad_cb_temp;
	if (product_lo < BD)
		product_hi++;

#ifndef FIXMATH_NO_OVERFLOW
	if ( ((product_hi & 0xFFFF8000) != 0xFFFF8000) && ((product_hi & 0xFFFF8000) != 0x00000000) ) {
	    return FIX16_OVERFLOW;
	}
#endif

#ifdef FIXMATH_NO_ROUNDING
	return (product_hi << 16) | (product_lo >> 16);
#else
	uint32_t product_lo_tmp = product_lo;
	product_lo -= 0x8000;
	product_lo -= (uint32_t)product_hi >> 31;
	if (product_lo > product_lo_tmp)
		product_hi--;
	fix16_t result = (product_hi << 16) | (product_lo >> 16);
	result += 1;
	return result;
#endif
}


/****************************************************************************
 */
static inline fix16_t FIX16_qAddSaturating(const fix16_t a, const fix16_t b)
{
	fix16_t result = FIX16_qAdd(a, b);

	if (result == FIX16_OVERFLOW) {
		return (a >= 0) ? FIX16_MAXIMUM : FIX16_MINIMUM;
	}

	return result;
}


/****************************************************************************
 */
static inline fix16_t FIX16_qSubSaturating(const fix16_t a, const fix16_t b)
{
	fix16_t result = FIX16_qSub(a, b);

	if (result == FIX16_OVERFLOW)
		return (a >= 0) ? FIX16_MAXIMUM : FIX16_MINIMUM;

	return result;
}


/****************************************************************************
 */
static inline fix16_t FIX16_qMulSaturating(const fix16_t a, const fix16_t b) {
	fix16_t result = FIX16_qMul(a, b);

	if (result == FIX16_OVERFLOW)
	{
		if ((a >= 0) == (a >= 0))
			return FIX16_MAXIMUM;
		else
			return FIX16_MINIMUM;
	}

	return result;
}

/****************************************************************************
 */
static inline fix16_t FIX16_qAdd5(const fix16_t a, const fix16_t b, const fix16_t c, const fix16_t d, const fix16_t e)
{
	fix16_t bc;
	fix16_t de;
	fix16_t sum;

	bc = FIX16_qAdd(b, c);
	if (bc == FIX16_OVERFLOW)
	{
		return FIX16_OVERFLOW;
	}

	de = FIX16_qAdd(d, e);
	if (de == FIX16_OVERFLOW)
	{
		return FIX16_OVERFLOW;
	}

	sum = FIX16_qAdd(bc, de);
	if (sum == FIX16_OVERFLOW)
	{
		return FIX16_OVERFLOW;
	}

	sum = FIX16_qAdd(a, sum);

	return sum;
}

#endif /* !QMTX_FIX16_IS_FLOAT */



#ifdef __cplusplus
        }
#endif /* __cplusplus */

#endif /* COMMON_FIX16_H_ */
