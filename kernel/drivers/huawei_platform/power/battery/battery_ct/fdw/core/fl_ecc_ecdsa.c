/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * fl_ecc_ecdsa.c
 *
 * ecc ecdsa
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
#include "fl_define.h"
#include "fl_rae_struct.h"
#include "fl_public.h"
#include "fl_ecc.h"

/*
 * 函数名称: fm_nn_rshift
 * 函数功能: 大整数右移函数,计算 a = b div 2^s
 * 输入参数:
 *          uint8_t * a,输出大整数的数据指针
 *          uint8_t * b,输入大整数的数据指针
 *          uint32_t lenbyword,,大整数的字长
 *          uint32_t s,需要移位的次数
 */
void fm_nn_rshift(uint8_t *a, uint8_t *b, uint32_t lenbyword, uint32_t s)
{
	uint32_t i;
	uint32_t shiftbyte;
	uint32_t shiftbyte2;
	uint8_t  bt_temp;
	uint8_t  bt_carry;
	uint8_t  n_shiftbits;

	shiftbyte = s / 8;
	n_shiftbits = s % 8;

	if (shiftbyte > 0)
		for (i = 0; i < shiftbyte; i++)
			a[i] = 0;
	if (n_shiftbits > 0) {
		bt_carry = 0;
		shiftbyte2 = lenbyword * 4 - shiftbyte;
		if (shiftbyte2 > 0) {
			for (i = 0; i < shiftbyte2; i++) {
				bt_temp = b[i];
				a[shiftbyte + i] = (bt_temp >> n_shiftbits) | bt_carry;
				bt_carry = bt_temp << (8 - n_shiftbits);
			}
		}
	} else {
		shiftbyte2 = lenbyword * 4 - shiftbyte;
		if (shiftbyte2 > 0) {
			for (i = 0; i < shiftbyte2; i++) {
				bt_temp = b[i];
				a[shiftbyte + i] = bt_temp;
			}
		}
	}
}

void ecc_hashformat(uint32_t *out_buf, uint8_t *hash_inbuf,
		uint32_t lenbyword)
{
	dma_xdatabyte2xdataword(hash_inbuf, out_buf, lenbyword);
}

void ecc_calculate_u1u2(struct str_ecc_key *key,
		uint8_t *hash_buf, uint8_t *sig, uint32_t *ecc_x3_buf,
		uint8_t *ecc_u1_bytebuf, uint8_t *ecc_u2_bytebuf,
		uint32_t lenbyword)
{
	uint32_t ecc_x1_buf[ECCMAXWORDLEN];
	uint32_t ecc_rr_n[ECCMAXWORDLEN];
	uint32_t ecc_e[ECCMAXWORDLEN];

	ecc_hashformat(ecc_e, hash_buf, lenbyword);

	/* 计算w=s^(-1)(mod n) */
	bignum_modinv_soft(key->pae_ctx, ecc_x1_buf, ecc_x3_buf,
		key->param->n, lenbyword);
	rsa_calculate_rr_n(key->pae_ctx, key->param->n, key->param->bitlen);
	dma_rsa2xdata(key->pae_ctx, PAE_RAM_BLOCK2, ecc_rr_n, lenbyword);

	/* 计算u1=ew mod n */
	dma_xdata2rsa(key->pae_ctx, ecc_e, PAE_RAM_BLOCK1, lenbyword);
	rsa_command(key->pae_ctx, RSA_MULT, BLOCK01, BLOCK01, BLOCK02);
	dma_xdata2rsa(key->pae_ctx, ecc_x1_buf, PAE_RAM_BLOCK2, lenbyword);
	rsa_command(key->pae_ctx, RSA_MULT, BLOCK01, BLOCK01, BLOCK02);
	rsa_command(key->pae_ctx, RSA_SUB,  BLOCK01, BLOCK01, BLOCK00);
	dma_rsa2xdata(key->pae_ctx, PAE_RAM_BLOCK1, ecc_x3_buf, lenbyword);
	dma_xdataword2xdatabyte(ecc_x3_buf, ecc_u1_bytebuf, lenbyword);

	/* 计算u2=rw mod n */
	dma_xdata_byte2rsa(key->pae_ctx, sig, PAE_RAM_BLOCK1, lenbyword);
	dma_xdata2rsa(key->pae_ctx, ecc_rr_n, PAE_RAM_BLOCK2, lenbyword);
	rsa_command(key->pae_ctx, RSA_MULT, BLOCK01, BLOCK01, BLOCK02);
	dma_xdata2rsa(key->pae_ctx, ecc_x1_buf, PAE_RAM_BLOCK2, lenbyword);
	rsa_command(key->pae_ctx, RSA_MULT, BLOCK01, BLOCK01, BLOCK02);
	rsa_command(key->pae_ctx, RSA_SUB,  BLOCK01, BLOCK01, BLOCK00);
	dma_rsa2xdata(key->pae_ctx, PAE_RAM_BLOCK1, ecc_x1_buf, lenbyword);
	dma_xdataword2xdatabyte(ecc_x1_buf, ecc_u2_bytebuf, lenbyword);
}

