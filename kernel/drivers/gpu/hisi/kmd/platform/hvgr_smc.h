/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

#ifndef HVGR_SMC_H
#define HVGR_SMC_H

#include <linux/types.h>
#include "hvgr_defs.h"

enum hvgr_smc_ops {
	HVGR_SMC_SEC_CONFIG,
	HVGR_SMC_CRG_RESET,
	HVGR_SMC_OPS_MAX
};

struct hvgr_smc_para {
	enum hvgr_smc_ops op;
	unsigned long a1;
	unsigned long a2;
	unsigned long a3;
	unsigned long a4;
	unsigned long a5;
	unsigned long a6;
	unsigned long a7;
};

uint64_t hvgr_smc_call(struct hvgr_device *gdev, struct hvgr_smc_para *para);

#endif /* HVGR_SMC_H */
