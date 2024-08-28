/*
 * Huffandpuff minimal Huffman coder
 *
 * (c)2013 Adam Ierymenko <adam.ierymenko@zerotier.com>
 * This code is in the public domain and is distributed with NO WARRANTY.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdint.h>

#include "huff_defs.h"
#include "huffman.h"

#ifdef HUFF_DO_ENCODE

static void _huffman_write_tree_and_make_encode_table(unsigned char *out,unsigned long *outbitctr,unsigned long outlen, _huffman_encode_table_entry_t *et,unsigned long code,unsigned int bits, _huffman_compress_node_t *t)
{
	_huffman_encode_table_entry_t *eti;
	unsigned int i;
	unsigned long byte_index;

	byte_index = (*outbitctr)++ >> 3;
	byte_index *= (byte_index < outlen);
	if (t->lr[0]) {
		out[byte_index] <<= 1;
		_huffman_write_tree_and_make_encode_table( out, outbitctr, outlen, et, code,               bits+1, t->lr[0]);
		_huffman_write_tree_and_make_encode_table( out, outbitctr, outlen, et, code | (1 << bits), bits+1, t->lr[1]);
	} else {
		out[byte_index] = (out[byte_index] << 1) | 1;
		for(i=0;i<9;++i) {
			byte_index = (*outbitctr)++ >> 3;
			if (byte_index >= outlen) return;
			out[byte_index] = (out[byte_index] << 1) | ((unsigned char)((t->c >> i) & 1));
		}
		eti = &(et[t->c]);

#ifdef HUFF_PARANOID
		if (code >= UINT32_MAX) {
			printf("Code length exceeded %02X -> %08X %d bits\n", t->c, code, bits);
			fflush(stdout);
		}

		if (bits >= sizeof(_huffman_code_t) * 8) {
			printf("Code length exceeded %d %02X -> %08X %d bits\n", t->c, t->c, code, bits);
			fflush(stdout);
		}
		assert(sizeof(_huffman_code_t) == 4);
		assert( bits<sizeof(_huffman_code_t)*8 );
		assert(code < UINT32_MAX);
#endif
		eti->code = (_huffman_code_t)code;
		eti->bits = bits;
	}
}


unsigned long huffman_compress(const unsigned char *in, unsigned long inlen, unsigned char *out, unsigned long outlen, void *huffheap, int* header_length, _huffman_encode_table_entry_t **known_table)
{
	_huffman_encode_table_entry_t *et, *eti;
	_huffman_compress_node_t *t, *n;
	_huffman_compress_node_t *pair[2];
	unsigned char *heapptr = (unsigned char *)huffheap;
	unsigned long i, code, byte_index, outbitctr;
	unsigned int bits, b;
	_huffman_count_t *counts;
	_huffman_prob_t lowest_prob;
	double total_symbols;

	counts = (_huffman_count_t *)heapptr;
	heapptr += (sizeof(_huffman_count_t) * (NUMBER_OF_SYMBOLS + 1));


	for (i = 0; i<NUMBER_OF_SYMBOLS; ++i)
		counts[i] = 0.0;
	counts[TERMINATOR_SYMBOL_CODE] = 1.0; /* one stop code at end */
	for (i = 0; i<inlen; ++i) {
		counts[(unsigned long)in[i]] += 1.0;
	}

	// enlist used symbols from tail to head and tail to head
	t = (_huffman_compress_node_t *)0;
	total_symbols = (double)(inlen + 1);
	for (i = 0; i <= NUMBER_OF_SYMBOLS; ++i) {
		if (counts[i] > 0.0) {
			n = (_huffman_compress_node_t *)heapptr;
			heapptr += sizeof(_huffman_compress_node_t);
			if (t)
				t->qprev = n;
			n->qprev = (_huffman_compress_node_t *)0;
			n->qnext = t;
			n->lr[0] = (_huffman_compress_node_t *)0;
			n->lr[1] = (_huffman_compress_node_t *)0;
			n->prob = counts[i] / total_symbols;
			n->c = (unsigned int)i;
			t = n;
		}
	}

	// up to this point t is simply a doubly linked list of used symbols at their prob with head at t

	while (t->qnext) {
		// find two lowest likely symbols
		for (i = 0; i<2; ++i) {
			lowest_prob = 1.0;
			pair[i] = (_huffman_compress_node_t *)0;
			n = t;
			// find lowest and link to pair [0/1]
			while (n) {
				if (n->prob <= lowest_prob) {
					lowest_prob = n->prob;
					pair[i] = n;
				}
				n = n->qnext;
			}
			// unlink lowest from doubly linked list
			if (pair[i]->qprev) {
				pair[i]->qprev->qnext = pair[i]->qnext;
			}
			else {
				t = pair[i]->qnext;
			}
			if (pair[i]->qnext) {
				pair[i]->qnext->qprev = pair[i]->qprev;
			}
		}

		// relink as tree
		n = (_huffman_compress_node_t *)heapptr;
		heapptr += sizeof(_huffman_compress_node_t);
		n->lr[0] = pair[0];
		n->lr[1] = pair[1];
		n->prob = pair[0]->prob + pair[1]->prob;
		if (t)
			t->qprev = n;
		n->qprev = (_huffman_compress_node_t *)0;
		n->qnext = t;
		t = n;
	}

	et = (_huffman_encode_table_entry_t *)heapptr;
	(*known_table) = et;

	heapptr += (sizeof(_huffman_encode_table_entry_t) * (NUMBER_OF_SYMBOLS + 1));
	outbitctr = 0;
	_huffman_write_tree_and_make_encode_table(out, &outbitctr, outlen, et, 0, 0, t);

	(*header_length) = outbitctr;

	// stream input data
	for (i = 0; i<inlen; ++i) {
		eti = &(et[(unsigned long)in[i]]);
		code = eti->code;
		bits = eti->bits;
		for (b = 0; b<bits; ++b) {
			byte_index = outbitctr++ >> 3;
			if (byte_index >= outlen) return 0;
			out[byte_index] = (out[byte_index] << 1) | (unsigned char)(code & 1);
			code >>= 1;
		}
	}

	// stream termimator
	code = et[TERMINATOR_SYMBOL_CODE].code;
	bits = et[TERMINATOR_SYMBOL_CODE].bits;
	for (b = 0; b<bits; ++b) {
		byte_index = outbitctr++ >> 3;
		if (byte_index >= outlen) return 0;
		out[byte_index] = (out[byte_index] << 1) | (unsigned char)(code & 1);
		code >>= 1;
	}

	// padding ?
	if (outbitctr >(outlen << 3))
		return 0;
	else if ((outbitctr & 7)) {
		out[i = (outbitctr >> 3)] <<= 8 - (outbitctr & 7);
		return (i + 1);
	}
	else return (outbitctr >> 3);
}

