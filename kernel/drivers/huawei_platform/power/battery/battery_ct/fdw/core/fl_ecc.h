/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * fl_ecc.h
 *
 * fl_ecc.c head file
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

#ifndef _FL_ECC_H_
#define _FL_ECC_H_

uint8_t ecc_pointadd(struct str_ecc_key *key,
	struct str_affpoint *in1, struct str_affpoint *in2,
	struct str_affpoint *out);
uint8_t ecc_precombpoint(struct str_ecc_key *key);
uint8_t ecc_pointmult_comb(struct str_ecc_key *key, uint32_t *p_comb_table,
	uint8_t *k, struct str_affpoint *out);

#endif /* END  _FL_ECC_H_ */
