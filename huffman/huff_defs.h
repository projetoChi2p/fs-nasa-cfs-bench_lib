
#ifndef __HUFF_DEFS_H_
#define __HUFF_DEFS_H_

#include <assert.h>
#include <stdio.h>

#include "huffman.h"

//#define huff_printf(x,...) printf(x, ##__VA_ARGS__)
//#define huff_assert(x) assert(x)

#define HUFF_TEST_MAX_ENTRIES   32
#define HUFF_MAX_TABLES_MASK  0x1F

#define HUFF_DO_DECODE
#define HUFF_DO_ENCODE
#define HUFF_DO_ENCODE_COMPRESS
//#define HUFF_DO_ENCODE_DECODE_TEST
#define HUFF_DO_ENCODE_DECODE_TEST_ALL
//#define HUFF_DO_ENCODE_DECODE_TEST_ALL
#undef HUFF_PARANOID


//#define HUFFANDPUFF_TEST
#undef HUFF_DEBUG_TREE

#ifndef HUFFANDPUFF_TEST
#ifdef CORTEX_M0
#define HUFF_TABLES_ENABLEx (\
	(1<<0)+   /* ASCII alice */ \
	(1<<1)+   /* ASCII as you like it */ \
	(1<<3)+   /* ASCII bible */ \
	(1<<6)+   /* ASCII HTML */ \
	(1<<7)+   /* ASCII DNA */ \
    (1<<11)+  /* Binary Excel */ \
	(1<<14)+  /* Binary Object code obj1 */ \
	(1<<15)+  /* Binary Object code obj2 */ \
	(1<<26)+  /* Binany Fax image ptt5 */ \
	(1<<27)   /* Binany Machine code sum */ \
)
#define HUFF_TABLES_ENABLE 0xFFFFFFFF
#else
#define HUFF_TABLES_ENABLE 0xFFFFFFFF
#endif
#endif


#ifdef HUFFANDPUFF_TEST
#ifndef HUFF_DO_ENCODE_DECODE_TEST
#define HUFF_DO_ENCODE_DECODE_TEST
#endif

#ifndef HUFF_DO_ENCODE_DECODE_TEST_ALL
#define HUFF_DO_ENCODE_DECODE_TEST_ALL
#endif

#ifndef HUFF_TABLES_ENABLE
#define HUFF_TABLES_ENABLE 0xFFFFFFFF
#endif

#endif

//#define HUFFANDPUFF_TEST_ITER 100
//#define HUFFANDPUFF_TEST_INTERNALS


typedef struct HuffmanTable_s {
	unsigned char max_bit_length;
	unsigned char total_count;
	const int* counts;
	const int* values;
} HuffmanTable_t;

#define HUFF_USE_DC
#define HUFF_USE_AC

#define HUFF_OK             0xFF0B
#define HUFF_ID_EXCEEDED    0xFF15
#define HUFF_FAILED_LOAD    0xFF1E
//#define HUFF_NOT_LOADED     0xFF26
//#define HUFF_INVALID_SYMBOL 0xFF2D
//0x33
//0x38
//0x47
//0x4C
//0x52
//0x59
//0x61
//0x6A
//0x74
//0x7F

extern uint8_t huffman_tables_count;

extern _huffman_symbol_t huffman_encode_entries[HUFF_TEST_MAX_ENTRIES];           // encoding input alphabet size
extern const _huffman_symbol_t* huffman_encode_symbols[HUFF_TEST_MAX_ENTRIES]; // encoding output symbol (left padded)
extern const _huffman_code_t* huffman_encode_codes[HUFF_TEST_MAX_ENTRIES]; // encoding output symbol (left padded)
extern const _huffman_bits_t* huffman_encode_bits[HUFF_TEST_MAX_ENTRIES];   // number os bits to stream into output when encoding a symbol from huffman_encode_codes

extern int huffman_decode_entries[HUFF_TEST_MAX_ENTRIES];           // number of octets on serialized tree buffer
extern const uint8_t* huffman_decode_tables[HUFF_TEST_MAX_ENTRIES]; // serialized tree buffer

extern unsigned long huffman_decode_test_length[HUFF_TEST_MAX_ENTRIES];              // number of octets on encoded and plain data buffers
extern const uint8_t* huffman_decode_test_plain[HUFF_TEST_MAX_ENTRIES];    // sample plain text data
extern const uint8_t* huffman_decode_test_encoded[HUFF_TEST_MAX_ENTRIES];  // sample encoded data, with bitstream starting on bit huffman_decode_test_encoded_offset_bits
extern int huffman_decode_test_encoded_offset_bits[HUFF_TEST_MAX_ENTRIES]; // offset to the first encoded bit into sample encoded bitstream

void huff_init_testset();


void HUFF_vInit(void);
uint8_t HUFF_u8GetTablesCount();
uint16_t HUFF_u16LoadTable(const uint8_t w);
uint16_t HUFF_u16FillRandomized(const uint8_t w, uint8_t* buffer, const uint16_t max_octets);
uint16_t HUFF_u16Decode(const uint8_t w, const uint8_t* in, const uint16_t inlen, uint8_t *out, const uint16_t outlen);
uint16_t HUFF_u16LRC(const uint8_t* in, const uint16_t inlen);


#endif//__HUFF_DEFS_H_
