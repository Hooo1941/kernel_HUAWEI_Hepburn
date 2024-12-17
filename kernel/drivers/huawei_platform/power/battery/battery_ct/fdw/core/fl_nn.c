/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * fl_nn.c
 *
 * RSA bottom-layer modulo addition modulo subtraction modular multiplication operation
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

#ifndef _FL_NN_H_
#define _FL_NN_H_

#include "fl_type.h"
#include "nn.h"
#include "fl_define.h"
#include "fl_rae_struct.h"

uint16_t rsa_soft_add(struct str_pae *pae_ctx,
		uint8_t cmd2, uint8_t cmd3, uint8_t cmd4)
{
	uint32_t buf0[BLOCK_LENGTH + 1];
	uint32_t buf1[BLOCK_LENGTH + 1];
	uint8_t i;
	uint8_t carry;
	uint16_t b_err;

	carry = 0;
	b_err = 0;
	carry = nn_add(pae_ctx->pae_ram + PAE_RAM_BLOCK_DIV4_LENTH * cmd2,
		pae_ctx->pae_ram + PAE_RAM_BLOCK_DIV4_LENTH * cmd3,
		pae_ctx->pae_ram + PAE_RAM_BLOCK_DIV4_LENTH * cmd4,
		pae_ctx->mod_word_len);
	if (carry == 1) {
		for (i = 0; i < pae_ctx->mod_word_len; i++) {
			buf0[i] = pae_ctx->pae_ram[i];
			buf1[i] = pae_ctx->pae_ram[i + PAE_RAM_BLOCK_DIV4_LENTH * cmd2];
		}
		buf0[pae_ctx->mod_word_len] = 0;
		buf1[pae_ctx->mod_word_len] = 1;
		carry = nn_sub(buf1, buf1, buf0, pae_ctx->mod_word_len + 1);
		if (buf1[pae_ctx->mod_word_len] == 1) {
			carry = nn_sub(buf1, buf1, buf0, pae_ctx->mod_word_len + 1);
			if (buf1[pae_ctx->mod_word_len] == 1)
				b_err = 1;
		}
		for (i = 0; i < pae_ctx->mod_word_len; i++)
			pae_ctx->pae_ram[i + PAE_RAM_BLOCK_DIV4_LENTH * cmd2] = buf1[i];
		return b_err;
	}
	return 0;
}

uint16_t rsa_soft_sub(struct str_pae *pae_ctx,
		uint8_t cmd2, uint8_t cmd3, uint8_t cmd4)
{
	uint8_t carry;
	uint16_t b_err;

	carry = 0;
	b_err = 0;
	carry = nn_sub(pae_ctx->pae_ram + PAE_RAM_BLOCK_DIV4_LENTH * cmd2,
		pae_ctx->pae_ram + PAE_RAM_BLOCK_DIV4_LENTH * cmd3,
		pae_ctx->pae_ram + PAE_RAM_BLOCK_DIV4_LENTH * cmd4,
		pae_ctx->mod_word_len);
	if (carry == 1) {
		carry = nn_add(pae_ctx->pae_ram + PAE_RAM_BLOCK_DIV4_LENTH * cmd2,
			pae_ctx->pae_ram + PAE_RAM_BLOCK_DIV4_LENTH * cmd2,
			pae_ctx->pae_ram, pae_ctx->mod_word_len);
		if (carry != 1) {
			carry = nn_add(pae_ctx->pae_ram + PAE_RAM_BLOCK_DIV4_LENTH * cmd2,
				pae_ctx->pae_ram + PAE_RAM_BLOCK_DIV4_LENTH * cmd2,
				pae_ctx->pae_ram, pae_ctx->mod_word_len);
			if (carry != 1)
				b_err = 1;
		}
		return b_err;
	}
	return 0;
}

