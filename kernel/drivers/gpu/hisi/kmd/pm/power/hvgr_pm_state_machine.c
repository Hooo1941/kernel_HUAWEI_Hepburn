/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 * This file is the dvfs implementation of the PM module.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */
#include "hvgr_pm_state_machine.h"
#include <linux/wait.h>
#include "hvgr_dm_api.h"
#include "hvgr_sch_api.h"
#include "hvgr_pm_api.h"
#include "hvgr_log_api.h"
#include "hvgr_pm_defs.h"
#include "hvgr_pm_policy.h"
#include "hvgr_pm_driver.h"
#include "hvgr_datan_api.h"

static int hvgr_pm_machine_state_power_on_proc(struct hvgr_device *gdev,
	enum hvgr_pm_gpu_status dest_state)
{
	struct hvgr_pm_pwr *pm_pwr = &gdev->pm_dev.pm_pwr;
	int ret;

	if (dest_state == HVGR_PM_GPU_READY &&
		hvgr_pm_is_need_power_on(gdev, false)) {
		hvgr_info(gdev, HVGR_PM, "Begin unreset core.");
		if (!hvgr_pm_driver_unreset_cores(gdev)) {
			pm_pwr->cur_status = HVGR_PM_CORE_ERROR;
			return HVGR_PM_CORE_FAIL;
		}

		hvgr_info(gdev, HVGR_PM, "Unreset core succeeded.");
		pm_pwr->cur_status = HVGR_PM_GPU_READY;
	} else if (dest_state == HVGR_PM_POWER_OFF &&
		hvgr_pm_is_need_power_off(gdev)) {
		hvgr_info(gdev, HVGR_PM, "Begin buck off.");
		ret = hvgr_pm_driver_clock_off(gdev);
		if (ret == HVGR_PM_CORE_FAIL) {
			pm_pwr->cur_status = HVGR_PM_CORE_ERROR;
			return HVGR_PM_CORE_FAIL;
		}

		hvgr_info(gdev, HVGR_PM, "Buck off succeeded.");
		pm_pwr->cur_status = HVGR_PM_POWER_OFF;
		wake_up(&pm_pwr->power_off_done_wait);
	} else {
		hvgr_info(gdev, HVGR_PM,
			"Invalid status, dest_state = %d, user_ref = %d",
			dest_state, atomic_read(&pm_pwr->user_ref));
		return HVGR_PM_INVALID_STATUS;
	}
	return HVGR_PM_OK;
}

static bool hvgr_pm_reset_core_proc(struct hvgr_device *gdev)
{
	struct hvgr_pm_pwr *pm_pwr = &gdev->pm_dev.pm_pwr;

	hvgr_info(gdev, HVGR_PM, "Begin reset core.");
	pm_pwr->cur_status = HVGR_PM_GPC_POWERING_OFF;
	if (!hvgr_pm_driver_reset_cores(gdev)) {
		hvgr_err(gdev, HVGR_PM, "Reset core failed.");
		return false;
	}
	hvgr_info(gdev, HVGR_PM, "Reset core ongoing");
	return true;
}

static void hvgr_pm_core_err_proc(struct hvgr_device *gdev)
{
	hvgr_datan_gpu_fault_type_set(gdev, DMD_PW_ONOFF_FAIL, "higpu power onoff fail");
	hvgr_pm_gpu_reset(gdev, GPU_RESET_TYPE_NORMAL);
}

static void hvgr_pm_machine_powering_off_proc(struct hvgr_device *gdev,
	enum hvgr_pm_gpu_status dest_state)
{
	struct hvgr_pm_pwr *pm_pwr = &gdev->pm_dev.pm_pwr;

	/* wait for GPC powering off */
	if (!hvgr_pm_wait_gpc_poweroff_ready(gdev)) {
		pm_pwr->cur_status = HVGR_PM_CORE_ERROR;
		return;
	}

	if (dest_state == HVGR_PM_GPU_READY) {
		if (!hvgr_pm_driver_unreset_cores(gdev))
			pm_pwr->cur_status = HVGR_PM_CORE_ERROR;
		else
			pm_pwr->cur_status = HVGR_PM_GPU_READY;
	} else {
		pm_pwr->cur_status = HVGR_PM_POWER_ON;
	}
	return;
}

int hvgr_pm_update_pwr_state(struct hvgr_device *gdev,
	enum hvgr_pm_gpu_status dest_state)
{
	struct hvgr_pm_pwr *pm_pwr = &gdev->pm_dev.pm_pwr;
	int ret;

	if (pm_pwr->cur_status == dest_state)
		return HVGR_PM_OK;

	if (pm_pwr->cur_status == HVGR_PM_CORE_ERROR)
		return HVGR_PM_CORE_FAIL;

	hvgr_dump_stack();
	hvgr_info(gdev, HVGR_PM,
		"%s, dest_state = %d, cur_status = %d",
		__func__, dest_state, pm_pwr->cur_status);
	hvgr_info(gdev, HVGR_PM, "%s, user_ref = %d",
		__func__, atomic_read(&pm_pwr->user_ref));

	do {
		switch (pm_pwr->cur_status) {
		case HVGR_PM_POWER_OFF:
			hvgr_info(gdev, HVGR_PM, "Begin buck on.");
			if (hvgr_pm_driver_clock_on(gdev) == HVGR_PM_CORE_FAIL) {
				pm_pwr->cur_status = HVGR_PM_CORE_ERROR;
				hvgr_info(gdev, HVGR_PM, "Buck on failed.");
			} else {
				pm_pwr->cur_status = HVGR_PM_POWER_ON;
				hvgr_info(gdev, HVGR_PM, "Buck on succeeded.");
			}
			break;
		case HVGR_PM_POWER_ON:
			ret = hvgr_pm_machine_state_power_on_proc(gdev, dest_state);
			if (ret == HVGR_PM_INVALID_STATUS)
				return HVGR_PM_OK;
			break;
		case HVGR_PM_GPU_READY:
			if (!hvgr_pm_reset_core_proc(gdev))
				pm_pwr->cur_status = HVGR_PM_CORE_ERROR;
			break;
		case HVGR_PM_GPC_POWERING_OFF:
			hvgr_pm_machine_powering_off_proc(gdev, dest_state);
			break;
		case HVGR_PM_CORE_ERROR:
			hvgr_pm_core_err_proc(gdev);
			return HVGR_PM_CORE_FAIL;
		default:
			hvgr_err(gdev, HVGR_PM,
				"Cur status is invalid, cur_status = %d.",
				pm_pwr->cur_status);
			return HVGR_PM_OK;
		}
	} while (pm_pwr->cur_status != dest_state);

	return HVGR_PM_OK;
}
