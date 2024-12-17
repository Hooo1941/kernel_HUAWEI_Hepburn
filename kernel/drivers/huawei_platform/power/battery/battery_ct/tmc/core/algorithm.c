/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * algorithm.c
 *
 * algorithm
 *
 * Copyright (c) 2023-2024 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/string.h>
#include "algorithm.h"

#define VERIFY_FAILED     (-1)
#define CALCULATE_SUCC    0xAAAAu
#define USCLEN_EXCEEDS    0x5555u
#define USCLEN_EDGE       56
#define LEN_OF_TMCSHA256  64
#define LEN_OF_UC_TMP     4
#define LEN_OF_ULMSGIN    32
#define LEN_OF_L2LT_ULMIN 16
#define LEN_OF_PUC_TMP    64
#define LEN_OF_ULRESOUT   8
#define LEN_OF_ULW        64
#define LEN_OF_LPOINTS    4
#define LEN_OF_H          8
#define UCNO_ULT2         0
#define UCNO_ULT1         1
#define UCNO_ULW2         2
#define UCNO_ULW1         3

#define INDEX0            0
#define INDEX1            1
#define INDEX2            2
#define INDEX3            3
#define INDEX4            4
#define INDEX5            5
#define INDEX6            6
#define INDEX7            7
#define INDEX15           15
#define INDEX16           16

#define OFFSET_BIT1       1
#define OFFSET_BIT2       2
#define OFFSET_BIT3       3
#define OFFSET_BIT6       6
#define OFFSET_BIT7       7
#define OFFSET_BIT10      10
#define OFFSET_BIT11      11
#define OFFSET_BIT13      13
#define OFFSET_BIT17      17
#define OFFSET_BIT18      18
#define OFFSET_BIT19      19
#define OFFSET_BIT22      22
#define OFFSET_BIT25      25
#define OFFSET_BIT63      63
#define BYT1              8
#define BYT2              16
#define BYT4              32
#define BYT8              64
#define BYT64             512
#define OFFSET_BYT1       8
#define OFFSET_BYT2       16
#define OFFSET_BYT3       24
#define OFFSET_BYT4       32
#define OFFSET_BYT5       40
#define OFFSET_BYT6       48
#define OFFSET_BYT7       56
#define OFFSET_BYT8       64

#define BOUNDARY_VALUE    0x80
#define MAX_OF_BYT4       0xffffffffull
#define OVERFLOW_OF_BYT4  0x100000000ull
#define LLONG_MAX         0x8000000000000000ull
#define CLEAR_LOW         0xffffffff00000000ull

#define NUM_ECC_DIGITS    (ECC_BYTES >> 3)
#define MAX_TRIES         16
#define LEN_OF_PRES       ((NUM_ECC_DIGITS << 1) - 1)
#define LEN_OF_LPRODUCT   (NUM_ECC_DIGITS << 1)

#define SUPPORTS_INT128   0

#if SUPPORTS_INT128
	typedef unsigned __int128 uint128_t;
#else
	typedef struct {
		uint64_t m_low;
		uint64_t m_high;
	} uint128_t;
#endif

typedef struct ecc_point {
	uint64_t x[NUM_ECC_DIGITS];
	uint64_t y[NUM_ECC_DIGITS];
} ecc_point;

#define concat_(a, b) a##b
#define concat(a, b) concat_(a, b)

#define CURVE_P_24 {0xFFFFFFFFFFFFFFFFull, 0xFFFFFFFFFFFFFFFEull, 0xFFFFFFFFFFFFFFFFull}
#define CURVE_P_32 {0xFFFFFFFFFFFFFFFFull, 0x00000000FFFFFFFFull, 0x0000000000000000ull, 0xFFFFFFFF00000001ull}
#define CURVE_B_24 {0xFEB8DEECC146B9B1ull, 0x0FA7E9AB72243049ull, 0x64210519E59C80E7ull}
#define CURVE_B_32 {0x3BCE3C3E27D2604Bull, 0x651D06B0CC53B0F6ull, 0xB3EBBD55769886BCull, 0x5AC635D8AA3A93E7ull}
#define CURVE_G_24 { \
			{0xF4FF0AFD82FF1012ull, 0x7CBF20EB43A18800ull, 0x188DA80EB03090F6ull}, \
			{0x73F977A11E794811ull, 0x631011ED6B24CDD5ull, 0x07192B95FFC8DA78ull}}
#define CURVE_G_32 { \
			{0xF4A13945D898C296ull, 0x77037D812DEB33A0ull, 0xF8BCE6E563A440F2ull, 0x6B17D1F2E12C4247ull}, \
			{0xCBB6406837BF51F5ull, 0x2BCE33576B315ECEull, 0x8EE7EB4A7C0F9E16ull, 0x4FE342E2FE1A7F9Bull}}
#define CURVE_N_24 {0x146BC9B1B4D22831ull, 0xFFFFFFFF99DEF836ull, 0xFFFFFFFFFFFFFFFFull}
#define CURVE_N_32 {0xF3B9CAC2FC632551ull, 0xBCE6FAADA7179E84ull, 0xFFFFFFFFFFFFFFFFull, 0xFFFFFFFF00000000ull}

static uint64_t curve_p[NUM_ECC_DIGITS] = concat(CURVE_P_, ECC_CURVE);
static uint64_t curve_b[NUM_ECC_DIGITS] = concat(CURVE_B_, ECC_CURVE);
static ecc_point curve_g = concat(CURVE_G_, ECC_CURVE);
static uint64_t curve_n[NUM_ECC_DIGITS] = concat(CURVE_N_, ECC_CURVE);

/* hash */
#define b2lt(x) (*(x) * 0x1000000 + *((x) + 1) * 0x10000 + *((x) + 2) * 0x100 + *((x) + 3))
#define l2lt(x) \
	(((x) & 0xff) * 0x1000000 + (((x) >> 8) & 0xff) * 0x10000 + (((x) >> 16) & 0xff) * 0x100 + (((x) >> 24) & 0xff))
uint32_t const tmcsha256_iv[] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c,
	0x1f83d9ab, 0x5be0cd19};

uint32_t const tmcsha256_k[LEN_OF_TMCSHA256] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
	0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
	0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
	0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
	0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
	0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
	0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
	0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
	0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

/* Assume that we are using a POSIX-like system with /dev/urandom or /dev/random. */
#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif

static void vli_clear(uint64_t *p_vli, size_t len)
{
	unsigned int i;

	if (!p_vli || !len)
		return;

	for (i = 0; i < len; ++i)
		p_vli[i] = 0;
}

/* Returns 1 if p_vli == 0, 0 otherwise. */
static int vli_iszero(uint64_t *p_vli)
{
	unsigned int i;

	if (!p_vli)
		return 0;

	for (i = 0; i < NUM_ECC_DIGITS; ++i) {
		if (p_vli[i])
			return 0;
	}
	return 1;
}