#endif

static _huffman_decompress_node_index_t _huffman_read_tree(const unsigned char *in,unsigned long *inbitctr,unsigned long inlen, _huffman_decompress_node_t* heapptr, int max_nodes, _huffman_decompress_node_index_t* offset, unsigned int* level)
{
	_huffman_decompress_node_t* n;
	_huffman_decompress_node_index_t n_index;
	unsigned int i;
	unsigned long byte_index;

	n_index = (*offset);
	n = &(heapptr[n_index]);
	(*offset) += 1;

	if ((*offset) > max_nodes) {
		return INDEX_NONE;
	}

	byte_index = *inbitctr >> 3;
	byte_index *= (byte_index < inlen);

	if (((in[byte_index] >> (~((*inbitctr)++) & 7)) & 1)) {

		#ifdef HUFF_DEBUG_TREE
			for (i = 0; i < (*level); i++) {
				printf("  ");
			}
			printf("-- n_index %d %p level %u\n", n_index, n, *level); fflush(stdout);
		#endif

		n->lr[0] = INDEX_NONE;
		n->lr[1] = INDEX_NONE;
		n->c = 0;
		for(i=0;i<9;++i) {
			byte_index = *inbitctr >> 3;
			if (byte_index >= inlen) {
				return INDEX_NONE;
			}
			n->c |= (((unsigned int)(in[byte_index] >> (~((*inbitctr)++) & 7))) & 1) << i;
		}

	} else {

		#ifdef HUFF_DEBUG_TREE
			for (i = 0; i < (*level); i++) {
				printf("  ");
			}
			printf(">> n_index %d %p level %u\n", n_index, n, *level); fflush(stdout);
			(*level)++;
		#endif

		n->lr[0] = _huffman_read_tree(in,inbitctr,inlen,heapptr,max_nodes, offset, level);
		(*level)++;
		n->lr[1] = _huffman_read_tree(in,inbitctr,inlen,heapptr,max_nodes, offset, level);
	
		if (!((n->lr[0])&&(n->lr[1])))
			return n->lr[1] = INDEX_NONE;
	
	}

	#ifdef HUFF_DEBUG_TREE
		for (i = 0; i < (*level); i++) {
			printf("  ");
		}
		assert((*level) < 25);
		printf("<< n_index %d level %u\n", n_index,*level); fflush(stdout);
		(*level)--;
	#endif

	return n_index;
}


