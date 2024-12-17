/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 * This file is the dvfs implementation of the PM module.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */
#include "hvgr_pm_gpu_reset.h"

#include <linux/wait.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/hrtimer.h>
#include <linux/workqueue.h>
#include <securec.h>

#ifdef CONFIG_DFX_BB
#include <linux/version.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
#include <rdr_platform.h>
#else
#include <platform_include/basicplatform/linux/rdr_platform.h>
#endif
#endif

#include "hvgr_dm_api.h"
#include "hvgr_pm_api.h"
#include "hvgr_mem_api.h"
#include "hvgr_debugfs_api.h"
#include "hvgr_asid_api.h"
#include "hvgr_pm_driver.h"
#include "hvgr_pm_dvfs.h"
#include "hvgr_log_api.h"
#include "hvgr_datan_api.h"
#include "hvgr_mmu_api.h"
#include "hvgr_protect_mode.h"
#include "hvgr_platform_api.h"
#include "hvgr_sch_api.h"

#define HVGR_MAX_GPU_DELAY_TIME_MS    600000
static bool hvgr_pm_wait_gpu_soft_reset_end(struct hvgr_device *gdev)
{
	unsigned long time_out;
	unsigned long wait_time;
	int ret;
	struct hvgr_pm_reset *pm_reset = &gdev->pm_dev.pm_reset;

	time_out = jiffies + msecs_to_jiffies(HVGR_PM_GPU_RESET_TIMEOUT);
	do {
		wait_time = time_out - jiffies;
		ret = (int)wait_event_interruptible_timeout(
			pm_reset->reset_done_wait,
			atomic_read(&pm_reset->reset_status) == GPU_RESET_SOFT_RESET_DONE,
			(long)wait_time);
		if (ret == -ERESTARTSYS)
			udelay(20); /* delay 20 us */
	} while ((ret == -ERESTARTSYS) && time_before(jiffies, time_out));
	if (ret <= 0) {
		hvgr_err(gdev, HVGR_PM,
			"Gpu reset time out, ret = 0x%x", ret);
		return false;
	}
	return true;
}

static bool hvgr_pm_is_need_reset(struct hvgr_device *gdev,
	enum hvgr_pm_gpu_reset_type reset_type)
{
	struct hvgr_pm_reset *pm_reset = &gdev->pm_dev.pm_reset;

	/*
	 * If the reset type is immediately, no delay is required. Therefore, we need to refresh
	 * the GPU reset status once.
	 */
	if (reset_type == GPU_RESET_TYPE_IMMEDIATELY &&
		atomic_read(&pm_reset->reset_status) <= GPU_RESET_JOB_STOP_DONE)
		return true;

	/*
	 * If the reset type is not immediately and other thread is already resetting the gpu,
	 * we do not need to reset gpu again.
	 */
	if (atomic_read(&pm_reset->reset_status) != GPU_RESET_END) {
		hvgr_info(gdev, HVGR_PM, "Gpu reset is pending, no need to reset again.");
		return false;
	}
	return true;
}

enum hrtimer_restart hvgr_pm_gpu_reset_callback(struct hrtimer *timer)
{
	struct hvgr_pm_reset *pm_reset = NULL;

	pm_reset = container_of(timer, struct hvgr_pm_reset,
		reset_timer);
	if (unlikely(pm_reset == NULL))
		return HRTIMER_NORESTART;

	(void)queue_work(pm_reset->reset_workq, &pm_reset->reset_work);
	return HRTIMER_NORESTART;
}