/* Returns nonzero if bit p_bit of p_vli is set. */
static uint64_t vli_testbit(uint64_t *p_vli, unsigned int p_bit)
{
	return (p_vli[p_bit >> 6] & ((uint64_t)1 << (p_bit & 0x3f))); /* 6 表示右移6位：p_bit / 64, 0x3f 表示p_bit % 64 */
}

/* Counts the number of 64-bit "digits" in p_vli. */
static unsigned int vli_numdigits(uint64_t *p_vli)
{
	int i;

	/*
	 * Search from the end until we find a non-zero digit.
	 * We do it in reverse because we expect that most digits will be nonzero.
	 */
	for (i = (NUM_ECC_DIGITS - 1); (i >= 0) && (p_vli[i] == 0); --i) {
	}

	return (i + 1);
}

/* Counts the number of bits required for p_vli. */
static unsigned int vli_numbits(uint64_t *p_vli)
{
	unsigned int i;
	uint64_t l_digit;

	unsigned int l_numdigits = vli_numdigits(p_vli);
	if (l_numdigits == 0)
		return 0;

	l_digit = p_vli[l_numdigits - 1];
	for (i = 0; l_digit; ++i)
		l_digit >>= 1;

	return (((l_numdigits - 1) * BYT8) + i);
}

/* Sets p_dest = p_src. */
static void vli_set(uint64_t *p_dest, size_t dest_len, uint64_t *p_src, size_t src_len)
{
	unsigned int i;

	if (!p_dest || !p_src || !dest_len || !src_len || (dest_len < src_len))
		return;

	for (i = 0; i < dest_len; ++i)
		p_dest[i] = p_src[i];
}

/* Returns sign of p_left - p_right. */
static int vli_cmp(uint64_t *p_left, uint64_t *p_right)
{
	int i;

	for (i = NUM_ECC_DIGITS - 1; i >= 0; --i) {
		if (p_left[i] > p_right[i])
			return 1;
		else if (p_left[i] < p_right[i])
			return -1;
	}
	return 0;
}

/* Computes p_result = p_in << c, returning carry. Can modify in place (if p_result == p_in). 0 < p_shift < 64. */
static uint64_t vli_lshift(uint64_t *p_result, uint64_t *p_in, unsigned int p_shift)
{
	uint64_t l_carry = 0;
	unsigned int i;

	for (i = 0; i < NUM_ECC_DIGITS; ++i) {
		uint64_t l_temp = p_in[i];
		p_result[i] = (l_temp << p_shift) | l_carry;
		l_carry = l_temp >> (BYT8 - p_shift); /* 右移64减去p_shift偏移位 */
	}

	return l_carry;
}

/* Computes p_vli = p_vli >> 1. */
static void vli_rshift1(uint64_t *p_vli)
{
	uint64_t *l_end = p_vli;
	uint64_t l_carry = 0;

	p_vli += NUM_ECC_DIGITS;
	while (p_vli-- > l_end) {
		uint64_t l_temp = *p_vli;
		*p_vli = (l_temp >> OFFSET_BIT1) | l_carry;
		l_carry = l_temp << 63; /* 63 表示左移63位 */
	}
}

/* Computes p_result = p_left + p_right, returning carry. Can modify in place. */
static uint64_t vli_add(uint64_t *p_result, uint64_t *p_left, uint64_t *p_right)
{
	uint64_t l_carry = 0;
	unsigned int i;

	for (i = 0; i < NUM_ECC_DIGITS; ++i) {
		uint64_t l_sum = p_left[i] + p_right[i] + l_carry;
		if (l_sum != p_left[i])
			l_carry = (l_sum < p_left[i]);
		p_result[i] = l_sum;
	}
	return l_carry;
}

/* Computes p_result = p_left - p_right, returning borrow. Can modify in place. */
static uint64_t vli_sub(uint64_t *p_result, uint64_t *p_left, uint64_t *p_right)
{
	uint64_t l_borrow = 0;
	unsigned int i;

	for (i = 0; i < NUM_ECC_DIGITS; ++i) {
		uint64_t l_diff = (p_left[i] - p_right[i] - l_borrow);
		if (l_diff != p_left[i])
			l_borrow = (l_diff > p_left[i]);
		p_result[i] = l_diff;
	}
	return l_borrow;
}

#if SUPPORTS_INT128
/* Computes p_result = p_left * p_right. */
static void vli_mult(uint64_t *p_result, uint64_t *p_left, uint64_t *p_right)
{
	uint128_t r01 = 0;
	uint64_t r2 = 0;
	unsigned int i, k;

	/* Compute each digit of p_result in sequence, maintaining the carries. */
	for (k = 0; k < LEN_OF_PRES; ++k) {
		unsigned int l_min = ((k < NUM_ECC_DIGITS) ? 0 : ((k + 1) - NUM_ECC_DIGITS));
		for (i = l_min; (i <= k) && (i < NUM_ECC_DIGITS); ++i) {
			uint128_t l_product = ((uint128_t)p_left[i] * p_right[k - i]);
			r01 += l_product;
			r2 += (r01 < l_product);
		}
		p_result[k] = (uint64_t)r01;
		r01 = ((r01 >> OFFSET_BYT8) | (((uint128_t)r2) << OFFSET_BYT8));
		r2 = 0;
	}

	p_result[LEN_OF_PRES] = (uint64_t)r01;
}

/* Computes p_result = p_left^2. */
static void vli_square(uint64_t *p_result, uint64_t *p_left)
{
	uint128_t r01 = 0;
	uint64_t r2 = 0;
	unsigned int i, k;

	for (k = 0; k < LEN_OF_PRES; ++k) {
		unsigned int l_min = ((k < NUM_ECC_DIGITS) ? 0 : ((k + 1) - NUM_ECC_DIGITS));
		for (i = l_min; (i <= k) && (i <= (k - i)); ++i) {
			uint128_t l_product = ((uint128_t)p_left[i] * p_left[k - i]);
			if (i < (k - i)) {
				r2 += l_product >> 127; /* 127 表示右移127位 */
				l_product *= 2; /* 2 表示计算公式中的亦或2 */
			}
			r01 += l_product;
			r2 += (r01 < l_product);
		}
		p_result[k] = (uint64_t)r01;
		r01 = (r01 >> OFFSET_BYT8) | (((uint128_t)r2) << OFFSET_BYT8);
		r2 = 0;
	}

	p_result[LEN_OF_PRES] = (uint64_t)r01;
}

