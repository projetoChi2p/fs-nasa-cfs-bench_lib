/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as “core Flight System: Bootes”
 *
 * Copyright (c) 2020 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ************************************************************************/

/**
 * @file
 *   Sample CFS library
 */

/*************************************************************************
** Includes
*************************************************************************/
#include "bench_lib_version.h"
#include "bench_lib_internal.h"

#include "cfe_config.h"

/* for "strncpy()" */
//#include <string.h>

/* Baremetal benchmark functions */
#include "mxm_task.h"
#include "huff_bench.h"

/*************************************************************************
** Private Data Structures
*************************************************************************/
//char BENCH_LIB_Buffer[BENCH_LIB_BUFFER_SIZE];

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Library Initialization Routine                                  */
/* cFE requires that a library have an initialization routine      */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 BENCH_LIB_Init(void)
{
    char VersionString[BENCH_LIB_CFG_MAX_VERSION_STR_LEN];

    /*
     * Call a C library function, like strcpy(), and test its result.
     *
     * This is primary for a unit test example, to have more than
     * one code path to exercise.
     *
     * The specification for strncpy() indicates that it should return
     * the pointer to the destination buffer, so it should be impossible
     * for this to ever fail when linked with a compliant C library.
     */
    //if (strncpy(BENCH_LIB_Buffer, "BENCH DATA", sizeof(BENCH_LIB_Buffer) - 1) != BENCH_LIB_Buffer)
    //{
    //    return CFE_STATUS_NOT_IMPLEMENTED;
    //}

    /* ensure termination */
    //BENCH_LIB_Buffer[sizeof(BENCH_LIB_Buffer) - 1] = 0;

    CFE_Config_GetVersionString(VersionString, BENCH_LIB_CFG_MAX_VERSION_STR_LEN, "Benchmark Lib",
        BENCH_LIB_VERSION, BENCH_LIB_BUILD_CODENAME, BENCH_LIB_LAST_OFFICIAL);

    OS_printf("Benchmark Lib Initialized.%s\n", VersionString);

    return CFE_SUCCESS;
}

// /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
// /*                                                                 */
// /* Sample Lib function                                             */
// /*                                                                 */
// /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
// int32 BENCH_LIB_Function(void)
// {
//     OS_printf("BENCH_LIB_Function called, buffer=\'%s\'\n", BENCH_LIB_Buffer);

//     return CFE_SUCCESS;
// }


/*******************************************************************/
/*                                                                 */
/* Matrix Multiplication Benchmark Function                        */
/*                                                                 */
/*******************************************************************/
int32 BENCH_LIB_MxmBenchTask(uint16_t seed, uint16_t* pcheck_a, uint16_t* pcheck_b, uint16_t* pcheck_c)
{
    //OS_printf("BENCH_LIB_MxmBenchTask called.\n");
    //OS_TaskDelay(6);
    // *pcheck_a = 0xFAB1;
    // *pcheck_b = 0xFAB2;
    // *pcheck_c = 0xFAB3+seed;
    mxm_bench_task(seed, pcheck_a, pcheck_b, pcheck_c);

    return CFE_SUCCESS;
}

/*******************************************************************/
/*                                                                 */
/* Huffmann Decoding Benchmark Function                            */
/*                                                                 */
/*******************************************************************/
int32 BENCH_LIB_HuffBenchTask(uint16_t seed, uint8_t* ptable, uint16_t* pcheck_e, uint16_t* pcheck_d)
{
    // OS_printf("BENCH_LIB_HuffBenchTask called.\n");
    // OS_TaskDelay(6);
    // *ptable = 0xF1;
    // *pcheck_e = 0xFAB2;
    // *pcheck_d = 0xFAB3+seed;
    huff_bench_task(seed, ptable, pcheck_e, pcheck_d);
    

    return CFE_SUCCESS;
}

/*********************************************************************
 */
static char cNible2Text(uint8_t nibble) {
    char result;

    if (nibble >= 62) {
        result = '~';
    }
    else if (nibble >= 36) {
        result = nibble + 'a' - 36;
    }
    else if (nibble >= 10) {
        result = nibble + 'A' - 10;
    }
    else {
        result = nibble + '0';
    }

    return result;
}


#define DEBUG_PRINT_NUMBER 0

