#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "prng.h"
#include "fix16.h"
#include "qmtx.h"

#include "app_helpers.h"

extern PRNG_stParms_t g_mxm_prng_instance;

/****************************************************************************
 */
void QMTX_vFillZeros (QMTX_FIX16* values, uint8_t* flags, const uint8_t r, const uint8_t c)
{
	uint8_t i;
	uint8_t j;

	(*flags) = 0;

	for ( j = 0;j < c; j++ ) {
		for ( i = 0; i < r; i++ ) {
			values[i+j*r] = 0;
		}
	}

}

/****************************************************************************
 */
void QMTX_vFillRandomized (QMTX_FIX16* values, uint8_t* flags, const uint8_t r, const uint8_t c)
{
	uint8_t i;
	uint8_t j;

	(*flags) = 0;

	for ( j = 0;j < c; j++ ) {
		for ( i = 0; i < r; i++ ) {
			values[i+j*r] = PRNG_q16Next(&g_mxm_prng_instance);
		}
	}
}


/****************************************************************************
 */
static inline fix16_t fa16_dot(const fix16_t *pva, const uint_fast8_t a_stride,
                 const fix16_t *pvb, const uint_fast8_t b_stride,
				 uint_fast8_t n)
{
    fix16_t sum = 0;
    fix16_t product;
    fix16_t a;
    fix16_t b;

    app_assert(sizeof(QMTX_FIX16)==sizeof(fix16_t));
    //assert(sizeof(fix16_t)==sizeof(intqw_t));
    //assert(sizeof(fix16_t)==sizeof(intqi_t));
    //assert(sizeof(fix16_t)==sizeof(intqiw_t));



    while (n--)
    {
		a = *pva;
		b = *pvb;

		if (a != 0 && b != 0)
        {
			product = FIX16_qMul(a, b);
            sum = FIX16_qAdd(sum, product);

            //sum = muladdiw_i_w_iw_saturating(a, b, sum);

            if (sum == FIX16_OVERFLOW || product == FIX16_OVERFLOW) {
                return FIX16_OVERFLOW;
            }

        }

        pva += a_stride;
        pvb += b_stride;
    }

    return sum;
}


/****************************************************************************
 */
static inline uint8_t vMultVector (const QMTX_FIX16* a, const QMTX_FIX16* b, QMTX_FIX16* c, uint8_t l, uint8_t m, uint8_t n)
{
	uint8_t i;
	uint8_t j;

	uint8_t flags = 0;

	for (i = 0; i < l; i++) {
		for (j = 0; j < n; j++) {

			c[i + j * l] = fa16_dot( &(a[i]), l, &(b[j*m]), 1, m);

			if (c[i + j * l] == FIX16_OVERFLOW) {
				flags |= FIXMATRIX_OVERFLOW;
			}

		}
	}

	return flags;
}

#if 0
/****************************************************************************
 */
static inline uint8_t  vMultTrivial (QMTX_FIX16* a_values, QMTX_FIX16* b_values, QMTX_FIX16* c_values, uint8_t l, uint8_t m, uint8_t n)
{
	uint8_t i;
	uint8_t j;
	uint8_t k;

	QMTX_FIX16 product;

	QMTX_FIX16 sum;
	uint8_t flags = 0;

	for (i = 0; i < l; i++) {
		for (j = 0; j < n; j++) {
			sum = 0;

			for (k = 0; k < m; k++) {

				product = FIX16_qMul(a_values[i + k * l], b_values[k + j * m]);
	            sum = FIX16_qAdd(sum, product);

			}
            c_values[i + j * l] = sum;

            if (sum == FIX16_OVERFLOW) {
                flags |= FIXMATRIX_OVERFLOW;
            }
		}
	}

	return flags;
}
#endif


/****************************************************************************
 */
void QMTX_vMult (const QMTX_FIX16* a_values, const uint8_t a_flags, const QMTX_FIX16* b_values, const uint8_t b_flags, QMTX_FIX16* c_values, uint8_t* c_flags, const uint8_t l, const uint8_t m, const uint8_t n)
{
	(*c_flags) = a_flags | b_flags;
	(*c_flags) |= vMultVector(a_values, b_values, c_values, l, m, n);
	//(*c_flags) |= vMultTrivial(a_values, b_values, c_values, l, m, n);
}

