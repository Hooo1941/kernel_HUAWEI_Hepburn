/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

#include "hvgr_smc.h"

#include <linux/version.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
#include <linux/arm-smccc.h>

#include "platform_include/see/bl31_smc.h"

static unsigned long hvgr_smc_ops_list[HVGR_SMC_OPS_MAX] = {
	FID_BL31_GPU_SEC_CONFIG,
	FID_BL31_GPU_CRG_RESET
};

uint64_t hvgr_smc_call(struct hvgr_device *gdev, struct hvgr_smc_para *para)
{
	struct arm_smccc_res res;

	if (para == NULL || para->op >= HVGR_SMC_OPS_MAX) {
		dev_err(gdev->dev, "smc input is invalid.");
		return (uint64_t)-EINVAL;
	}

	arm_smccc_smc(hvgr_smc_ops_list[para->op], para->a1, para->a2, para->a3, para->a4, para->a5,
		para->a6, para->a7, &res);
	return (uint64_t)res.a0;
}
#else
uint64_t hvgr_smc_call(struct hvgr_device *gdev, struct hvgr_smc_para *para)
{
	dev_err(gdev->dev, "linux version is less than 5.10, not support arm_smccc_smc!");
	return (uint64_t)-EINVAL;
}
#endif