static void vPrintNumber(
    uint8_t buffer[16],
    const int32_t n,
    const uint8_t base,
    const uint8_t unsigned_flag,
    const uint8_t do_padding,
    const uint8_t pad_character,
    const uint8_t num1
)
{
    int32_t len;
    int32_t negative;
    int32_t i;
    int32_t j;
    uint8_t outbuf[16];
    const uint8_t digits[] = "0123456789ABCDEF";
    uint32_t num;
    for (i = 0; i < sizeof(outbuf); i++) {
        outbuf[i] = '0';
    }

#if DEBUG_PRINT_NUMBER
    printf("\r\nvPrintNumber() ********************\r\nn: %d [%d]\r\n", n, __LINE__);
#endif

    /* Check if number is negative                   */
    if ((unsigned_flag == 0) && (base == 10) && (n < 0L)) {
        negative = 1;
        num = (-(n));
    }
    else {
        num = n;
        negative = 0;
    }

    /* Build number (backwards) in outbuf            */
    i = 0;
    do {
        outbuf[i] = digits[(num % base)];
        i++;
        num /= base;
    } while (num > 0);

    if (negative != 0) {
        outbuf[i] = '-';
        i++;
    }

    outbuf[i] = 0;
    i--;

    len = strlen((char*)outbuf);

#if DEBUG_PRINT_NUMBER
    printf("outbuf: '");
    for (j = 0; j < 16; j++) {
        if (outbuf[j] <= ' ') {
            printf("<%d>", outbuf[j]);
        }
        else {
            printf("%c", outbuf[j]);
        }
    }
    printf("'\r\n");
    printf("len: %d\r\n", len);
    printf("i: %d\r\n", i);
#endif

    if ((do_padding != 0) && (len < num1)) {
        i = len;
        for (; i < num1; i++) {
            outbuf[i] = pad_character;
        }

        outbuf[i] = 0;
        i--;
    }

#if DEBUG_PRINT_NUMBER
    len = strlen(outbuf);
    printf("x outbuf: '");
    for (j = 0; j < 16; j++) {
        if (outbuf[j] <= ' ') {
            printf("<%d>", outbuf[j]);
        }
        else {
            printf("%c", outbuf[j]);
        }
    }
    printf("'\r\n");
    printf("x len: %d\r\n", len);
    printf("x i: %d\r\n", i);
#endif

    for (j = 0; j < 16; j++) {
        buffer[j] = 0;
    }

    j = 0;
    while (&outbuf[i] >= outbuf) {
        buffer[j] = outbuf[i];
        i--;
        j++;
    }

#if DEBUG_PRINT_NUMBER
    buffer[j] = 0;
    printf("z buffer: '");
    for (j = 0; j < 16; j++) {
        if (buffer[j] <= ' ') {
            printf("<%d>", buffer[j]);
        }
        else {
            printf("%c", buffer[j]);
        }
    }
    printf("'\r\n");
#endif
}


/*********************************************************************
 * from MicroNMEA
 */
char* BENCH_LIB_pcGenerateChecksum(char* s, char* checksum) {
    uint8_t c = 0;
    // Initial $ is omitted from checksum, if present ignore it.
    if (*s == '$')
        ++s;

    while (*s != '\0' && *s != '*')
        c ^= *s++;

    if (checksum) {
        checksum[0] = cNible2Text(c / 16);
        checksum[1] = cNible2Text(c % 16);
    }
    return s;
}

uint16_t BENCH_LIB_u16Maj(volatile const uint16_t v1, volatile const uint16_t v2, volatile const uint16_t v3) {
    return ((v2 & v3) | (v1 & v3) | (v1 & v2) | (v1 & v2 & v3));
}



void BENCH_LIB_vPrintU32(uint8_t outbuf[16], const uint32_t n) {
    vPrintNumber(
        outbuf, n,
        10,   /* base */
        1,    /* unsigned_flag */
        0,    /* do_padding */
        ' ',  /* pad_character */
        0     /* num1 */);
}

void BENCH_LIB_vPrintHexU32(uint8_t outbuf[16], const uint32_t n) {
    vPrintNumber(
        outbuf, n,
        16,   /* base */
        1,    /* unsigned_flag */
        1,    /* do_padding */
        '0',  /* pad_character */
        8     /* num1 */);
}

void BENCH_LIB_vPrintHexU16(uint8_t outbuf[16], const uint16_t n) {
    vPrintNumber(
        outbuf, n,
        16,   /* base */
        1,    /* unsigned_flag */
        1,    /* do_padding */
        '0',  /* pad_character */
        4     /* num1 */);
}

void BENCH_LIB_vPrintHexU8(uint8_t outbuf[16], const uint8_t n) {
    vPrintNumber(
        outbuf, n,
        16,   /* base */
        1,    /* unsigned_flag */
        1,    /* do_padding */
        '0',  /* pad_character */
        2     /* num1 */);
}


#ifdef __NO_INLINE__
#define HLP_BUILD_MAY_INLINE 0
#else
#define HLP_BUILD_MAY_INLINE 1
#endif

