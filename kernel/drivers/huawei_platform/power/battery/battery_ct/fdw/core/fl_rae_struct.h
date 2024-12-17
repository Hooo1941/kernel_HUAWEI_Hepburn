/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * fl_rae_struct.h
 *
 * fl_rae_struct
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

#ifndef _FL_RAE_STRUCT_H_
#define _FL_RAE_STRUCT_H_

struct str_affpoint {
	uint32_t aff_xy[ECC_AFF_POINT_WORDLEN];
};

struct str_jacpoint {
	uint32_t jac_xyz[ECC_JAC_POINT_WORDLEN];
};

struct str_rr_n {
	uint32_t rr_n[ECCMAXWORDLEN];
};

/*
 * PAE模型类型定义
 */
struct str_pae {
	uint32_t pae_ram[0x100];
	uint32_t mod_word_len;
	uint32_t mod_param;
};

struct str_ecc_param {
	uint32_t bitlen;
	uint32_t *p;
	uint32_t *a;
	uint32_t *b;
	uint32_t *n;
	struct str_affpoint *g;
	uint32_t *ptr_comb_table_g;
	uint8_t h;
	uint8_t a_is_minus3;
};

struct str_ecc_key {
	struct str_ecc_param *param;
	struct str_pae *pae_ctx;
	uint8_t *d;
	struct str_affpoint *q;
	uint32_t *comb_table_q;
	uint8_t protect_level;
};

#define ALG_SHA_256          0x06

#endif /* END _FL_RAE_STRUCT_H_ */