#if 0

float q_to_float(intqi_t i) {
    float f;

    assert(WEIGHT_FIX_ONE==IMAGE_FIX_ONE);
    assert(WEIGHT_FIX_ONE==IMAGE_WEIGHT_FIX_ONE);


#if (IMAGE_FRACTIONAL_PRECISION == 0)
    f = (float)i;
#else
    f = i / ((float)IMAGE_FIX_ONE);
#endif
    return f;
}
#endif

/****************************************************************************
 */
void vDump (const QMTX_FIX16* v, const uint8_t mtx_rows, const uint8_t mtx_cols, uint8_t max_rows, uint8_t max_cols)
{
	uint8_t i;
	uint8_t j;
	uint8_t print_buffer[16];

	if (mtx_rows<max_rows) {
		max_rows = mtx_rows;
	}
	if (mtx_cols<max_cols) {
		max_cols = mtx_cols;
	}

	for ( i = 0; i < max_rows; i++ ) {
		app_printf("%-3d", i);

		for ( j = 0;j < max_cols; j++ ) {
		    //HLP_vPrintFloat(print_buffer, q_to_float(v[i+j*mtx_rows]), 8, 8);
		    HLP_vPrintFloat(print_buffer, FIX16_fToFloat(v[i+j*mtx_rows]), 8, 8);
			app_printf(" %s", print_buffer);
		}
		if (max_cols<mtx_cols) {
			app_printf(" ...");
		}

		app_printf("\r\n");
	}

	if (max_rows<mtx_rows) {
		app_printf("...\r\n");
	}
}


/****************************************************************************
 */
void QMTX_vDump (const QMTX_FIX16* values, const uint8_t flags, const uint8_t r, const uint8_t c, uint8_t r_max, uint8_t c_max)
{

	app_printf("Flags: %u\r\n", flags);
	vDump(values, r, c, r_max, c_max);
}


/****************************************************************************
 */
void QMTX_vInject (QMTX_FIX16* values, const uint8_t r, const uint8_t c)
{
	uint8_t i;
	uint8_t j;
	/* For test purposes only, not to be used on final system. */
	while ((i = (PRNG_u32Next(&g_mxm_prng_instance) % r )) >= r);
	while ((j = (PRNG_u32Next(&g_mxm_prng_instance) % c )) >= c);

	values[i+j*r] = PRNG_q16Next(&g_mxm_prng_instance);
}

/****************************************************************************
 */
void QMTX_vCopy (const QMTX_FIX16* a_values, const uint8_t a_flags, QMTX_FIX16* b_values, uint8_t *b_flags, const uint8_t r, const uint8_t c)
{
	uint8_t i;
	uint8_t j;

	(*b_flags) = a_flags;

	for ( j = 0;j < c; j++ ) {
		for ( i = 0; i < r; i++ ) {
			b_values[i+j*r] = a_values[i+j*r];
		}
	}

}

void QMTX_vSub (const QMTX_FIX16* a_values, const uint8_t a_flags, const QMTX_FIX16* b_values, const uint8_t b_flags, QMTX_FIX16* c_values, uint8_t* c_flags, const uint8_t r, const uint8_t c)
{
	uint8_t i;
	uint8_t j;

	QMTX_FIX16 rem;
	uint8_t flags = 0;

	(*c_flags) = a_flags | b_flags;

	for ( j = 0;j < c; j++ ) {
		for ( i = 0; i < r; i++ ) {

			rem =  FIX16_qSub(a_values[i+j*r], b_values[i+j*r]);

			c_values[i+j*r] = rem;

			if (rem == FIX16_OVERFLOW) {
                flags |= FIXMATRIX_OVERFLOW;
            }

		}
	}

	(*c_flags) |= flags;
}