#else /* #if SUPPORTS_INT128 */
static uint128_t mul_64_64(uint64_t p_left, uint64_t p_right)
{
	uint128_t l_result;

	uint64_t a0 = (p_left & MAX_OF_BYT4);
	uint64_t a1 = (p_left >> OFFSET_BYT4);
	uint64_t b0 = (p_right & MAX_OF_BYT4);
	uint64_t b1 = (p_right >> OFFSET_BYT4);

	uint64_t m0 = (a0 * b0);
	uint64_t m1 = (a0 * b1);
	uint64_t m2 = (a1 * b0);
	uint64_t m3 = (a1 * b1);

	m2 += (m0 >> OFFSET_BYT4);
	m2 += m1;
	/* overflow */
	if (m2 < m1)
		m3 += OVERFLOW_OF_BYT4;

	l_result.m_low = ((m0 & MAX_OF_BYT4) | (m2 << OFFSET_BYT4));
	l_result.m_high = (m3 + (m2 >> OFFSET_BYT4));

	return l_result;
}

static uint128_t add_128_128(uint128_t a, uint128_t b)
{
	uint128_t l_result;

	l_result.m_low = (a.m_low + b.m_low);
	l_result.m_high = (a.m_high + b.m_high + (l_result.m_low < a.m_low));
	return l_result;
}

static void vli_mult(uint64_t *p_result, uint64_t *p_left, uint64_t *p_right)
{
	uint128_t r01 = {0, 0};
	uint64_t r2 = 0;
	unsigned int i, k;

	/* Compute each digit of p_result in sequence, maintaining the carries. */
	for (k = 0; k < LEN_OF_PRES; ++k) {
		unsigned int l_min = ((k < NUM_ECC_DIGITS) ? 0 : ((k + 1) - NUM_ECC_DIGITS));
		for (i = l_min; (i <= k) && (i < NUM_ECC_DIGITS); ++i) {
			uint128_t l_product = mul_64_64(p_left[i], p_right[k - i]);
			r01 = add_128_128(r01, l_product);
			r2 += (r01.m_high < l_product.m_high);
		}
		p_result[k] = r01.m_low;
		r01.m_low = r01.m_high;
		r01.m_high = r2;
		r2 = 0;
	}

	p_result[LEN_OF_PRES] = r01.m_low;
}

static void vli_square(uint64_t *p_result, uint64_t *p_left)
{
	uint128_t r01 = {0, 0};
	uint64_t r2 = 0;
	unsigned int i, k;

	for (k = 0; k < LEN_OF_PRES; ++k) {
		unsigned int l_min = ((k < NUM_ECC_DIGITS) ? 0 : ((k + 1) - NUM_ECC_DIGITS));
		for (i = l_min; (i <= k) && (i <= (k - i)); ++i) {
			uint128_t l_product = mul_64_64(p_left[i], p_left[k - i]);
			if (i < (k - i)) {
				r2 += l_product.m_high >> 63;
				l_product.m_high = (l_product.m_high << OFFSET_BIT1) | (l_product.m_low >> 63);
				l_product.m_low <<= 1;
			}
			r01 = add_128_128(r01, l_product);
			r2 += (r01.m_high < l_product.m_high);
		}
		p_result[k] = r01.m_low;
		r01.m_low = r01.m_high;
		r01.m_high = r2;
		r2 = 0;
	}

	p_result[LEN_OF_PRES] = r01.m_low;
}
#endif /* SUPPORTS_INT128 */


/*
 * Computes p_result = (p_left + p_right) % p_mod.
 * Assumes that p_left < p_mod and p_right < p_mod, p_result != p_mod.
 */
static void vli_modadd(uint64_t *p_result, uint64_t *p_left, uint64_t *p_right, uint64_t *p_mod)
{
	uint64_t l_carry = vli_add(p_result, p_left, p_right);
	/* p_result > p_mod (p_result = p_mod + remainder), so subtract p_mod to get remainder. */
	if (l_carry || (vli_cmp(p_result, p_mod) >= 0))
		vli_sub(p_result, p_result, p_mod);
}

/*
 * Computes p_result = (p_left - p_right) % p_mod.
 * Assumes that p_left < p_mod and p_right < p_mod, p_result != p_mod.
 */
static void vli_modsub(uint64_t *p_result, uint64_t *p_left, uint64_t *p_right, uint64_t *p_mod)
{
	uint64_t l_borrow = vli_sub(p_result, p_left, p_right);
	/*
	 * In this case, p_result == -diff == (max int) - diff.
	 * Since -x % d == d - x, we can get the correct result from p_result + p_mod (with overflow).
	 */
	if (l_borrow)
		vli_add(p_result, p_result, p_mod);
}

#if ECC_CURVE == SECP192R1
/*
 * Computes p_result = p_product % curve_p.
 * See algorithm 5 and 6 from http://www.isys.uni-klu.ac.at/PDF/2001-0126-MT.pdf
 */
static void vli_mmod_fast(uint64_t *p_result, uint64_t *p_product)
{
	uint64_t l_tmp[NUM_ECC_DIGITS];
	int l_carry;

	vli_set(p_result, NUM_ECC_DIGITS, p_product, NUM_ECC_DIGITS);

	vli_set(l_tmp, NUM_ECC_DIGITS, &p_product[INDEX3], NUM_ECC_DIGITS);
	l_carry = vli_add(p_result, p_result, l_tmp);

	l_tmp[INDEX0] = 0;
	l_tmp[INDEX1] = p_product[INDEX3];
	l_tmp[INDEX2] = p_product[INDEX4];
	l_carry += vli_add(p_result, p_result, l_tmp);

	l_tmp[INDEX0] = l_tmp[INDEX1] = p_product[INDEX5];
	l_tmp[INDEX2] = 0;
	l_carry += vli_add(p_result, p_result, l_tmp);

	while (l_carry || (vli_cmp(curve_p, p_result) != 1))
		l_carry -= vli_sub(p_result, p_result, curve_p);
}

#elif ECC_CURVE == SECP256R1
/*
 * Computes p_result = p_product % curve_p
 * from http://www.nsa.gov/ia/_files/nist-routines.pdf
 */
