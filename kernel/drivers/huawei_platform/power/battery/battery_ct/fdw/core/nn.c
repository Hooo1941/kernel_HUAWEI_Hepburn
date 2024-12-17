/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * nn.c
 *
 * big number arithmetical operations
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
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

#include "fl_type.h"
#include "nn.h"

void nn_assign(uint32_t *a, uint32_t *b, uint32_t digits)
{
	if (digits) {
		do
			*a++ = *b++;
		while (--digits);
	}
}

void nn_assignzero(uint32_t *a, uint32_t digits)
{
	if (digits) {
		do
			*a++ = 0;
		while (--digits);
	}
}

static uint32_t nn_digit_bits(uint32_t a)
{
	uint32_t i;

	for (i = 0; i < NN_DIGIT_BITS; i++, a >>= 1)
		if (a == 0)
			break;
	return i;
}

int nn_cmp(uint32_t *a, uint32_t *b, uint32_t digits)
{
	if (digits) {
		do {
			digits--;
			if (*(a + digits) > *(b + digits))
				return 1;
			if (*(a + digits) < *(b + digits))
				return -1;
		} while (digits);
	}
	return 0;
}

void dmult(uint32_t a, uint32_t b, uint32_t *high, uint32_t *low)
{
	uint16_t aa[4];
	uint32_t m1;
	uint32_t m2;
	uint32_t m;
	uint32_t ml;
	uint32_t mh;
	uint32_t carry;

	carry = 0;
	aa[0] = (uint16_t)(a & MAX_NN_HALF_DIGIT);
	aa[1] = (uint16_t)((a >> NN_HALF_DIGIT_BITS) & MAX_NN_HALF_DIGIT);
	aa[2] = (uint16_t)(b & MAX_NN_HALF_DIGIT);
	aa[3] = (uint16_t)((b >> NN_HALF_DIGIT_BITS) & MAX_NN_HALF_DIGIT);
	*low = (uint32_t)aa[0] * aa[2];
	*high = (uint32_t)aa[1] * aa[3];
	m1 = (uint32_t)aa[0] * aa[3];
	m2 = (uint32_t)aa[1] * aa[2];
	m = m1 + m2;
	if (m < m1)
		carry = 1L << (NN_DIGIT_BITS / 2);
	ml = (m & MAX_NN_HALF_DIGIT) << (NN_DIGIT_BITS / 2);
	mh = m >> (NN_DIGIT_BITS / 2);
	*low += ml;
	if (*low < ml)
		carry++;
	*high += carry + mh;
}

uint32_t subdigitmult(uint32_t *a, uint32_t *b, uint32_t c,
		uint32_t *d, uint32_t digits)
{
	uint32_t borrow;
	uint32_t thigh;
	uint32_t tlow;
	uint32_t i;

	borrow = 0;

	if (c != 0) {
		for (i = 0; i < digits; i++) {
			dmult(c, d[i], &thigh, &tlow);
			a[i] = b[i] - borrow;
			if (a[i] > (MAX_NN_DIGIT - borrow))
				borrow = 1;
			else
				borrow = 0;
			a[i] -= tlow;
			if (a[i] > (MAX_NN_DIGIT - tlow))
				borrow++;
			borrow += thigh;
		}
	}
	return borrow;
}

uint32_t nn_add_generate_carry(uint32_t t1, uint32_t c)
{
	if (t1 < c)
		return 1;
	else
		return 0;
}

/*
 * Computes a = b + c. Returns carry.
 *
 *       Lengths: a[digits], b[digits], c[digits].
 */
uint32_t nn_add(uint32_t *a, uint32_t *b, uint32_t *c, uint32_t digits)
{
	uint32_t temp;
	uint32_t carry;

	carry = 0;
	if (digits) {
		do {
			temp = (*b++) + carry;
			if (temp < carry) {
				temp = *c;
			} else {
				temp += *c;
				carry = nn_add_generate_carry(temp, *c);
			}
			*a++ = temp;
			c++;
		} while (--digits);
	}
	return carry;
}