static void hvgr_pm_gpu_reset_begin_proc(struct hvgr_device *gdev)
{
	struct hvgr_pm_reset *pm_reset = &gdev->pm_dev.pm_reset;

	if (gdev->cq_dev.protect_mode_info.protected_mode && hvgr_get_prot_enhance(gdev)) {
		/* protect_mode fault should del timer at once */
		struct hvgr_cq_dev * const gcqctx = &gdev->cq_dev;
		if (gcqctx->protect_mode_info.running_queue != NULL)
			(void)del_timer(&gcqctx->protect_mode_info.running_queue->backup_timer_protect_mode);

		hvgr_protect_mode_disable_interrupts(gdev);
	}

	if (!gdev->cq_dev.protect_mode_info.protected_mode &&
		!gdev->cq_dev.protect_mode_info.protected_mode_transition) {
		hvgr_info(gdev, HVGR_CQ, "%s submit_allow=%d,sch_switch_kref=%d.\n", __func__,
			gdev->cq_dev.submit_allow, atomic_read(&gdev->cq_dev.sch_switch_kref));
		hvgr_sch_stop(gdev);
	}
	hvgr_sch_reset(gdev, HVGR_SCH_SOFT_CLEAR);
	if (atomic_read(&pm_reset->reset_type) == GPU_RESET_TYPE_NORMAL)
		atomic_set(&pm_reset->reset_status, GPU_RESET_JOB_STOP);
	else
		atomic_set(&pm_reset->reset_status, GPU_RESET_JOB_STOP_DONE);
}

static void hvgr_pm_gpu_reset_job_stop_proc(struct hvgr_device *gdev)
{
	struct hvgr_pm_reset *pm_reset = &gdev->pm_dev.pm_reset;

	/* soft stop job. */
	hvgr_sch_reset(gdev, HVGR_SCH_SOFT_STOP);
	/* start timer */
	hrtimer_start(&pm_reset->reset_timer,
		ns_to_ktime((uint64_t)pm_reset->reset_delay_time * 1000000U),
		HRTIMER_MODE_REL);
	atomic_set(&pm_reset->reset_status, GPU_RESET_JOB_STOP_DONE);
}

static void hvgr_pm_job_stop_done_proc(struct hvgr_device *gdev)
{
	struct hvgr_pm_reset *pm_reset = &gdev->pm_dev.pm_reset;

	(void)hrtimer_cancel(&pm_reset->reset_timer);
	if (hvgr_pm_request_gpu(gdev, true) == false) {
		/*
		 * This would re-activate the GPU. Since it's already idle,
		 * there's no need to reset it
		 */
		hvgr_pm_release_gpu(gdev);
		atomic_set(&pm_reset->reset_status, GPU_RESET_END);
		return;
	}

	hvgr_disable_interrupts(gdev);
	hvgr_synchronize_irqs(gdev);
	hvgr_flush_mmu_faults(gdev);

	/* Not protect mode reset, dump debug bus */
	if (atomic_read(&pm_reset->reset_type) != GPU_RESET_TYPE_EXIT_PROTECT_MODE) {
		hvgr_dump_registers(gdev);
		hvgr_dmd_msg_set(gdev, gdev->datan_dev.gpu_fault_msg.gpu_fault_id,
			gdev->datan_dev.gpu_fault_msg.gpu_fault_log);
		hvgr_datan_dump_debug_bus(gdev);
		hvgr_debug(gdev, HVGR_PM, "Dump debugbus done.");
		hvgr_dmd_msg_report(gdev);
		hvgr_datan_gpu_fault_type_reset(gdev);
	}

	/* flush l2 */
	hvgr_mmu_flush_l2_caches(gdev);
	hvgr_debug(gdev, HVGR_PM, "Flush L2 done.");
	/* clear err task */
	hvgr_sch_reset(gdev, HVGR_SCH_SOFT_CLEAR_GPU_TASK);
	/* update gpu buys/idle time */
	hvgr_pm_dvfs_gpu_reset_proc(gdev);
	atomic_set(&pm_reset->reset_status, GPU_RESET_RESET_SUBMMIT);
}

