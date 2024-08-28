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
 * \file
 *   Specification for the bench library functions.
 */
#ifndef BENCH_LIB_H
#define BENCH_LIB_H

/************************************************************************
** Includes
*************************************************************************/
#include "cfe.h"

/************************************************************************
** Type Definitions
*************************************************************************/

/*************************************************************************
** Exported Functions
*************************************************************************/

/************************************************************************/
/** \brief Library Initialization Function
**
**  \par Description
**        This function is required by CFE to initialize the library
**        It should be specified in the cfe_es_startup.scr file as part
**        of loading this library.  It is not directly invoked by
**        applications.
**
**  \par Assumptions, External Events, and Notes:
**        None
**
**  \return Execution status, see \ref CFEReturnCodes
**
**
*************************************************************************/
int32 BENCH_LIB_Init(void);

/************************************************************************/
/** \brief Matrix Multiplication Benchmark Function
**
**  \par Description
**        This benchmark uses seed to randomize synthetic data 
**        to fill matrices A and B, computes checksum of that
**        matrices, compute the multiplication C=A*B, and,
**        finally, computed the checksum of result matrix C.
**
**  \par Assumptions, External Events, and Notes:
**        None
**
**  \return Execution status, see \ref CFEReturnCodes
**
**
*************************************************************************/
int32 BENCH_LIB_MxmBenchTask(uint16_t seed, uint16_t* pcheck_a, uint16_t* pcheck_b, uint16_t* pcheck_c);

/************************************************************************/
/** \brief Huffmann Decoding Benchmark Function
**
**  \par Description
**        This benchmark uses the seed to randomize a code table
**        and an encoded message, computes que checksum of the
**        coded message, decodes the message, and computes the
**        checksum of the decoded message.
**
**  \par Assumptions, External Events, and Notes:
**        None
**
**  \return Execution status, see \ref CFEReturnCodes
**
**
*************************************************************************/
int32 BENCH_LIB_HuffBenchTask(uint16_t seed, uint8_t* ptable, uint16_t* pcheck_e, uint16_t* pcheck_d);

char* BENCH_LIB_pcGenerateChecksum(char* s, char* checksum);
uint16_t BENCH_LIB_u16Maj(volatile const uint16_t v1, volatile const uint16_t v2, volatile const uint16_t v3);
void BENCH_LIB_vPrintU32(uint8_t outbuf[16], const uint32_t n);
void BENCH_LIB_vPrintHexU32(uint8_t outbuf[16], const uint32_t n);
void BENCH_LIB_vPrintHexU16(uint8_t outbuf[16], const uint16_t n);
void BENCH_LIB_vPrintHexU8(uint8_t outbuf[16], const uint8_t n);
uint8_t BENCH_LIB_u8GetCacheSettings(void);
uint8_t BENCH_LIB_u8BuildFlags(void);

#endif