uint32_t nn_sub_generate_carry(uint32_t t1, uint32_t t2)
{
	if (t1 > t2)
		return 1;
	else
		return 0;
}
/*
 * Computes a = b - c. Returns borrow.
 *
 *       Lengths: a[digits], b[digits], c[digits].
 */
uint32_t nn_sub(uint32_t *a, uint32_t *b, uint32_t *c, uint32_t digits)
{
	uint32_t temp;
	uint32_t temp2;
	uint32_t borrow;

	borrow = 0;
	if (digits) {
		do {
			temp = (*b++) - borrow;
			if (temp == MAX_NN_DIGIT) {
				temp = MAX_NN_DIGIT - *c++;
			} else {
				temp  = temp - *c;
				temp2 = MAX_NN_DIGIT - *c;
				c++;
				borrow = nn_sub_generate_carry(temp, temp2);
			}
			*a++ = temp;
		} while (--digits);
	}

	return borrow;
}

uint32_t nn_digits(uint32_t *a, uint32_t digits)
{
	if (digits) {
		digits--;
		do {
			if (*(a + digits))
				break;
		} while (digits--);
		return (digits + 1);
	}
	return digits;
}

uint32_t nn_mult_c_loop(uint32_t *t, uint32_t *b, uint32_t *c,
		uint32_t i, uint32_t c_digits)
{
	uint32_t dhigh;
	uint32_t dlow;
	uint32_t carry;
	uint32_t j;

	carry = 0;
	for (j = 0; j < c_digits; j++) {
		dmult(*(b + i), *(c + j), &dhigh, &dlow);
		*(t + (i + j)) = *(t + (i + j)) + carry;
		if (*(t + (i + j)) < carry)
			carry = 1;
		else
			carry = 0;
		*(t + (i + j)) += dlow;
		if (*(t + (i + j)) < dlow)
			carry++;
		carry += dhigh;
	}
	return carry;
}

/*
 * Computes a = b * c.
 *       Lengths: a[2*digits], b[digits], c[digits].
 *       Assumes digits < MAX_NN_DIGITS.
 */
void nn_mult(uint32_t *a, uint32_t *b, uint32_t *c, uint32_t digits)
{
	uint32_t t[2 * MAX_NN_DIGITS];
	uint32_t carry;
	uint32_t b_digits;
	uint32_t c_digits;
	uint32_t i;

	nn_assignzero(t, 2 * digits);
	b_digits = nn_digits(b, digits);
	c_digits = nn_digits(c, digits);
	for (i = 0; i < b_digits; i++) {
		if (*(b + i) != 0)
			carry = nn_mult_c_loop(t, b, c, i, c_digits);
		else
			carry = 0;

		*(t + (i + c_digits)) += carry;
	}
	nn_assign(a, t, 2 * digits);
}

/*
 * Computes a = b * 2^c (i.e., shifts left c bits), returning carry.
 * Requires c < NN_DIGIT_BITS.
 */
uint32_t nn_lshift(uint32_t *a, uint32_t *b, uint32_t c, uint32_t digits)
{
	uint32_t temp;
	uint32_t carry = 0;
	uint32_t t;

	if (c < NN_DIGIT_BITS) {
		if (digits) {
			t = NN_DIGIT_BITS - c;

			do {
				temp = *b++;
				*a++ = (temp << c) | carry;
				carry = c ? (temp >> t) : 0;
			} while (--digits);
		}
	}
	return carry;
}

/*
 * Computes a = b div 2^c (i.e., shifts right c bits), returning carry.
 * Requires: c < NN_DIGIT_BITS.
 */