/*
 * 函数名称: rsa_command
 * 函数功能: RSA底层模加模减.模乘.操作
 * 输入参数:	struct str_pae *pae_ctx, pae 结构体指针
 * 			uint8_t cmd1-ins,
 * 			uint8_t cmd2-result block
 * 			uint8_t cmd3-op1block
 * 			uint8_t cmd4-op2block
 * 输出参数:	N/A
 */
uint16_t rsa_command(struct str_pae *pae_ctx,
		uint8_t cmd1, uint8_t cmd2, uint8_t cmd3, uint8_t cmd4)
{
	if (cmd1 == RSA_ADD) {
		return rsa_soft_add(pae_ctx, cmd2, cmd3, cmd4);
	} else if (cmd1 == RSA_SUB) {
		return rsa_soft_sub(pae_ctx, cmd2, cmd3, cmd4);
	} else if (cmd1 == RSA_MULT) {
		nn_montgomery_modmult(
			pae_ctx->pae_ram + PAE_RAM_BLOCK_DIV4_LENTH * cmd2,
			pae_ctx->pae_ram + PAE_RAM_BLOCK_DIV4_LENTH * cmd3,
			pae_ctx->pae_ram + PAE_RAM_BLOCK_DIV4_LENTH * cmd4,
			pae_ctx->pae_ram, &(pae_ctx->mod_param),
			pae_ctx->mod_word_len);
		return 0;
	}
	return 1;
}

uint16_t rsa_soft_adc_add(struct str_pae *pae_ctx,
		uint8_t cmd2, uint8_t cmd3, uint8_t cmd4)
{
	uint32_t buf0[BLOCK_LENGTH + 1];
	uint32_t buf1[BLOCK_LENGTH + 1];
	uint8_t i;
	uint8_t carry;
	uint16_t b_err;

	carry = 0;
	b_err = 0;
	carry = nn_add(pae_ctx->pae_ram + PAE_RAM_BLOCK_DIV16_LENTH * cmd2,
		pae_ctx->pae_ram + PAE_RAM_BLOCK_DIV16_LENTH * cmd3,
		pae_ctx->pae_ram + PAE_RAM_BLOCK_DIV16_LENTH * cmd4,
		pae_ctx->mod_word_len);
	if (carry == 1) {
		for (i = 0; i < pae_ctx->mod_word_len; i++) {
			buf0[i] = pae_ctx->pae_ram[i];
			buf1[i] = pae_ctx->pae_ram[i + PAE_RAM_BLOCK_DIV16_LENTH * cmd2];
		}
		buf0[pae_ctx->mod_word_len] = 0;
		buf1[pae_ctx->mod_word_len] = 1;
		carry = nn_sub(buf1, buf1, buf0, pae_ctx->mod_word_len + 1);
		if (buf1[pae_ctx->mod_word_len] == 1) {
			carry = nn_sub(buf1, buf1, buf0, pae_ctx->mod_word_len + 1);
			if (buf1[pae_ctx->mod_word_len] == 1)
				b_err = 1;
		}
		for (i = 0; i < pae_ctx->mod_word_len; i++)
			pae_ctx->pae_ram[i + PAE_RAM_BLOCK_DIV16_LENTH * cmd2] = buf1[i];
		return b_err;
	}
	return 0;
}

uint16_t rsa_soft_adc_sub(struct str_pae *pae_ctx,
		uint8_t cmd2, uint8_t cmd3, uint8_t cmd4)
{
	uint8_t carry;
	uint16_t b_err;

	carry = 0;
	b_err  = 0;
	carry = nn_sub(pae_ctx->pae_ram + PAE_RAM_BLOCK_DIV16_LENTH * cmd2,
		pae_ctx->pae_ram + PAE_RAM_BLOCK_DIV16_LENTH * cmd3,
		pae_ctx->pae_ram + PAE_RAM_BLOCK_DIV16_LENTH * cmd4,
		pae_ctx->mod_word_len);
	if (carry == 1) {
		carry = nn_add(pae_ctx->pae_ram + PAE_RAM_BLOCK_DIV16_LENTH * cmd2,
			pae_ctx->pae_ram + PAE_RAM_BLOCK_DIV16_LENTH * cmd2,
			pae_ctx->pae_ram, pae_ctx->mod_word_len);
		if (carry != 1) {
			carry = nn_add(pae_ctx->pae_ram + PAE_RAM_BLOCK_DIV16_LENTH * cmd2,
				pae_ctx->pae_ram + PAE_RAM_BLOCK_DIV16_LENTH * cmd2,
				pae_ctx->pae_ram, pae_ctx->mod_word_len);
			if (carry != 1)
				b_err = 1;
		}
		return b_err;
	}
	return 0;
}