static void hvgr_pm_reset_submmit_proc(struct hvgr_device *gdev)
{
	struct hvgr_pm_reset *pm_reset = &gdev->pm_dev.pm_reset;
	struct hvgr_pm_pwr *pm_pwr = &gdev->pm_dev.pm_pwr;

	mutex_lock(&gdev->pm_dev.pm_pwr.power_lock);
	hvgr_datan_ct_disable_by_power(gdev, false);
	/* Portect mode reset, need to reset core before soft reset */
	if (atomic_read(&pm_reset->reset_type) == GPU_RESET_TYPE_EXIT_PROTECT_MODE) {
		hvgr_pm_driver_reset_gpc_and_bl(gdev);
	} else {
		/* Soft reset gpu */
		if (pm_pwr->cur_status == HVGR_PM_GPC_POWERING_OFF) {
			if (!hvgr_pm_wait_gpc_poweroff_ready(gdev)) {
				hvgr_err(gdev, HVGR_PM, "%s wait gpc poweroff fail", __func__);
				goto err;
			}
		}
		if (!hvgr_hw_has_issue(gdev, HVGR_HW_ISSUE_50210)) {
			hvgr_pm_driver_gpu_soft_reset(gdev);
			if (hvgr_pm_wait_gpu_soft_reset_end(gdev) == false)
				hvgr_pm_driver_gpu_reset_timeout_proc(gdev);
		}
	}

	(void)hvgr_crg_reset(gdev);

	if (!hvgr_pm_driver_gpu_config(gdev))
		goto err;
	if (!hvgr_pm_driver_unreset_cores(gdev))
		goto err;

	hvgr_pm_driver_re_enable_cycle_counter(gdev);
	hvgr_sch_set_mode(gdev, HVGR_SCH_MODE_CQ, HVGR_SCH_SUBMIT_IGNORE);
	hvgr_enable_interrupts(gdev);

	pm_pwr->cur_status = HVGR_PM_GPU_READY;
	atomic_set(&pm_reset->reset_status, GPU_RESET_RESET_DONE);
	hvgr_datan_ct_enable_by_power(gdev);
	mutex_unlock(&gdev->pm_dev.pm_pwr.power_lock);
	return;
err:
	pm_pwr->cur_status = HVGR_PM_CORE_ERROR;
	atomic_set(&pm_reset->reset_status, GPU_RESET_RESET_ERROR);
	mutex_unlock(&gdev->pm_dev.pm_pwr.power_lock);
}

static bool hvgr_pm_protect_mode_re_enter(struct hvgr_device *gdev)
{
	struct hvgr_cq_dev * const gcqctx = &gdev->cq_dev;
	int i;

	hvgr_info(gdev, HVGR_CQ, "%s in.", __func__);
	if (gcqctx->protect_mode_info.bak_running_queue[0] == NULL)
		return false;

	hvgr_info(gdev, HVGR_CQ, "%s will re-enter protect mode.", __func__);

	mutex_lock(&gcqctx->submit_lock);
	if (atomic_cmpxchg(&gcqctx->pm_pwr_flag, 0, 1) == 0) {
		(void)hvgr_pm_request_gpu(gdev, false);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
		flush_workqueue(gcqctx->sched_pm_wq);
#else
		kthread_flush_work(&gcqctx->sched_pm_work);
#endif
	}
	mutex_unlock(&gcqctx->submit_lock);

	/* Re-enter protect mode like get hold-en In. */
	gcqctx->protect_mode_info.protected_mode_transition = true;
	gcqctx->protect_mode_info.protected_mode = false;
	gcqctx->protect_mode_info.protected_mode_refcount--;
	gcqctx->protect_mode_info.protected_state.enter = HVGR_CQ_ENTER_PROTECTED_WAIT_IDLE;

	hvgr_info(gdev, HVGR_CQ, "%s refcount=%d.", __func__,
		gcqctx->protect_mode_info.protected_mode_refcount);

	for (i = (int)ARRAY_SIZE(gcqctx->protect_mode_info.bak_running_queue) - 1; i >= 0; i--)
		if (gcqctx->protect_mode_info.bak_running_queue[i] != NULL) {
			gcqctx->protect_mode_info.running_queue =
				gcqctx->protect_mode_info.bak_running_queue[i];
			gcqctx->protect_mode_info.bak_running_queue[i] = NULL;
			break;
		}

	hvgr_cq_protect_mode_enter(gcqctx->protect_mode_info.running_queue, gdev, 0);
	return true;
}