/*
 *  函数名称: ecdsa_verifyhash
 *  函数功能: 数字签名验证
 *  输入参数:
 *              struct str_ecc_key *key,ECC密钥数据结构指针
 *              uint8_t *hash_buf,Message的摘要指针
 *              uint8_t *sig,数字签名数据结构指针(r||s)
 *  返回值:
 *              'RESULT_OK   ' - 签名验证通过
 *              'RESULT_ERROR' - 签名验证不通过
 */
uint8_t ecdsa_verifyhash(struct str_ecc_key *key,
		uint8_t *hash_buf, uint8_t *sig)
{
	uint32_t ecc_x1_buf[ECCMAXWORDLEN];
	uint32_t ecc_x3_buf[ECCMAXWORDLEN];
	uint8_t  ecc_u1_bytebuf[ECCMAXLEN];
	uint8_t  ecc_u2_bytebuf[ECCMAXLEN];
	struct str_affpoint point1;
	struct str_affpoint point2;
	uint32_t lenbyword;

	lenbyword = key->param->bitlen / WORD_BITS;

	/* Check r,s */
	dma_xdatabyte2xdataword(sig, ecc_x1_buf, lenbyword);
	if (check_klown(ecc_x1_buf, key->param->n, lenbyword) == 0x00)
		return RESULT_ERROR;
	dma_xdatabyte2xdataword(sig + lenbyword * 4, ecc_x3_buf, lenbyword);
	if (check_klown(ecc_x3_buf, key->param->n, lenbyword) == 0x00)
		return RESULT_ERROR;

	/* calculate u1,u2 */
	ecc_calculate_u1u2(key, hash_buf, sig, ecc_x3_buf,
		ecc_u1_bytebuf, ecc_u2_bytebuf, lenbyword);

	/* u1G */
	if (ecc_pointmult_comb(key, key->param->ptr_comb_table_g,
		ecc_u1_bytebuf, &point1) != RESULT_OK)
		return RESULT_ERROR;

	/* u2Q */
	if (ecc_pointmult_comb(key, key->comb_table_q,
		ecc_u2_bytebuf, &point2) != RESULT_OK)
		return RESULT_ERROR;

	/* u1G + u2Q */
	if (ecc_pointadd(key, &point1, &point2, &point1) != RESULT_OK)
		return RESULT_ERROR;

	dma_xdata2rsa(key->pae_ctx, point1.aff_xy, PAE_RAM_BLOCK1, lenbyword);

	rsa_write_len(key->pae_ctx, lenbyword);
	dma_xdata2rsa(key->pae_ctx, key->param->n, 0x00, lenbyword);
	rsa_command(key->pae_ctx, RSA_SUB, BLOCK01, BLOCK01, BLOCK00);

	dma_rsa2xdata(key->pae_ctx, PAE_RAM_BLOCK1, ecc_x1_buf, lenbyword);
	dma_xdatabyte2xdataword(sig, ecc_x3_buf, lenbyword);
	if (cmp_bn(ecc_x1_buf, ecc_x3_buf, lenbyword) == 0x00)
		return RESULT_OK;

	return RESULT_ERROR;
}
