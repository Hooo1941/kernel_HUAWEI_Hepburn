/*
 * npu_pm_framework.c
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
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <platform_include/basicplatform/linux/rdr_pub.h>
#include <linux/timer.h>
#include <linux/workqueue.h>

#include "npu_pm_framework.h"
#include "bbox/npu_dfx_black_box.h"
#include "npu_proc_ctx.h"
#include "npu_common.h"
#include "npu_log.h"
#include "npu_platform.h"
#include "npu_shm.h"
#include "npu_autofs.h"
#include "npu_powercapping.h"
#include "npu_dpm.h"
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
#ifndef CONFIG_NPU_SWTS
#include "npu_dfx_log.h"
#endif
#ifdef CONFIG_NPU_LOAD_TS
#include "npu_load_ts_img.h"
#endif
#include "npu_counter.h"

#define npu_plat_power_status_vote(pm_flag) ((pm_flag) | (u32)npu_plat_get_npu_power_status())
#define npu_plat_power_status_unvote(pm_flag) ((~(pm_flag)) & (u32)npu_plat_get_npu_power_status())
#define WAKEUP_LOCK_MASK (~((1 << NPU_HTS) | (1 << NPU_HTS_GE) | (1 << NPU_HTS_EFF)))

static inline void npu_pm_set_power_on(u32 work_mode)
{
	uint32 pm_flag;

	pm_flag = (work_mode == NPU_SEC) ? DRV_NPU_POWER_ON_SEC_FLAG : DRV_NPU_POWER_ON_FLAG;

	npu_plat_set_npu_power_status(npu_plat_power_status_vote(pm_flag));
}

static inline void npu_pm_set_power_off(u32 work_mode)
{
	uint32 pm_flag;

	pm_flag = (work_mode == NPU_SEC) ? DRV_NPU_POWER_ON_SEC_FLAG : DRV_NPU_POWER_ON_FLAG;

	npu_plat_set_npu_power_status(npu_plat_power_status_unvote(pm_flag));
}

static inline struct npu_pm_work_strategy *npu_pm_get_work_strategy(
	struct npu_power_mgr *handle, u32 work_mode)
{
	struct npu_pm_work_strategy *strategy = NULL;

	strategy = (struct npu_pm_work_strategy *)(uintptr_t)(
		handle->strategy_table_addr +
		sizeof(struct npu_pm_work_strategy) * (work_mode));

	return strategy;
}

static inline struct npu_pm_subip_action *npu_pm_get_subip_action(
	struct npu_power_mgr *handle, u32 subip)
{
	struct npu_pm_subip_action *action = NULL;

	action = (struct npu_pm_subip_action *)(uintptr_t)(
		handle->action_table_addr +
		sizeof(struct npu_pm_subip_action) * (subip));

	return action;
}

static u32 npu_pm_get_delta_subip_set(
	struct npu_power_mgr *handle, u32 work_mode, int pm_ops)
{
	u32 mode_idx;
	u32 delta_subip_set;
	u32 old_subip_set;
	u32 new_subip_set = 0;
	struct npu_pm_work_strategy *work_strategy = NULL;

	work_strategy = npu_pm_get_work_strategy(handle, work_mode);
	delta_subip_set = work_strategy->subip_set.data;
	old_subip_set = handle->cur_subip_set;

	if (pm_ops == POWER_UP) {
		delta_subip_set &= (~old_subip_set);
	} else {
		for (mode_idx = 0; mode_idx < NPU_WORKMODE_MAX; mode_idx++) {
			if ((npu_bitmap_get(handle->work_mode, mode_idx) != 0) &&
				(mode_idx != work_mode)) {
				work_strategy = npu_pm_get_work_strategy(handle, mode_idx);
				new_subip_set |= work_strategy->subip_set.data;
			}
		}
		delta_subip_set = (old_subip_set & (~new_subip_set));
	}

	return delta_subip_set;
}

static void npu_cancel_idle_power_down(struct npu_power_mgr *power_mgr)
{
	int work_state;

	cond_return_void(power_mgr == NULL, "power_mgr is null\n");

	work_state = atomic_cmpxchg(&power_mgr->idle_wq_processing, WORK_ADDED,
		WORK_CANCELING);
	if (work_state == WORK_ADDED) {
		cancel_delayed_work_sync(&power_mgr->idle_work);
		atomic_set(&power_mgr->idle_wq_processing, WORK_IDLE);
	} else if (work_state != WORK_IDLE) {
		npu_drv_warn("work state = %d\n", work_state);
	}
}

static void npu_pm_delete_idle_timer(struct npu_dev_ctx *dev_ctx)
{
	struct npu_platform_info *plat_info = npu_plat_get_info();

	cond_return_void(plat_info == NULL, "plat info is null\n");

	npu_cancel_idle_power_down(&dev_ctx->pm);

	if (plat_info->dts_info.feature_switch[NPU_FEATURE_INTERFRAME_IDLE_POWER_DOWN])
		npu_cancel_interframe_power_down(&dev_ctx->pm.interframe_idle_manager);
}

static void npu_open_idle_power_down(struct npu_power_mgr *power_mgr)
{
	int work_state;

	cond_return_void(power_mgr == NULL, "power_mgr is null\n");

	work_state = atomic_cmpxchg(&power_mgr->idle_wq_processing, WORK_IDLE,
		WORK_ADDING);
	if (work_state == WORK_IDLE) {
		schedule_delayed_work(&power_mgr->idle_work,
			msecs_to_jiffies(power_mgr->npu_idle_time_out));
		atomic_set(&power_mgr->idle_wq_processing, WORK_ADDED);
	} else if (work_state != WORK_ADDED) {
		npu_drv_warn("work state = %d\n", work_state);
	}
}

static bool is_nsec_wm_cnt_zero(struct npu_power_mgr *pm)
{
	if (pm->wm_cnt[NPU_NONSEC] == 0 && pm->wm_cnt[NPU_NONSEC_GE] == 0 &&
		pm->wm_cnt[NPU_NONSEC_EFF] == 0 && pm->wm_cnt[NPU_NONSEC_TS] == 0)
		return true;

	return false;
}

static bool is_nsec_aicore_wm_cnt_zero(struct npu_power_mgr *pm)
{
	if (pm->wm_cnt[NPU_NONSEC] == 0 && pm->wm_cnt[NPU_NONSEC_GE] == 0 &&
		pm->wm_cnt[NPU_NONSEC_EFF] == 0)
		return true;

	return false;
}

static bool is_hts_wm_cnt_zero(struct npu_power_mgr *pm)
{
	if (pm->wm_cnt[NPU_HTS] == 0 && pm->wm_cnt[NPU_HTS_GE] == 0 &&
		pm->wm_cnt[NPU_HTS_EFF] == 0)
		return true;

	return false;
}

static void npu_pm_add_idle_timer(struct npu_dev_ctx *dev_ctx)
{
	struct npu_platform_info *plat_info = npu_plat_get_info();

	cond_return_void(plat_info == NULL, "plat info is null\n");

	if ((dev_ctx->pm.work_mode & NPU_NOSEC_BITMAP_MASK) == 0 ||
		(dev_ctx->pm.power_stage != NPU_PM_UP)) {
		npu_drv_warn("Can not add idle timer, for work mode: %d, power stage: %d!\n",
			dev_ctx->pm.work_mode, dev_ctx->pm.power_stage);
		return;
	}

	if (plat_info->dts_info.feature_switch[NPU_FEATURE_AUTO_POWER_DOWN] == NPU_FEATURE_OFF) {
		npu_drv_warn("npu auto power down switch off\n");
		return;
	}
	if (is_nsec_wm_cnt_zero(&dev_ctx->pm))
		npu_open_idle_power_down(&dev_ctx->pm);

	if ((plat_info->dts_info.feature_switch[NPU_FEATURE_INTERFRAME_IDLE_POWER_DOWN] != 0) &&
		(atomic_read(&dev_ctx->pm.interframe_idle_manager.enable) == 1))
		npu_schedule_interframe_power_down(&dev_ctx->pm.interframe_idle_manager);
}

int npu_pm_powerdown_proc(struct npu_power_mgr *power_mgr,
	u32 work_mode, u32 delta_subip)
{
	int ret = 0;
	int fail_flag = false;
	u32 subip, subip_idx, curr_subip_state;
	struct npu_pm_subip_action *subip_action = NULL;
	u32 *powerdown_order = power_mgr->powerdown_order;

	curr_subip_state = power_mgr->cur_subip_set;
	npu_drv_info("delta_subip : 0x%x, curr_subip_state: 0x%x\n",
		delta_subip, curr_subip_state);

	cond_return_error(powerdown_order == NULL, -1, "powerdown_order is null\n");

	for (subip_idx = 0; subip_idx < power_mgr->power_array_size; subip_idx++) {
		subip = powerdown_order[subip_idx];
		if ((npu_bitmap_get(delta_subip, subip) != 0) &&
			(npu_bitmap_get(curr_subip_state, subip) == 1)) {
			subip_action = npu_pm_get_subip_action(power_mgr, subip);
			ret = subip_action->power_down(work_mode, subip);
			if (ret != 0) {
				fail_flag = true;
				npu_drv_err("subip power down fail : subip %u ret = %d\n",
					subip, ret);
			}
			curr_subip_state = npu_bitmap_clear(curr_subip_state, subip);
			power_mgr->cur_subip_set = curr_subip_state;
			npu_drv_info("subip_%u power down succ, state : 0x%x\n",
				subip, curr_subip_state);
		}
	}

	if (fail_flag == true)
		npu_rdr_exception_report(RDR_EXC_TYPE_NPU_POWERDOWN_FAIL);

	return ret;
}

int npu_pm_powerup_proc(struct npu_power_mgr *power_mgr,
	u32 work_mode, u32 delta_subip)
{
	int ret = 0;
	u32 subip, subip_idx, curr_subip_set;
	struct npu_pm_subip_action *subip_action = NULL;
	u32 *powerup_order = power_mgr->powerup_order;
#ifdef CONFIG_HUAWEI_DSM
	struct dsm_client *tmp_dsm_client = npu_get_dsm_client();
#endif
	cond_goto_error(powerup_order == NULL, POWER_UP_FAIL, ret, -1,
				"powerup_order is null\n");
	curr_subip_set = power_mgr->cur_subip_set;
	npu_drv_info("delta_subip : 0x%x, curr_subip_state: 0x%x\n",
		delta_subip, curr_subip_set);
	for (subip_idx = 0; subip_idx < power_mgr->power_array_size; subip_idx++) {
		subip = powerup_order[subip_idx];
		if ((npu_bitmap_get(delta_subip, subip) != 0) &&
			(npu_bitmap_get(curr_subip_set, subip) == 0)) {
			subip_action = npu_pm_get_subip_action(power_mgr, subip);
			ret = subip_action->power_up(work_mode, subip);
			cond_goto_error(ret != 0, POWER_UP_FAIL, ret, ret,
				"subip power up fail : subip %u ret = %d\n", subip, ret);
			curr_subip_set = npu_bitmap_set(curr_subip_set, subip);
			power_mgr->cur_subip_set = curr_subip_set;
			npu_drv_info("subip_%d power up succ, state : 0x%x\n",
				subip, power_mgr->cur_subip_set);
		}
	}

	return ret;
POWER_UP_FAIL:
	(void)npu_pm_powerdown_proc(power_mgr, work_mode, delta_subip);
	if (ret != -NOT_SUPPORT_ISPNN && ret != -NPU_EXCEPTION_GLOBAL) {
		npu_rdr_exception_report(RDR_EXC_TYPE_NPU_POWERUP_FAIL);
#ifdef CONFIG_HUAWEI_DSM
		if (tmp_dsm_client != NULL && (dsm_client_ocuppy(tmp_dsm_client) == 0)) {
			dsm_client_record(tmp_dsm_client, "npu power up failed\n");
			dsm_client_notify(tmp_dsm_client, DSM_AI_KERN_POWER_UP_ERR_NO);
			npu_drv_err("[I/DSM] %s dmd report\n",
				tmp_dsm_client->client_name);
		}
#endif
	}
	return ret;
}

static int npu_pm_l2_swap_in(struct npu_proc_ctx *proc_ctx,
	struct npu_dev_ctx *dev_ctx, u32 work_mode)
{
	int ret = 0;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;
	struct npu_vma_mmapping *npu_vma_map = NULL;
	struct npu_platform_info *plat_info = npu_plat_get_info();

	cond_return_error(plat_info == NULL, -1, "get plat info fail\n");
	if (plat_info->dts_info.feature_switch[NPU_FEATURE_L2_BUFF] == 0)
		return 0;

	if ((list_empty_careful(&proc_ctx->l2_vma_list) == 0) &&
		(npu_bitmap_get(dev_ctx->pm.work_mode, NPU_SEC) == 0) &&
		(work_mode == NPU_INIT)) {
		list_for_each_safe(pos, n, &proc_ctx->l2_vma_list) {
			npu_vma_map = list_entry(pos, struct npu_vma_mmapping, list);
			ret = l2_mem_swapin(npu_vma_map->vma, proc_ctx->pid);
		}
	} else {
		npu_drv_warn("l2_vma_list is empty or work_mode:0x%x is sec,"
			" l2_mem_swapin is not necessary",
			dev_ctx->pm.work_mode);
	}

	return ret;
}

static int npu_pm_l2_swap_out(struct npu_proc_ctx *proc_ctx,
	struct npu_dev_ctx *dev_ctx, u32 work_mode)
{
	int ret = 0;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;
	struct npu_vma_mmapping *npu_vma_map = NULL;
	struct npu_platform_info *plat_info = npu_plat_get_info();

	cond_return_error(plat_info == NULL, -1, "get plat info fail\n");
	if (plat_info->dts_info.feature_switch[NPU_FEATURE_L2_BUFF] == 0)
		return 0;

	if ((list_empty_careful(&proc_ctx->l2_vma_list) == 0) &&
		(npu_bitmap_get(dev_ctx->pm.work_mode, NPU_SEC) == 0) &&
		(work_mode == NPU_INIT)) {
		list_for_each_safe(pos, n, &proc_ctx->l2_vma_list) {
			npu_vma_map = list_entry(pos, struct npu_vma_mmapping, list);
			ret = l2_mem_swapout(npu_vma_map->vma, dev_ctx->devid, proc_ctx->pid);
		}
	} else {
		npu_drv_warn("l2_vma_list is empty or work_mode:0x%x is sec,"
			" l2_mem_swapout is not necessary",
			dev_ctx->pm.work_mode);
	}

	return ret;
}

static int npu_pm_powerdown_pre_proc(struct npu_dev_ctx *dev_ctx, u32 work_mode)
{
	int ret;
	npu_work_mode_info_t wm_info = {
		.work_mode = work_mode,
		.flags = 0
	};

	struct npu_proc_ctx *proc_ctx = NULL;
	struct npu_proc_ctx *next_proc_ctx = NULL;
	struct npd_registry *npd = NULL;

	list_for_each_entry_safe(proc_ctx, next_proc_ctx,
		&dev_ctx->proc_ctx_list, dev_ctx_list) {
		ret = npu_pm_l2_swap_out(proc_ctx, dev_ctx, work_mode);
		if (ret != 0)
			npu_drv_err("l2 swap out fail, ret = %d\n", ret);
	}

	npd = get_npd();
	cond_return_error(npd == NULL, -1, "npd ptr is NULL\n");
	/* hwts aicore pool switch back */
	ret = npd->npd_switch_unit(dev_ctx, NPU_SFU_HWTS_AICORE_POOL, POWER_OFF, &wm_info);
	if (ret != 0)
		npu_drv_err("hwts return aicore to pool fail, ret = %d\n", ret);

	/* True pm dfx disable when total workmode is 0 except for sec bit. */
	if (npu_bitmap_clear(npu_bitmap_clear(dev_ctx->pm.work_mode, work_mode), NPU_SEC) == 0 && work_mode != NPU_SEC) {
#ifndef CONFIG_NPU_SWTS
		/* bbox heart beat exit */
		npu_heart_beat_exit(dev_ctx);
#endif

#if defined CONFIG_DPM_HWMON && defined CONFIG_NPU_DPM_ENABLED
		npu_dpm_exit();
#endif

#ifdef CONFIG_NPU_PCR_ENABLED
		npu_powercapping_disable();
#endif
	}

	return 0;
}

