
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <stdint.h>

#include "mxm_task.h"
#include "internals.h"
#include "mxm_defs.h"
#include "qmtx.h"
#include "prng.h"


static QMTX_FIX16 qa_values[MAT_A_ROWS*MAT_A_COLS];
static QMTX_FIX16 qb_values[MAT_B_ROWS*MAT_B_COLS];
static QMTX_FIX16 qc_values[MAT_C_ROWS*MAT_C_COLS];

uint8_t qa_flags;
uint8_t qb_flags;
uint8_t qc_flags;


#undef MXM_DEBUG

#undef MXM_VERBOSE

//#define MXM_DEBUG
//#define MXM_VERBOSE

#ifdef MXM_VERBOSE
    #define mxm_printf(x,...) printf(x, ##__VA_ARGS__)
#endif

static __inline uint16_t _u16Maj(volatile const uint16_t v1, volatile const uint16_t v2, volatile const uint16_t v3) {
    return ( (v2 & v3) | (v1 & v3) | (v1 & v2) | (v1 & v2 & v3) );
}


void mxm_bench_task(uint16_t seed, uint16_t* pcheck_a, uint16_t* pcheck_b, uint16_t* pcheck_c) {

	uint16_t check;

    volatile uint16_t check_a_1;
    volatile uint16_t check_a_2;
    volatile uint16_t check_a_3;

    volatile uint16_t check_b_1;
    volatile uint16_t check_b_2;
    volatile uint16_t check_b_3;

	qa_flags = 0;
	qb_flags = 0;
	qc_flags = 0;

    (*pcheck_a) = 0;
    (*pcheck_b) = 0;
    (*pcheck_c) = 0;

#ifdef MXM_VERBOSE
	app_assert(MAT_A_ROWS==MAT_C_ROWS);
	app_assert(MAT_A_COLS==MAT_B_ROWS);
	app_assert(MAT_B_COLS==MAT_C_COLS);
#endif

#ifdef MXM_VERBOSE
	mxm_printf("Diversification is %x\r\n", seed);
#endif
    PRNG_vInit(seed);
    QMTX_vFillRandomized(qa_values, &qa_flags, MAT_A_ROWS, MAT_A_COLS);
    check = QMTX_u16LRC(qa_values, MAT_A_ROWS, MAT_A_COLS);
    check_a_1 = check;
    check_a_2 = check;
    check_a_3 = check;
#ifdef MXM_VERBOSE
    mxm_printf("Checksum A is %x\r\n", check);
#endif
    QMTX_vFillRandomized(qb_values, &qb_flags, MAT_B_ROWS, MAT_B_COLS);
    check = QMTX_u16LRC(qb_values, MAT_B_ROWS, MAT_B_COLS);
    check_b_1 = check;
    check_b_2 = check;
    check_b_3 = check;
#ifdef MXM_VERBOSE
    mxm_printf("Checksum B is %x\r\n", check);
#endif
    QMTX_vMult(qa_values, qa_flags, qb_values, qb_flags, qc_values, &qc_flags, MAT_A_ROWS, MAT_A_COLS, MAT_B_COLS);
    check = QMTX_u16LRC(qc_values, MAT_C_ROWS, MAT_C_COLS);
    (*pcheck_c) = check;
    (*pcheck_a) = _u16Maj(check_a_1, check_a_2, check_a_3);
    (*pcheck_b) = _u16Maj(check_b_1, check_b_2, check_b_3);
#ifdef MXM_VERBOSE
    mxm_printf("Checksum C is %x\r\n", check);
#endif

#ifdef MXM_DEBUG
    app_printf("Fixed A\r\n");
    QMTX_vDump(qa_values, qa_flags, MAT_A_ROWS, MAT_A_COLS, 8, 8);

    app_printf("Fixed B\r\n");
    QMTX_vDump(qb_values, qb_flags, MAT_B_ROWS, MAT_B_COLS, 8, 8);

    app_printf("Fixed C=A*B\r\n");
    QMTX_vDump(qc_values, qc_flags, MAT_C_ROWS, MAT_C_COLS, 8, 8);
#endif


#ifdef MXM_VERBOSE
    mxm_printf("Processing cycle completed.\r\n");
#endif
}

