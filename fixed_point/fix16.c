
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <math.h>

#include "internals.h"
#include "fix16.h"

/***************************************************************************
 * Local definitions
 */

/***************************************************************************
 * Public (extern'd) functions implementation
 */


/****************************************************************************
 */
void FIX16_vInit(void)
{
	app_assert(sizeof(fix16_t)==4);
	app_assert(sizeof(QMTX_FIX16)==4);
	app_assert(sizeof(int32_t)==4);
	app_assert(sizeof(uint16_t)==2);
	app_assert(sizeof(uint8_t)==1);
}



/****************************************************************************
 */
void FIX16_vTerm(void)
{
	/* no action */
}


#ifdef QMTX_FIX16_IS_FLOAT

fix16_t FIX16_qFromFloat(const float a) {
	return a;
}

float FIX16_fToFloat(const fix16_t a) {
	return a;
}


#else /* QMTX_FIX16_IS_FLOAT */

/****************************************************************************
 */
fix16_t FIX16_qFromFloat(const float a) {
	float temp = a * FIX16_ONE;
#ifndef FIXMATH_NO_ROUNDING
	temp += (temp >= 0) ? 0.5f : -0.5f;
#endif
	return (fix16_t)temp;
}



/****************************************************************************
 */
float FIX16_fToFloat(const fix16_t a) {
	return (float)a / FIX16_ONE;
}

#endif /* QMTX_FIX16_IS_FLOAT */