_huffman_decompress_node_index_t huffman_decompress_prepare(const unsigned char *in, unsigned long inlen, unsigned long *inbitctr, _huffman_decompress_node_t* huffheap) {
	_huffman_decompress_node_index_t t_index;
	_huffman_decompress_node_index_t offset = 0;
	unsigned int level;
	level = 0;
	// load and assemble tree
	t_index = _huffman_read_tree(in, inbitctr, inlen, huffheap, (NUMBER_OF_SYMBOLS + 1) * 3, &offset, &level);

	#ifdef HUFF_DEBUG_TREE
		fprintf(stderr, "Table read. Bits processed on header: %lu, offset %lu, root index %lu\n", (*inbitctr), offset, t_index);
	#endif

#ifdef HUFF_PARANOID
	assert( (t_index == 0) || (t_index == INDEX_NONE));
#endif

	if ((t_index == 0) || (t_index == INDEX_NONE)) {
		return t_index;
	}

	return INDEX_NONE;
}

unsigned long huffman_decompress_decode(const unsigned char *in, unsigned long inlen, unsigned char *out, unsigned long outlen, unsigned long *inbitctr, _huffman_decompress_node_t* huffheap) {
	_huffman_decompress_node_index_t n_index;
	_huffman_decompress_node_t *t, *n;
	unsigned long outptr, byte_index = 0;

	_huffman_decompress_node_index_t offset = 0;

	_huffman_bits_t bits;

	unsigned char input;
	unsigned char payload;
	unsigned char lr;

	t = &(huffheap[0]);

	outptr = 0;

	// load and decode data
	for (;;) {
		n = t;
		while (n->lr[0] != INDEX_NONE) {
			byte_index = (*inbitctr) >> 3;
			// terminate on input exceeded
			if (byte_index >= inlen) {
				return outptr;
			}

			input = in[byte_index];
			payload = (~((*inbitctr)++) & 7);
			bits = (input >> payload);
			lr = (bits) & 1;

			n_index = n->lr[lr];
			n = &(huffheap[n_index]);
		}

		// terminate on terminator symbol
		if (n->c == 256) {
			return outptr;
		}
		// terminate on output exceeded
		if (outptr == outlen) {
			return outptr;
		}
		// put output
		out[outptr++] = (unsigned char)n->c;
	}

	return outptr;
}

#ifndef HUFFANDPUFF_TEST_INTERNALS

unsigned long huffman_decompress(const unsigned char *in, unsigned long inlen, unsigned char *out,unsigned long outlen, _huffman_decompress_node_t*huffheap)
{
	_huffman_decompress_node_index_t t_index;
	unsigned long inbitctr;

	inbitctr = 0;
	// load and assemble tree
	t_index = huffman_decompress_prepare(in, inlen, &inbitctr, huffheap);
	if (t_index==INDEX_NONE) {
		return 0;
	}

	return huffman_decompress_decode(in, inlen, out, outlen, &inbitctr, huffheap);

}
#endif

#ifdef HUFFANDPUFF_TEST

static unsigned char compress_huffbuf[COMPRESS_HUFFHEAP_SIZE];
static _huffman_decompress_node_t decompress_huffbuf[DECOMPRESS_HUFFHEAP_SIZE];



#ifdef HUFFANDPUFF_TEST_ITER

#define HUFFANDPUFF_TEST_MAXLEN 1048576

static unsigned char testin[HUFFANDPUFF_TEST_MAXLEN];
static unsigned char testout[HUFFANDPUFF_TEST_MAXLEN * 2];
static unsigned char testver[HUFFANDPUFF_TEST_MAXLEN];

#else

unsigned char* testin;
unsigned char* testout;
unsigned char* testver;

#ifdef HUFFANDPUFF_TEST_INTERNALS


#endif