QMTX_FIX16 QMTX_q16Sum (const QMTX_FIX16* a_values, const uint8_t r, const uint8_t c)
{
	uint8_t i;
	uint8_t j;

	QMTX_FIX16 sum;

	sum = 0;
	for ( j = 0;j < c; j++ ) {
		for ( i = 0; i < r; i++ ) {
			sum = FIX16_qAdd(a_values[i+j*r], sum);
		}
	}

	return sum;
}


uint8_t QMTX_u8LRC (QMTX_FIX16* values, const uint8_t r, const uint8_t c)
{
	uint8_t i;
	uint8_t j;
	uint8_t k;

	uint8_t lrc;

	app_assert(sizeof(QMTX_FIX16)==4);
	app_assert(sizeof(uint8_t)==1);

	union bits_u {
		QMTX_FIX16 q;
		uint8_t o[sizeof(QMTX_FIX16)];
	} octets;

	// ISO 1155
	lrc = 0;
	for ( j = 0;j < c; j++ ) {
		for ( i = 0; i < r; i++ ) {
			octets.q = values[i+j*r];
			for ( k = 0; k < sizeof(QMTX_FIX16); k++ ) {
				lrc += octets.o[k];
			}
		}
	}

	lrc = (((lrc ^ 0xFF) + 1) & 0xFF);

	return lrc;
}

uint16_t QMTX_u16LRC (QMTX_FIX16* values, const uint8_t r, const uint8_t c)
{
	uint8_t i;
	uint8_t j;

	uint16_t lrc;

	app_assert(sizeof(QMTX_FIX16)==4);
	app_assert(sizeof(uint16_t)==2);

	union bits_u {
		QMTX_FIX16 q;
		uint16_t   w[2];
	} octets;

	lrc = 0;
	for ( j = 0;j < c; j++ ) {
		for ( i = 0; i < r; i++ ) {
			octets.q = values[i+j*r];
			lrc += octets.w[0];
			lrc += octets.w[1];
		}
	}

	lrc = (((lrc ^ 0xFFFF) + 1) & 0xFFFF);

	return lrc;
}


uint8_t QMTX_u8J1708 (QMTX_FIX16* values, const uint8_t r, const uint8_t c) {
	uint8_t i;
	uint8_t j;
	uint8_t k;

	uint8_t j1708;

	app_assert(sizeof(QMTX_FIX16)==4);
	app_assert(sizeof(uint8_t)==1);

	union bits_u {
		QMTX_FIX16 q;
		uint8_t o[sizeof(QMTX_FIX16)];
	} octets;


	// SAE J1708
	j1708 = 0;
	for ( j = 0;j < c; j++ ) {
		for ( i = 0; i < r; i++ ) {
			octets.q = values[i+j*r];
			for ( k = 0; k < sizeof(QMTX_FIX16); k++ ) {
				j1708 = (j1708+octets.o[k]) & 0xFF;
			}
		}
	}

	app_assert( (((~j1708)+1) & 0xFF) == ((0x100-j1708)& 0xFF) );

	j1708 = ((~j1708)+1) & 0xFF;

	return j1708;
}


uint8_t QMTX_u8Fletcher (QMTX_FIX16* values, const uint8_t r, const uint8_t c) {
	uint8_t i;
	uint8_t j;
	uint8_t k;

	uint8_t sum_a;
	uint8_t sum_b;

	app_assert(sizeof(QMTX_FIX16)==4);
	app_assert(sizeof(uint8_t)==1);

	union bits_u {
		QMTX_FIX16 q;
		uint8_t o[sizeof(QMTX_FIX16)];
	} octets;


	sum_a = 0;
	sum_b = 0;

	for ( j = 0;j < c; j++ ) {
		for ( i = 0; i < r; i++ ) {
			octets.q = values[i+j*r];
			for ( k = 0; k < sizeof(QMTX_FIX16); k++ ) {
//				sum_a = ( sum_a + (((octets.o[k])>>4) & 0x0F) );
//				sum_b = sum_b + sum_a;
//
//				sum_a = ( sum_a + (((octets.o[k])>>0) & 0x0F) );
//				sum_b = sum_b + sum_a;

				sum_a += (octets.o[k]);
				sum_b += sum_a;
			}
			sum_a %= 0x0F;
			sum_b %= 0x0F;
		}
	}

	return (sum_b << 4) + sum_a;
}

