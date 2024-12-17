/*
 * npu_pm_top_common.c
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

#include "npu_pm_top_common.h"
#include <platform_include/cee/linux/npu_pm.h>
#include "npu_log.h"
#include "npu_except_mgr.h"

int npu_top_common_powerup(u32 work_mode, u32 subip_set)
{
	int ret = 0;
	int32_t except_bits;
	unused(work_mode);
	unused(subip_set);

	npu_drv_warn("top common power up start\n");
	npu_drv_boot_time_tag("start top common power up\n");

	except_bits = npu_get_exception_bits();
	if (except_bits != 0) {
		npu_drv_err("get exception bits failed, ret:%d", except_bits);
		return -NPU_EXCEPTION_GLOBAL;
	}

#ifndef NPU_POWER_UP_SELF_CONTROL
	npu_drv_warn("call npu_pm_power_on\n");
	ret = npu_pm_power_on();
	if (ret)
		npu_drv_err("top common power up failed, ret:%d", ret);
#endif

	npu_drv_warn("top common power up succ\n");
	return ret;
}

int npu_top_common_powerdown(u32 work_mode, u32 subip_set)
{
	int ret = 0;
	unused(work_mode);
	unused(subip_set);

	npu_drv_warn("top common power down start\n");

#ifndef NPU_POWER_UP_SELF_CONTROL
	ret = npu_pm_power_off();
	if (ret) {
		npu_drv_err("top common power down failed, ret:%d", ret);
		return ret;
	}
#endif

	npu_clear_exception_bits();

	npu_drv_warn("top common power down succ\n");
	return ret;
}