#endif

char* canterbury[32] = {
	"none",
	"W:\\mic92\\benchmarks\\canterbury\\alice29.txt",
	"W:\\mic92\\benchmarks\\canterbury\\asyoulik.txt",
	"W:\\mic92\\benchmarks\\canterbury\\bib",
	"W:\\mic92\\benchmarks\\canterbury\\bible.txt",
	"W:\\mic92\\benchmarks\\canterbury\\book1",
	"W:\\mic92\\benchmarks\\canterbury\\book2",
	"W:\\mic92\\benchmarks\\canterbury\\cp.html",
	"W:\\mic92\\benchmarks\\canterbury\\E.coli",
	"W:\\mic92\\benchmarks\\canterbury\\fields.c",
	"W:\\mic92\\benchmarks\\canterbury\\geo",
	"W:\\mic92\\benchmarks\\canterbury\\grammar.lsp",
	"W:\\mic92\\benchmarks\\canterbury\\kennedy.xls",
	"W:\\mic92\\benchmarks\\canterbury\\lcet10.txt",
	"W:\\mic92\\benchmarks\\canterbury\\news",
	"W:\\mic92\\benchmarks\\canterbury\\obj1",
	"W:\\mic92\\benchmarks\\canterbury\\obj2",
	"W:\\mic92\\benchmarks\\canterbury\\paper1",
	"W:\\mic92\\benchmarks\\canterbury\\paper2",
	"W:\\mic92\\benchmarks\\canterbury\\paper3",
	"W:\\mic92\\benchmarks\\canterbury\\paper4",
	"W:\\mic92\\benchmarks\\canterbury\\paper5",
	"W:\\mic92\\benchmarks\\canterbury\\paper6",
	"W:\\mic92\\benchmarks\\canterbury\\plrabn12.txt",
	"W:\\mic92\\benchmarks\\canterbury\\progc",
	"W:\\mic92\\benchmarks\\canterbury\\progl",
	"W:\\mic92\\benchmarks\\canterbury\\progp",
	"W:\\mic92\\benchmarks\\canterbury\\ptt5",
	"W:\\mic92\\benchmarks\\canterbury\\sum",
	"W:\\mic92\\benchmarks\\canterbury\\trans",
	"W:\\mic92\\benchmarks\\canterbury\\world192.txt",
	"W:\\mic92\\benchmarks\\canterbury\\xargs.1"
};