static int npu_pm_powerdown_post_proc(struct npu_dev_ctx *dev_ctx, u32 work_mode)
{
	struct npu_proc_ctx *proc_ctx = NULL;
	struct npu_proc_ctx *next_proc_ctx = NULL;
	u32 goal_work_mode_set;

	goal_work_mode_set = npu_bitmap_clear(dev_ctx->pm.work_mode, work_mode);
	if (npu_bitmap_clear(goal_work_mode_set, NPU_SEC) == 0 && work_mode != NPU_SEC) {
		dev_ctx->pm.ts_work_status = NPU_TS_DOWN;
		/* clear sqcq info */
		list_for_each_entry_safe(proc_ctx, next_proc_ctx, &dev_ctx->proc_ctx_list,
			dev_ctx_list) {
			npu_proc_clear_sqcq_info(proc_ctx);
		}
		npu_clear_pid_ssid_table(dev_ctx->devid, 0, 0);
		npu_recycle_rubbish_stream(dev_ctx);
	}
	if (((goal_work_mode_set & WAKEUP_LOCK_MASK) == 0) && (!is_work_mode_hts(work_mode)))
		__pm_relax(dev_ctx->pm.wakeup);

	if (goal_work_mode_set == 0) {
		dev_ctx->pm.power_stage = NPU_PM_DOWN;
		atomic_set(&dev_ctx->pm.power_access, 1);
	}

	return 0;
}

