/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * fl_public.h
 *
 * fl_public.c head file
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

#ifndef _FL_PUBLIC_H_
#define _FL_PUBLIC_H_

uint16_t rsa_command(struct str_pae *pae_ctx,
	uint8_t cmd1, uint8_t cmd2, uint8_t cmd3, uint8_t cmd4);
uint16_t rsa_adccommand(struct str_pae *pae_ctx,
	uint8_t cmd1, uint8_t cmd2, uint8_t cmd3, uint8_t cmd4);

void rsa_write_len(struct str_pae *pae_ctx, uint32_t n_word_len);
void rsa_write_ram(struct str_pae *pae_ctx,
	uint32_t ram_addr, uint32_t n_value);

uint16_t  nn_cmpbignumber_word(uint32_t *a, uint32_t *b, uint16_t words);
void dma_wr_allblock(struct str_pae *pae_ctx, uint32_t n_value,
	uint32_t ptr_rsaram, uint32_t nlenbyword);
void dma_wr_allxdata(uint32_t bt_value, uint32_t *ptr_xram, uint32_t len);
void dma_xdata2xdata(uint32_t *ptr_xram_src, uint32_t *ptr_xram_dest,
	uint32_t n_lenbyword);
void dma_xdatabyte2xdataword(uint8_t *ptr_xram_src,
	uint32_t *ptr_xram_dest, uint32_t nlenbyword);
void dma_xdataword2xdatabyte(uint32_t *ptr_xram_src,
	uint8_t *ptr_xram_dest, uint32_t nlenbyword);
void dma_xdata2rsa(struct str_pae *pae_ctx,
	uint32_t *ptr_xram, uint32_t rsa_ramoffset, uint32_t nlenbyword);
void dma_xdata_byte2rsa(struct str_pae *pae_ctx, uint8_t *ptr_xram,
	uint32_t rsa_ramoffset, uint32_t nlenbyword);
void dma_rsa2xdata(struct str_pae *pae_ctx, uint32_t rsa_ram_offset,
	uint32_t *ptr_xram, uint32_t nlenbyword);

uint16_t cmp_bn(uint32_t *a, uint32_t *b, uint32_t lenbyword);
uint16_t check_allzero(uint32_t *buf, uint32_t lenbyword);
uint16_t check_klown(uint32_t *k_buf, uint32_t *n_buf, uint32_t lenbyword);
uint32_t rsa_getmodparam(uint32_t n0);

uint8_t  bignum_modinv_soft(struct str_pae *pae_ctx,
	uint32_t *z, uint32_t *x, uint32_t *m, uint32_t n_wordlen);
void rsa_calculate_rr_n(struct str_pae *pae_ctx,
	uint32_t *ptr_m, uint32_t n_lenbybit);

void ecc_point_dbl(struct str_pae *pae_ctx, uint8_t a_equ_inv3);
void ecc_point_mix_add(struct str_pae *pae_ctx);

void get_u32_be(uint32_t *pt_n, uint8_t *b, uint32_t offset);
void put_u32_be(uint32_t n, uint8_t *b, uint32_t offset);

#endif /* END _FL_PUBLIC_H_ */