uint16_t rsa_adccommand(struct str_pae *pae_ctx,
		uint8_t cmd1, uint8_t cmd2, uint8_t cmd3, uint8_t cmd4)
{
	if (cmd1 == RSA_ADD) {
		return rsa_soft_adc_add(pae_ctx, cmd2, cmd3, cmd4);
	} else if (cmd1 == RSA_SUB) {
		return rsa_soft_adc_sub(pae_ctx, cmd2, cmd3, cmd4);
	} else if (cmd1 == RSA_MULT) {
		nn_montgomery_modmult(
			pae_ctx->pae_ram + PAE_RAM_BLOCK_DIV16_LENTH * cmd2,
			pae_ctx->pae_ram + PAE_RAM_BLOCK_DIV16_LENTH * cmd3,
			pae_ctx->pae_ram + PAE_RAM_BLOCK_DIV16_LENTH * cmd4,
			pae_ctx->pae_ram, &(pae_ctx->mod_param),
			pae_ctx->mod_word_len);
		return 0;
	}
	return 1;
}

void ecc_point_dbl(struct str_pae *pae_ctx, uint8_t a_equ_inv3)
{
	rsa_adccommand(pae_ctx, RSA_MULT, BLOCK07, BLOCK02, BLOCK02);
	rsa_adccommand(pae_ctx, RSA_MULT, BLOCK08, BLOCK01, BLOCK07);
	rsa_adccommand(pae_ctx, RSA_ADD,  BLOCK08, BLOCK08, BLOCK08);
	rsa_adccommand(pae_ctx, RSA_ADD,  BLOCK08, BLOCK08, BLOCK08);
	rsa_adccommand(pae_ctx, RSA_MULT, BLOCK09, BLOCK07, BLOCK07);
	rsa_adccommand(pae_ctx, RSA_ADD,  BLOCK09, BLOCK09, BLOCK09);
	rsa_adccommand(pae_ctx, RSA_ADD,  BLOCK09, BLOCK09, BLOCK09);
	rsa_adccommand(pae_ctx, RSA_ADD,  BLOCK09, BLOCK09, BLOCK09);

	if (a_equ_inv3 == 0x00) {
		rsa_adccommand(pae_ctx, RSA_MULT, BLOCK07, BLOCK01, BLOCK01);
		rsa_adccommand(pae_ctx, RSA_ADD,  BLOCK0A, BLOCK07, BLOCK07);
		rsa_adccommand(pae_ctx, RSA_ADD,  BLOCK07, BLOCK07, BLOCK0A);
		rsa_adccommand(pae_ctx, RSA_MULT, BLOCK0A, BLOCK03, BLOCK03);
		rsa_adccommand(pae_ctx, RSA_MULT, BLOCK0A, BLOCK0A, BLOCK0A);
		rsa_adccommand(pae_ctx, RSA_MULT, BLOCK0A, BLOCK06, BLOCK0A);
		rsa_adccommand(pae_ctx, RSA_ADD,  BLOCK0A, BLOCK07, BLOCK0A);
	} else {
		rsa_adccommand(pae_ctx, RSA_MULT, BLOCK0A, BLOCK03, BLOCK03);
		rsa_adccommand(pae_ctx, RSA_ADD,  BLOCK07, BLOCK01, BLOCK0A);
		rsa_adccommand(pae_ctx, RSA_SUB,  BLOCK0A, BLOCK01, BLOCK0A);
		rsa_adccommand(pae_ctx, RSA_MULT, BLOCK0A, BLOCK07, BLOCK0A);
		rsa_adccommand(pae_ctx, RSA_ADD,  BLOCK07, BLOCK0A, BLOCK0A);
		rsa_adccommand(pae_ctx, RSA_ADD,  BLOCK0A, BLOCK07, BLOCK0A);
	}

	rsa_adccommand(pae_ctx, RSA_MULT, BLOCK07, BLOCK0A, BLOCK0A);
	rsa_adccommand(pae_ctx, RSA_SUB,  BLOCK07, BLOCK07, BLOCK08);
	rsa_adccommand(pae_ctx, RSA_SUB,  BLOCK01, BLOCK07, BLOCK08);
	rsa_adccommand(pae_ctx, RSA_SUB,  BLOCK07, BLOCK08, BLOCK01);
	rsa_adccommand(pae_ctx, RSA_MULT, BLOCK07, BLOCK07, BLOCK0A);
	rsa_adccommand(pae_ctx, RSA_ADD,  BLOCK08, BLOCK02, BLOCK0E);
	rsa_adccommand(pae_ctx, RSA_SUB,  BLOCK02, BLOCK07, BLOCK09);
	rsa_adccommand(pae_ctx, RSA_MULT, BLOCK08, BLOCK08, BLOCK03);
	rsa_adccommand(pae_ctx, RSA_ADD,  BLOCK03, BLOCK08, BLOCK08);
}