static int npu_pm_powerup_pre_proc(struct npu_dev_ctx *dev_ctx, u32 work_mode)
{
#ifdef CONFIG_NPU_LOAD_TS
	int ret = load_npu_verify_image();
	if (ret != 0)
		return ret;
#endif
	atomic_set(&dev_ctx->pm.power_access, 0);
	unused(work_mode);
	return 0;
}

static int npu_pm_powerup_post_proc(struct npu_dev_ctx *dev_ctx, npu_work_mode_info_t *wm_info)
{
	int ret;
	unsigned long flags;
	struct npu_platform_info *plat_info = NULL;
	struct npu_proc_ctx *proc_ctx = NULL;
	struct npu_proc_ctx *next_proc_ctx = NULL;
	struct npd_registry *npd = NULL;

	plat_info = npu_plat_get_info();
	cond_return_error(plat_info == NULL, -1, "get plat infofailed\n");
	dev_ctx->inuse.devid = dev_ctx->devid;
	spin_lock_irqsave(&dev_ctx->ts_spinlock, flags);
	dev_ctx->inuse.ai_core_num = plat_info->spec.aicore_max;
	dev_ctx->inuse.ai_core_error_bitmap = 0;
	spin_unlock_irqrestore(&dev_ctx->ts_spinlock, flags);
	dev_ctx->pm.power_stage = NPU_PM_UP;
	npu_counter_clear();
	npu_drv_warn("npu dev %u hardware powerup successfully!\n",
		dev_ctx->devid);

	if ((wm_info->work_mode != NPU_SEC) &&
		(npu_bitmap_clear(dev_ctx->pm.work_mode, NPU_SEC) == 0)) {
#ifndef CONFIG_NPU_SWTS
		/* clear sqcq info */
		list_for_each_entry_safe(proc_ctx, next_proc_ctx, &dev_ctx->proc_ctx_list,
			dev_ctx_list) {
			npu_proc_clear_sqcq_info(proc_ctx);
		}
		dev_ctx->pm.ts_work_status = NPU_TS_WORK;
		ret = log_wakeup(LOG_CHANNEL_TS_ID);
		if (ret != 0)
			npu_drv_err("npu ts get log wake up fail, ret = %d\n", ret);
		dev_ctx->heart_beat.hwts_exception_callback = npu_raise_hwts_exception;
		/* bbox heart beat init in non_secure mode */
		ret = npu_heart_beat_init(dev_ctx);
		if (ret != 0)
			npu_drv_err("npu heart beat init failed, ret = %d\n", ret);
#endif

#ifdef CONFIG_NPU_PCR_ENABLED
		npu_powercapping_enable();
#endif

#if defined CONFIG_DPM_HWMON && defined CONFIG_NPU_DPM_ENABLED
		npu_dpm_init();
#endif
		/* set hwts log&profiling gobal config when hwts init ok */
		profiling_clear_mem_ptr();

		/* get hiaiserver ssid, ensure hiai server ssid is 0 */
		(void)npu_manager_svm_bind(dev_ctx);
	}

	if (((dev_ctx->pm.work_mode & WAKEUP_LOCK_MASK) == 0) && (!is_work_mode_hts(wm_info->work_mode)))
		__pm_stay_awake(dev_ctx->pm.wakeup);

	npd = get_npd();
	cond_return_error(npd == NULL, -1, "npd ptr is NULL\n");

	/* hwts aicore pool switch */
	ret = npd->npd_switch_unit(dev_ctx, NPU_SFU_HWTS_AICORE_POOL, POWER_ON, wm_info);
	cond_return_error(ret != 0, ret, "hwts pull aicore from pool fail, ret = %d\n", ret);
	if (is_workmode_nsec(wm_info->work_mode)) {
		// lock power mutex, no need dev_ctx->proc_ctx_lock
#ifndef CONFIG_NPU_SWTS
		list_for_each_entry_safe(proc_ctx, next_proc_ctx, &dev_ctx->proc_ctx_list,
			dev_ctx_list) {
			ret = npu_proc_send_alloc_stream_mailbox(proc_ctx);
			cond_return_error(ret != 0, ret,
					"npu send stream mailbox failed\n");
		}
#endif
	} else {
		npu_drv_warn("secure or ispnn npu power up,no need send mbx to tscpu,return directly\n");
	}

	list_for_each_entry_safe(proc_ctx, next_proc_ctx, &dev_ctx->proc_ctx_list,
		dev_ctx_list) {
		ret = npu_pm_l2_swap_in(proc_ctx, dev_ctx, wm_info->work_mode);
		if (ret != 0)
			npu_drv_err("l2 swap in fail, ret = %d\n", ret);
	}

	return 0;
}