uint16_t QMTX_u16Fletcher (QMTX_FIX16* values, const uint8_t r, const uint8_t c) {
	uint8_t i;
	uint8_t j;
	uint8_t k;

	uint8_t sum_a;
	uint8_t sum_b;

	app_assert(sizeof(QMTX_FIX16)==4);
	app_assert(sizeof(uint8_t)==1);

	union bits_u {
		QMTX_FIX16 q;
		uint8_t o[sizeof(QMTX_FIX16)];
	} octets;


	sum_a = 0;
	sum_b = 0;

	for ( j = 0;j < c; j++ ) {
		for ( i = 0; i < r; i++ ) {
			octets.q = values[i+j*r];
			for ( k = 0; k < sizeof(QMTX_FIX16); k++ ) {
				sum_a += (octets.o[k]);
				sum_b += sum_a;
			}
		}
	}

	return (sum_b << 8) + sum_a;
}

uint8_t QMTX_u8Pearson (QMTX_FIX16* values, const uint8_t r, const uint8_t c) {
	uint8_t i;
	uint8_t j;
	uint8_t k;

	uint8_t h;

	uint8_t table[256] = {19,
						  87,
						  208,
						  197,
						  225,
						  59,
						  27,
						  29,
						  24,
						  26,
						  199,
						  54,
						  215,
						  149,
						  102,
						  95,
						  31,
						  121,
						  18,
						  158,
						  231,
						  90,
						  52,
						  209,
						  82,
						  138,
						  226,
						  126,
						  114,
						  177,
						  156,
						  49,
						  224,
						  74,
						  93,
						  85,
						  157,
						  207,
						  117,
						  105,
						  196,
						  20,
						  148,
						  221,
						  35,
						  51,
						  124,
						  205,
						  116,
						  237,
						  254,
						  142,
						  22,
						  99,
						  55,
						  0,
						  11,
						  244,
						  172,
						  89,
						  139,
						  176,
						  2,
						  94,
						  202,
						  178,
						  165,
						  44,
						  201,
						  152,
						  43,
						  204,
						  109,
						  163,
						  168,
						  184,
						  183,
						  169,
						  13,
						  166,
						  28,
						  65,
						  56,
						  40,
						  21,
						  100,
						  41,
						  7,
						  135,
						  189,
						  252,
						  245,
						  213,
						  240,
						  58,
						  110,
						  66,
						  103,
						  63,
						  16,
						  71,
						  79,
						  4,
						  241,
						  227,
						  251,
						  136,
						  33,
						  232,
						  132,
						  255,
						  151,
						  104,
						  210,
						  88,
						  228,
						  160,
						  98,
						  239,
						  144,
						  216,
						  195,
						  236,
						  46,
						  212,
						  73,
						  217,
						  25,
						  194,
						  134,
						  9,
						  192,
						  203,
						  75,
						  84,
						  78,
						  68,
						  206,
						  5,
						  234,
						  12,
						  37,
						  223,
						  113,
						  3,
						  233,
						  101,
						  146,
						  141,
						  173,
						  80,
						  14,
						  167,
						  128,
						  249,
						  123,
						  36,
						  6,
						  170,
						  86,
						  130,
						  235,
						  32,
						  250,
						  115,
						  64,
						  164,
						  211,
						  70,
						  242,
						  248,
						  62,
						  129,
						  127,
						  180,
						  17,
						  145,
						  34,
						  131,
						  47,
						  81,
						  161,
						  193,
						  222,
						  140,
						  83,
						  133,
						  107,
						  198,
						  253,
						  190,
						  188,
						  23,
						  53,
						  162,
						  229,
						  69,
						  186,
						  159,
						  187,
						  238,
						  38,
						  181,
						  96,
						  61,
						  91,
						  106,
						  220,
						  147,
						  72,
						  171,
						  125,
						  153,
						  8,
						  122,
						  174,
						  155,
						  57,
						  191,
						  200,
						  185,
						  76,
						  214,
						  150,
						  67,
						  39,
						  48,
						  218,
						  77,
						  243,
						  182,
						  120,
						  111,
						  92,
						  143,
						  10,
						  247,
						  50,
						  15,
						  119,
						  45,
						  30,
						  42,
						  1,
						  108,
						  230,
						  154,
						  175,
						  137,
						  112,
						  97,
						  60,
						  246,
						  118,
						  219,
						  179};

	app_assert(sizeof(QMTX_FIX16)==4);
	app_assert(sizeof(uint8_t)==1);

	union bits_u {
		QMTX_FIX16 q;
		uint8_t o[sizeof(QMTX_FIX16)];
	} octets;


	h = (r * c * sizeof(QMTX_FIX16)) & 256;

	for ( j = 0;j < c; j++ ) {
		for ( i = 0; i < r; i++ ) {
			octets.q = values[i+j*r];
			for ( k = 0; k < sizeof(QMTX_FIX16); k++ ) {
				h = table[(h ^ octets.o[k]) % 256];
			}
		}
	}

	return h;
}