static void hvgr_pm_gpu_reset_done_proc(struct hvgr_device *gdev)
{
	struct hvgr_pm_reset *pm_reset = &gdev->pm_dev.pm_reset;
	struct hvgr_cq_dev * const gcqctx = &gdev->cq_dev;

	if (gcqctx->protect_mode_info.protected_mode && hvgr_get_prot_enhance(gdev)) {
		/* protect_mode exit TZPC before clear protect_mode_info */
		hvgr_info(gdev, HVGR_PM, "%s will exit TZPC!", __func__);
		(void)hvgr_platform_protected_cfg(gdev, 1);
	}

	/* If there is no bak running queue, clear it. */
	if (gcqctx->protect_mode_info.bak_running_queue[0] == NULL)
		(void)memset_s(&gcqctx->protect_mode_info, sizeof(gcqctx->protect_mode_info), 0,
			sizeof(gcqctx->protect_mode_info));

	hvgr_pm_release_gpu(gdev);
	hvgr_dmd_msg_clear(gdev);
	atomic_set(&pm_reset->reset_status, GPU_RESET_END);
	wake_up(&gdev->cq_dev.stop_queue_done_wait);

	hvgr_info(gdev, HVGR_CQ, "%s submit_allow=%d,sch_switch_kref=%d.\n", __func__,
		gdev->cq_dev.submit_allow, atomic_read(&gdev->cq_dev.sch_switch_kref));

	if(!hvgr_pm_protect_mode_re_enter(gdev))
		hvgr_sch_start(gdev);
}

static inline void hvgr_pm_gpu_reset_error_proc(struct hvgr_device *gdev)
{
	struct hvgr_pm_reset *pm_reset = &gdev->pm_dev.pm_reset;

	/* Re start timer, after 2.5s, try to reset gpu again. */
	hrtimer_start(&pm_reset->reset_timer,
		ns_to_ktime((uint64_t)pm_reset->reset_delay_time * 1000000U),
		HRTIMER_MODE_REL);

	atomic_set(&pm_reset->reset_status, GPU_RESET_RESET_SUBMMIT);
}

static void hvgr_pm_update_gpu_reset_machine(struct hvgr_device *gdev)
{
	int reset_status;
	struct hvgr_pm_reset *pm_reset = &gdev->pm_dev.pm_reset;

	do {
		reset_status = atomic_read(&pm_reset->reset_status);
		switch (reset_status) {
		case GPU_RESET_BEGIN:
			hvgr_pm_gpu_reset_begin_proc(gdev);
			break;
		case GPU_RESET_JOB_STOP:
			hvgr_pm_gpu_reset_job_stop_proc(gdev);
			return;
		case GPU_RESET_JOB_STOP_DONE:
			hvgr_pm_job_stop_done_proc(gdev);
			break;
		case GPU_RESET_RESET_SUBMMIT:
			hvgr_pm_reset_submmit_proc(gdev);
			break;
		case GPU_RESET_RESET_DONE:
			hvgr_pm_gpu_reset_done_proc(gdev);
			break;
		case GPU_RESET_END:
			/* Gpu reset end */
			hvgr_debug(gdev, HVGR_PM, "Reset gpu done.");
			break;
		case GPU_RESET_RESET_ERROR:
			hvgr_pm_gpu_reset_error_proc(gdev);
			return;
		default:
			hvgr_err(gdev, HVGR_PM, "Reset status is invalidated");
		}
	} while (reset_status != GPU_RESET_END);
}

static void hvgr_pm_gpu_reset_wq(struct work_struct *data)
{
	struct hvgr_device *gdev = NULL;

	gdev = container_of(data, struct hvgr_device,
		pm_dev.pm_reset.reset_work);
	if (unlikely(gdev == NULL))
		return;
	hvgr_pm_update_gpu_reset_machine(gdev);
}

