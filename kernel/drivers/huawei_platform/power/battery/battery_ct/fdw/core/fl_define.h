/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * fl_define.h
 *
 * fl define
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

#ifndef _FL_DEFINE_H_
#define _FL_DEFINE_H_

#ifndef BIT0
#define BIT0            (((uint32_t)1) << 0x00)
#define BIT1            (((uint32_t)1) << 0x01)
#define BIT2            (((uint32_t)1) << 0x02)
#define BIT3            (((uint32_t)1) << 0x03)
#define BIT4            (((uint32_t)1) << 0x04)
#define BIT5            (((uint32_t)1) << 0x05)
#define BIT6            (((uint32_t)1) << 0x06)
#define BIT7            (((uint32_t)1) << 0x07)
#define BIT8            (((uint32_t)1) << 0x08)
#define BIT9            (((uint32_t)1) << 0x09)
#define BIT10           (((uint32_t)1) << 0x0A)
#define BIT11           (((uint32_t)1) << 0x0B)
#define BIT12           (((uint32_t)1) << 0x0C)
#define BIT13           (((uint32_t)1) << 0x0D)
#define BIT14           (((uint32_t)1) << 0x0E)
#define BIT15           (((uint32_t)1) << 0x0F)
#define BIT16           (((uint32_t)1) << 0x10)
#define BIT17           (((uint32_t)1) << 0x11)
#define BIT18           (((uint32_t)1) << 0x12)
#define BIT19           (((uint32_t)1) << 0x13)
#define BIT20           (((uint32_t)1) << 0x14)
#define BIT21           (((uint32_t)1) << 0x15)
#define BIT22           (((uint32_t)1) << 0x16)
#define BIT23           (((uint32_t)1) << 0x17)
#define BIT24           (((uint32_t)1) << 0x18)
#define BIT25           (((uint32_t)1) << 0x19)
#define BIT26           (((uint32_t)1) << 0x1A)
#define BIT27           (((uint32_t)1) << 0x1B)
#define BIT28           (((uint32_t)1) << 0x1C)
#define BIT29           (((uint32_t)1) << 0x1D)
#define BIT30           (((uint32_t)1) << 0x1E)
#define BIT31           (((uint32_t)1) << 0x1F)
#endif  /* END BIT0~31  */

#define WORD_BITS           32
#define HALF_WORD_BITS      16
#define WORD_MASK_VALUE     0xFFFFFFFFU
#define ECC_RR_N_MAX        4

#define RESULT_ERROR            0x45
#define RESULT_OK               0xBA
#define RESULT_INFINITY         0x8C

#define HASH_WORD_LEN_MAX       (0x08)
#define HASH_BYTE_LEN_MAX       (HASH_WORD_LEN_MAX * 4)

#define PAE_LEVEL0_F            (0x5A)
#define PAE_LEVEL1_M            (0x2D)

#define ECC_WORD_LEN_MAX        (0x10)
#define ECC_BYTE_LEN_MAX        (ECC_WORD_LEN_MAX * 4)

#define RSA_WORD_LEN_MAX        (0x40)
#define RSA_HALF_WORD_LEN_MAX   (0x20)
#define RSA_BYTE_LEN_MAX        (RSA_WORD_LEN_MAX * 4)

#define ECCMAXLEN               0x20
#define ECCMAXWORDLEN           0x08
#define ECC_AFF_POINT_WORDLEN   0x10
#define ECC_JAC_POINT_WORDLEN   0x18

#define RSA_MULT                0x00
#define RSA_ADD                 0x01
#define RSA_SUB                 0x03

#define PAE_RAM_BLOCK_DIV4_LENTH  0x40
#define PAE_RAM_BLOCK_DIV16_LENTH 0x10

#define BLOCK0                  (0x00)
#define BLOCK1                  (0x01)
#define BLOCK2                  (0x02)
#define BLOCK3                  (0x03)

#define BLOCK00                 (0x00)
#define BLOCK01                 (0x01)
#define BLOCK02                 (0x02)
#define BLOCK03                 (0x03)
#define BLOCK04                 (0x04)
#define BLOCK05                 (0x05)
#define BLOCK06                 (0x06)
#define BLOCK07                 (0x07)
#define BLOCK08                 (0x08)
#define BLOCK09                 (0x09)
#define BLOCK0A                 (0x0A)
#define BLOCK0B                 (0x0B)
#define BLOCK0C                 (0x0C)
#define BLOCK0D                 (0x0D)
#define BLOCK0E                 (0x0E)
#define BLOCK0F                 (0x0F)

#define PAE_RAM_BLOCK0          (0x00 * PAE_RAM_BLOCK_DIV4_LENTH)
#define PAE_RAM_BLOCK1          (0x01 * PAE_RAM_BLOCK_DIV4_LENTH)
#define PAE_RAM_BLOCK2          (0x02 * PAE_RAM_BLOCK_DIV4_LENTH)
#define PAE_RAM_BLOCK3          (0x03 * PAE_RAM_BLOCK_DIV4_LENTH)

#define PAE_RAM_BLOCK00         (0x00 * PAE_RAM_BLOCK_DIV16_LENTH)
#define PAE_RAM_BLOCK01         (0x01 * PAE_RAM_BLOCK_DIV16_LENTH)
#define PAE_RAM_BLOCK02         (0x02 * PAE_RAM_BLOCK_DIV16_LENTH)
#define PAE_RAM_BLOCK03         (0x03 * PAE_RAM_BLOCK_DIV16_LENTH)
#define PAE_RAM_BLOCK04         (0x04 * PAE_RAM_BLOCK_DIV16_LENTH)
#define PAE_RAM_BLOCK05         (0x05 * PAE_RAM_BLOCK_DIV16_LENTH)
#define PAE_RAM_BLOCK06         (0x06 * PAE_RAM_BLOCK_DIV16_LENTH)
#define PAE_RAM_BLOCK07         (0x07 * PAE_RAM_BLOCK_DIV16_LENTH)
#define PAE_RAM_BLOCK08         (0x08 * PAE_RAM_BLOCK_DIV16_LENTH)
#define PAE_RAM_BLOCK09         (0x09 * PAE_RAM_BLOCK_DIV16_LENTH)
#define PAE_RAM_BLOCK0A         (0x0A * PAE_RAM_BLOCK_DIV16_LENTH)
#define PAE_RAM_BLOCK0B         (0x0B * PAE_RAM_BLOCK_DIV16_LENTH)
#define PAE_RAM_BLOCK0C         (0x0C * PAE_RAM_BLOCK_DIV16_LENTH)
#define PAE_RAM_BLOCK0D         (0x0D * PAE_RAM_BLOCK_DIV16_LENTH)
#define PAE_RAM_BLOCK0E         (0x0E * PAE_RAM_BLOCK_DIV16_LENTH)
#define PAE_RAM_BLOCK0F         (0x0F * PAE_RAM_BLOCK_DIV16_LENTH)

#endif /* END _FL_DEFINE_H_ */