static int npu_pm_unvote(struct npu_dev_ctx *dev_ctx, u32 workmode)
{
	int ret, ret1;
	u32 goal_work_mode_set;
	u32 delta_subip;
	struct npu_platform_info *plat_info = npu_plat_get_info();

	cond_return_error(plat_info == NULL, -1, "plat_info is null\n");

	if (plat_info->dts_info.feature_switch[NPU_FEATURE_AUTO_POWER_DOWN] == NPU_FEATURE_OFF) {
		npu_drv_warn("npu auto power down switch off\n");
		return 0;
	}

	goal_work_mode_set = npu_bitmap_clear(dev_ctx->pm.work_mode, workmode);
	if (dev_ctx->pm.work_mode == goal_work_mode_set)
		return 0;

	npu_drv_warn("enter powerdown, workmode = %u\n", workmode);

	/* lite & ispnn use the same power_status, so unsec bits can reset only after lite & ispnn workmodes are all 0s */
	if (workmode == NPU_SEC || (npu_bitmap_clear(goal_work_mode_set, NPU_SEC) == 0))
		npu_pm_set_power_off(workmode);

	ret1 = npu_pm_powerdown_pre_proc(dev_ctx, workmode);
	if (ret1 != 0)
		npu_drv_err("power down pre_porc fail: ret = %d\n", ret1);

	delta_subip = npu_pm_get_delta_subip_set(&dev_ctx->pm, workmode, POWER_DOWN);
	ret = npu_pm_powerdown_proc(&dev_ctx->pm, workmode, delta_subip);
	if (ret != 0) {
		npu_drv_err("power down fail: ret = %d\n", ret);
		ret1 += ret;
	}

	ret = npu_pm_powerdown_post_proc(dev_ctx, workmode);
	if (ret != 0) {
		npu_drv_err("power down post_proc fail : ret = %d\n", ret);
		ret1 += ret;
	}

	dev_ctx->pm.work_mode = goal_work_mode_set;
	npu_drv_warn("powerdown succ, workmode_set = 0x%x, delta_subip = 0x%x\n",
		goal_work_mode_set, delta_subip);

	return ret1;
}

static int npu_pm_vote(struct npu_dev_ctx *dev_ctx, npu_work_mode_info_t *wm_info)
{
	int ret;
	u32 delta_subip;
	u32 goal_work_mode_set;
	u32 work_mode = wm_info->work_mode;

	goal_work_mode_set = npu_bitmap_set(dev_ctx->pm.work_mode, work_mode);
	if (dev_ctx->pm.work_mode == goal_work_mode_set)
		return 0;

	npu_drv_warn("enter powerup, work_mode = %u\n", work_mode);

	ret = npu_pm_powerup_pre_proc(dev_ctx, work_mode);
	cond_return_error(ret != 0, ret,
		"power up pre_porc fail: ret = %d\n", ret);

	delta_subip = npu_pm_get_delta_subip_set(&dev_ctx->pm, work_mode, POWER_UP);
	ret = npu_pm_powerup_proc(&dev_ctx->pm, work_mode, delta_subip);
	cond_goto_error(ret != 0, fail, ret, ret,
		"power up fail: ret = %d\n", ret);

	ret = npu_pm_powerup_post_proc(dev_ctx, wm_info);
	dev_ctx->pm.work_mode = goal_work_mode_set;
	cond_goto_error(ret != 0, post_fail, ret, ret,
		"power up post_porc fail: ret = %d\n", ret);

	npu_pm_set_power_on(work_mode);
	npu_drv_warn("powerup succ, workmode_set = 0x%x, delta_subip = 0x%x\n",
		goal_work_mode_set, delta_subip);

	return ret;

post_fail:
	(void)npu_pm_unvote(dev_ctx, work_mode);
fail:
	if (dev_ctx->pm.work_mode == 0) {
		dev_ctx->pm.power_stage = NPU_PM_DOWN;
		atomic_set(&dev_ctx->pm.power_access, 1);
	}
	return ret;
}

static int npu_pm_dev_enter_wm(struct npu_dev_ctx *dev_ctx, npu_work_mode_info_t *wm_info)
{
	int ret;
	u32 work_mode = wm_info->work_mode;
	struct npu_pm_work_strategy *work_strategy = NULL;

	if (dev_ctx->pm.work_mode == 0 && is_workmode_nsec(work_mode))
		npu_rdr_exception_reset();

	if (dev_ctx->server_proc_num == 0) {
		npu_drv_err("there is no server proc, cannot power up");
		return -1;
	}

	npu_drv_debug("dev work_mode_set = 0x%x, work_mode = %d\n",
		dev_ctx->pm.work_mode, work_mode);
	if (npu_bitmap_get(dev_ctx->pm.work_mode, work_mode) != 0) {
		dev_ctx->pm.wm_cnt[work_mode]++;
		return 0;
	}

	work_strategy = npu_pm_get_work_strategy(&dev_ctx->pm, work_mode);
	if (work_strategy->work_mode_set.data == UINT32_MAX ||
		(dev_ctx->pm.work_mode & work_strategy->work_mode_set.data) != 0) {
		npu_drv_err("work mode conflict, work_mode_set = 0x%x, work_mode = %d\n",
			dev_ctx->pm.work_mode, work_mode);
		return -1;
	}

	ret = npu_pm_vote(dev_ctx, wm_info);
	if (ret != 0)
		return ret;
	dev_ctx->pm.wm_cnt[work_mode]++;

	return ret;
}

static int npu_pm_dev_exit_wm(struct npu_dev_ctx *dev_ctx, u32 work_mode)
{
	int ret = 0;

	if (dev_ctx->pm.wm_cnt[work_mode] == 0) {
		npu_drv_err("dev work_mode_cnt = %u, work_mode = %u\n",
				dev_ctx->pm.wm_cnt[work_mode], work_mode);
		goto sec_check;
	}

	dev_ctx->pm.wm_cnt[work_mode] --;
	if (dev_ctx->pm.wm_cnt[work_mode] != 0) {
		npu_drv_debug("dev work_mode_cnt = %d work_mode = %d\n",
			dev_ctx->pm.wm_cnt[work_mode], work_mode);
		goto sec_check;
	}

	if (npu_bitmap_get(dev_ctx->pm.work_mode, work_mode) != 0)
		ret = npu_pm_unvote(dev_ctx, work_mode);
	else
		npu_drv_err("dev work_mode_set = 0x%x, work_mode = %d\n",
			dev_ctx->pm.work_mode, work_mode);

sec_check:
	if (npu_bitmap_get(dev_ctx->pm.work_mode, work_mode) == 0) {
		if (work_mode == NPU_SEC) {
			npu_drv_debug("wake up work_mode = %d\n", work_mode);
			wake_up(&dev_ctx->pm.nosec_wait);
		} else if (is_workmode_nsec(work_mode)) {
			npu_drv_debug("wake up work_mode = %d\n", work_mode);
			wake_up(&dev_ctx->pm.sec_wait);
		}
	}
	return ret;
}

