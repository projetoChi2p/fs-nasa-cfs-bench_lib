
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <stdint.h>

#include "huff_bench.h"

#include "internals.h"
#include "huffman.h"
#include "huff_defs.h"
#include "prng.h"

#undef HUFF_DEBUG
//#define HUFF_DEBUG

#undef HUFF_VERBOSE
//#define HUFF_VERBOSE

#ifdef HUFF_VERBOSE
#define huff_printf(x,...) printf(x, ##__VA_ARGS__)
#endif

#define HUFF_TEST_BUFFER_SIZE 4096

uint8_t g_test_plain[HUFF_TEST_BUFFER_SIZE];
uint8_t g_test_coded[HUFF_TEST_BUFFER_SIZE];



static __inline uint8_t _u8Maj(volatile const uint8_t v1, volatile const uint8_t v2, volatile const uint8_t v3) {
    return ( (v2 & v3) | (v1 & v3) | (v1 & v2) | (v1 & v2 & v3) );
}

static __inline uint16_t _u16Maj(volatile const uint16_t v1, volatile const uint16_t v2, volatile const uint16_t v3) {
    return ( (v2 & v3) | (v1 & v3) | (v1 & v2) | (v1 & v2 & v3) );
}


void huff_bench_task(uint16_t seed, uint8_t* ptable, uint16_t* pcheck_e, uint16_t* pcheck_d) {

	uint16_t check;

	volatile uint16_t check_e_1;
	volatile uint16_t check_e_2;
	volatile uint16_t check_e_3;

	uint32_t func;

	uint8_t table;

	volatile uint8_t table_1;
	volatile uint8_t table_2;
	volatile uint8_t table_3;

	uint16_t u16;
	uint16_t encoded_length;
	uint16_t decoded_length;

	uint8_t n_tables;

#ifdef HUFF_DEBUG
	int k;
	int b;
	#define DUMP_CHUNK 64
	char datastream[DUMP_CHUNK + (2 * DUMP_CHUNK / 8)];
#endif



	HUFF_vInit();

	n_tables = HUFF_u8GetTablesCount();

#ifdef HUFF_VERBOSE
	huff_printf("Huffman tables: %d\r\n", n_tables);
#endif

#ifdef HUFF_VERBOSE
	huff_printf("Diversification is %x\r\n", seed);
#endif

    PRNG_vInit(seed);

    while ((table = (PRNG_u8Next() & HUFF_MAX_TABLES_MASK)) >= n_tables);
    table_1 = table;
    table_2 = table;
    table_3 = table;

    encoded_length = HUFF_u16FillRandomized(table, g_test_coded, sizeof(g_test_coded));
    if (encoded_length == HUFF_ID_EXCEEDED) {
        (*pcheck_e) = HUFF_ID_EXCEEDED;
        (*pcheck_d) = HUFF_ID_EXCEEDED;
        return;
    }
    check = HUFF_u16LRC(g_test_coded, encoded_length);
    check_e_1 = check;
    check_e_2 = check;
    check_e_3 = check;
    decoded_length = 0;

    u16 = HUFF_u16LoadTable(table);
    if (u16 == HUFF_OK) {
        decoded_length = HUFF_u16Decode(table, g_test_coded, encoded_length, g_test_plain, sizeof(g_test_plain));
        check = HUFF_u16LRC(g_test_plain, decoded_length);
    }
    else {
        check = u16;
    }
    (*pcheck_d) = check;
    (*pcheck_e) = _u16Maj(check_e_1, check_e_2, check_e_3);
    (*ptable) = _u8Maj(table_1, table_2, table_3);

#ifdef HUFF_VERBOSE
    huff_printf("Checksum is %x\r\n", check);
#endif

#ifdef HUFF_DEBUG

    huff_printf("Encoded %d bytes into %d bytes\r\n", encoded_length, sizeof(g_test_coded));

    huff_printf("ENCODED = {\r\n\t");

    b = 0;
    for (k = 0; k < encoded_length; k++) {
        if (b > 0) {
            huff_printf(", ");
        }
        if ((b > 0) && ((b % 32) == 0)) {
            huff_printf(" // %s \r\n\t", datastream);
            b = 0;
        }
        huff_printf("%3d", g_test_coded[k]);
        // ASCII printable
        if ((g_test_coded[k] >= '!') && (g_test_coded[k] <= '~')) {
            datastream[b++] = g_test_coded[k];
        }
        else {
            datastream[b++] = '.';
        }
        datastream[b] = '\0';
    }
    if (b > 0) {
        while ((k % 32) != 0) {
            printf("       ");
            k++;
        }
        huff_printf(" // %s \r\n", datastream);

    }
    huff_printf("\r\n\r\n");

    huff_printf("Decoded %d bytes into %d bytes\r\n", decoded_length, sizeof(g_test_plain));

    huff_printf("DECODED = {\r\n\t");

    b = 0;
    for (k = 0; k < decoded_length; k++) {
        if (b > 0) {
            huff_printf(", ");
        }
        if ((b > 0) && ((b % 32) == 0)) {
            huff_printf(" // %s \r\n\t", datastream);
            b = 0;
        }
        huff_printf("%3d", g_test_plain[k]);
        // ASCII printable
        if ((g_test_plain[k] >= '!') && (g_test_plain[k] <= '~')) {
            datastream[b++] = g_test_plain[k];
        }
        else {
            datastream[b++] = '.';
        }
        datastream[b] = '\0';
    }
    if (b > 0) {
        while ((k % 32) != 0) {
            printf("      ");
            k++;
        }
        huff_printf(" // %s \r\n", datastream);

    }
    huff_printf("\r\n\r\n");
#endif


#ifdef HUFF_VERBOSE
    huff_printf("Processing cycle completed.\r\n");
#endif
}
