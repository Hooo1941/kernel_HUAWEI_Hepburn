// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2021 ARM Ltd.
 */

#include <linux/printk.h>

#include "common.h"

static void __arm_ffa_fn_smc(ffa_value_t args, ffa_value_t *res)
{
	arm_smccc_1_2_smc(&args, res);
}

static void __arm_ffa_fn_hvc(ffa_value_t args, ffa_value_t *res)
{
	arm_smccc_1_2_hvc(&args, res);
}

int __init ffa_transport_init(ffa_fn **invoke_ffa_fn)
{
	*invoke_ffa_fn = __arm_ffa_fn_smc;

	return 0;
}
