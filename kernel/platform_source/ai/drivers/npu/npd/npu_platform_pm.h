/*
 * npu_platform_pm.h
 *
 * about npu hwts plat
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
#ifndef __NPU_PLATFORM_PM_H
#define __NPU_PLATFORM_PM_H
#include "npu_common.h"

int npu_pm_init(struct npu_dev_ctx *dev_ctx);
int npu_plat_switch_hwts_aicore_pool(struct npu_dev_ctx *dev_ctx,
	npu_work_mode_info_t *work_mode_info, uint32_t power_status);
int npu_plat_dev_pm_suspend(void);

#ifdef NPU_ARCH_V100

int npu_plat_pm_powerup(struct npu_dev_ctx *dev_ctx, u32 work_mode);
int npu_plat_pm_powerdown(struct npu_dev_ctx *dev_ctx, u32 work_mode);

#else

#include "npu_platform_pm_spec.h"
#define NPU_PM_RET_TINY_ON      1

#define NPU_SUBIP_ALL \
	(~((0xFFFFFFFFU >> (unsigned int)(NPU_SUBIP_MAX)) << (unsigned int)(NPU_SUBIP_MAX)))

static inline int npu_plat_pm_powerup(struct npu_dev_ctx *dev_ctx, u32 work_mode)
{
	unused(dev_ctx);
	unused(work_mode);
	return 0;
}

static inline int npu_plat_pm_powerdown(struct npu_dev_ctx *dev_ctx, u32 work_mode)
{
	unused(dev_ctx);
	unused(work_mode);
	return 0;
}

#endif

#endif
