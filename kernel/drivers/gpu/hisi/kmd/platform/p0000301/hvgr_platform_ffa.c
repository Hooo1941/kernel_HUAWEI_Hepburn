/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

#include "hvgr_platform_ffa.h"

#include "hvgr_ffa.h"

uint64_t hvgr_platform_sh_runtime(struct hvgr_device *gdev, unsigned char *buf, unsigned long size)
{
	uint64_t ret;

	ret = hvgr_ffa_call_with_shmem(gdev, HVGR_FFA_SH_RUNTIME, buf, size);
	if (ret != 0)
		dev_err(gdev->dev, "send sh runtime fail ret %lu", ret);

	return ret;
}

