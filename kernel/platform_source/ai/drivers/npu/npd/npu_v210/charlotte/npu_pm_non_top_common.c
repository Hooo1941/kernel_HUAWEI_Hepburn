/*
 * npu_pm_non_top_common.c
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

#include "npu_pm_non_top_common.h"
#include "npu_atf_subsys.h"
#include "npu_pm_internal.h"
#include "npu_log.h"

int npu_non_top_common_powerup(u32 work_mode, u32 subip_set)
{
	int ret;
	unused(subip_set);

	npu_drv_warn("non top comm power up start\n");

	ret = atfd_service_npu_smc(NPU_POWER_UP_NON_TOP_COMMON, 0, work_mode, 0);
	if (ret)
		npu_drv_err("non top common power up failed, ret:%d", ret);

	npu_drv_warn("non top comm power up succ\n");
	return ret;
}

int npu_non_top_common_powerdown(u32 work_mode, u32 subip_set)
{
	int ret;
	unused(subip_set);

	npu_drv_warn("non top comm power down start\n");

	ret = atfd_service_npu_smc(NPU_POWER_DOWN_NON_TOP_COMMON, 0, work_mode, 0);
	if (ret)
		npu_drv_err("non top common power down failed, ret:%d", ret);

	npu_drv_warn("non top comm power down succ\n");
	return ret;
}

