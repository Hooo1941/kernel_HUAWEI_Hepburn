
/*
 * smc_abi.c
 *
 * functions for legacy tzdriver smc transport
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

#include <linux/arm-smccc.h>
#include "smc_call.h"
#include "teek_ns_client.h"

#ifndef CONFIG_ARCH32
void do_smc_transport(struct smc_in_params *in, struct smc_out_params *out, uint8_t wait)
{
	isb();
	wmb();
	do {
		asm volatile(
			"mov x0, %[fid]\n"
			"mov x1, %[a1]\n"
			"mov x2, %[a2]\n"
			"mov x3, %[a3]\n"
			"mov x4, %[a4]\n"
			"mov x5, %[a5]\n"
			"mov x6, %[a6]\n"
			"mov x7, %[a7]\n"
			SMCCC_SMC_INST"\n"
			"str x0, [%[re0]]\n"
			"str x1, [%[re1]]\n"
			"str x2, [%[re2]]\n"
			"str x3, [%[re3]]\n" :
			[fid] "+r"(in->x0),
			[a1] "+r"(in->x1),
			[a2] "+r"(in->x2),
			[a3] "+r"(in->x3),
			[a4] "+r"(in->x4),
			[a5] "+r"(in->x5),
			[a6] "+r"(in->x6),
			[a7] "+r"(in->x7):
			[re0] "r"(&out->ret),
			[re1] "r"(&out->exit_reason),
			[re2] "r"(&out->ta),
			[re3] "r"(&out->target) :
			"x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7");
	} while (out->ret == TSP_REQUEST && wait != 0);
	isb();
	wmb();
}
#else
void do_smc_transport(struct smc_in_params *in, struct smc_out_params *out, uint8_t wait)
{
	isb();
	wmb();
	do {
		asm volatile(
			"mov r0, %[fid]\n"
			"mov r1, %[a1]\n"
			"mov r2, %[a2]\n"
			"mov r3, %[a3]\n"
			".arch_extension sec\n"
			SMCCC_SMC_INST"\n"
			"str r0, [%[re0]]\n"
			"str r1, [%[re1]]\n"
			"str r2, [%[re2]]\n"
			"str r3, [%[re3]]\n" :
			[fid] "+r"(in->x0),
			[a1] "+r"(in->x1),
			[a2] "+r"(in->x2),
			[a3] "+r"(in->x3):
			[re0] "r"(&out->ret),
			[re1] "r"(&out->exit_reason),
			[re2] "r"(&out->ta),
			[re3] "r"(&out->target) :
			"r0", "r1", "r2", "r3");
	} while (out->ret == TSP_REQUEST && wait != 0);
	isb();
	wmb();
}
#endif

void smc_req(struct smc_in_params *in, struct smc_out_params *out, uint8_t wait)
{
	do_smc_transport(in, out, wait);
}