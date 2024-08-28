/*
 * Huffandpuff minimal Huffman coder
 *
 * (c)2013 Adam Ierymenko <adam.ierymenko@zerotier.com>
 * This code is in the public domain and is distributed with NO WARRANTY.
 */

#ifndef ____HUFFMAN_H
#define ____HUFFMAN_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Required size of huffheap parameter to compress and decompress
 *
 * Note: if you change any of the data types in the _huffman_node
 * or _huffman_encode_table structs in huffman.c, this also must be
 * changed.
 */

typedef double _huffman_count_t;
typedef double _huffman_prob_t;
typedef unsigned long _huffman_code_t;
typedef unsigned char _huffman_bits_t;
typedef unsigned short _huffman_symbol_t;

typedef struct _huffman_compress_node_s _huffman_compress_node_t;

struct _huffman_compress_node_s
{
	_huffman_compress_node_t *lr[2];
	_huffman_compress_node_t *qprev;
	_huffman_compress_node_t *qnext;
	_huffman_prob_t prob; // symbol probability
	_huffman_code_t c; // symbol / output code
};

typedef struct _huffman_decompress_node_s _huffman_decompress_node_t;

typedef unsigned short _huffman_decompress_node_index_t;

#define INDEX_NONE 0xFFFF

struct _huffman_decompress_node_s
{
	_huffman_decompress_node_index_t lr[2];
	_huffman_code_t c; // symbol / output code
};

typedef struct _huffman_encode_table_entry_s
{
	_huffman_code_t code;
	_huffman_bits_t bits;
} _huffman_encode_table_entry_t;

#define TERMINATOR_SYMBOL_CODE 256
#define NUMBER_OF_SYMBOLS 256

#define COMPRESS_HUFFHEAP_SIZE ( \
	( sizeof(_huffman_count_t) * 257 ) + \
	( sizeof(_huffman_compress_node_t) * (257 * 3) ) + \
	( sizeof(_huffman_encode_table_entry_t) * 257) \
)

#define DECOMPRESS_HUFFHEAP_SIZE ((NUMBER_OF_SYMBOLS+1) * 3)

/**
 * Huffman encode a block of data
 *
 * @param in Input data
 * @param inlen Input data length
 * @param out Output buffer
 * @param outlen Output buffer length
 * @param huffheap Heap memory to use for compression (must be HUFFHEAP_SIZE in size)
 * @return Size of encoded result or 0 on out buffer overrun
 */
extern unsigned long huffman_compress(const unsigned char *in,unsigned long inlen,unsigned char *out,unsigned long outlen,void *huffheap, int* header_length, _huffman_encode_table_entry_t **known_table);

/**
 * Huffman decode a block of data
 *
 * @param in Input data
 * @param inlen Length of input data
 * @param out Output buffer
 * @param outlen Length of output buffer
 * @param huffheap Heap memory to use for decompression (must be HUFFHEAP_SIZE in size)
 * @return Size of decoded result or 0 on out buffer overrun or corrupt input data
 */
extern unsigned long huffman_decompress(const unsigned char *in, unsigned long inlen, unsigned char *out, unsigned long outlen, _huffman_decompress_node_t*huffheap);

extern unsigned long huffman_decompress_decode(const unsigned char *in, unsigned long inlen, unsigned char *out, unsigned long outlen, unsigned long *inbitctr, _huffman_decompress_node_t* huffheap);
extern _huffman_decompress_node_index_t huffman_decompress_prepare(const unsigned char *in, unsigned long inlen, unsigned long *inbitctr, _huffman_decompress_node_t* huffheap);

#ifdef __cplusplus
}
#endif

#endif
