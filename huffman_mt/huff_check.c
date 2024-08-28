#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdint.h>

#include <time.h>

#ifdef CORTEX_M0
#include "uart_stdout.h"
#include "CMSDK_CM0.h"
#endif

#include "internals.h"
#include "huffman.h"
#include "huff_defs.h"
#include "prng.h"

#define HUFF_VERBOSE

#ifdef HUFF_VERBOSE
#define huff_printf(x,...) printf(x, ##__VA_ARGS__)
#endif

extern PRNG_stParms_t g_huff_prng_instance;


#ifndef HUFFANDPUFF_TEST
static _huffman_decompress_node_t g_decode_tree[DECOMPRESS_HUFFHEAP_SIZE];

#ifndef HUFF_TASK
#define HUFF_TEST_BUFFER_SIZE 64
#define HUFF_MAX_BUFFER_SIZE 0x3F

//#define HUFF_TEST_BUFFER_SIZE 1024
//#define HUFF_MAX_BUFFER_SIZE 0x3FF

// uint8_t g_test_plain[HUFF_TEST_BUFFER_SIZE];
// uint8_t g_test_coded[HUFF_TEST_BUFFER_SIZE];
// uint8_t g_test_plain_2[HUFF_TEST_BUFFER_SIZE];

#endif

_huffman_decompress_node_index_t g_t_index;
uint8_t g_table_loaded;



/****************************************************************************
*/
void HUFF_vInit(void)
{
	app_assert(sizeof(fix16_t) == 4);
	app_assert(sizeof(int32_t) == 4);
	app_assert(sizeof(uint16_t) == 2);
	app_assert(sizeof(uint8_t) == 1);
	app_assert(sizeof(unsigned short) > sizeof(unsigned char));

	app_assert(sizeof(uint32_t) == sizeof(_huffman_code_t));
	app_assert(sizeof(uint8_t) == sizeof(_huffman_bits_t));

	huff_init_testset();
	g_t_index = INDEX_NONE;
	g_table_loaded = HUFF_TEST_MAX_ENTRIES;
}

uint8_t HUFF_u8GetTablesCount() {
	return huffman_tables_count;
}

uint16_t HUFF_u16LoadTable(const uint8_t w) {
	unsigned long inbitctr;
	inbitctr = 0;
	// load and assemble tree
	g_t_index = huffman_decompress_prepare(huffman_decode_tables[w], huffman_decode_entries[w], &inbitctr, g_decode_tree);
	if (g_t_index == 0) {
		g_table_loaded = w;
		return HUFF_OK;
	}

	g_table_loaded = HUFF_TEST_MAX_ENTRIES;
	return HUFF_FAILED_LOAD;
}

uint16_t HUFF_u16FillRandomized(const uint8_t w, uint8_t* buffer, const uint16_t max_octets) {
	uint32_t outbitctr;
	uint32_t byte_index;

	uint8_t symbol;
	int n_symbols;

	const _huffman_code_t* code_table;
	const _huffman_bits_t* bits_table;
#ifdef HUFF_DEBUG
	const _huffman_symbol_t* symbols_table;
#endif

	_huffman_code_t code;
	_huffman_bits_t bits;
	_huffman_bits_t b;

	if (w > huffman_tables_count) {
		return HUFF_ID_EXCEEDED;
	}

	bits_table = huffman_encode_bits[w];
	code_table = huffman_encode_codes[w];
#ifdef HUFF_DEBUG
	symbols_table = huffman_encode_symbols[w];
#endif

	n_symbols = huffman_encode_entries[w];
	n_symbols -= 1; // invalidate TERMINATOR_SYMBOL_CODE

	//for (b = 0; b < n_symbols; b++) {
	//	huff_printf("i %3d s %3d\t ", b, symbols_table[b]);
	//}

	outbitctr = 0;

#ifdef HUFF_DEBUG
	huff_printf("Filling with table %u. %u symbols\n", w, n_symbols);
#endif

	while (1) {
		
		/* not best, but good enough */
		while ((symbol = PRNG_u8Next(&g_huff_prng_instance)) >= n_symbols);


		code = code_table[symbol];
		bits = bits_table[symbol];


		if (((outbitctr + bits) >> 3) >= max_octets) {
			return (outbitctr >> 3);
		}

#ifdef HUFF_DEBUG
		huff_printf("(%u,%u->%u,%08X) ", symbol, symbols_table[symbol], bits, code);
#endif
		for (b = 0; b<bits; ++b) {
			byte_index = outbitctr++ >> 3;
			buffer[byte_index] = (buffer[byte_index] << 1) | (unsigned char)(code & 1);
			code >>= 1;
		}

	}
}

uint16_t HUFF_u16Decode(const uint8_t w, const uint8_t* in, const uint16_t inlen, uint8_t *out, const uint16_t outlen) {
	unsigned long inbitctr;
	unsigned long decompressed_length;

	if ( (g_table_loaded != w)||(g_t_index != 0) ) {
		return 0;
	}

	//unsigned long huffman_decompress_decode(const unsigned char *in, unsigned long inlen, unsigned char *out, unsigned long outlen, unsigned long *inbitctr, _huffman_decompress_node_t* huffheap)
	inbitctr = 0;
	decompressed_length = huffman_decompress_decode(in, inlen, out, outlen, &inbitctr, g_decode_tree);

	return (uint16_t)(decompressed_length);
}

uint8_t HUFF_u8LRC(const uint8_t* in, const uint16_t inlen)
{
	uint16_t k;
	uint8_t lrc;

	// ISO 1155
	lrc = 0;
	for (k = 0; k < inlen; k++) {
		lrc += in[k];
	}

	lrc = (((lrc ^ 0xFF) + 1) & 0xFF);

	return lrc;
}

uint16_t HUFF_u16LRC(const uint8_t* in, const uint16_t inlen)
{
	uint16_t k;
	uint16_t v;

	uint16_t lrc;

	lrc = 0;

	for (k = 0; k < (inlen-1); k+=2) {
		v = (in[k + 1] << 8) + in[k];
		lrc += v;
	}
	// odd
	if (inlen & 0x1) {
		v = in[inlen-1];
		lrc += v;
	}

	lrc = (((lrc ^ 0xFFFF) + 1) & 0xFFFF);

	return lrc;
}


#endif
