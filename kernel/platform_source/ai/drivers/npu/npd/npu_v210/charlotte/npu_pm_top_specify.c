/*
 * npu_pm_top_specify.c
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

#include "npu_pm_top_specify.h"
#include <linux/iommu/mm_svm.h>
#include "npu_atf_subsys.h"
#include "npu_pm_internal.h"
#include "npu_log.h"
#include "npu_platform.h"

int npu_top_specify_powerup(u32 work_mode, u32 subip_set)
{
	int ret;
	struct npu_platform_info *plat_info = NULL;
	unused(subip_set);

	npu_drv_warn("top spec power up start\n");

	ret = atfd_service_npu_smc(NPU_POWER_UP_TOP_SPECIFY, 0, work_mode, 0);
	if (ret != 0) {
		npu_drv_err("top specify power up failed, ret:%d", ret);
		return ret;
	}

	plat_info = npu_plat_get_info();
	cond_return_error(plat_info == NULL, -1, "get platform info failed\n");
	npu_drv_warn("mm_smmu_poweron start\n");
	ret = mm_smmu_poweron(plat_info->pdev);
	if (ret != 0) {
		npu_drv_err("smmu poweron failed\n");
		return ret;
	}

	npu_drv_warn("top spec power up succ\n");
	return ret;
}

int npu_top_sepcify_powerdown(u32 work_mode, u32 subip_set)
{
	int ret;
	struct npu_platform_info *plat_info = NULL;
	unused(subip_set);

	npu_drv_warn("top spec power down start\n");

	plat_info = npu_plat_get_info();
	cond_return_error(plat_info == NULL, -1, "get platform info failed\n");
	(void)mm_smmu_poweroff(plat_info->pdev);

	ret = atfd_service_npu_smc(NPU_POWER_DOWN_TOP_SPECIFY, 0, work_mode, 0);
	if (ret != 0)
		npu_drv_err("top specify power down failed, ret:%d", ret);

	npu_drv_warn("top spec power down succ\n");
	return ret;
}