uint32_t nn_rshift(uint32_t *a, uint32_t *b, uint32_t c, uint32_t digits)
{
	uint32_t temp;
	uint32_t t;
	uint32_t carry = 0;

	if (c < NN_DIGIT_BITS) {
		if (digits) {
			t = NN_DIGIT_BITS - c;

			do {
				digits--;
				temp = *(b + digits);
				*(a + digits) = (temp >> c) | carry;
				carry = c ? (temp << t) : 0;
			} while (digits);
		}
	}
	return carry;
}

void nn_div_process_two_word(uint32_t s, uint32_t *ccptr, uint32_t *ai)
{
	uint32_t t[2];
	uint32_t uv[2];
	uint32_t a[2];
	uint32_t c[2];

	c[0] = (uint16_t)(s >> NN_HALF_DIGIT_BITS);
	c[1] = (uint16_t)(s & MAX_NN_HALF_DIGIT);
	*t = *ccptr;
	*(t + 1) = *(ccptr + 1);
	if (c[0] == MAX_NN_HALF_DIGIT)
		a[0] = (uint16_t)((*(t + 1)) >> NN_HALF_DIGIT_BITS);
	else
		a[0] = (uint16_t)(*(t + 1) / (c[0] + 1));
	uv[0] = (uint32_t)a[0] * (uint32_t)c[1];
	uv[1] = (uint32_t)a[0] * (uint32_t)c[0];
	*t -= (uv[0] << NN_HALF_DIGIT_BITS);
	if (*t > (MAX_NN_DIGIT - (uv[0] << NN_HALF_DIGIT_BITS)))
		t[1]--;
	*(t + 1) -= uv[0] >> NN_HALF_DIGIT_BITS;
	*(t + 1) -= uv[1];
	while ((*(t + 1) > c[0]) ||
		((*(t + 1) == c[0]) && (*t >= (c[1] << 16)))) {
		*t -= (c[1] << NN_HALF_DIGIT_BITS);
		if (*t > MAX_NN_DIGIT - (c[1] << NN_HALF_DIGIT_BITS))
			t[1]--;
		*(t + 1) -= c[0];
		a[0]++;
	}
	if (c[0] == MAX_NN_HALF_DIGIT)
		a[1] = (uint16_t)((*(t + 1)) & MAX_NN_HALF_DIGIT);
	else
		a[1] = (uint16_t)(((*(t + 1)) << NN_HALF_DIGIT_BITS) +
			((*t) >> NN_HALF_DIGIT_BITS) / (c[0] + 1));
	uv[0] = (uint32_t)a[1] * (uint32_t)c[1];
	uv[1] = (uint32_t)a[1] * (uint32_t)c[0];
	*t -= uv[0];
	if (*t > (MAX_NN_DIGIT - uv[0]))
		t[1]--;
	*t -= (uv[1] << NN_HALF_DIGIT_BITS);
	if (*t > (MAX_NN_DIGIT - (uv[1] << NN_HALF_DIGIT_BITS)))
		t[1]--;
	*(t + 1) -= (uv[1] >> NN_HALF_DIGIT_BITS);
	while ((*(t + 1) > 0) || ((*(t + 1) == 0) && (*t >= s))) {
		*t -= s;
		if (*t > (MAX_NN_DIGIT - s))
			t[1]--;
		a[1]++;
	}
	*ai = (a[0] << NN_HALF_DIGIT_BITS) + a[1];
}

/*
 * Computes a = c div d and b = c mod d.
 *
 *       Lengths: a[c_digits], b[d_digits], c[c_digits], d[d_digits].
 *       Assumes d > 0, c_digits < 2 * MAX_NN_DIGITS,
 *                                       d_digits < MAX_NN_DIGITS.
 */
