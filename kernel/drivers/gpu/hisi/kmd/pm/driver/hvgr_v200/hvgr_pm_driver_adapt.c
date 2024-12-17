/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 * This file is the dvfs implementation of the PM module.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include "hvgr_dm_api.h"
#include "hvgr_log_api.h"
#include "hvgr_pm_api.h"
#include "hvgr_regmap.h"

#include "hvgr_pm_defs.h"
#include "hvgr_pm_dfx.h"

#include "hvgr_pm_driver.h"
#include "hvgr_pm_driver_base.h"
#include "hvgr_pm_driver_adapt_defs.h"


bool hvgr_pm_driver_unreset_cores(struct hvgr_device *gdev)
{
	bool ret = true;
	uint64_t start_time;
	uint64_t dfx_time;

	start_time = hvgr_start_timing();
	dfx_time = hvgr_pm_dfx_get_curtime(gdev);

	ret = hvgr_pm_driver_gpc_pwr_on_by_sw(gdev);

	hvgr_pm_dfx_core_on_statistics(gdev, dfx_time);
	hvgr_end_timing_and_log(gdev, HVGR_PM, start_time, "Unreset core");

	return ret;
}

bool hvgr_pm_driver_reset_cores(struct hvgr_device *gdev)
{
	bool ret = true;
	uint64_t start_time;
	uint64_t dfx_time;

	dfx_time = hvgr_pm_dfx_get_curtime(gdev);
	start_time = hvgr_start_timing();

	ret = hvgr_pm_driver_gpc_pwr_off_by_sw(gdev);

	hvgr_pm_dfx_core_off_statistics(gdev, dfx_time);
	hvgr_end_timing_and_log(gdev, HVGR_PM, start_time, "Reset core");

	return ret;
}

bool hvgr_pm_wait_gpc_poweroff_ready(struct hvgr_device *gdev)
{
	return hvgr_pm_driver_wait_gpc_pwr_off_by_sw(gdev);
}

bool hvgr_pm_driver_gpu_config(struct hvgr_device *gdev)
{
	return hvgr_pm_driver_gpu_config_by_sw(gdev);
}

void hvgr_pm_driver_set_core_mask(struct hvgr_device *gdev)
{
	hvgr_pm_driver_set_core_mast_by_sw(gdev);
}

void hvgr_pm_driver_cbit_config(struct hvgr_device *gdev)
{
	hvgr_pm_driver_cbit_config_by_sw(gdev);

	hvgr_write_reg(gdev, (uint64_t)fcp_mb_reg(gdev, FCP_MB_EDMA2MCU_CF_MASKINT),
		FCP_INT_MASK); /* DMA channel 0~31 default mask */
	hvgr_write_reg(gdev, (uint64_t)fcp_mb_reg(gdev, FCP_MB_EDMA2MCU_CQ_MASKINT),
		FCP_INT_MASK); /* DMA channel 0~31 default mask */
	hvgr_write_reg(gdev, (uint64_t)fcp_mb_reg(gdev, FCP_MB_EDMA2JM_MASKINT),
		FCP_INT_UNMASK); /* DMA channel 0~31 default unmask */
	hvgr_write_reg(gdev, (uint64_t)fcp_mb_reg(gdev, FCP_MB_EDMA2HOST_MASKINT),
		FCP_INT_MASK); /* DMA channel 0~31 default mask */
}

int hvgr_pm_driver_clock_on(struct hvgr_device *gdev)
{
	return hvgr_pm_driver_clock_on_by_sw(gdev);
}

int hvgr_pm_driver_clock_off(struct hvgr_device *gdev)
{
	return hvgr_pm_driver_clock_off_by_sw(gdev);
}