uint16_t QMTX_u16Combined (QMTX_FIX16* values, const uint8_t r, const uint8_t c) {
	uint8_t i;
	uint8_t j;
	uint8_t k;

	uint8_t h;
	uint8_t lrc;

	uint8_t table[256] = {19, 87, 208, 197, 225, 59, 27, 29, 24, 26, 199, 54, 215, 149,
102, 95, 31, 121, 18, 158, 231, 90, 52, 209, 82, 138, 226, 126,
114, 177, 156, 49, 224, 74, 93, 85, 157, 207, 117, 105, 196,
20, 148, 221, 35, 51, 124, 205, 116, 237, 254, 142, 22, 99,
55, 0, 11, 244, 172, 89, 139, 176, 2, 94, 202, 178, 165, 44,
201, 152, 43, 204, 109, 163, 168, 184, 183, 169, 13, 166, 28,
65, 56, 40, 21, 100, 41, 7, 135,
189,
252,
245,
213,
240,
58,
110,
66,
103,
63,
16,
71,
79,
4,
241,
227,
251,
136,
33,
232,
132,
255,
151,
104,
210,
88,
228,
160,
98,
239,
144,
216,
195,
236,
46,
212,
73,
217,
25,
194,
134,
9,
192,
203,
75,
84,
78,
68,
206,
5,
234,
12,
37,
223,
113,
3,
233,
101,
146,
141,
173,
80,
14,
167,
128,
249,
123,
36,
6,
170,
86,
130,
235,
32,
250,
115,
64,
164,
211,
70,
242,
248,
62,
129,
127,
180,
17,
145,
34,
131,
47,
81,
161,
193,
222,
140,
83,
133,
107,
198,
253,
190,
188,
23,
53,
162,
229,
69,
186,
159,
187,
238,
38,
181,
96,
61,
91,
106,
220,
147,
72,
171,
125,
153,
8,
122,
174,
155,
57,
191,
200,
185,
76,
214,
150,
67,
39,
48,
218,
77,
243,
182,
120,
111,
92,
143,
10,
247,
50,
15,
119,
45,
30,
42,
1,
108,
230,
154,
175,
137,
112,
97,
60,
246,
118,
219,
179};

	app_assert(sizeof(QMTX_FIX16)==4);
	app_assert(sizeof(uint8_t)==1);

	union bits_u {
		QMTX_FIX16 q;
		uint8_t o[sizeof(QMTX_FIX16)];
	} octets;


	lrc = 0;
	h = (r * c * sizeof(QMTX_FIX16)) & 256;

	for ( j = 0;j < c; j++ ) {
		for ( i = 0; i < r; i++ ) {
			octets.q = values[i+j*r];
			for ( k = 0; k < sizeof(QMTX_FIX16); k++ ) {
				h = table[(h ^ octets.o[k]) % 256];
				lrc = (lrc+octets.o[k]);
			}
		}
	}

	lrc = (((lrc ^ 0xFF) + 1) & 0xFF);

	return (h<<8)+lrc;
}