void ecc_point_mix_add(struct str_pae *pae_ctx)
{
	rsa_adccommand(pae_ctx, RSA_MULT, BLOCK0A, BLOCK03, BLOCK03);
	rsa_adccommand(pae_ctx, RSA_MULT, BLOCK07, BLOCK04, BLOCK0A);
	rsa_adccommand(pae_ctx, RSA_SUB,  BLOCK09, BLOCK07, BLOCK01);
	rsa_adccommand(pae_ctx, RSA_MULT, BLOCK08, BLOCK03, BLOCK0A);
	rsa_adccommand(pae_ctx, RSA_MULT, BLOCK08, BLOCK05, BLOCK08);
	rsa_adccommand(pae_ctx, RSA_SUB,  BLOCK07, BLOCK08, BLOCK02);
	rsa_adccommand(pae_ctx, RSA_MULT, BLOCK0A, BLOCK09, BLOCK09);
	rsa_adccommand(pae_ctx, RSA_MULT, BLOCK0B, BLOCK0A, BLOCK09);
	rsa_adccommand(pae_ctx, RSA_MULT, BLOCK0A, BLOCK01, BLOCK0A);
	rsa_adccommand(pae_ctx, RSA_ADD,  BLOCK0C, BLOCK0A, BLOCK0A);
	rsa_adccommand(pae_ctx, RSA_ADD,  BLOCK0C, BLOCK0B, BLOCK0C);
	rsa_adccommand(pae_ctx, RSA_MULT, BLOCK08, BLOCK07, BLOCK07);
	rsa_adccommand(pae_ctx, RSA_SUB,  BLOCK01, BLOCK08, BLOCK0C);
	rsa_adccommand(pae_ctx, RSA_SUB,  BLOCK08, BLOCK0A, BLOCK01);
	rsa_adccommand(pae_ctx, RSA_MULT, BLOCK08, BLOCK08, BLOCK07);
	rsa_adccommand(pae_ctx, RSA_MULT, BLOCK0B, BLOCK02, BLOCK0B);
	rsa_adccommand(pae_ctx, RSA_SUB,  BLOCK02, BLOCK08, BLOCK0B);
	rsa_adccommand(pae_ctx, RSA_MULT, BLOCK03, BLOCK03, BLOCK09);
}

#endif /* END _FL_NN_H_ */
