/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * nn.h
 *
 * nn.c head file
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

#ifndef _NN_H_
#define _NN_H_

#define MAX_RSA_MODULUS_BITS            2048  /* max rsa modulus bits length */

#define MAX_RSA_MODULUS_LEN             ((MAX_RSA_MODULUS_BITS + 7) / 8)
#define MAX_RSA_PRIME_BITS              ((MAX_RSA_MODULUS_BITS + 1) / 2)
#define MAX_RSA_PRIME_LEN               ((MAX_RSA_PRIME_BITS   + 7) / 8)

#define	NN_DIGIT_BITS                   32
#define	NN_HALF_DIGIT_BITS              16
#define	NN_DIGIT_LEN                    (NN_DIGIT_BITS / 8)
#define	MAX_NN_DIGITS \
		((MAX_RSA_MODULUS_LEN + NN_DIGIT_LEN - 1) / NN_DIGIT_LEN + 1)
#define MAX_NN_DIGIT                    0xFFFFFFFFL
#define MAX_NN_HALF_DIGIT               0xFFFFL

#define BLOCK_LENGTH (MAX_RSA_MODULUS_BITS / 32)

uint32_t nn_add(uint32_t *a, uint32_t *b, uint32_t *c, uint32_t digits);
uint32_t nn_sub(uint32_t *a, uint32_t *b, uint32_t *c, uint32_t digits);
uint32_t nn_montgomery_modmult(uint32_t *a, uint32_t *b, uint32_t *c,
	uint32_t *n, uint32_t *modm, uint32_t digits);
uint32_t nn_digits(uint32_t *a, uint32_t digits);

#endif /* END _NN_H_ */