static int npu_pm_dev_send_task_enter_wm(struct npu_dev_ctx *dev_ctx, uint32_t work_mode)
{
	int ret;
	npu_work_mode_info_t wm_info = {
		.work_mode = work_mode,
		.flags = 0
	};

	struct npu_platform_info *plat_info = npu_plat_get_info();
	cond_return_error(plat_info == NULL, -1, "get plat info fail\n");

	if (is_workmode_nsec(work_mode) == 0) {
		npu_drv_err("invalid nosec work mode = %u", work_mode);
		return -1;
	}

	mutex_lock(&dev_ctx->pm.npu_power_mutex);
	if (plat_info->dts_info.feature_switch[NPU_FEATURE_SEC_NONSEC_CONCURRENCY] == 0) {
		if (atomic_read(&dev_ctx->pm.security_state) != 0 || npu_bitmap_get(dev_ctx->pm.work_mode, NPU_SEC) != 0) {
			mutex_unlock(&dev_ctx->pm.npu_power_mutex);
			npu_drv_warn("wait for safe workmode exit\n");
			return -EBUSY;
		}
	}

	ret = npu_pm_dev_enter_wm(dev_ctx, &wm_info);
	if (ret != 0) {
		mutex_unlock(&dev_ctx->pm.npu_power_mutex);
		return ret;
	}
	ret = npu_handle_interframe_powerup(&dev_ctx->pm, wm_info.work_mode);
	mutex_unlock(&dev_ctx->pm.npu_power_mutex);

	npu_pm_delete_idle_timer(dev_ctx);
	return ret;
}

static void npu_pm_nsec_direct_power_down(struct npu_dev_ctx *dev_ctx)
{
	int ret = 0;
	u32 goal_work_mode_set = dev_ctx->pm.work_mode;
	u32 i;

	for (i = 0; i < NPU_NOSEC_WORKMODE_NUM; i++) {
		if (npu_bitmap_get(dev_ctx->pm.work_mode, nonsec_workmode[i]) != 0) {
			goal_work_mode_set = npu_bitmap_clear(dev_ctx->pm.work_mode, nonsec_workmode[i]);
			ret += npu_pm_unvote(dev_ctx, nonsec_workmode[i]);
			npu_drv_warn("npu nonsec exit, workmode = %d\n", nonsec_workmode[i]);
		} else {
			npu_drv_warn("npu workmode %d already exit, workmode_set = 0x%x\n",
				nonsec_workmode[i], dev_ctx->pm.work_mode);
		}
	}
	dev_ctx->pm.work_mode = goal_work_mode_set;
	if (ret != 0)
		npu_drv_err("un vote nonsec fail, ret = %d\n", ret);
}

static int npu_pm_dev_release_report_exit_wm(struct npu_dev_ctx *dev_ctx, uint32_t work_mode)
{
	mutex_lock(&dev_ctx->pm.npu_power_mutex);
	if (dev_ctx->pm.wm_cnt[work_mode] == 0) {
		npu_drv_err("dev work_mode_cnt = %d\n", dev_ctx->pm.wm_cnt[work_mode]);
		mutex_unlock(&dev_ctx->pm.npu_power_mutex);
		return 0;
	}

	dev_ctx->pm.wm_cnt[work_mode] --;
	npu_drv_debug("dev idle, work_mode_cnt = %d\n", dev_ctx->pm.wm_cnt[work_mode]);

	if (is_nsec_wm_cnt_zero(&dev_ctx->pm)) {
		if (unlikely(atomic_read(&dev_ctx->pm.security_state) == 1)) {
			npu_drv_warn("npu_sec want to powerup, so nonsec should powerdown now");
			npu_pm_nsec_direct_power_down(dev_ctx);
			wake_up(&dev_ctx->pm.sec_wait);
		} else if ((dev_ctx->pm.work_mode & NPU_NOSEC_BITMAP_MASK) != 0) {
			npu_pm_add_idle_timer(dev_ctx);
		}
	}

#ifdef CONFIG_NPU_SYSCACHE
	if (is_nsec_aicore_wm_cnt_zero(&dev_ctx->pm) && is_hts_wm_cnt_zero(&dev_ctx->pm))
		npu_sc_low(dev_ctx);
#endif
	mutex_unlock(&dev_ctx->pm.npu_power_mutex);

	return 0;
}

static int npu_pm_proc_mutex(struct npu_dev_ctx *dev_ctx, npu_work_mode_info_t *wm_info)
{
	int ret = 0;

	atomic_set(&dev_ctx->pm.security_state, 1);
	npu_drv_warn("sec wait for unsec exit\n");
	mutex_lock(&dev_ctx->pm.npu_power_mutex);
	if (unlikely(is_nsec_wm_cnt_zero(&dev_ctx->pm) != 1)) {
		mutex_unlock(&dev_ctx->pm.npu_power_mutex);
		npu_drv_warn("wait nonsec task complete!!!");
		return -EBUSY;
	}

	if ((dev_ctx->pm.work_mode & NPU_NOSEC_BITMAP_MASK) != 0) {
		mutex_unlock(&dev_ctx->pm.npu_power_mutex);
		npu_drv_warn("delete idle timer, should out of power mutex lock!!!\n");
		npu_pm_delete_idle_timer(dev_ctx);
		mutex_lock(&dev_ctx->pm.npu_power_mutex);
		if ((dev_ctx->pm.work_mode & NPU_NOSEC_BITMAP_MASK) != 0) {
			npu_drv_warn("to powerup npu_sec, it should powerdown nonsec now");
			npu_pm_nsec_direct_power_down(dev_ctx);
		}
	}
	ret = npu_pm_dev_enter_wm(dev_ctx, wm_info);
	atomic_set(&dev_ctx->pm.security_state, 0);
	mutex_unlock(&dev_ctx->pm.npu_power_mutex);
	if (ret != 0) {
		wake_up(&dev_ctx->pm.nosec_wait);
		npu_drv_err("enter work_mode fail, ret = %d\n", ret);
	}
	return ret;
}

static int npu_pm_proc_ioctl_enter_wm_(struct npu_proc_ctx *proc_ctx,
		struct npu_dev_ctx *dev_ctx, npu_work_mode_info_t *wm_info)
{
	int ret;
	u32 work_mode;
	uint32_t wm_set;

	struct npu_platform_info *plat_info = npu_plat_get_info();
	cond_return_error(plat_info == NULL, -1, "get plat info fail\n");

	work_mode = wm_info->work_mode;

	npu_drv_debug("proc work_mode_set = 0x%x, work_mode = %d\n",
		(uint32_t)atomic_read(&proc_ctx->wm_set), work_mode);
	if (is_workmode_nsec(work_mode)) {
		npu_drv_err("proc work_mode_set = 0x%x, work_mode = %d\n",
		(uint32_t)atomic_read(&proc_ctx->wm_set), work_mode);
		return -1;
	}

	down_read(&dev_ctx->except_manager.status_lock);
	if (dev_ctx->except_manager.status != NPU_STATUS_NORMAL) {
		npu_drv_err("npu have exception, cant power up, except status: %d\n",
			dev_ctx->except_manager.status);
		up_read(&dev_ctx->except_manager.status_lock);
		return -1;
	}

