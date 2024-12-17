/*
 * npu_pm_interframe.c
 *
 * inter-frame power management for NPU, separated from npu_pm_framework.c
 * during the refactoring project in 2023
 *
 * Copyright (c) 2012-2023 Huawei Technologies Co., Ltd.
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
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <platform_include/basicplatform/linux/rdr_pub.h>
#include <linux/timer.h>
#include <linux/workqueue.h>

#include "npu_pm_framework.h"
#include "npu_pm_interframe.h"
#include "bbox/npu_dfx_black_box.h"
#include "npu_log.h"
#include "npu_platform.h"
#include "npu_shm.h"
#include "npu_autofs.h"
#include "npu_powercapping.h"
#include "npu_message.h"
#include "npu_heart_beat.h"
#include "npu_io.h"
#include "npu_hwts_plat.h"
#include "npu_reg.h"
#include "npu_task_message.h"
#include "npu_manager.h"
#include "npu_svm.h"
#include "profiling/npu_dfx_profiling.h"
#include "npu_stream.h"
#include "npd_interface.h"
#include "npu_syscache.h"

static struct npu_pm_work_strategy *npu_pm_interframe_get_work_strategy(
	struct npu_power_mgr *handle, u32 work_mode)
{
	struct npu_pm_work_strategy *strategy = NULL;
	struct npu_interframe_idle_manager *interframe_idle_manager =
		&handle->interframe_idle_manager;

	strategy = (struct npu_pm_work_strategy *)(uintptr_t)(
		interframe_idle_manager->strategy_table_addr +
		sizeof(struct npu_pm_work_strategy) * (work_mode));

	return strategy;
}

// npu_pm_interframe_idle_get_powerup_delta_subip
static u32 npu_interframe_get_delta_up_subip(
	struct npu_power_mgr *handle, u32 work_mode)
{
	struct npu_pm_work_strategy *strategy = NULL;
	u32 delta_subip;
	u32 cur_subip;

	strategy = npu_pm_interframe_get_work_strategy(handle, work_mode);
	delta_subip = strategy->subip_set.data;
	cur_subip = handle->cur_subip_set;
	delta_subip &= (~cur_subip);

	return delta_subip;
}
// npu_pm_interframe_idle_get_powerdown_delta_subip
static u32 npu_interframe_get_delta_down_subip(
	struct npu_power_mgr *handle, u32 work_mode)
{
	struct npu_pm_work_strategy *strategy = NULL;
	u32 mode_idx;
	u32 idle_subip;
	u32 delta_subip;
	u32 used_subip = 0;

	for (mode_idx = 0; mode_idx < NPU_WORKMODE_MAX; mode_idx++) {
		if (mode_idx == work_mode)
			continue;
		if (npu_bitmap_get(handle->work_mode, mode_idx) == 0x1) {
			strategy = npu_pm_interframe_get_work_strategy(
				handle, mode_idx);
			used_subip |= strategy->subip_set.data;
		}
	}
	idle_subip = handle->interframe_idle_manager.idle_subip;
	delta_subip = idle_subip & (~used_subip);

	return delta_subip;
}
// npu_pm_interframe_idle_get_delta_subip
static u32 npu_pm_interframe_get_delta_subip(
	struct npu_power_mgr *handle, u32 work_mode, int pm_ops)
{
	u32 delta_subip;

	if (pm_ops == POWER_UP)
		delta_subip = npu_interframe_get_delta_up_subip(handle, work_mode);
	else
		delta_subip = npu_interframe_get_delta_down_subip(handle, work_mode);

	return delta_subip;
}

static int npu_pm_interframe_idle_powerdown(struct npu_power_mgr *power_mgr, u32 work_mode)
{
	int ret;
	u32 delta_subip;

	delta_subip = npu_pm_interframe_get_delta_subip(power_mgr,
		work_mode, POWER_DOWN);
	npu_drv_warn("interframe idle power down enter, delta_subip= 0x%x\n", delta_subip);
	ret = npu_pm_powerdown_proc(power_mgr, work_mode, delta_subip);

	return ret;
}

static int npu_pm_interframe_check_need_powerup(struct npu_power_mgr *power_mgr,
	u32 work_mode)
{
	u32 cur_subip;
	u32 workmode_subip;
	struct npu_pm_work_strategy *work_strategy = NULL;

	work_strategy = npu_pm_interframe_get_work_strategy(power_mgr,
		work_mode);
	workmode_subip = work_strategy->subip_set.data;
	cur_subip = power_mgr->cur_subip_set;
	npu_drv_debug("cur_subip_set = 0x%x, subip_set = 0x%x\n", cur_subip,
		workmode_subip);
	if ((workmode_subip & cur_subip) != workmode_subip)
		return 1;
	else
		return 0;
}

static int npu_pm_interframe_powerup(struct npu_power_mgr *power_mgr, u32 workmode)
{
	int ret;
	u32 delta_subip;

	delta_subip = npu_pm_interframe_get_delta_subip(power_mgr, workmode, POWER_UP);
	npu_drv_warn("interframe idle power up enter, delta_subip= 0x%x\n", delta_subip);
	ret = npu_pm_powerup_proc(power_mgr, workmode, delta_subip);
	cond_return_error(ret != 0, ret, "power down fail: ret = %d\n", ret);

	npu_drv_info("interframe power up\n");

	return ret;
}

int npu_handle_interframe_powerup(struct npu_power_mgr *power_mgr, u32 workmode)
{
	int ret = 0;
	struct npu_platform_info *plat_info = npu_plat_get_info();

	if (plat_info == NULL ||
		plat_info->dts_info.feature_switch[NPU_FEATURE_INTERFRAME_IDLE_POWER_DOWN] == 0)
		return ret;

	if (workmode == NPU_NONSEC) {
		if (npu_pm_interframe_check_need_powerup(power_mgr, workmode)) {
#ifdef CONFIG_NPU_AUTOFS
			if ((plat_info->dts_info.feature_switch[NPU_FEATURE_NPU_AUTOFS] == 1) &&
				(power_mgr->power_stage == NPU_PM_UP))
				npu_autofs_disable();
#endif
			ret = npu_pm_interframe_powerup(power_mgr, workmode);
		}
	}

	return ret;
}

static void npu_pm_interframe_idle_work(struct work_struct *interframe_idle_work)
{
	struct npu_interframe_idle_manager *idle_manager = NULL;
	struct npu_dev_ctx *dev_ctx = NULL;
	struct npu_platform_info *plat_info = npu_plat_get_info();
	int work_state;

	npu_drv_info("interframe idle timer work enter\n");
	cond_return_void(interframe_idle_work == NULL,
		"interframe_idle_work is null\n");
	cond_return_void(plat_info == NULL, "plat_info is null\n");
	idle_manager = container_of(interframe_idle_work,
		struct npu_interframe_idle_manager, work.work);
	dev_ctx = container_of(idle_manager,
		struct npu_dev_ctx, pm.interframe_idle_manager);

	mutex_lock(&dev_ctx->pm.npu_power_mutex);
	if (dev_ctx->pm.wm_cnt[NPU_NONSEC] != 0) {
		atomic_cmpxchg(&idle_manager->wq_processing, WORK_ADDED, WORK_IDLE);
		mutex_unlock(&dev_ctx->pm.npu_power_mutex);
		return;
	}
	work_state = atomic_read(&idle_manager->wq_processing);
	if (work_state == WORK_ADDED) {
		(void)npu_pm_interframe_idle_powerdown(&dev_ctx->pm, NPU_NONSEC);
#ifdef CONFIG_NPU_AUTOFS
		if ((plat_info->dts_info.feature_switch[NPU_FEATURE_NPU_AUTOFS] == 1) &&
			(npu_bitmap_get(dev_ctx->pm.work_mode, NPU_ISPNN_SEPARATED) == 0) &&
			(npu_bitmap_get(dev_ctx->pm.work_mode, NPU_ISPNN_SHARED) == 0) &&
			(dev_ctx->pm.power_stage == NPU_PM_UP))
			npu_autofs_enable();
#endif
		atomic_set(&idle_manager->wq_processing, WORK_IDLE);
	} else {
		npu_drv_warn("work state = %d\n", work_state);
	}
	mutex_unlock(&dev_ctx->pm.npu_power_mutex);
}

int npu_enable_interframe_ilde(struct npu_proc_ctx *proc_ctx, uint32_t enable)
{
	struct npu_dev_ctx *dev_ctx = NULL;

	dev_ctx = get_dev_ctx_by_id(proc_ctx->devid);
	cond_return_error(dev_ctx == NULL, -1, "invalid dev ctx\n");

	atomic_set(&dev_ctx->pm.interframe_idle_manager.enable, (int)enable);
	npu_drv_debug("interframe enable = %u",
		atomic_read(&dev_ctx->pm.interframe_idle_manager.enable));

	return 0;
}

void npu_pm_interframe_delay_work_init(struct npu_interframe_idle_manager *idle_manager)
{
	struct npu_platform_info *plat_info = npu_plat_get_info();
	cond_return_void(plat_info == NULL, "plat_info is null\n");

	if (plat_info->dts_info.feature_switch[NPU_FEATURE_INTERFRAME_IDLE_POWER_DOWN]) {
		idle_manager->idle_time_out =
			NPU_INTERFRAME_IDLE_TIME_OUT_DEFAULT_VALUE;
		atomic_set(&idle_manager->wq_processing, WORK_IDLE);
		INIT_DELAYED_WORK(&idle_manager->work, npu_pm_interframe_idle_work);
	}
}

// npu_open_interframe_power_down
// npu_plan_interframe_power_down
void npu_schedule_interframe_power_down(struct npu_interframe_idle_manager *idle_manager)
{
	int work_state;

	cond_return_void(idle_manager == NULL, "idle_manager is null\n");

	work_state = atomic_cmpxchg(&idle_manager->wq_processing, WORK_IDLE, WORK_ADDING);
	if (work_state == WORK_IDLE) {
		schedule_delayed_work(&idle_manager->work, msecs_to_jiffies(idle_manager->idle_time_out));
		atomic_set(&idle_manager->wq_processing, WORK_ADDED);
	} else if (work_state != WORK_ADDED) {
		npu_drv_warn("work state = %d\n", work_state);
	}
}
// npu_close_interframe_power_down
void npu_cancel_interframe_power_down(struct npu_interframe_idle_manager *idle_manager)
{
	int work_state;

	cond_return_void(idle_manager == NULL, "idle_manager is null\n");

	work_state = atomic_cmpxchg(&idle_manager->wq_processing, WORK_ADDED, WORK_CANCELING);
	if (work_state == WORK_ADDED) {
		cancel_delayed_work_sync(&idle_manager->work);
		atomic_set(&idle_manager->wq_processing, WORK_IDLE);
	} else if (work_state != WORK_IDLE) {
		npu_drv_warn("work state = %d\n", work_state);
	}
}