static void vli_mmod_fast(uint64_t *p_result, uint64_t *p_product)
{
	/* t */
	vli_set(p_result, NUM_ECC_DIGITS, p_product, NUM_ECC_DIGITS);

	/* s1 */
	uint64_t l_tmp[NUM_ECC_DIGITS] = {0, (p_product[INDEX5] & CLEAR_LOW), p_product[INDEX6],
		p_product[INDEX7]};
	int l_carry = vli_lshift(l_tmp, l_tmp, 1);
	l_carry += vli_add(p_result, p_result, l_tmp);

	/* s2 */
	l_tmp[INDEX1] = p_product[INDEX6] << OFFSET_BYT4;
	l_tmp[INDEX2] = (p_product[INDEX6] >> OFFSET_BYT4) | (p_product[INDEX7] << OFFSET_BYT4);
	l_tmp[INDEX3] = p_product[INDEX7] >> OFFSET_BYT4;
	l_carry += vli_lshift(l_tmp, l_tmp, 1);
	l_carry += vli_add(p_result, p_result, l_tmp);

	/* s3 */
	l_tmp[INDEX0] = p_product[INDEX4];
	l_tmp[INDEX1] = (p_product[INDEX5] & 0xffffffff); /* 0xffffffff max of 4 byte */
	l_tmp[INDEX2] = 0;
	l_tmp[INDEX3] = p_product[INDEX7];
	l_carry += vli_add(p_result, p_result, l_tmp);

	/* s4 */
	l_tmp[INDEX0] = ((p_product[INDEX4] >> OFFSET_BYT4) | (p_product[INDEX5] << OFFSET_BYT4));
	l_tmp[INDEX1] = ((p_product[INDEX5] >> OFFSET_BYT4) | (p_product[INDEX6] & CLEAR_LOW));
	l_tmp[INDEX2] = p_product[INDEX7];
	l_tmp[INDEX3] = ((p_product[INDEX6] >> OFFSET_BYT4) | (p_product[INDEX4] << OFFSET_BYT4));
	l_carry += vli_add(p_result, p_result, l_tmp);

	/* d1 */
	l_tmp[INDEX0] = ((p_product[INDEX5] >> OFFSET_BYT4) | (p_product[INDEX6] << OFFSET_BYT4));
	l_tmp[INDEX1] = (p_product[INDEX6] >> OFFSET_BYT4);
	l_tmp[INDEX2] = 0;
	l_tmp[INDEX3] = ((p_product[INDEX4] & 0xffffffff) | (p_product[INDEX5] << OFFSET_BYT4)); /* max of 4 byte */
	l_carry -= vli_sub(p_result, p_result, l_tmp);

	/* d2 */
	l_tmp[INDEX0] = p_product[INDEX6];
	l_tmp[INDEX1] = p_product[INDEX7];
	l_tmp[INDEX2] = 0;
	l_tmp[INDEX3] = ((p_product[INDEX4] >> OFFSET_BYT4) | (p_product[INDEX5] & CLEAR_LOW));
	l_carry -= vli_sub(p_result, p_result, l_tmp);

	/* d3 */
	l_tmp[INDEX0] = ((p_product[INDEX6] >> OFFSET_BYT4) | (p_product[INDEX7] << OFFSET_BYT4));
	l_tmp[INDEX1] = ((p_product[INDEX7] >> OFFSET_BYT4) | (p_product[INDEX4] << OFFSET_BYT4));
	l_tmp[INDEX2] = ((p_product[INDEX4] >> OFFSET_BYT4) | (p_product[INDEX5] << OFFSET_BYT4));
	l_tmp[INDEX3] = (p_product[INDEX6] << OFFSET_BYT4);
	l_carry -= vli_sub(p_result, p_result, l_tmp);

	/* d4 */
	l_tmp[INDEX0] = p_product[INDEX7];
	l_tmp[INDEX1] = (p_product[INDEX4] & CLEAR_LOW);
	l_tmp[INDEX2] = p_product[INDEX5];
	l_tmp[INDEX3] = (p_product[INDEX6] & CLEAR_LOW);
	l_carry -= vli_sub(p_result, p_result, l_tmp);
	if (l_carry < 0) {
		do
			l_carry += vli_add(p_result, p_result, curve_p);
		while (l_carry < 0);
	} else {
		while (l_carry || (vli_cmp(curve_p, p_result) != 1))
			l_carry -= vli_sub(p_result, p_result, curve_p);
	}
}
#endif

/* Computes p_result = (p_left * p_right) % curve_p. */
static void vli_modmult_fast(uint64_t *p_result, uint64_t *p_left, uint64_t *p_right)
{
	uint64_t l_product[LEN_OF_LPRODUCT];

	vli_mult(l_product, p_left, p_right);
	vli_mmod_fast(p_result, l_product);
}

/* Computes p_result = p_left^2 % curve_p. */
static void vli_modsquare_fast(uint64_t *p_result, uint64_t *p_left)
{
	uint64_t l_product[LEN_OF_LPRODUCT];

	vli_square(l_product, p_left);
	vli_mmod_fast(p_result, l_product);
}

static void carry_handle(uint64_t l_carry, uint64_t *u, unsigned char len)
{
	if (!u || !len)
		return;
	if (l_carry)
		u[len - 1] |= LLONG_MAX;
}

#define even(vli) (!((vli) & (1)))
/*
 * Computes p_result = (1 / p_input) % p_mod. All VLIs are the same size.
 * See "From Euclid's GCD to Montgomery Multiplication to the Great Divide"
 * https://labs.oracle.com/techrep/2001/smli_tr-2001-95.pdf
 */
static void vli_modinv(uint64_t *p_result, uint64_t *p_input, uint64_t *p_mod)
{
	uint64_t a[NUM_ECC_DIGITS], b[NUM_ECC_DIGITS], u[NUM_ECC_DIGITS], v[NUM_ECC_DIGITS], l_carry;
	int l_cmp_result;

	if (vli_iszero(p_input)) {
		vli_clear(p_result, NUM_ECC_DIGITS);
		return;
	}

	vli_set(a, NUM_ECC_DIGITS, p_input, NUM_ECC_DIGITS);
	vli_set(b, NUM_ECC_DIGITS, p_mod, NUM_ECC_DIGITS);
	vli_clear(u, NUM_ECC_DIGITS);
	u[INDEX0] = 1;
	vli_clear(v, NUM_ECC_DIGITS);

	while ((l_cmp_result = vli_cmp(a, b)) != 0) {
		l_carry = 0;
		if (even(a[INDEX0])) {
			vli_rshift1(a);
			if (!even(u[INDEX0]))
				l_carry = vli_add(u, u, p_mod);
			vli_rshift1(u);
			carry_handle(l_carry, u, NUM_ECC_DIGITS);
		} else if (even(b[INDEX0])) {
			vli_rshift1(b);
			if (!even(v[INDEX0]))
				l_carry = vli_add(v, v, p_mod);
			vli_rshift1(v);
			carry_handle(l_carry, v, NUM_ECC_DIGITS);
		} else if (l_cmp_result > 0) {
			vli_sub(a, a, b);
			vli_rshift1(a);
			if (vli_cmp(u, v) < 0)
				vli_add(u, u, p_mod);
			vli_sub(u, u, v);
			if (!even(u[INDEX0]))
				l_carry = vli_add(u, u, p_mod);
			vli_rshift1(u);
			carry_handle(l_carry, u, NUM_ECC_DIGITS);
		} else {
			vli_sub(b, b, a);
			vli_rshift1(b);
			if (vli_cmp(v, u) < 0)
				vli_add(v, v, p_mod);
			vli_sub(v, v, u);
			if (!even(v[INDEX0]))
				l_carry = vli_add(v, v, p_mod);
			vli_rshift1(v);
			carry_handle(l_carry, v, NUM_ECC_DIGITS);
		}
	}

	vli_set(p_result, NUM_ECC_DIGITS, u, NUM_ECC_DIGITS);
}

