/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * algorithm.h
 *
 * algorithm.c head file
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

#ifndef TMC_ALGORITHM_H
#define TMC_ALGORITHM_H

#include <uapi/linux/sched/types.h>
#include <linux/ctype.h>

#define VERIFY_VALID      1
#define VERIFY_INVALID    0

/* The ECC algorithm can support. */
#define SECP192R1         24
#define SECP256R1         32

#ifndef ECC_CURVE
#define ECC_CURVE         SECP256R1
#endif

#if (ECC_CURVE != secp128r1 && ECC_CURVE != SECP192R1 && ECC_CURVE != SECP256R1 && ECC_CURVE != secp384r1)
#error "Must define ECC_CURVE to one of the available curves"
#endif

#define ECC_BYTES         ECC_CURVE
#define ECC_VERIFY_BYTES  (ECC_BYTES << 1)

void tmc_sha256update(unsigned char* pucOut, unsigned char* pucIn, unsigned char* pucLastBlk, unsigned char ucInit);
unsigned short tmc_sha256final(unsigned char* pucOut, unsigned char* pucIn, unsigned char* pucLastBlk,
	unsigned short usBlkDone, unsigned short usCLen);
int tmc_ecc_verify(const unsigned char p_publicKey[ECC_VERIFY_BYTES], const unsigned char p_hash[ECC_BYTES],
	const unsigned char p_signature[ECC_VERIFY_BYTES]);
#endif