void hvgr_pm_gpu_reset(struct hvgr_device *gdev, enum hvgr_pm_gpu_reset_type reset_type)
{
	struct hvgr_pm_reset *pm_reset = &gdev->pm_dev.pm_reset;

#if defined(CONFIG_LP_ENABLE_HPM_DATA_COLLECT) && defined(CONFIG_DFX_BB)
		/* benchmark data collect */
		rdr_syserr_process_for_ap((u32)MODID_AP_S_PANIC_GPU,
			0ull, 0ull);
#endif
	if (hvgr_pm_is_need_reset(gdev, reset_type) == false)
		return;

	if (reset_type == GPU_RESET_TYPE_EXIT_PROTECT_MODE)
		hvgr_info(gdev, HVGR_PM, "Silent reset GPU\n");
	else
		hvgr_debug(gdev, HVGR_PM, "Sending reset to GPU - all running jobs will be lost\n");
	atomic_set(&pm_reset->reset_type, (int)reset_type);
	(void)atomic_cmpxchg(&pm_reset->reset_status, GPU_RESET_END, GPU_RESET_BEGIN);
	(void)queue_work(pm_reset->reset_workq, &pm_reset->reset_work);
}

void hvgr_pm_reset_done(struct hvgr_device *gdev)
{
	struct hvgr_pm_reset *pm_reset = &gdev->pm_dev.pm_reset;

	atomic_set(&pm_reset->reset_status, GPU_RESET_SOFT_RESET_DONE);
	wake_up(&pm_reset->reset_done_wait);
}

ssize_t hvgr_pm_reset_timeout_read(void *data, char * const buf)
{
	struct hvgr_device *gdev = (struct hvgr_device *)data;
	ssize_t ret;

	ret = scnprintf(buf, PAGE_SIZE, "Reset delay time: %d ms\n",
		gdev->pm_dev.pm_reset.reset_delay_time);
	return ret;
}

ssize_t hvgr_pm_reset_timeout_write(void *data, char * const buf,
	size_t count)
{
	struct hvgr_device *gdev = (struct hvgr_device *)data;
	uint32_t delay_time = 0;

	if (kstrtouint(buf, 0, &delay_time) != 0)
		return -EFAULT;

	if (delay_time == 0 || delay_time > HVGR_MAX_GPU_DELAY_TIME_MS) {
		hvgr_err(gdev, HVGR_PM, "Input delay time is invalided, delay_time = %d",
			delay_time);
		return -EINVAL;
	}

	gdev->pm_dev.pm_reset.reset_delay_time = delay_time;
	return (ssize_t)count;
}

hvgr_debugfs_read_write_declare(fops_reset_timeout, hvgr_pm_reset_timeout_read,
	hvgr_pm_reset_timeout_write);

int hvgr_pm_gpu_reset_soft_init(struct hvgr_device *gdev)
{
	struct workqueue_struct *wq = NULL;
	struct hvgr_pm_reset *pm_reset = &gdev->pm_dev.pm_reset;

	init_waitqueue_head(&pm_reset->reset_done_wait);

	atomic_set(&pm_reset->reset_status, GPU_RESET_END);

	wq = alloc_workqueue("hvgr_pm_gpureset", WQ_HIGHPRI | WQ_UNBOUND, 1);
	if (wq == NULL)
		return -ENOMEM;

	pm_reset->reset_workq = wq;
	INIT_WORK(&pm_reset->reset_work, hvgr_pm_gpu_reset_wq);
	hrtimer_init(&pm_reset->reset_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	pm_reset->reset_timer.function = hvgr_pm_gpu_reset_callback;
	pm_reset->reset_delay_time = HVGR_PM_GPU_RESET_DELAY_TIME_MS;
#ifdef CONFIG_DFX_DEBUG_FS
	hvgr_debugfs_register(gdev, "reset_timeout", gdev->pm_dev.pm_dir, &fops_reset_timeout);
#endif
	return 0;
}