/*
 * ------ Point operations ------
 * Returns 1 if p_point is the point at infinity, 0 otherwise.
 */
static int eccpoint_iszero(ecc_point *p_point)
{
	return (vli_iszero(p_point->x) && vli_iszero(p_point->y));
}

/*
 * Point multiplication algorithm using Montgomery's ladder with co-Z coordinates.
 * From http://eprint.iacr.org/2011/338.pdf
 */
/* Double in place */
static void eccpoint_double_jacobian(uint64_t *x1, uint64_t *y1, uint64_t *z1)
{
	/* t1 = X, t2 = Y, t3 = Z */
	uint64_t t4[NUM_ECC_DIGITS];
	uint64_t t5[NUM_ECC_DIGITS];
	uint64_t l_carry;

	if (vli_iszero(z1))
		return;

	vli_modsquare_fast(t4, y1);   /* t4 = y1^2 */
	vli_modmult_fast(t5, x1, t4); /* t5 = x1*y1^2 = A */
	vli_modsquare_fast(t4, t4);   /* t4 = y1^4 */
	vli_modmult_fast(y1, y1, z1); /* t2 = y1*z1 = z3 */
	vli_modsquare_fast(z1, z1);   /* t3 = z1^2 */

	vli_modadd(x1, x1, z1, curve_p); /* t1 = x1 + z1^2 */
	vli_modadd(z1, z1, z1, curve_p); /* t3 = 2*z1^2 */
	vli_modsub(z1, x1, z1, curve_p); /* t3 = x1 - z1^2 */
	vli_modmult_fast(x1, x1, z1);    /* t1 = x1^2 - z1^4 */

	vli_modadd(z1, x1, x1, curve_p); /* t3 = 2*(x1^2 - z1^4) */
	vli_modadd(x1, x1, z1, curve_p); /* t1 = 3*(x1^2 - z1^4) */
	/* t1 = 3/2*(x1^2 - z1^4) = B */
	if (vli_testbit(x1, 0)) {
		l_carry = vli_add(x1, x1, curve_p);
		vli_rshift1(x1);
		x1[NUM_ECC_DIGITS - 1] |= (l_carry << OFFSET_BIT63);
	} else {
		vli_rshift1(x1);
	}

	vli_modsquare_fast(z1, x1);      /* t3 = B^2 */
	vli_modsub(z1, z1, t5, curve_p); /* t3 = B^2 - A */
	vli_modsub(z1, z1, t5, curve_p); /* t3 = B^2 - 2A = x3 */
	vli_modsub(t5, t5, z1, curve_p); /* t5 = A - x3 */
	vli_modmult_fast(x1, x1, t5);    /* t1 = B * (A - x3) */
	vli_modsub(t4, x1, t4, curve_p); /* t4 = B * (A - x3) - y1^4 = y3 */

	vli_set(x1, NUM_ECC_DIGITS, z1, NUM_ECC_DIGITS);
	vli_set(z1, NUM_ECC_DIGITS, y1, NUM_ECC_DIGITS);
	vli_set(y1, NUM_ECC_DIGITS, t4, NUM_ECC_DIGITS);
}

/* Modify (x1, y1) => (x1 * z^2, y1 * z^3) */
static void apply_z(uint64_t *x1, uint64_t *y1, uint64_t *z)
{
	uint64_t t1[NUM_ECC_DIGITS];

	vli_modsquare_fast(t1, z);    /* z^2 */
	vli_modmult_fast(x1, x1, t1); /* x1 * z^2 */
	vli_modmult_fast(t1, t1, z);  /* z^3 */
	vli_modmult_fast(y1, y1, t1); /* y1 * z^3 */
}

/*
 * Input P = (x1, y1, Z), Q = (x2, y2, Z)
 * Output P' = (x1', y1', Z3), P + Q = (x3, y3, Z3)
 * or P => P', Q => P + Q
 */
static void xycz_add(uint64_t *x1, uint64_t *y1, uint64_t *x2, uint64_t *y2)
{
	/* t1 = x1, t2 = y1, t3 = x2, t4 = y2 */
	uint64_t t5[NUM_ECC_DIGITS];

	vli_modsub(t5, x2, x1, curve_p); /* t5 = x2 - x1 */
	vli_modsquare_fast(t5, t5);      /* t5 = (x2 - x1)^2 = A */
	vli_modmult_fast(x1, x1, t5);    /* t1 = x1*A = B */
	vli_modmult_fast(x2, x2, t5);    /* t3 = x2*A = C */
	vli_modsub(y2, y2, y1, curve_p); /* t4 = y2 - y1 */
	vli_modsquare_fast(t5, y2);      /* t5 = (y2 - y1)^2 = D */

	vli_modsub(t5, t5, x1, curve_p); /* t5 = D - B */
	vli_modsub(t5, t5, x2, curve_p); /* t5 = D - B - C = x3 */
	vli_modsub(x2, x2, x1, curve_p); /* t3 = C - B */
	vli_modmult_fast(y1, y1, x2);    /* t2 = y1*(C - B) */
	vli_modsub(x2, x1, t5, curve_p); /* t3 = B - x3 */
	vli_modmult_fast(y2, y2, x2);    /* t4 = (y2 - y1)*(B - x3) */
	vli_modsub(y2, y2, y1, curve_p); /* t4 = y3 */

	vli_set(x2, NUM_ECC_DIGITS, t5, NUM_ECC_DIGITS);
}

static void ecc_bytes2native(uint64_t p_native[NUM_ECC_DIGITS], const unsigned char p_bytes[ECC_BYTES])
{
	unsigned i;
	const unsigned char *p_digit;

	for (i = 0; i < NUM_ECC_DIGITS; ++i) {
		p_digit = p_bytes + (BYT1 * (NUM_ECC_DIGITS - 1 - i));
		p_native[i] = ((uint64_t)p_digit[INDEX0] << OFFSET_BYT7) | ((uint64_t)p_digit[INDEX1] << OFFSET_BYT6) |
			((uint64_t)p_digit[INDEX2] << OFFSET_BYT5) | ((uint64_t)p_digit[INDEX3] << OFFSET_BYT4) |
			((uint64_t)p_digit[INDEX4] << OFFSET_BYT3) | ((uint64_t)p_digit[INDEX5] << OFFSET_BYT2) |
			((uint64_t)p_digit[INDEX6] << OFFSET_BYT1) | (uint64_t)p_digit[INDEX7];
	}
}

