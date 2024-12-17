/*
 * smc_call.h
 *
 * declarations of unified API for tzdriver
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
 */

#ifndef SMC_CALL_H
#define SMC_CALL_H

#include <linux/types.h>

struct smc_in_params {
	unsigned long x0;
	unsigned long x1;
	unsigned long x2;
	unsigned long x3;
	unsigned long x4;
	unsigned long x5;
	unsigned long x6;
	unsigned long x7;
};

struct smc_out_params {
	unsigned long ret;
	unsigned long exit_reason;
	unsigned long ta;
	unsigned long target;
};

void smc_req(struct smc_in_params *in, struct smc_out_params *out, uint8_t wait);

#endif