	mutex_lock(&proc_ctx->wm_lock);
	wm_set = (uint32_t)atomic_read(&proc_ctx->wm_set);
	if (npu_bitmap_get(wm_set, work_mode) != 0) {
		npu_drv_warn("proc work_mode_set = 0x%x, work_mode = %d\n",
			wm_set, work_mode);
		proc_ctx->task_cnt[work_mode]++;
		mutex_unlock(&proc_ctx->wm_lock);
		up_read(&dev_ctx->except_manager.status_lock);
		return 0;
	}

	if ((work_mode == NPU_SEC) &&
		(plat_info->dts_info.feature_switch[NPU_FEATURE_SEC_NONSEC_CONCURRENCY] == 0)) {
		ret = npu_pm_proc_mutex(dev_ctx, wm_info);
	} else {
		mutex_lock(&dev_ctx->pm.npu_power_mutex);
		ret = npu_pm_dev_enter_wm(dev_ctx, wm_info);
		mutex_unlock(&dev_ctx->pm.npu_power_mutex);
	}
	if (ret == 0) {
		wm_set = (uint32_t)atomic_read(&proc_ctx->wm_set);
		atomic_set(&proc_ctx->wm_set, (int)npu_bitmap_set(wm_set, work_mode));
		proc_ctx->task_cnt[work_mode]++;
	} else {
		npu_drv_err("powerup failed, ret = %d\n", ret);
	}
	mutex_unlock(&proc_ctx->wm_lock);
	up_read(&dev_ctx->except_manager.status_lock);
	return ret;
}

int npu_pm_proc_ioctl_enter_wm(struct npu_proc_ctx *proc_ctx,
	struct npu_dev_ctx *dev_ctx, npu_work_mode_info_t *wm_info)
{
	int ret;
	u32 work_mode;

	ret = npu_pm_proc_ioctl_enter_wm_(proc_ctx, dev_ctx, wm_info);
	if (ret == 0)
		return 0;
	
	work_mode = wm_info->work_mode;
	// sec mdoe may need to wait
	if (work_mode == NPU_SEC) {
		while (ret == -EBUSY) {
			wait_event(dev_ctx->pm.sec_wait,
				dev_ctx->pm.wm_cnt[NPU_NONSEC] == 0 || npu_bitmap_get(dev_ctx->pm.work_mode, NPU_NONSEC) == 0);
			npu_drv_warn("wakeuped! now it can power up npu_sec");
			ret = npu_pm_proc_ioctl_enter_wm_(proc_ctx, dev_ctx, wm_info);
		}
	}

	return ret;
}

int npu_pm_proc_ioctl_exit_wm(struct npu_proc_ctx *proc_ctx,
	struct npu_dev_ctx *dev_ctx, u32 work_mode)
{
	int ret = 0;
	uint32_t wm_set;

	npu_drv_debug("proc work_mode_set = 0x%x, work_mode = %d\n",
		atomic_read(&proc_ctx->wm_set), work_mode);

	if (is_workmode_nsec(work_mode)) {
		npu_drv_err("proc work_mode_set = 0x%x, work_mode = %d\n",
		atomic_read(&proc_ctx->wm_set), work_mode);
		return -1;
	}

	mutex_lock(&proc_ctx->wm_lock);
	wm_set = (uint32_t)atomic_read(&proc_ctx->wm_set);
	if ((npu_bitmap_get(wm_set, work_mode) == 0) && (proc_ctx->task_cnt[work_mode] == 0)) {
		npu_drv_err("proc work_mode_set = 0x%x, work_mode = %d\n",
			wm_set, work_mode);
		mutex_unlock(&proc_ctx->wm_lock);
		return 0;
	}

	proc_ctx->task_cnt[work_mode]--;
	if (proc_ctx->task_cnt[work_mode] == 0) {
		mutex_lock(&dev_ctx->pm.npu_power_mutex);
		ret = npu_pm_dev_exit_wm(dev_ctx, work_mode);
		mutex_unlock(&dev_ctx->pm.npu_power_mutex);
		if (ret != 0) {
			mutex_unlock(&proc_ctx->wm_lock);
			return ret;
		}
	}

	atomic_set(&proc_ctx->wm_set, (int)npu_bitmap_clear(wm_set, work_mode));
	mutex_unlock(&proc_ctx->wm_lock);

	return ret;
}

int npu_pm_proc_send_task_enter_wm(struct npu_proc_ctx *proc_ctx,
	struct npu_dev_ctx *dev_ctx, uint32_t work_mode)
{
	int ret;
	uint32_t wm_set;

	mutex_lock(&proc_ctx->wm_lock);
	wm_set = (uint32_t)atomic_read(&proc_ctx->wm_set);
	if (npu_bitmap_get(wm_set, work_mode) == 0) {
		ret = npu_pm_dev_send_task_enter_wm(dev_ctx, work_mode);
		if (ret != 0) {
			mutex_unlock(&proc_ctx->wm_lock);
			return ret;
		}
		atomic_set(&proc_ctx->wm_set, (int)npu_bitmap_set(wm_set, work_mode));
	}
	proc_ctx->task_cnt[work_mode]++;
	npu_drv_debug("proc_ctx task cnt %u = %u", work_mode, proc_ctx->task_cnt[work_mode]);
	mutex_unlock(&proc_ctx->wm_lock);

	return 0;
}

int npu_pm_proc_ffrt_enter_wm(struct npu_proc_ctx *proc_ctx,
	struct npu_dev_ctx *dev_ctx, u32 work_mode)
{
	int ret;
	uint32_t wm_set;

	npu_work_mode_info_t wm_info = {
		.work_mode = work_mode,
		.flags = 0
	};

	mutex_lock(&proc_ctx->wm_lock);
	wm_set = (uint32_t)atomic_read(&proc_ctx->wm_set);
	if (npu_bitmap_get(wm_set, work_mode) != 0) {
		npu_drv_warn("proc work_mode_set = 0x%x, work_mode = %d\n",
			wm_set, work_mode);
		proc_ctx->task_cnt[work_mode]++;
		mutex_unlock(&proc_ctx->wm_lock);
		return 0;
	}
	mutex_lock(&dev_ctx->pm.npu_power_mutex);
	ret = npu_pm_dev_enter_wm(dev_ctx, &wm_info);
	mutex_unlock(&dev_ctx->pm.npu_power_mutex);
	if (ret != 0) {
		mutex_unlock(&proc_ctx->wm_lock);
		return ret;
	}

	atomic_set(&proc_ctx->wm_set, (int)npu_bitmap_set(wm_set, work_mode));
	proc_ctx->task_cnt[work_mode]++;
	npu_drv_debug("proc_ctx task cnt %u = %u", work_mode, proc_ctx->task_cnt[work_mode]);
	mutex_unlock(&proc_ctx->wm_lock);

	return 0;
}

int npu_pm_proc_ffrt_exit_wm(struct npu_proc_ctx *proc_ctx,
	struct npu_dev_ctx *dev_ctx, u32 work_mode)
{
	uint32_t wm_set;

	npu_drv_debug("proc work_mode_set = 0x%x\n",
		(uint32_t)atomic_read(&proc_ctx->wm_set));

	mutex_lock(&proc_ctx->wm_lock);
	wm_set = (uint32_t)atomic_read(&proc_ctx->wm_set);
	if (proc_ctx->task_cnt[work_mode] == 0) {
		npu_drv_err("proc work_mode_set = 0x%x\n",
			wm_set);
		mutex_unlock(&proc_ctx->wm_lock);
		return -1;
	}
	npu_drv_debug("proc work_mode_cnt = %d\n",
		proc_ctx->task_cnt[work_mode]);
	proc_ctx->task_cnt[work_mode]--;