static void ecc_native2bytes(unsigned char p_bytes[ECC_BYTES], const uint64_t p_native[NUM_ECC_DIGITS])
{
	unsigned i;
	unsigned char *p_digit;

	for (i = 0; i < NUM_ECC_DIGITS; ++i) {
		p_digit = p_bytes + (BYT1 * (NUM_ECC_DIGITS - 1 - i));
		p_digit[INDEX0] = p_native[i] >> OFFSET_BYT7;
		p_digit[INDEX1] = p_native[i] >> OFFSET_BYT6;
		p_digit[INDEX2] = p_native[i] >> OFFSET_BYT5;
		p_digit[INDEX3] = p_native[i] >> OFFSET_BYT4;
		p_digit[INDEX4] = p_native[i] >> OFFSET_BYT3;
		p_digit[INDEX5] = p_native[i] >> OFFSET_BYT2;
		p_digit[INDEX6] = p_native[i] >> OFFSET_BYT1;
		p_digit[INDEX7] = p_native[i];
	}
}

/* Compute a = sqrt(a) (mod curve_p). */
static void mod_sqrt(uint64_t a[NUM_ECC_DIGITS])
{
	unsigned i;
	uint64_t p1[NUM_ECC_DIGITS] = { 1 };
	uint64_t l_result[NUM_ECC_DIGITS] = { 1 };

	/*
	 * Since curve_p == 3 (mod 4) for all supported curves, we can
	 * compute sqrt(a) = a^((curve_p + 1) / 4) (mod curve_p).
	 */
	vli_add(p1, curve_p, p1); /* p1 = curve_p + 1 */
	for (i = (vli_numbits(p1) - 1); i > 1; --i) {
		vli_modsquare_fast(l_result, l_result);
		if (vli_testbit(p1, i))
			vli_modmult_fast(l_result, l_result, a);
	}
	vli_set(a, NUM_ECC_DIGITS, l_result, NUM_ECC_DIGITS);
}

static void ecc_point_decompress(ecc_point *p_point, const unsigned char p_compressed[ECC_BYTES + 1])
{
	uint64_t _3[NUM_ECC_DIGITS] = { 3 }; /* -a = 3 */

	ecc_bytes2native(p_point->x, (p_compressed + 1));
	vli_modsquare_fast(p_point->y, p_point->x); /* y = x^2 */
	vli_modsub(p_point->y, p_point->y, _3, curve_p); /* y = x^2 - 3 */
	vli_modmult_fast(p_point->y, p_point->y, p_point->x); /* y = x^3 - 3x */
	vli_modadd(p_point->y, p_point->y, curve_b, curve_p); /* y = x^3 - 3x + b */

	mod_sqrt(p_point->y);

	if ((p_point->y[INDEX0] & 1) != (p_compressed[INDEX0] & 1))
		vli_sub(p_point->y, curve_p, p_point->y);
}

/*
 * -------- ECDSA code --------
 * Computes p_result = (p_left * p_right) % p_mod.
 */
static void vli_modmult(uint64_t *p_result, uint64_t *p_left, uint64_t *p_right, uint64_t *p_mod)
{
	uint64_t l_product[LEN_OF_LPRODUCT];
	uint64_t l_mod_multiple[LEN_OF_LPRODUCT];
	unsigned int l_digit_shift, l_bit_shift;
	unsigned int l_product_bits;
	unsigned int l_mod_bits = vli_numbits(p_mod);

	vli_mult(l_product, p_left, p_right);
	l_product_bits = vli_numbits(l_product + NUM_ECC_DIGITS);
	if (l_product_bits)
		l_product_bits += NUM_ECC_DIGITS * BYT8;
	else
		l_product_bits = vli_numbits(l_product);

	if (l_product_bits < l_mod_bits) { /* l_product < p_mod. */
		vli_set(p_result, NUM_ECC_DIGITS, l_product, NUM_ECC_DIGITS);
		return;
	}

	/*
	 * Shift p_mod by (l_leftBits - l_mod_bits). This multiplies p_mod by the largest
	 * power of two possible while still resulting in a number less than p_left.
	 */
	vli_clear(l_mod_multiple, NUM_ECC_DIGITS);
	vli_clear((l_mod_multiple + NUM_ECC_DIGITS), NUM_ECC_DIGITS);
	l_digit_shift = (l_product_bits - l_mod_bits) / BYT8; /* 表示bit转digit的位数 */
	l_bit_shift = (l_product_bits - l_mod_bits) % BYT8; /* 表示bit转digit后剩余的位数 */
	if (l_bit_shift)
		l_mod_multiple[l_digit_shift + NUM_ECC_DIGITS] = vli_lshift((l_mod_multiple + l_digit_shift), p_mod,
			l_bit_shift);
	else
		vli_set((l_mod_multiple + l_digit_shift), NUM_ECC_DIGITS, p_mod, NUM_ECC_DIGITS);

	/* Subtract all multiples of p_mod to get the remainder. */
	vli_clear(p_result, NUM_ECC_DIGITS);
	p_result[INDEX0] = 1; /* Use p_result as a temp var to store 1 (for subtraction) */
	while ((l_product_bits > (NUM_ECC_DIGITS * BYT8)) || (vli_cmp(l_mod_multiple, p_mod) >= 0)) {
		int l_cmp = vli_cmp((l_mod_multiple + NUM_ECC_DIGITS), (l_product + NUM_ECC_DIGITS));
		if ((l_cmp < 0) || ((l_cmp == 0) && (vli_cmp(l_mod_multiple, l_product) <= 0))) {
			/* borrow */
			if (vli_sub(l_product, l_product, l_mod_multiple))
				vli_sub((l_product + NUM_ECC_DIGITS), (l_product + NUM_ECC_DIGITS), p_result);
			vli_sub((l_product + NUM_ECC_DIGITS), (l_product + NUM_ECC_DIGITS), (l_mod_multiple + NUM_ECC_DIGITS));
		}
		uint64_t l_carry = ((l_mod_multiple[NUM_ECC_DIGITS] & 0x01) << OFFSET_BIT63);
		vli_rshift1(l_mod_multiple + NUM_ECC_DIGITS);
		vli_rshift1(l_mod_multiple);
		l_mod_multiple[NUM_ECC_DIGITS - 1] |= l_carry;

		--l_product_bits;
	}
	vli_set(p_result, NUM_ECC_DIGITS, l_product, NUM_ECC_DIGITS);
}

static unsigned int umax(unsigned int a, unsigned int b)
{
	return (a > b ? a : b);
}