void nn_div(uint32_t *a, uint32_t *b, uint32_t *c,
		uint32_t c_digits, uint32_t *d, uint32_t d_digits)
{
	uint32_t ai;
	uint32_t cc[2 * MAX_NN_DIGITS + 1];
	uint32_t dd[MAX_NN_DIGITS];
	uint32_t s;
	uint32_t *ccptr;
	int i;
	uint32_t dd_digits, shift;

	dd_digits = nn_digits(d, d_digits);
	if (dd_digits == 0)
		return;
	shift = NN_DIGIT_BITS - nn_digit_bits(d[dd_digits - 1]);
	nn_assignzero(cc, dd_digits);
	cc[c_digits] = nn_lshift(cc, c, shift, c_digits);
	nn_lshift(dd, d, shift, dd_digits);
	s = dd[dd_digits - 1];

	nn_assignzero(a, c_digits);

	for (i = c_digits - dd_digits; i >= 0; i--) {
		if (s == MAX_NN_DIGIT) {
			ai = cc[i + dd_digits];
		} else {
			ccptr = &cc[i + dd_digits - 1];
			s++;
			nn_div_process_two_word(s, ccptr, &ai);
			s--;
		}
		cc[i + dd_digits] -= subdigitmult(&cc[i], &cc[i], ai, dd, dd_digits);
		while (cc[i + dd_digits] || (nn_cmp(&cc[i], dd, dd_digits) >= 0)) {
			ai++;
			cc[i + dd_digits] -= nn_sub(&cc[i], &cc[i], dd, dd_digits);
		}
		a[i] = ai;
	}
	nn_assignzero(b, d_digits);
	nn_rshift(b, cc, shift, dd_digits);
}

/*
 * Computes a = b mod c.
 *
 * Lengths: a[c_digits], b[b_digits], c[c_digits].
 * Assumes c > 0, b_digits < 2 * MAX_NN_DIGITS, c_digits < MAX_NN_DIGITS.
 */
void nn_mod(uint32_t *a, uint32_t *b, uint32_t b_digits,
		uint32_t *c, uint32_t c_digits)
{
	uint32_t t[2 * MAX_NN_DIGITS];

	nn_div(t, a, b, b_digits, c, c_digits);
}

/*
 * Computes a = b * c * R^-1mod d.
 *
 * Lengths: a[digits], b[digits], c[digits], d[digits].
 * Assumes d > 0, digits < MAX_NN_DIGITS.
 */
uint32_t nn_montgomery_modmult(uint32_t *a, uint32_t *b, uint32_t *c,
		uint32_t *n, uint32_t *modm, uint32_t digits)
{
	uint32_t i;
	uint32_t j;
	uint32_t t[2 * MAX_NN_DIGITS];
	uint32_t temp[2];
	uint32_t temp32[MAX_NN_DIGITS];
	uint32_t temp64[2 * MAX_NN_DIGITS];

	if (digits == 0)
		return 0;
	for (i = 0; i < digits; i++)
		temp32[digits - 1 - i] = 0;
	for (i = 0; i < 2 * digits; i++) {
		t[2 * digits - 1 - i] = 0;
		temp64[2 * digits - 1 - i] = 0;
	}
	for (i = 0; i < digits; i++) {
		nn_mult(temp, &b[i], &c[0], 1);
		nn_add(&temp[0], &temp[0], &t[0], 1);
		nn_mult(temp, &temp[0], modm, 1);

		temp32[0] = temp[0];
		nn_mult(temp64, temp32, n, digits);

		nn_add(t, t, temp64, 2 * digits);
		temp32[0] = b[i];
		nn_mult(temp64, temp32, c, digits);
		nn_add(t, t, temp64, 2 * digits);

		for (j = 0; j < 2 * digits - 1; j++)
			t[j] = t[j + 1];
	}

	if (t[digits] > 0) {
		nn_assignzero(temp64, 2 * digits);
		nn_assign(temp64, n, digits);
		nn_sub(t, t, temp64, 2 * digits);
		for (i = 0; i < digits; i++)
			a[i] = t[i];
		return 1;
	}
	for (i = 0; i < digits; i++)
		a[i] = t[i];
	return 0;
}