	if (proc_ctx->task_cnt[work_mode] > 0) {
		mutex_unlock(&proc_ctx->wm_lock);
		return 0;
	}

	mutex_lock(&dev_ctx->pm.npu_power_mutex);
	(void)npu_pm_dev_exit_wm(dev_ctx, work_mode);
#ifdef CONFIG_NPU_SYSCACHE
	if (is_nsec_aicore_wm_cnt_zero(&dev_ctx->pm) && is_hts_wm_cnt_zero(&dev_ctx->pm))
		npu_sc_low(dev_ctx);
#endif
	mutex_unlock(&dev_ctx->pm.npu_power_mutex);
	if (npu_bitmap_get(wm_set, work_mode) != 0)
		atomic_set(&proc_ctx->wm_set, (int)npu_bitmap_clear(wm_set, work_mode));

	mutex_unlock(&proc_ctx->wm_lock);

	return 0;
}

int npu_pm_proc_release_task_exit_wm(struct npu_proc_ctx *proc_ctx,
	struct npu_dev_ctx *dev_ctx, u32 num, u32 work_mode)
{
	uint32_t wm_set;

	npu_drv_debug("proc work_mode_set = 0x%x\n",
		(uint32_t)atomic_read(&proc_ctx->wm_set));

	mutex_lock(&proc_ctx->wm_lock);
	wm_set = (uint32_t)atomic_read(&proc_ctx->wm_set);
	if (proc_ctx->task_cnt[work_mode] < num) {
		npu_drv_err("proc work_mode_set = 0x%x, num = %d\n",
			wm_set, num);
		mutex_unlock(&proc_ctx->wm_lock);
		return -1;
	}
	npu_drv_debug("proc work_mode_cnt = %d, num = %d\n",
		proc_ctx->task_cnt[work_mode], num);
	proc_ctx->task_cnt[work_mode] -= num;

	if (proc_ctx->task_cnt[work_mode] == 0) {
		(void)npu_pm_dev_release_report_exit_wm(dev_ctx, work_mode);
		if (npu_bitmap_get(wm_set, work_mode) != 0)
			atomic_set(&proc_ctx->wm_set, (int)npu_bitmap_clear(wm_set, work_mode));
	}

	mutex_unlock(&proc_ctx->wm_lock);

	return 0;
}

int npu_pm_proc_release_exit_wm(struct npu_proc_ctx *proc_ctx,
	struct npu_dev_ctx *dev_ctx)
{
	u32 work_mode;
	u32 i;
	npu_drv_info("proc work_mode_set = 0x%x\n", (uint32_t)atomic_read(&proc_ctx->wm_set));

	mutex_lock(&proc_ctx->wm_lock);

	/* non secure normal exit */
	for (i = 0; i < NPU_NOSEC_WORKMODE_NUM; i++) {
		if ((npu_bitmap_get(dev_ctx->pm.work_mode, nonsec_workmode[i]) != 0) &&
			(dev_ctx->pm.wm_cnt[nonsec_workmode[i]] == 0) &&
			(list_empty_careful(&dev_ctx->proc_ctx_list) != 0)) {
			mutex_lock(&dev_ctx->pm.npu_power_mutex);
#ifdef CONFIG_NPU_SYSCACHE
			npu_drv_debug("before npu_sc_disable\n");
			npu_sc_disable(dev_ctx);
#endif
			(void)npu_pm_unvote(dev_ctx, nonsec_workmode[i]);
			mutex_unlock(&dev_ctx->pm.npu_power_mutex);
		}
	}
	if ((dev_ctx->pm.work_mode & NPU_NOSEC_BITMAP_MASK) == 0)
		npu_pm_delete_idle_timer(dev_ctx);

	down_write(&dev_ctx->except_manager.status_lock);
	mutex_lock(&dev_ctx->pm.npu_power_mutex);
	for (work_mode = 0; work_mode < NPU_WORKMODE_MAX; work_mode++) {
		if (npu_bitmap_get((uint32_t)atomic_read(&proc_ctx->wm_set), work_mode) != 0) {
			(void)npu_pm_dev_exit_wm(dev_ctx, work_mode);
#ifdef CONFIG_NPU_SYSCACHE
			if (is_workmode_nsec(work_mode) && dev_ctx->pm.wm_cnt[work_mode] == 0)
					npu_sc_disable(dev_ctx);
#endif
		}
	}
	if (is_nsec_wm_cnt_zero(&dev_ctx->pm))
		dev_ctx->except_manager.status = NPU_STATUS_NORMAL;
	mutex_unlock(&dev_ctx->pm.npu_power_mutex);
	up_write(&dev_ctx->except_manager.status_lock);

	atomic_set(&proc_ctx->wm_set, 0);
	mutex_unlock(&proc_ctx->wm_lock);

	return 0;
}

int npu_pm_handle_power_up(struct npu_dev_ctx *dev_ctx)
{
	int ret = 0;
	u32 workmode_idx;
	npu_work_mode_info_t wm_info = {0};

	struct npu_platform_info *plat_info = npu_plat_get_info();

	cond_return_error(plat_info == NULL, -1, "plat_info is null\n");

	npu_drv_warn("exception powerup enter");
	mutex_lock(&dev_ctx->pm.npu_power_mutex);
	for (workmode_idx = 0; workmode_idx < NPU_WORKMODE_MAX; workmode_idx++) {
		if ((workmode_idx == NPU_SEC) || is_workmode_nsec(workmode_idx))
			continue;
		if (dev_ctx->pm.wm_cnt[workmode_idx] > 0) {
			npu_drv_err("enter power up, work_mode:%d", workmode_idx);
			wm_info.work_mode = workmode_idx;
			ret = npu_pm_vote(dev_ctx, &wm_info);
			cond_goto_error(ret != 0, fail, ret, ret,
				"power up fail: ret = %d, workmode = %d\n", ret, workmode_idx);
		}
	}

	if (dev_ctx->pm.work_mode != 0)
		dev_ctx->pm.power_stage = NPU_PM_UP;

	npu_drv_warn("power up succ, work_mode_set = 0x%x\n",
		dev_ctx->pm.work_mode);
	mutex_unlock(&dev_ctx->pm.npu_power_mutex);
	return ret;
fail:
	mutex_unlock(&dev_ctx->pm.npu_power_mutex);
	return ret;
}