static uint64_t* tmc_ecc_verify_cal(uint64_t *u1, uint64_t *u2, struct ecc_point *l_public, uint64_t *z, uint64_t *l_s)
{
	/* Calculate l_sum = G + Q. */
	uint64_t tx[NUM_ECC_DIGITS];
	uint64_t ty[NUM_ECC_DIGITS];
	ecc_point l_sum;
	static uint64_t rx[NUM_ECC_DIGITS];
	uint64_t ry[NUM_ECC_DIGITS];
	uint64_t tz[NUM_ECC_DIGITS];

	vli_set(l_sum.x, NUM_ECC_DIGITS, l_public->x, NUM_ECC_DIGITS);
	vli_set(l_sum.y, NUM_ECC_DIGITS, l_public->y, NUM_ECC_DIGITS);
	vli_set(tx, NUM_ECC_DIGITS, curve_g.x, NUM_ECC_DIGITS);
	vli_set(ty, NUM_ECC_DIGITS, curve_g.y, NUM_ECC_DIGITS);
	vli_modsub(z, l_sum.x, tx, curve_p); /* Z = x2 - x1 */
	xycz_add(tx, ty, l_sum.x, l_sum.y);
	vli_modinv(z, z, curve_p); /* Z = 1/Z */
	apply_z(l_sum.x, l_sum.y, z);

	/* Use Shamir's trick to calculate u1*G + u2*Q */
	ecc_point *l_points[LEN_OF_LPOINTS] = {NULL, &curve_g, l_public, &l_sum};
	ecc_point *l_point = l_points[(!!vli_testbit(u1, (umax(vli_numbits(u1), vli_numbits(u2))) - 1)) |
		((!!vli_testbit(u2, (umax(vli_numbits(u1), vli_numbits(u2))) - 1)) << OFFSET_BIT1)];
	vli_set(rx, NUM_ECC_DIGITS, l_point->x, NUM_ECC_DIGITS);
	vli_set(ry, NUM_ECC_DIGITS, l_point->y, NUM_ECC_DIGITS);
	vli_clear(z, NUM_ECC_DIGITS);
	z[INDEX0] = 1;

	for (int i = (umax(vli_numbits(u1), vli_numbits(u2))) - INDEX2; i >= 0; --i) {
		eccpoint_double_jacobian(rx, ry, z);
		ecc_point *l_pointt = l_points[(!!vli_testbit(u1, i)) | ((!!vli_testbit(u2, i)) << OFFSET_BIT1)];
		if (l_pointt) {
			vli_set(tx, NUM_ECC_DIGITS, l_pointt->x, NUM_ECC_DIGITS);
			vli_set(ty, NUM_ECC_DIGITS, l_pointt->y, NUM_ECC_DIGITS);
			apply_z(tx, ty, z);
			vli_modsub(tz, rx, tx, curve_p); /* Z = x2 - x1 */
			xycz_add(tx, ty, rx, ry);
			vli_modmult_fast(z, z, tz);
		}
	}

	vli_modinv(z, z, curve_p); /* Z = 1/Z */
	apply_z(rx, ry, z);

	/* v = x1 (mod n) */
	if (vli_cmp(curve_n, rx) != 1)
		vli_sub(rx, rx, curve_n);
	return rx;
}

/*
 * @function	tmc_ecc_verify
 * @brief	    Verify an ECDSA signature. Compute the hash of the signed data using the same hash as the signer
 *				and pass it to this function along with the signer's public key and the signature values (r and s).
 * @param[in]	p_publickey	The signer's public key.
 * @param[in]	p_hash		The hash of the signed data.
 * @param[in]	p_signature	The signature value.
 * @return	1		Signture is valid.
 *          0		Signture is invalid.
 */
int tmc_ecc_verify(const unsigned char p_publickey[ECC_VERIFY_BYTES], const unsigned char p_hash[ECC_BYTES],
	const unsigned char p_signature[ECC_VERIFY_BYTES])
{
	uint64_t u1[NUM_ECC_DIGITS], u2[NUM_ECC_DIGITS], z[NUM_ECC_DIGITS];
	ecc_point l_public;
	uint64_t l_r[NUM_ECC_DIGITS], l_s[NUM_ECC_DIGITS];
	uint64_t* rx1;

	if (!p_publickey || !p_hash || !p_signature)
		return -1;

	ecc_bytes2native(l_public.x, &p_publickey[INDEX0]);
	ecc_bytes2native(l_public.y, &p_publickey[ECC_BYTES]);
	ecc_bytes2native(l_r, p_signature);
	ecc_bytes2native(l_s, p_signature + ECC_BYTES);

	/* r, s must not be 0. */
	if (vli_iszero(l_r) || vli_iszero(l_s))
		return 0;

	/* r, s must be < n. */
	if ((vli_cmp(curve_n, l_r) != 1) || (vli_cmp(curve_n, l_s) != 1))
		return 0;

	/* Calculate u1 and u2. */
	vli_modinv(z, l_s, curve_n); /* Z = s^-1 */
	ecc_bytes2native(u1, p_hash);
	vli_modmult(u1, u1, z, curve_n); /* u1 = e/s */
	vli_modmult(u2, l_r, z, curve_n); /* u2 = r/s */

	rx1 = tmc_ecc_verify_cal(u1, u2, &l_public, z, l_s);
	/* Accept only if v == r. */
	return (vli_cmp(rx1, l_r) == 0);
}


uint32_t left_shift(uint32_t word, int bits)
{
	return (word << bits) | (word >> (BYT4 - bits));
}
uint32_t right_shift(uint32_t word, int bits)
{
	return (word >> bits) | (word << (BYT4 - bits));
}

uint32_t sha1_func(uint32_t ulx, uint32_t uly, uint32_t ulz, unsigned char ucsection)
{
	switch (ucsection) {
	case 0:
		return (ulx & uly) | (~ulx & ulz);
	case 2:
		return (ulx & uly) | (ulx & ulz) | (uly & ulz);
	default:
		return 	ulx ^ uly ^ ulz;
	}
}

uint32_t sha256_sigs(uint32_t ulin, unsigned char ucno)
{
	switch (ucno) {
	case UCNO_ULT2:
		return right_shift(ulin, OFFSET_BIT2) ^ right_shift(ulin, OFFSET_BIT13) ^ right_shift(ulin, OFFSET_BIT22);
	case UCNO_ULT1:
		return right_shift(ulin, OFFSET_BIT6) ^ right_shift(ulin, OFFSET_BIT11) ^ right_shift(ulin, OFFSET_BIT25);
	case UCNO_ULW2:
		return right_shift(ulin, OFFSET_BIT7) ^ right_shift(ulin, OFFSET_BIT18) ^ (ulin >> OFFSET_BIT3);
	case UCNO_ULW1:
		return right_shift(ulin, OFFSET_BIT17) ^ right_shift(ulin, OFFSET_BIT19) ^ (ulin >> OFFSET_BIT10);
	default:
		return 0;
	}
}

