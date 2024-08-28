#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#ifdef CORTEX_M0
#include "internals.h"
#include "fix16.h"
#include "prng.h"
#endif

#include "internals.h"
#include "huffman.h"
#include "huff_defs.h"


uint8_t huffman_tables_count;

#ifdef HUFF_DO_ENCODE
_huffman_symbol_t huffman_encode_entries[HUFF_TEST_MAX_ENTRIES];           // encoding input alphabet size
const _huffman_symbol_t* huffman_encode_symbols[HUFF_TEST_MAX_ENTRIES]; // encoding output symbol (left padded)
const _huffman_code_t* huffman_encode_codes[HUFF_TEST_MAX_ENTRIES]; // encoding output symbol (left padded)
const _huffman_bits_t* huffman_encode_bits[HUFF_TEST_MAX_ENTRIES];   // number os bits to stream into output when encoding a symbol from huffman_encode_codes
#endif

#ifdef HUFF_DO_DECODE
int huffman_decode_entries[HUFF_TEST_MAX_ENTRIES];           // number of octets on serialized tree buffer
const uint8_t* huffman_decode_tables[HUFF_TEST_MAX_ENTRIES]; // serialized tree buffer
#endif

#ifdef HUFF_DO_ENCODE_DECODE_TEST
unsigned long huffman_decode_test_length[HUFF_TEST_MAX_ENTRIES];              // number of octets on encoded and plain data buffers
const uint8_t* huffman_decode_test_plain[HUFF_TEST_MAX_ENTRIES];    // sample plain text data
const uint8_t* huffman_decode_test_encoded[HUFF_TEST_MAX_ENTRIES];  // sample encoded data, with bitstream starting on bit huffman_decode_test_encoded_offset_bits
int huffman_decode_test_encoded_offset_bits[HUFF_TEST_MAX_ENTRIES]; // offset to the first encoded bit into sample encoded bitstream
#endif

void huff_init_testset() {

	huffman_tables_count = 0;

	#include "huff_test_dataset.c"

	app_assert(huffman_tables_count <= HUFF_TEST_MAX_ENTRIES);

}