int npu_pm_handle_exception(struct npu_dev_ctx *dev_ctx, struct npu_exception_record* er)
{
	int ret = 0;
	int workmode_idx;
	u32 except_bits;
	u32 goal_work_mode_set;
	struct npu_platform_info *plat_info = npu_plat_get_info();
	unused(er);

	cond_return_error(plat_info == NULL, -1, "plat_info is null\n");

	npu_drv_warn("exception powerdown enter");
	if (plat_info->dts_info.feature_switch[NPU_FEATURE_AUTO_POWER_DOWN] == NPU_FEATURE_OFF) {
		npu_drv_warn("npu auto power down switch off\n");
		return 0;
	}

	npu_pm_delete_idle_timer(dev_ctx);

	mutex_lock(&dev_ctx->pm.npu_power_mutex);
	goal_work_mode_set = dev_ctx->pm.work_mode;
	for (workmode_idx = 0; workmode_idx < NPU_WORKMODE_MAX; workmode_idx++) {
		if (workmode_idx == NPU_SEC)
			continue;
		if (npu_bitmap_get(dev_ctx->pm.work_mode, workmode_idx) != 0) {
			goal_work_mode_set = npu_bitmap_clear(dev_ctx->pm.work_mode, workmode_idx);
			ret = npu_pm_unvote(dev_ctx, (u32)workmode_idx);
			if(ret != 0) {
				npu_drv_err("power down fail: ret = %d, workmode = %d\n",ret, workmode_idx);
				dev_ctx->pm.work_mode = goal_work_mode_set;
			}
		}
	}

	npu_drv_warn("power down succ, work_mode_set = 0x%x\n",
		dev_ctx->pm.work_mode);
	mutex_unlock(&dev_ctx->pm.npu_power_mutex);

	// exception power up
	except_bits = (uint32_t)npu_get_exception_bits();
	if ((except_bits & 0x3) != 0)
		return -1;

	ret = npu_pm_handle_power_up(dev_ctx);

	if (dev_ctx->pm.work_mode == 0) {
		dev_ctx->pm.power_stage = NPU_PM_DOWN;
		atomic_set(&dev_ctx->pm.power_access, 1);
	}
	return ret;
}

static void npu_pm_idle_work(struct work_struct *npu_idle_work)
{
	int ret;
	struct npu_power_mgr *power_mgr = NULL;
	struct npu_dev_ctx *dev_ctx = NULL;
	u32 goal_work_mode_set;
	int work_state;
	u32 i;

	npu_drv_warn("idle timer work enter\n");
	cond_return_void(npu_idle_work == NULL, "idle_work is null\n");
	power_mgr = container_of(npu_idle_work, struct npu_power_mgr,
		idle_work.work);
	cond_return_void(power_mgr == NULL, "power_mgr is null\n");
	dev_ctx = container_of(power_mgr, struct npu_dev_ctx,
		pm);

	mutex_lock(&power_mgr->npu_power_mutex);
	if (is_nsec_wm_cnt_zero(&dev_ctx->pm) == 0) {
		atomic_cmpxchg(&power_mgr->idle_wq_processing, WORK_ADDED, WORK_IDLE);
		mutex_unlock(&power_mgr->npu_power_mutex);
		return;
	}

	work_state = atomic_read(&power_mgr->idle_wq_processing);
	if (work_state == WORK_ADDED) {
		for (i = 0; i < NPU_NOSEC_WORKMODE_NUM; i++) {
			if (npu_bitmap_get(power_mgr->work_mode, nonsec_workmode[i]) != 0) {
				goal_work_mode_set = npu_bitmap_clear(power_mgr->work_mode, nonsec_workmode[i]);
				ret = npu_pm_unvote(dev_ctx, nonsec_workmode[i]);
				cond_goto_error(ret != 0, fail, ret, ret,
					"fail in power down : ret = %d\n", ret);
				npu_drv_warn("npu nonsec exit succ, workmode_set = 0x%x\n",
					goal_work_mode_set);
			} else {
				npu_drv_warn("npu workmode %d already exit, workmode_set = 0x%x\n",
					nonsec_workmode[i], power_mgr->work_mode);
			}
		}

		if (power_mgr->work_mode == 0)
			power_mgr->power_stage = NPU_PM_DOWN;

		atomic_set(&power_mgr->idle_wq_processing, WORK_IDLE);
	} else {
		npu_drv_warn("work state = %d\n", work_state);
	}

	mutex_unlock(&power_mgr->npu_power_mutex);

	return;
fail:
	power_mgr->work_mode = goal_work_mode_set;
	if (power_mgr->work_mode == 0)
		power_mgr->power_stage = NPU_PM_DOWN;
	mutex_unlock(&power_mgr->npu_power_mutex);
	atomic_set(&power_mgr->idle_wq_processing, WORK_IDLE);
}

static int npu_pm_resource_init(struct npu_dev_ctx *dev_ctx)
{
	int ret;
	u32 work_mode;
	struct npd_registry *npd = NULL;
	struct npu_power_mgr *power_mgr = &dev_ctx->pm;

	npd = get_npd();
	cond_return_error(npd == NULL, -1, "npd ptr is NULL\n");
	ret = npd->npd_notify(dev_ctx, NPD_NOTICE_PM_INIT, 0, 0);
	cond_return_error(ret != 0, -1, "npu power manager init failed\n");

	for (work_mode = 0; work_mode < NPU_WORKMODE_MAX; work_mode++)
		power_mgr->wm_cnt[work_mode] = 0;

	return 0;
}

static void npu_pm_delay_work_init(struct npu_power_mgr *power_mgr)
{
	atomic_set(&power_mgr->idle_wq_processing, WORK_IDLE);
	INIT_DELAYED_WORK(&power_mgr->idle_work, npu_pm_idle_work);

	npu_pm_interframe_delay_work_init(&power_mgr->interframe_idle_manager);
}

void npu_pm_adapt_init(struct npu_dev_ctx *dev_ctx)
{
	npu_pm_delay_work_init(&dev_ctx->pm);
	npu_pm_resource_init(dev_ctx);
}

int npu_ctrl_core(u8 dev_id, u32 core_num)
{
	int ret = 0;
	struct npu_platform_info *plat_info = NULL;
	struct npu_dev_ctx *dev_ctx = NULL;

	plat_info = npu_plat_get_info();
	if (plat_info == NULL) {
		npu_drv_err("get plat_ops failed\n");
		return -EINVAL;
	}

	if (dev_id > NPU_DEV_NUM) {
		npu_drv_err("invalid id\n");
		return -EINVAL;
	}

	if ((core_num == 0) || (core_num > plat_info->spec.aicore_max)) {
		npu_drv_err("invalid core num %u\n", core_num);
		return -EINVAL;
	}

	if (plat_info->adapter.res_ops.npu_ctrl_core == NULL) {
		npu_drv_err("do not support ctrl core num %u\n", core_num);
		return -EINVAL;
	}

	dev_ctx = get_dev_ctx_by_id(dev_id);
	if (dev_ctx == NULL) {
		npu_drv_err("get device ctx fail\n");
		return -EINVAL;
	}

	mutex_lock(&dev_ctx->pm.npu_power_mutex);
	if ((dev_ctx->pm.power_stage == NPU_PM_UP) &&
		(npu_bitmap_get(dev_ctx->pm.work_mode, NPU_NONSEC) != 0)) {
		ret = plat_info->adapter.res_ops.npu_ctrl_core(core_num);
		if (ret != 0)
			npu_drv_err("ctrl device core num %u fail ret %d\n",
				core_num, ret);
		else
			npu_drv_warn("ctrl device core num %u success\n", core_num);
	}
	dev_ctx->ctrl_core_num = core_num;
	mutex_unlock(&dev_ctx->pm.npu_power_mutex);

	return ret;
}

// for v300 cpro_cdc
int npu_suspend_powerdown(struct npu_dev_ctx *dev_ctx)
{
	int ret;

	npu_drv_warn("npu suspend powerdown enter\n");
	cond_return_error(dev_ctx == NULL, -1, "dev_ctx is null\n");

	ret = npu_pm_unvote(dev_ctx, NPU_NONSEC);
	cond_return_error(ret != 0, ret, "fail in power down : ret = %d\n", ret);

	if (dev_ctx->pm.work_mode == 0) {
		npu_drv_warn("npu suspend powerdown succ\n");
		dev_ctx->pm.power_stage = NPU_PM_DOWN;
		return 0;
	}

	return -1;
}