/*
 * @function	tmc_sha256blk
 * @brief	Calculate of SHA256.
 * @return
 */
void tmc_sha256blk(uint32_t *pulmsgblk, uint32_t *puloutblk)
{
	uint32_t h[LEN_OF_H];
	uint32_t ul_t1, ul_t2;
	uint32_t ul_w[LEN_OF_ULW];
	int a1, a2;

	if (!pulmsgblk || !puloutblk)
		return;

	for (a1 = 0; a1 < BYT2; a1++) /* 16 表示低于16位直接赋值 */
		ul_w[a1] = pulmsgblk[a1];
	for	(a1 = BYT2; a1 < LEN_OF_ULW; a1++) /* 16 表示高于16位进行SHA256运算 */
		ul_w[a1] = sha256_sigs(ul_w[a1 - INDEX2], UCNO_ULW1) + ul_w[a1 - INDEX7] +
				sha256_sigs(ul_w[a1 - INDEX15], UCNO_ULW2) + ul_w[a1 - INDEX16];

	for	(a1 = 0; a1 < LEN_OF_H; a1++)
		h[a1]= puloutblk[a1];

	for	(a1 = 0; a1 < LEN_OF_ULW; a1++) {
		ul_t1 = h[INDEX7] + sha256_sigs(h[INDEX4], UCNO_ULT1) + ((h[INDEX4] & h[INDEX5]) ^ (~h[INDEX4] & h[INDEX6])) +
			tmcsha256_k[a1] + ul_w[a1];
		ul_t2 = sha256_sigs(h[INDEX0], UCNO_ULT2) +
			((h[INDEX0] & h[INDEX1]) ^ (h[INDEX0] & h[INDEX2]) ^ (h[INDEX1] & h[INDEX2]));
		for (a2 = 7; a2 > 0; a2--) /* 7 表示最低8位，重新赋值为低一位 */
			h[a2] = h[a2 - 1];
		h[INDEX4] += ul_t1;
		h[INDEX0] = ul_t1 + ul_t2;
	}

	for (a1 = 0; a1 < LEN_OF_H; a1++)
		puloutblk[a1] += h[a1];
}

/*
 * @function	tmc_sha256update
 * @brief	Update of SHA256.
 * @param[in]
 * @param[out]
 * @return
 */
void tmc_sha256update(unsigned char* pucout, unsigned char* pucin, unsigned char* puclastblk, unsigned char ucinit)
{
	int i;
	uint32_t ul_msg_in[LEN_OF_ULMSGIN], ul_res_out[LEN_OF_ULRESOUT];
	unsigned char* puc_tmp;

	if (!pucout || !pucin || !puclastblk)
		return;

	puc_tmp = (unsigned char*)ul_msg_in;
	for (i = 0; i < LEN_OF_PUC_TMP; i++)
		puc_tmp[i] = pucin[i];
	for (i = 0; i < LEN_OF_L2LT_ULMIN; i++)
		ul_msg_in[i] = l2lt(ul_msg_in[i]);

	puc_tmp = (unsigned char*)ul_res_out;
	if (ucinit) {
		for (i = 0; i < LEN_OF_ULRESOUT; i++)
			ul_res_out[i] = tmcsha256_iv[i];
	} else {
		for (i = 0; i < LEN_OF_ULMSGIN; i++)
			puc_tmp[i] = puclastblk[i];
		for (i = 0; i < LEN_OF_ULRESOUT; i++)
			ul_res_out[i] = l2lt(ul_res_out[i]);
	}

	tmc_sha256blk(ul_msg_in, ul_res_out);

	for (i = 0; i < LEN_OF_ULRESOUT; i++)
		ul_res_out[i] = l2lt(ul_res_out[i]);
	for (i = 0; i < LEN_OF_ULMSGIN; i++)
		pucout[i] = puc_tmp[i];
}

/*
 * @function	tmc_sha256final
 * @brief	The last of SHA256.
 * @param[in]
 * @param[out]
 * @return	0xAAAA	Calculate success.
 *          Other	Calculate failed.
 */
unsigned short tmc_sha256final(unsigned char *pucout, unsigned char* pucin, unsigned char* puclastblk,
	unsigned short usblkdone, unsigned short usclen)
{
	uint32_t ul_tmp, i, j, k;
	unsigned char uc_tmp[LEN_OF_UC_TMP] = {0, 0, 0, 0};
	uint32_t ul_msg_in[LEN_OF_ULMSGIN], ul_res_out[LEN_OF_ULRESOUT];
	unsigned char* puc_tmp;

	if (!pucout || !pucin || !puclastblk)
		return VERIFY_FAILED;

	/* usclen 最大值64 */
	if (usclen > (BYT64 / BYT1))
		return USCLEN_EXCEEDS;

	for (i = 0; i < LEN_OF_ULMSGIN; i++)
		ul_msg_in[i] = 0;

	puc_tmp = (unsigned char*)ul_msg_in;

	for (i = 0; i < usclen; i++)
		puc_tmp[i] = pucin[i];

	puc_tmp[i] = BOUNDARY_VALUE; /* 给最后一位赋边界值 */

	for (i = 0; i < LEN_OF_ULMSGIN; i++)
		ul_msg_in[i] = l2lt(ul_msg_in[i]);

	/*
	 * usclen>=USCLEN_EDGE时，ul_msg_in[31] = usclen * 8 + 512 * usblkdone
	 * usclen<USCLEN_EDGE时，ul_msg_in[15] = usclen * 8 + 512 * usblkdone
	 */
	ul_msg_in[INDEX15 + (BYT2 * (usclen >= USCLEN_EDGE))] = (usclen * BYT1) + (BYT64 * usblkdone);

	puc_tmp = (unsigned char*)ul_res_out;
	if (!usblkdone) {
		for (i = 0; i < LEN_OF_ULRESOUT; i++)
			ul_res_out[i] = tmcsha256_iv[i];
	} else {
		for (i = 0; i < LEN_OF_ULMSGIN; i++)
			puc_tmp[i] = puclastblk[i];
		for (i = 0; i < LEN_OF_ULRESOUT; i++)
			ul_res_out[i] = l2lt(ul_res_out[i]);
	}
	tmc_sha256blk(ul_msg_in, ul_res_out);
	if (usclen >= USCLEN_EDGE)
		tmc_sha256blk(ul_msg_in + BYT2, ul_res_out);

	for (i = 0; i < LEN_OF_ULRESOUT; i++)
		ul_res_out[i] = l2lt(ul_res_out[i]);
	for (i = 0; i < LEN_OF_ULMSGIN; i++)
		pucout[i] = puc_tmp[i];

	return CALCULATE_SUCC;
}