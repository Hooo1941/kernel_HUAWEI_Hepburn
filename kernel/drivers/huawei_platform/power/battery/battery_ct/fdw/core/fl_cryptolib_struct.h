/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * fl_cryptolib_struct.h
 *
 * fl cryptolib struct
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

#ifndef _FL_CRYPTO_STRUCT_H_
#define _FL_CRYPTO_STRUCT_H_

#include <linux/string.h>

#define ECCMAXLEN               0x20
#define ECCMAXWORDLEN           0x08
#define ECC_AFF_POINT_WORDLEN   0x10

#define RESULT_ERROR            0x45
#define RESULT_OK               0xBA
#define RESULT_INFINITY         0x8C

/*
 * 彷射坐标点数据结构定义
 */
struct str_affpoint {
	uint32_t aff_xy[ECC_AFF_POINT_WORDLEN];
};

/*
 * PAE模型类型定义
 */
struct str_pae {
	uint32_t pae_ram[0x100];
	uint32_t mod_word_len;
	uint32_t mod_param;
};

/*
 * ECC曲线参数类型定义
 */
struct str_ecc_param {
	uint32_t bitlen;
	uint32_t *p;
	uint32_t *a;
	uint32_t *b;
	uint32_t *n;
	struct str_affpoint *g;
	uint32_t *ptr_comb_table_g;
	uint8_t h;
	uint8_t	a_is_minus3;
};

/*
 * ECC密钥对数据结构
 */
struct str_ecc_key {
	struct str_ecc_param *param;
	struct str_pae *pae_ctx;
	uint8_t *d;
	struct str_affpoint *q;
	uint32_t *comb_table_q;
	uint8_t protect_level;
};

#define PAE_LEVEL0_F    0x5A    /* RFU */
#define PAE_LEVEL1_M    0x2D    /* RFU */

/*
 * SHA functions const define
 */
enum {
	SHA256_MESSAGE_BLOCK_SIZE = 64,
	SHA256_HASHSIZE           = 32,
	SHA256_HASHWORDSIZE       = 8
};

/*
 *  All SHA functions return one of these values.
 */
enum {
	SHA_SUCCESS = 0,
	SHA_NULL,               /* Null pointer parameter */
	SHA_INTOOLONG,          /* input data too long */
	SHA_STATEERROR,         /* called Input after FinalBits or Result */
	SHA_BADPARAM            /* passed a bad parameter */
};
/* _SHA_enum_ */

/*
 *  This structure will hold context information for the SHA-256
 *  hashing operation.
 */
struct str_sha256_context {
	uint32_t    state[8];               /* Message Digest */
	uint32_t    length_high;            /* Message length in bits */
	uint32_t    length_low;             /* Message length in bits */
	uint16_t    message_block_index;    /* msg_block array index */
	uint8_t     message_block[64];      /* 512-bit msg blocks */
	int         alg;                    /* Alg type */
	int         computed;               /* Is the hash computed? */
	int         corrupted;              /* Cumulative corruption code */
};

#endif  /* END _FL_CRYPTO_STRUCT_H_ */