int main(int argc, char **argv)
{
	unsigned long k;
	int v;
	unsigned long length;

#if !defined(HUFFANDPUFF_TEST_INTERNALS)
	unsigned long compressed_length;
#endif

#if defined(HUFFANDPUFF_TEST_ITER)
	unsigned char mask;
	_huffman_encode_table_entry_t *et;
	unsigned long header_bits;
	unsigned long i;
#endif

#if defined(HUFFANDPUFF_TEST_INTERNALS)
	unsigned long b;
	unsigned long i;
	unsigned long test_repeat_count;
	_huffman_decompress_node_index_t t_index;
	unsigned long inbitctr;
	#define DUMP_CHUNK 64
	char datastream[DUMP_CHUNK + (2 * DUMP_CHUNK / 8)];
#endif

#if (defined(HUFFANDPUFF_TEST_ITER)||defined(HUFFANDPUFF_TEST_INTERNALS))
	unsigned long decompressed_length;
#endif

#if (!defined(HUFFANDPUFF_TEST_ITER)&&!defined(HUFFANDPUFF_TEST_INTERNALS))
	unsigned long i;
	unsigned long header_bits;
	unsigned long decompressed_length;
	unsigned long data_offset;
	_huffman_encode_table_entry_t *et;
	FILE *fp;
	char** filenames;
	size_t r;
	char base[256];
	char* dot;
	_huffman_encode_table_entry_t *eti;
	unsigned long header_length;
	unsigned long j;
	unsigned long b;
	unsigned long data_offset_bits;
	#define DUMP_CHUNK 64
	char datastream[DUMP_CHUNK + (2 * DUMP_CHUNK / 8)];
	unsigned long symbol_count;
	unsigned long test_repeat_count;
#endif

	srand((unsigned int)time(0));

	assert(sizeof(unsigned char) == 1);
	assert(sizeof(unsigned short) > sizeof(unsigned char));

	fprintf(stderr, "Compress heap size: %lu\n", COMPRESS_HUFFHEAP_SIZE*sizeof(compress_huffbuf[0]));
	fprintf(stderr, "Decompress heap size: %lu\n", DECOMPRESS_HUFFHEAP_SIZE*sizeof(decompress_huffbuf[0]));
	fprintf(stderr, "Size of frequency count: %lu bits\n", sizeof(_huffman_count_t) * 8);
	fprintf(stderr, "Size of output code: %lu bits\n", sizeof(_huffman_code_t) * 8);
	fprintf(stderr, "Size of code bits: %lu bits\n", sizeof(_huffman_bits_t) * 8);
	fprintf(stderr, "Node compress node struct size: %lu\n", sizeof(_huffman_compress_node_t));
	fprintf(stderr, "Table entry size: %lu\n", sizeof(_huffman_encode_table_entry_t));

#ifdef HUFFANDPUFF_TEST_ITER
	for (k = 0; k < HUFFANDPUFF_TEST_ITER; ++k) {
		length = (rand() % HUFFANDPUFF_TEST_MAXLEN) + 1;
		mask = (rand() & 0xff);
		for (i = 0; i < length; ++i)
			testin[i] = (unsigned char)(rand() & 0xff) & mask;
		memset(compress_huffbuf, 0, sizeof(compress_huffbuf));
		compressed_length = huffman_compress(testin, length, testout, length, compress_huffbuf, &header_bits, &et);
		if (compressed_length) {
			memset(testver, 0, sizeof(testver));
			memset(decompress_huffbuf, 0, sizeof(decompress_huffbuf));
			decompressed_length = huffman_decompress(testout, compressed_length, testver, sizeof(testver), decompress_huffbuf);
			v = ((decompressed_length) && (!memcmp(testver, testin, length)));
			printf("[%d] in: %d, out: %d, verified: %s\n", (int)k, (int)length, (int)compressed_length, (v) ? "OK" : "FAIL");
			if (!v) {
				exit(-1);
			}
		}
		else printf("[%d] in: %d, out: FAIL\n", (int)k, (int)length);
	}


#if 0
	printf("\nFuzzing decompress function...\n");
	for (;;) {
		l = (rand() % HUFFANDPUFF_TEST_MAXLEN) + 1;
		mask = (rand() & 0xff);
		for (i = 0; i < l; ++i)
			testin[i] = (unsigned char)(rand() & 0xff) & mask;
		huffman_decompress(testin, l, testver, sizeof(testver), huffbuf);
		printf("."); fflush(stdout);
	}
#endif

#else /* HUFFANDPUFF_TEST_ITER */

#ifdef HUFFANDPUFF_TEST_INTERNALS
	huff_init_testset();

	test_repeat_count = huffman_tables_count;

	fprintf(stderr, "Registered tests: %d\n", huffman_tables_count);
	for (k = 0; k < test_repeat_count; k++) {

		//if (huffman_decode_test_encoded_offset_bits[k] != 0) {
			length = huffman_decode_test_length[k];

			fprintf(stderr, "Processing internal test data %d max. length %d\n", k, length);

			testin = malloc(sizeof(char)*length);
			testout = malloc(sizeof(char)*length);
			testver = malloc(sizeof(char)*length);
			if ((testin == NULL) || (testout == NULL) || (testver == NULL)) {
				printf("malloc failed\n");
				return -85;
			}

			memcpy(testin, huffman_decode_test_plain[k], length);
			memcpy(testout, huffman_decode_test_encoded[k], length);
			memset(decompress_huffbuf, 0, sizeof(decompress_huffbuf));

			memset(testver, 0, length);
			decompressed_length = 0;

			inbitctr = 0;
			// load and assemble tree
			t_index = huffman_decompress_prepare(huffman_decode_tables[k], huffman_decode_entries[k], &inbitctr, decompress_huffbuf);
			printf("t_index %lu inbitctr %lu \n", t_index, inbitctr);

			if (t_index != INDEX_NONE) {
				inbitctr = huffman_decode_test_encoded_offset_bits[k];
				//inbitctr = 0;
				printf("inbitctr bits %lu %lu octets %lu\n", huffman_decode_test_encoded_offset_bits[k], inbitctr, inbitctr >> 3);
				decompressed_length = huffman_decompress_decode(huffman_decode_test_encoded[k], huffman_decode_test_length[k], testver, length, &inbitctr, decompress_huffbuf);
				printf("inbitctr bits %lu octets %lu\n", inbitctr, inbitctr >> 3);
			}



			v = ((decompressed_length) && (!memcmp(testver, testin, decompressed_length)));
			fprintf(stderr, "[%d] in: %d, out: %d, verified: %s\n", (int)k, (int)length, (int)decompressed_length, (v) ? "OK" : "FAIL");
			if (!v) {
				printf("GOLD[%d] = {\n\t", k);

				b = 0;
				for (i = 0; i < length; i++) {
					if (b > 0) {
						printf(", ");
					}
					if ((b > 0) && ((b % 32) == 0)) {
						printf(" // %s \n\t", datastream);
						b = 0;
					}
					printf("%3d", huffman_decode_test_plain[k][i]);
					// ASCII printable
					if ((huffman_decode_test_plain[k][i] >= '!') && (huffman_decode_test_plain[k][i] <= '~')) {
						datastream[b++] = huffman_decode_test_plain[k][i];
					}
					else {
						datastream[b++] = '.';
					}
					datastream[b] = '\0';
				}
				printf("\n\n");

				printf("COMPUTED[%d] = {\n\t", k);

				b = 0;
				for (i = 0; i < length; i++) {
					if (b > 0) {
						printf(", ");
					}
					if ((b > 0) && ((b % 32) == 0)) {
						printf(" // %s \n\t", datastream);
						b = 0;
					}
					printf("%3d", testver[i]);
					// ASCII printable
					if ((testver[i] >= '!') && (testver[i] <= '~')) {
						datastream[b++] = testver[i];
					}
					else {
						datastream[b++] = '.';
					}
					datastream[b] = '\0';
				}
				printf("\n\n");


				exit(-1);
			}

			free(testin);
			testin = NULL;
			free(testout);
			testout = NULL;
			free(testver);
			testver = NULL;
		//}
	}
#else /* HUFFANDPUFF_TEST_INTERNALS */
	if (argc > 1) {
		filenames = argv;
		test_repeat_count = argc-1;
	}
	else {
		filenames = canterbury;

		test_repeat_count = sizeof(canterbury)/sizeof(canterbury[0])-1;
	}




	for (k = 0; k < test_repeat_count; k++) {
			strcpy(base, filenames[k+1]);
			while (dot = strstr(base, "\\")) {
				(*dot) = '/';
			}
			while (dot = strstr(base, ".")) {
				(*dot) = '_';
			}
			dot = strrchr(base, '/');
			if (dot == NULL) {
				dot = base;
			}
			else {
				dot++;
			}


			fprintf(stderr, "input file name [%d]: %s -> %s\n", k, filenames[k+1], dot);
			fp = fopen(filenames[k + 1], "rb");

			if (fp == NULL) {
				printf("file not found!\n");
				return -89;
			}

			fseek(fp, 0L, SEEK_END);
			length = ftell(fp);
			fseek(fp, 0L, SEEK_SET);

			fprintf(stderr, "the file's length is %1d octets\n", length);
			assert(length >= 1024);
			testin = malloc(sizeof(char)*length);
			testout = malloc(sizeof(char)*length * 2);
			testver = malloc(sizeof(char)*length);
			if ((testin == NULL) || (testout == NULL) || (testver == NULL)) {
				fclose(fp);
				printf("malloc failed\n");
				return -85;
			}

			r = fread(testin, sizeof(char), length, fp);
			if (r != length) {
				free(testin);
				fclose(fp);
				printf("read failed got %d\n", r);
				return -83;
			}

			fclose(fp);

			header_bits = 0;
			b = 0;
			memset(compress_huffbuf, 0, sizeof(compress_huffbuf));
			memset(testout, 0, length*2);
			compressed_length = huffman_compress(testin, length, testout, length, compress_huffbuf, &header_bits, &et);
			header_length = (header_bits + 8 - 1) / 8;
			if (compressed_length) {

				printf("#if (HUFF_TABLES_ENABLE & 1<<%d) // %s\n\n", k, dot);

				printf("////////////////////////////////////////////////////////////////////////\n");
				printf("// Data file %s\n", filenames[k + 1]);
				printf("////////////////////////////////////////////////////////////////////////\n\n");

				printf("#ifdef HUFF_DO_DECODE\n\n");
				printf("// Header length: %d bits, %f %d octets. Header data:\n", header_bits, header_length / 8.0, (header_length + 8 - 1) / 8);
				printf("static const uint8_t decode_table_%s_%d[%d] = {\n\t", dot, header_length, header_length);

				for (i = 0; i < header_length; i++) {
					if (i > 0) {
						printf(", ");
					}
					if ((i > 0) && ((i % (DUMP_CHUNK / 8)) == 0)) {
						printf(" // %s \n\t", datastream);
						b = 0;
					}
					printf("%3dU", testout[i]);
					for (j = 0; j < 8; j++) {
						datastream[b++] = (testout[i] & (0x80 >> j)) ? '1' : '0';
					}
					datastream[b++] = ' ';
					datastream[b] = '\0';
				}
				if (b != 0) {
					printf("  ");
					while ((i % (DUMP_CHUNK / 8)) != 0) {
						printf("      ");
						i++;
					}
					printf(" // %s \n", datastream);
				}

				printf("};\n\n");

				printf("huffman_decode_entries[huffman_tables_count] = %d ; // decode_table_%s_%d\n", header_length, dot, header_length);
				printf("huffman_decode_tables[huffman_tables_count] = &(decode_table_%s_%d[0]) ;\n\n", dot, header_length);

				printf("#endif /* HUFF_DO_DECODE */\n\n");

				symbol_count = 0;
				for (i = 0; i < (NUMBER_OF_SYMBOLS + 1); i++) {
					eti = &(et[i]);
					if (eti->bits != 0) {
						symbol_count++;
					}
				}

				printf("#ifdef HUFF_DO_ENCODE\n\n");
				printf("// Number of symbols: %d\n", symbol_count);
				printf("static const _huffman_symbol_t encode_symbols_%s_%d[%d] = {\n\t", dot, symbol_count, symbol_count);

				b = 0;
				for (i = 0; i < (NUMBER_OF_SYMBOLS + 1); i++) {
					eti = &(et[i]);
					if (eti->bits != 0) {
						if (b > 0) {
							printf(", ");
						}
						if ((b > 0) && ((b % 8) == 0)) {
							printf("\n\t");
						}
						printf("%3dU", i);
						b++;
					}
				}
				printf("\n};\n\n");
				printf("static const _huffman_bits_t encode_bits_%s_%d[%d] = {\n\t", dot, symbol_count, symbol_count);

				b = 0;
				for (i = 0; i < (NUMBER_OF_SYMBOLS + 1); i++) {
					eti = &(et[i]);
					if (eti->bits != 0) {
						if (b > 0) {
							printf(", ");
						}
						if ((b > 0) && ((b % 8) == 0)) {
							printf("\n\t");
						}
						printf("%3dU", eti->bits);
						b++;
					}
				}
				printf("\n};\n\n");

				printf("static const _huffman_code_t encode_codes_%s_%d[%d] = {\n\t", dot, symbol_count, symbol_count);

				b = 0;
				for (i = 0; i < (NUMBER_OF_SYMBOLS + 1); i++) {
					eti = &(et[i]);
					if (eti->bits != 0) {
						if (b > 0) {
							printf(", ");
						}
						if ((b > 0) && ((b % 8) == 0)) {
							printf("\n\t");
						}
						printf("0x%08XU", eti->code);
						b++;
					}
				}
				printf("\n};\n\n");

				printf("huffman_encode_entries[huffman_tables_count] = %d ; // encode_bits_%s_%d\n", symbol_count, dot, symbol_count);
				printf("huffman_encode_symbols[huffman_tables_count] = &(encode_symbols_%s_%d[0]) ;\n", dot, symbol_count);
				printf("huffman_encode_bits[huffman_tables_count] = &(encode_bits_%s_%d[0]) ;\n", dot, symbol_count);
				printf("huffman_encode_codes[huffman_tables_count] = &(encode_codes_%s_%d[0]) ;\n\n", dot, symbol_count);

				printf("#endif /* HUFF_DO_ENCODE */\n\n");

				//40 00 0C D2 C8 6D 17 C1 52 46 8F A2 B8 1D 26 48 EE 28 83 48 CA 1F C5 88 10 54 F0 C2 2A 82 24 F1
				//12 A7 08 2E 48 E1 F2 7B 04 89 4A 34 4B 62 80 D0 00 4B 19 C3 58 E6 0B 42 30 6A 10 40 F4 21 47 11
				//0C 11 44 A0 CC 37 00 D8 29 85 09 50 17 04 08 A4 3B 01 82 56 03 CE BF 92 76 9D 78 05 DD 5F 10 0D
				//9D 97 79 03

				if ( header_bits == (header_length * 8) ) {
					data_offset = header_length;
					data_offset_bits = 0;
				}
				else {
					data_offset = header_length - 1;
					data_offset_bits = header_bits - (data_offset*8);
				}
				
				//data_offset_bits = header_length * 8 - header_bits;
				//if (data_offset_bits == 0) {
				//	data_offset = header_length;
				//}
				//else {
				//	data_offset = header_length - 1;
				//}

				printf("#ifdef HUFF_DO_ENCODE_DECODE_TEST\n\n");

				printf("// %s Header lengh bits %d octets %d, Data offset bits %d octets %d.\n", dot, header_bits, header_length, data_offset_bits, data_offset);

				printf("static const uint8_t decode_test_1k_encoded_%s_%d[%d] = {\n\t", dot, symbol_count, 1024);

				b = 0;
				for (i = (0 + data_offset); i < (data_offset + 1024); i++) {
					if (b > 0) {
						printf(", ");
					}
					if ((b > 0) && ((b % 32) == 0)) {
						printf(" // %s \n\t", datastream);
						b = 0;
					}
					printf("%3dU", testout[i]);
					// ASCII printable
					if ((testout[i] >= '!') && (testout[i] <= '~')) {
						datastream[b++] = testout[i];
					}
					else {
						datastream[b++] = '.';
					}
					datastream[b] = '\0';
				}
				printf("\n};\n\n");

				printf("static const uint8_t decode_test_1k_plain_%s_%d[%d] = {\n\t", dot, symbol_count, 1024);

				b = 0;
				for (i = 0; i < 1024; i++) {
					if (b > 0) {
						printf(", ");
					}
					if ((b > 0) && ((b % 32) == 0)) {
						printf(" // %s \n\t", datastream);
						b = 0;
					}
					printf("%3dU", testin[i]);
					// ASCII printable
					if ((testin[i] >= '!') && (testin[i] <= '~')) {
						datastream[b++] = testin[i];
					}
					else {
						datastream[b++] = '.';
					}
					datastream[b] = '\0';
				}
				printf("\n};\n\n");

				printf("huffman_decode_test_length[huffman_tables_count] = 1024 ; // decode_test_1k_encoded_%s_%d and decode_test_1k_plain_%s_%d\n", dot, symbol_count, dot, symbol_count);
				printf("huffman_decode_test_encoded[huffman_tables_count] = decode_test_1k_encoded_%s_%d ;\n", dot, symbol_count);
				printf("huffman_decode_test_encoded_offset_bits[huffman_tables_count] = %d ; // decode_test_1k_encoded_%s_%d\n", data_offset_bits, dot, symbol_count);
				printf("huffman_decode_test_plain[huffman_tables_count] = decode_test_1k_plain_%s_%d ;\n", dot, symbol_count);

				printf("#endif /*HUFF_DO_ENCODE_DECODE_TEST*/\n\n");

				printf("huffman_tables_count++; // %s\n\n\n\n", dot);
				printf("#endif // %s (HUFF_TABLES_ENABLE & 1<<%d) \n\n\n\n", dot, k);



				memset(testver, 0, length);
				memset(decompress_huffbuf, 0, sizeof(decompress_huffbuf));
				decompressed_length = huffman_decompress(testout, compressed_length, testver, length, decompress_huffbuf);
				v = ((decompressed_length) && (!memcmp(testver, testin, length)));
				fprintf(stderr, "[%d] in: %d, out: %d, verified: %s\n", (int)k, (int)length, (int)compressed_length, (v) ? "OK" : "FAIL");
				if (!v) {
					exit(-1);
				}
			}
			else {
				fprintf(stderr, "[%d] in: %d, out: %d FAIL\n", (int)k, (int)length, compressed_length);
			}

		free(testin);
		testin = NULL;
		free(testout);
		testout = NULL;
		free(testver);
		testver = NULL;
	}

#endif /* !HUFFANDPUFF_TEST_INTERNALS */
#endif /* HUFFANDPUFF_TEST_ITER */

	return 0;
}

#endif