//__OPTIMIZE__ is defined in all optimizing compilations. 
//__OPTIMIZE_SIZE__ is defined if the compiler is optimizing for size, not speed.
#ifdef __OPTIMIZE__

#ifdef __OPTIMIZE_SIZE__
#define HLP_BUILD_OPTIMIZE (8+1)
#else
#ifdef __OPTIMIZATION_LEVEL__
#define HLP_BUILD_OPTIMIZE ((__OPTIMIZATION_LEVEL__<<1)+1)
#else
#define HLP_BUILD_OPTIMIZE (1)
#endif
#endif

#else
#define HLP_BUILD_OPTIMIZE 0x0
#endif

#define HLP_BUILD_FLAGS ((HLP_BUILD_MAY_INLINE << 4)+HLP_BUILD_OPTIMIZE)

/*********************************************************************
 */
uint8_t BENCH_LIB_u8BuildFlags(void)
{
    return HLP_BUILD_FLAGS;
}



#define FLASH_ICACHE_PREFETCH_ENABLED   (1<<0)
#define FLASH_ICACHE_ENABLED            (1<<1)
#define FLASH_DCACHE_ENABLED            (1<<2)

#define CORTEX_ICACHE_ENABLED           (1<<4)
#define CORTEX_DCACHE_ENABLED           (1<<5)
#define CORTEX_CACHE_WRITE_THROUGH      (1<<6)

uint8_t BENCH_LIB_u8GetCacheSettings(void)
{
    uint8_t cache_settings = 0;
   
    //
    // Vendor specific SoC Flash cache
    //
    // STM ART settings, stm32l4xx_hal_conf.h or stm32f7xx_hal_conf.h
    #ifdef PREFETCH_ENABLE
    #if (PREFETCH_ENABLE)
        cache_settings |= FLASH_ICACHE_PREFETCH_ENABLED;
    #endif
    #endif
    
    // STM ART settings, stm32l4xx_hal_conf.h
    #ifdef INSTRUCTION_CACHE_ENABLE
    #if (INSTRUCTION_CACHE_ENABLE)
        cache_settings |= FLASH_ICACHE_ENABLED;
    #endif
    #endif

    // STM ART settings, stm32f7xx_hal_conf.h
    //For stm32f7 the ART cache covers only accesses through ITCM interface
    // ART_ACCLERATOR_ENABLE (!sic), not ART_ACCELERATOR_ENABLE
    #ifdef ART_ACCLERATOR_ENABLE
    #if (ART_ACCLERATOR_ENABLE)
        cache_settings |= FLASH_ICACHE_ENABLED;
    #endif
    #endif
    
    // Cypress PSoC 5 LP
    #ifdef CYDEV_INSTRUCT_CACHE_ENABLED
    #if (CYDEV_INSTRUCT_CACHE_ENABLED)
        cache_settings |= FLASH_ICACHE_ENABLED;
    #endif
    #endif
        
    // STM ART settings, stm32l4xx_hal_conf.h
    #ifdef DATA_CACHE_ENABLE
    #if (DATA_CACHE_ENABLE)
        cache_settings |= FLASH_DCACHE_ENABLED;
    #endif
    #endif
    
    
    //
    // Arm Cortex CPU cache
    //
    
    //stm32f7xx
    //app_helpers.h see also HLP_vSystemConfig() for MPU_Config() and/or CPU_CACHE_Enable()
    #ifdef HLP_INSTRUCTION_CACHE_ENABLE
    #if (HLP_INSTRUCTION_CACHE_ENABLE)
        cache_settings |= CORTEX_ICACHE_ENABLED;
    #endif
    #endif
            
    #ifdef HLP_DATA_CACHE_ENABLE
    #if (HLP_DATA_CACHE_ENABLE)
        cache_settings |= CORTEX_DCACHE_ENABLED;
    #endif
    #endif
    
    #ifdef HLP_MPU_SET_WRITE_THROUGH
    #if (HLP_MPU_SET_WRITE_THROUGH)
        cache_settings |= CORTEX_CACHE_WRITE_THROUGH;
    #endif
    #endif
    
	//zynq-7000
	#ifdef APP_L1_ICACHE
	#if (APP_L1_ICACHE == 1)
        cache_settings |= CORTEX_ICACHE_ENABLED;
	#endif
    #endif

	#ifdef APP_L1_DCACHE
	#if (APP_L1_DCACHE == 1)
        cache_settings |= CORTEX_DCACHE_ENABLED;
	#endif
    #endif

	#ifdef APP_L1_CACHE_WRITE_THROUGH
	#if (APP_L1_CACHE_WRITE_THROUGH == 1)
        cache_settings |= CORTEX_CACHE_WRITE_THROUGH;
	#endif
    #endif
	
    return cache_settings;
}
