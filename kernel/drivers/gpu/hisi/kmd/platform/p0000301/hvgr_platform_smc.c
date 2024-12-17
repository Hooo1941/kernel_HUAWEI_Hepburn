/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2021.
 * All rights reserved.
 *
 * Description: This file is for the gpu smc process.
 * Author:
 * Create: 2021-02-03
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_smc.h"
#include "hvgr_platform_api.h"
#include <linux/delay.h>


uint64_t hvgr_platform_protected_cfg(struct hvgr_device *gdev, u32 mode)
{
	u64 ret;
	struct hvgr_smc_para para = {0};

	para.op = HVGR_SMC_SEC_CONFIG;
	para.a1 = mode;
	ret = hvgr_smc_call(gdev, &para);
	if (ret != 0)
		dev_err(gdev->dev, "sec config failed");

	return ret;
}

int hvgr_platform_smmu_tcu_on(struct hvgr_device *gdev)
{
	return 0;
}

int hvgr_platform_smmu_tcu_off(struct hvgr_device *gdev)
{
	return 0;
}

uint64_t hvgr_platform_smmu_set_sid(struct hvgr_device *gdev)
{
	return 0;
}

uint64_t hvgr_platform_smmu_tbu_connect(struct hvgr_device *gdev)
{
	return 0;
}

uint64_t hvgr_platform_smmu_tbu_disconnect(struct hvgr_device *gdev)
{
	return 0;
}

uint64_t hvgr_platform_smmu_bypass(struct hvgr_device *gdev, u32 partition)
{
	return 0;
}

uint64_t hvgr_platform_sec_cfg(struct hvgr_device *gdev, u32 partition)
{
	return 0;
}

uint64_t hvgr_platform_set_mode(struct hvgr_device *gdev)
{
	return 0;
}

uint64_t hvgr_platform_set_qos(struct hvgr_device *gdev, u32 partition)
{
	return 0;
}

