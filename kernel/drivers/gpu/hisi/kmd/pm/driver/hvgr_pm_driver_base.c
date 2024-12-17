/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 * This file is the dvfs implementation of the PM module.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#include <linux/pm_runtime.h>
#include <linux/regulator/consumer.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/version.h>
#include <linux/bitops.h>

#include "hvgr_dm_api.h"
#include "hvgr_pm_api.h"
#include "hvgr_mem_api.h"
#include "hvgr_platform_api.h"
#include "hvgr_regmap.h"
#include "hvgr_pm_dfx.h"
#include "hvgr_log_api.h"
#include "hvgr_pm_defs.h"
#include "hvgr_pm_policy.h"
#include "hvgr_datan_api.h"
#include "hvgr_sch_api.h"
#include "hvgr_pm_driver.h"
#include "hvgr_pm_driver_adapt_defs.h"
#include "hvgr_dm_ctx.h"
#include "hvgr_pm_driver_base.h"

enum core_status {
	PM_CORE_POWERED_OFF,
	PM_CORE_POWERING_ON,
	PM_CORE_POWERING_OFF,
	PM_CORE_POWERED_ON,
};

#define PM_CORE_STATUS_MASK          0x3U
#define HVGR_PM_GPU_FLUSH_TIMEOUT    500U

#define PM_SCHEDULE_SUSPEND_INTERVAL 50
#define PM_SCHEDULE_SUSPEND_RETRY_TIMES 20

#define HVGR_PM_TRANS_DELAY_TIME 50
#define HVGR_PM_TRANS_RETRY_TIMES 100
#define HVGR_PM_POWER_MAX_RETRY_TIMES 3

bool hvgr_pm_driver_trans_time_out_proc(struct hvgr_device *gdev,
	uint32_t cores, uint32_t trans_reg, hvgr_pm_is_core_trans_end pfunc)
{
	int timeout = HVGR_PM_TRANS_RETRY_TIMES;
	uint32_t core_status;

	do {
		core_status = hvgr_read_reg(gdev, gpu_control_reg(gdev, trans_reg));
		if (pfunc != NULL && pfunc(cores, core_status))
			return true;

		msleep(HVGR_PM_TRANS_DELAY_TIME);
	} while (--timeout > 0);

	hvgr_err(gdev, HVGR_PM,
		"After 5s, Transion core time out [0x%x, 0x%x, 0x%x ].",
		cores, core_status, trans_reg);
	hvgr_pm_dump_reg(gdev, HVGR_LOG_ERROR);
	return false;
}

bool hvgr_pm_driver_wait_core_trans_end(struct hvgr_device *gdev,
	uint32_t cores, uint32_t trans_reg, hvgr_pm_is_core_trans_end pfunc)
{
	int timeout = HVGR_PM_CORE_TRANS_OUT_TIME;
	uint32_t core_status;

	do {
		core_status = hvgr_read_reg(gdev, gpu_control_reg(gdev, trans_reg));
		if (pfunc != NULL && pfunc(cores, core_status)) {
			hvgr_info(gdev, HVGR_PM,
				"Transion core time, [ %d, 0x%x, 0x%x, 0x%x ]",
				HVGR_PM_CORE_TRANS_OUT_TIME - timeout,
				cores, core_status, trans_reg);
			return true;
		}

		udelay(1);
	} while (--timeout > 0);

	hvgr_err(gdev, HVGR_PM,
		"Transion core time out [0x%x, 0x%x, 0x%x ].",
		cores, core_status, trans_reg);
	hvgr_pm_dump_reg(gdev, HVGR_LOG_ERROR);
	return hvgr_pm_driver_trans_time_out_proc(gdev, cores,
		trans_reg, pfunc);
}

void hvgr_pm_config_override(struct hvgr_device *gdev)
{
	hvgr_write_reg(gdev, gpu_control_reg(gdev, PWR_KEY), HVGR_PWR_KEY_VALUE);
	hvgr_write_reg(gdev, gpu_control_reg(gdev, PWR_OVERRIDE0), HVGR_PWR_TRANS_NUM_VALUE);
}

static void hvgr_pm_driver_platform_on(struct hvgr_device *gdev)
{
	uint64_t start_time;
	struct hvgr_pm_pwr *pm_pwr = &gdev->pm_dev.pm_pwr;
	int try_times = 0;
	int ret;

	if (unlikely(pm_pwr->regulator == NULL) ||
		unlikely(gdev->subsys_ops->regulator_enable == NULL))
		return;

	start_time = hvgr_start_timing();
	do {
		ret = gdev->subsys_ops->regulator_enable(pm_pwr->regulator);
		try_times++;
		if (unlikely(ret))
			hvgr_err(gdev, HVGR_PM, "Failed to enable regulator , try_times = %d.",
				try_times);
	} while (unlikely(ret) && try_times < HVGR_PM_POWER_MAX_RETRY_TIMES);
	hvgr_end_timing_and_log(gdev, HVGR_PM, start_time, "Regulator enable");
	if (unlikely(ret)) {
		hvgr_err(gdev, HVGR_PM, "Driver Buck on failed.");
		WARN_ON(1);
	} else {
		hvgr_info(gdev, HVGR_PM, "Driver Buck on succeeded.");
	}

	start_time = hvgr_start_timing();

#ifdef CONFIG_HVGR_VIRTUAL_HOST
	(void)hvgr_platform_smmu_bypass(gdev, HVGR_PF_P1);
	if (unlikely(hvgr_platform_smmu_tcu_on(gdev) != 0))
		hvgr_err(gdev, HVGR_PM, "driver platform on:smmu tcu on failed.");
	(void)hvgr_platform_sec_cfg(gdev, HVGR_PF_P1);
	(void)hvgr_platform_set_qos(gdev, HVGR_PF_P1);
#else
	(void)hvgr_platform_smmu_bypass(gdev, HVGR_PF_P0);
	if (unlikely(hvgr_platform_smmu_tcu_on(gdev) != 0))
		hvgr_err(gdev, HVGR_PM, "driver platform on:smmu tcu on failed.");
	hvgr_platform_enable_ecc(gdev);
	if (unlikely(hvgr_platform_smmu_tbu_connect(gdev) != 0))
		hvgr_err(gdev, HVGR_PM, "driver platform on:smmu tbu connect failed.");
	(void)hvgr_platform_sec_cfg(gdev, HVGR_PF_P0);
#ifdef CONFIG_HVGR_VIRTUAL_GUEST
	(void)hvgr_platform_set_qos(gdev, HVGR_PF_P0);
#else
	(void)hvgr_platform_set_qos(gdev, HVGR_PF_PALL);
#endif
#endif

#ifdef HVGR_FEATURE_SYSTEM_CACHE
	hvgr_sc_config_remap_register(gdev);
#endif

	if (likely(gdev->subsys_ops->update_devfreq_status != NULL))
		gdev->subsys_ops->update_devfreq_status(HVGR_DEVFREQ_RESUME);

	hvgr_end_timing_and_log(gdev, HVGR_PM, start_time, "Init tbu");
}

static void hvgr_pm_driver_platform_off(struct hvgr_device *gdev)
{
	uint64_t start_time;
	struct hvgr_pm_pwr *pm_pwr = &gdev->pm_dev.pm_pwr;
	int try_times = 0;
	int ret;

	if (unlikely(pm_pwr->regulator == NULL) ||
		unlikely(gdev->subsys_ops->regulator_disable == NULL))
		return;

	start_time = hvgr_start_timing();

	if (likely(gdev->subsys_ops->update_devfreq_status != NULL))
		gdev->subsys_ops->update_devfreq_status(HVGR_DEVFREQ_SUSPEND);

	if (unlikely(hvgr_platform_smmu_tbu_disconnect(gdev) != 0))
		hvgr_err(gdev, HVGR_PM, "platform off:smmu tbu disconnect failed.");
	if (unlikely(hvgr_platform_smmu_tcu_off(gdev) != 0))
		hvgr_err(gdev, HVGR_PM, "platform off:smmu tcu off failed.");

	do {
		ret = gdev->subsys_ops->regulator_disable(pm_pwr->regulator);
		try_times++;
		if (unlikely(ret))
			hvgr_err(gdev, HVGR_PM, "Failed to disable regulator , try_times = %d.",
				try_times);
	} while (unlikely(ret) && try_times < HVGR_PM_POWER_MAX_RETRY_TIMES);
	hvgr_end_timing_and_log(gdev, HVGR_PM, start_time, "Regulator disable");
	if (unlikely(ret)) {
		hvgr_err(gdev, HVGR_PM, "Driver Buck off failed.");
	} else {
		hvgr_info(gdev, HVGR_PM, "Driver Buck off succeeded.");
	}
}

#ifdef CONFIG_HVGR_GPU_RT_PM
int hvgr_pm_driver_runtime_suspend(struct hvgr_device *gdev)
{
	hvgr_pm_driver_platform_off(gdev);
	return 0;
}

int hvgr_pm_driver_runtime_resume(struct hvgr_device *gdev)
{
	hvgr_pm_driver_platform_on(gdev);
	return 0;
}

int hvgr_pm_driver_runtime_idle(struct hvgr_device *gdev)
{
	return 1;
}

void hvgr_pm_driver_runtime_init(struct hvgr_device *gdev)
{
	pm_suspend_ignore_children(gdev->dev, true);
	pm_runtime_enable(gdev->dev);
}

void hvgr_pm_driver_runtime_term(struct hvgr_device *gdev)
{
	pm_runtime_disable(gdev->dev);
}
#endif

void hvgr_pm_driver_buck_on(struct hvgr_device *gdev)
{
#ifdef CONFIG_HVGR_GPU_RT_PM
	int ret;
	struct device *dev = gdev->dev;
	struct hvgr_pm_pwr *pm_pwr = &gdev->pm_dev.pm_pwr;

	hvgr_info(gdev, HVGR_PM, "%s Enter.", __func__);

	if (pm_pwr->powered_off_directly) {
		pm_pwr->powered_off_directly = false;
		hvgr_info(gdev, HVGR_PM,
			"%s Gpu is not runtime suspend.", __func__);
		hvgr_pm_driver_runtime_resume(gdev);
		return;
	}

	if (unlikely(dev->power.disable_depth > 0)) {
		hvgr_info(gdev, HVGR_PM,
			"%s Run time is not enable.", __func__);
		hvgr_pm_driver_platform_on(gdev);
		return;
	}
	ret = pm_runtime_resume(dev);
	if (unlikely(ret == -EAGAIN)) {
		hvgr_info(gdev, HVGR_PM,
			"%s runtime resume failed and buck on dricetly.",
			__func__);
		hvgr_pm_driver_platform_on(gdev);
		return;
	}

	if (unlikely(ret < 0)) {
		hvgr_err(gdev, HVGR_PM,
			"%s runtime resume failed %d", __func__, ret);
		return;
	}
	hvgr_info(gdev, HVGR_PM, "%s Run time resume succeeded.", __func__);
	return;
#else
	hvgr_pm_driver_platform_on(gdev);
	return;
#endif
}

#ifdef CONFIG_HVGR_GPU_RT_PM
static int hvgr_pm_driver_do_buck_off(struct hvgr_device *gdev)
{
	int ret = 0;
	struct device *dev = gdev->dev;
	struct hvgr_pm_pwr *pm_pwr = &gdev->pm_dev.pm_pwr;

	ret = pm_runtime_suspend(dev);
	if (ret == -EAGAIN) {
#if defined(HVGR_USE_PCIE) && HVGR_USE_PCIE
		hvgr_info(gdev, HVGR_PM,
			"%s schedule suspend failed and pci-e do not needed to try again.",
			__func__);
		return 0;
#endif
		/*
		 * If suspend or resume is not being processed, Call
		 * pm_runtime_suspend to power off the gpu again.
		 * Therefore, -EAGAIN needs to be returned.
		 */
		if (atomic_read(&pm_pwr->sr_state) ==
			HVGR_PM_ACTIVE) {
			hvgr_info(gdev, HVGR_PM,
				"%s schedule suspend failed and try again.",
				__func__);
			msleep(PM_SCHEDULE_SUSPEND_INTERVAL);
			return -EAGAIN;
		}

		hvgr_info(gdev, HVGR_PM,
			"%s Enter suspending.", __func__);
		if (pm_runtime_status_suspended(gdev->dev)) {
			hvgr_info(gdev, HVGR_PM,
				"%s The GPU is powered off when the GPU suspend.",
				__func__);
			return 0;
		}
		pm_pwr->powered_off_directly = true;
		/*
		 * If the buck is powered off during suspend or resume,
		 * pm_runtime_suspend returns -EAGAIN. The
		 * hvgr_pm_driver_runtime_suspend needs to be called to
		 * directly power off the buck. You do not need to call
		 * pm_runtime_suspend again.
		 */
		hvgr_pm_driver_runtime_suspend(gdev);
		hvgr_info(gdev, HVGR_PM,
			"%s gpu powered off succeeded.", __func__);
		return 0;
	}

	if (unlikely(ret < 0)) {
		hvgr_err(gdev, HVGR_PM,
			"schedule suspend failed %d", ret);
		WARN_ON(1);
		return ret;
	}

	return 0;
}
#endif

void hvgr_pm_driver_buck_off(struct hvgr_device *gdev)
{
#ifdef CONFIG_HVGR_GPU_RT_PM
	struct device *dev = gdev->dev;
	int ret = 0;
	int retry = 0;

	if (unlikely(dev->power.disable_depth > 0)) {
		hvgr_info(gdev, HVGR_PM,
			"%s Run time is not enable.", __func__);
		hvgr_pm_driver_platform_off(gdev);
		return;
	}

	do {
		ret = hvgr_pm_driver_do_buck_off(gdev);
		if (ret != -EAGAIN)
			return;
	} while (++retry < PM_SCHEDULE_SUSPEND_RETRY_TIMES);

	hvgr_err(gdev, HVGR_PM, "%s Run time suspend time out.", __func__);
#else
	hvgr_pm_driver_platform_off(gdev);
#endif
}

int hvgr_pm_driver_soft_init(struct hvgr_device *gdev)
{
	struct hvgr_pm_pwr *pm_pwr = &gdev->pm_dev.pm_pwr;

	pm_pwr->regulator = devm_regulator_get(gdev->dev, "gpu");
	if (unlikely(IS_ERR(pm_pwr->regulator))) {
		hvgr_err(gdev, HVGR_PM, "Failed to get regulator\n");
		return -EINVAL;
	}

	return 0;
}

void hvgr_pm_driver_soft_term(struct hvgr_device *gdev)
{
	struct hvgr_pm_pwr *pm_pwr = &gdev->pm_dev.pm_pwr;

	if (likely(pm_pwr->regulator)) {
		devm_regulator_put(pm_pwr->regulator);
		pm_pwr->regulator = NULL;
	}
}

void hvgr_pm_driver_request_cycle_counter(struct hvgr_device *gdev)
{
	hvgr_info(gdev, HVGR_PM, "%s enter ref = %d.",
		__func__, atomic_read(&gdev->pm_dev.cycle_counter_ref));
	if (atomic_inc_return(&gdev->pm_dev.cycle_counter_ref) != 1)
		return;
	hvgr_info(gdev, HVGR_PM, "Cycle counter enable.");
	hvgr_write_reg(gdev, gpu_control_reg(gdev, GPU_COMMAND),
		GPU_COMMAND_CYCLE_COUNT_START);
}

void hvgr_pm_driver_release_cycle_counter(struct hvgr_device *gdev)
{
	hvgr_info(gdev, HVGR_PM, "%s enter ref = %d.",
		__func__, atomic_read(&gdev->pm_dev.cycle_counter_ref));
	if (atomic_dec_return(&gdev->pm_dev.cycle_counter_ref) != 0)
		return;
	hvgr_info(gdev, HVGR_PM, "Cycle counter disable.");
	hvgr_write_reg(gdev, gpu_control_reg(gdev, GPU_COMMAND),
		GPU_COMMAND_CYCLE_COUNT_STOP);
}

void hvgr_pm_driver_re_enable_cycle_counter(struct hvgr_device *gdev)
{
	if (atomic_read(&gdev->pm_dev.cycle_counter_ref) == 0)
		return;

	hvgr_write_reg(gdev, gpu_control_reg(gdev, GPU_COMMAND),
		GPU_COMMAND_CYCLE_COUNT_START);
}

void hvgr_pm_driver_gpu_reset_timeout_proc(struct hvgr_device *gdev)
{
	hvgr_dmd_msg_set(gdev, DMD_HARD_RESET, "Higpu hard reset failed");
	hvgr_dmd_msg_report(gdev);
}

void hvgr_pm_driver_clock_off_pre_proc(struct hvgr_device *gdev)
{
	uint32_t gpu_status;

	hvgr_synchronize_irqs(gdev);
	hvgr_flush_mmu_faults(gdev);
	gpu_status = hvgr_read_reg(gdev, gpu_control_reg(gdev, GPU_STATUS));
	if (gpu_status != 0)
		hvgr_info(gdev, HVGR_PM, "%s , gpu_status = 0x%x",
			__func__, gpu_status);
}

bool hvgr_pm_driver_update_gpc_status(struct hvgr_device *gdev, uint32_t mask, uint32_t pwrcmd,
	hvgr_pm_is_core_trans_end pfunc)
{
	struct hvgr_pm_pwr *pm_pwr = &gdev->pm_dev.pm_pwr;
	uint32_t cores_nums = (uint32_t)fls(pm_pwr->present_cores[HVGR_PM_CORE_GPC]);
	uint32_t cycle_count;
	uint32_t index;
	uint32_t cores;

	if (unlikely(cores_nums == 0)) {
		hvgr_err(gdev, HVGR_PM, "get cores num is zero.");
		return false;
	}

	cycle_count = ((cores_nums - 1) / HVGR_MAX_GPC_NUM_POWER_UPDATE_ONCE) + 1;
	hvgr_info(gdev, HVGR_PM, "%s GPC status mask=0x%x cores_nums=%d cycle_count=%u.",
		(pwrcmd == GPC_PWRON_CMD) ? "power on" : "power off",
		mask, cores_nums, cycle_count);

	for (index = 0; index < cycle_count; ++index) {
		cores = (((1 << HVGR_MAX_GPC_NUM_POWER_UPDATE_ONCE) - 1) <<
			(index * HVGR_MAX_GPC_NUM_POWER_UPDATE_ONCE)) & mask;
		if (cores == 0)
			continue;

		hvgr_info(gdev, HVGR_PM, "%s GPC status index=%u cores=0x%x.",
			(pwrcmd == GPC_PWRON_CMD) ? "power on" : "power off", index, cores);
		hvgr_write_reg(gdev, gpu_control_reg(gdev, pwrcmd), cores);
		if (unlikely(!hvgr_pm_driver_wait_core_trans_end(
			gdev, cores, GPC_PWR_STATUS, pfunc))) {
			hvgr_err(gdev, HVGR_PM, "%s GPC status failed.",
				(pwrcmd == GPC_PWRON_CMD) ? "power on" : "power off");
			return false;
		}
	}

	return true;
}

static irqreturn_t hvgr_pm_cc_irq(int irq, void *data)
{
	struct hvgr_device *gdev = data;
	uint32_t val;

	if (hvgr_pm_get_gpu_status(gdev) == HVGR_PM_POWER_OFF)
		return IRQ_NONE;

	val = hvgr_read_reg(gdev, gpu_control_reg(gdev, CC_IRQ_STATUS));
	if (val == 0)
		return IRQ_NONE;

	hvgr_write_reg(gdev, gpu_control_reg(gdev, CC_IRQ_CLEAR), val);

	if (val & RESET_COMPLETED) {
		hvgr_info(gdev, HVGR_PM, "Gpu soft reset completed.");
		atomic_set(&gdev->pm_dev.pm_reset.reset_status, GPU_RESET_SOFT_RESET_DONE);
		wake_up(&gdev->pm_dev.pm_reset.reset_done_wait);
	}
	return IRQ_HANDLED;
}

static void hvgr_pm_driver_job_rjd_os_config(struct hvgr_device *gdev, uint32_t gpc_mask)
{
	/*
	 * The value of the JOB_RJD_OS_CONFIG register is provided by the
	 * hardware. The mapping between the number of powered-on cores and
	 * the value of the JOB_RJD_OS_CONFIG register is as follows:
	 *
	 *  Powered-on cores          JOB_RJD_OS_CONFIG value
	 *          1                       0x20081008
	 *          2                       0x20081008
	 *          3                       0x0C040602
	 *          4                       0x10040802
	 *          5                       0x14040A02
	 *          6                       0x18040C02
	 *          7                       0x1C040E02
	 *          8                       0x20041002
	 */
	uint32_t gpc_number = hweight32(gpc_mask);
	uint32_t job_rjd_value[] = {
		0x20081008,
		0x20081008,
		0x0C040602,
		0x10040802,
		0x14040A02,
		0x18040C02,
		0x1C040E02,
		0x20041002
	};

	if (!hvgr_is_job_slot_support())
		return;

	if (unlikely(gpc_number == 0 ||
		gpc_number > ARRAY_SIZE(job_rjd_value))) {
		hvgr_err(gdev, HVGR_PM, "gpc number is invalid, gpc number = %d",
			gpc_number);
		return;
	}
	hvgr_write_reg(gdev, job_control_reg(gdev, JOB_RJD_OS_CONFIG),
		job_rjd_value[gpc_number - 1]);
}

static void hvgr_pm_wait_cycle_cnt_oper_done(struct hvgr_device *gdev)
{
	int i = 0;
	uint32_t gpu_irq;

	do {
		gpu_irq = hvgr_read_reg(gdev, gpu_control_reg(gdev, GPU_IRQ_RAWSTAT));
		i++;
	} while ((i < HVGR_OPER_CYCLE_TIMEOUT) && ((gpu_irq & CYCLECOUNT_DONE) == 0));

	if (i == HVGR_OPER_CYCLE_TIMEOUT)
		hvgr_err(gdev, HVGR_PM, "Operate cycle counter time out.");

	hvgr_write_reg(gdev, gpu_control_reg(gdev, GPU_IRQ_CLEAR), CYCLECOUNT_DONE);
	hvgr_info(gdev, HVGR_PM, "%s, wait times = %d, gpu_irq = 0x%x.",
		__func__, i, gpu_irq);
}

static void hvgr_pm_start_cycle_count(struct hvgr_device *gdev)
{
	hvgr_write_reg(gdev, gpu_control_reg(gdev, GPU_COMMAND), GPU_COMMAND_CYCLE_COUNT_START);
	hvgr_pm_wait_cycle_cnt_oper_done(gdev);
}

static void hvgr_pm_stop_cycle_count(struct hvgr_device *gdev)
{
	hvgr_write_reg(gdev, gpu_control_reg(gdev, GPU_COMMAND), GPU_COMMAND_CYCLE_COUNT_STOP);
	hvgr_pm_wait_cycle_cnt_oper_done(gdev);
}

static bool hvgr_pm_is_core_stable(uint32_t cores, uint32_t core_status,
	uint32_t dst_stat)
{
	uint32_t bit_pos;

	while (cores) {
		bit_pos = (uint32_t)ffs((int)cores) - 1;
		if (((core_status >> (bit_pos * 2)) & PM_CORE_STATUS_MASK)
			!= dst_stat)
			return false;

		cores &= ~(1U << bit_pos);
	}
	return true;
}

void hvgr_dump_registers(struct hvgr_device *gdev)
{
	unsigned int i, j, jc_idx;

	hvgr_debug(gdev, HVGR_PM, "Register state:");
	hvgr_debug(gdev, HVGR_PM, "  GPU_IRQ_RAWSTAT=0x%08x GPU_STATUS=0x%08x",
		hvgr_read_reg(gdev, gpu_control_reg(gdev, GPU_IRQ_RAWSTAT)),
		hvgr_read_reg(gdev, gpu_control_reg(gdev, GPU_STATUS)));

	hvgr_debug(gdev, HVGR_PM, "  JOB_IRQ_STATUS=0x%08x",
		hvgr_read_reg(gdev, job_control_reg(gdev, JOB_IRQ_STATUS)));

	for (i = 0; i < MAX_JOB_SLOT_NR; i++) {
		hvgr_debug(gdev, HVGR_PM, "  JS%u_IRQ_STATUS=0x%08x ", i,
			hvgr_read_reg(gdev, job_slot_x_reg(gdev, i, JSX_IRQ_STATUS)));

		for (j = 0; j < MAX_JOB_SLOT_CHAIN_NR; j++) {
			jc_idx = (i << 2) + j;
			hvgr_debug(gdev, HVGR_PM,
				"  JS%u_JC%u_HEAD_LO=0x%08x ", i, j,
				hvgr_read_reg(gdev,
					job_jc_y_reg(gdev, jc_idx, JSX_JCY_HEAD_LO)));
		}
	}

	hvgr_debug(gdev, HVGR_PM, "  MMU_IRQ_RAWSTAT=0x%08x GPU_FAULTSTATUS=0x%08x",
		hvgr_read_reg(gdev, mmu_control_reg(gdev, MMU_IRQ_RAWSTAT)),
		hvgr_read_reg(gdev, gpu_control_reg(gdev, GPU_FAULTSTATUS)));

	hvgr_debug(gdev, HVGR_PM,
		"  GPU_IRQ_MASK=0x%08x    MMU_IRQ_MASK=0x%08x",
		hvgr_read_reg(gdev, gpu_control_reg(gdev, GPU_IRQ_MASK)),
		hvgr_read_reg(gdev, mmu_control_reg(gdev, MMU_IRQ_MASK)));
	for (i = 0; i < MAX_JOB_SLOT_NR; i++)
		hvgr_debug(gdev, HVGR_PM,
			"  JS%u_IRQ_MASK=0x%08x ", i,
			hvgr_read_reg(gdev, job_slot_x_reg(gdev, i, JSX_IRQ_MASK)));

	hvgr_debug(gdev, HVGR_PM, "  PWR_OVERRIDE0=0x%08x   PWR_OVERRIDE1=0x%08x",
		hvgr_read_reg(
			gdev, gpu_control_reg(gdev, PWR_OVERRIDE0)),
		hvgr_read_reg(
			gdev, gpu_control_reg(gdev, PWR_OVERRIDE1)));
	hvgr_debug(gdev, HVGR_PM, "  SHADER_CONFIG=0x%08x   L2_MMU_CONFIG=0x%08x",
		hvgr_read_reg(gdev, gpu_control_reg(gdev, GPC_CONFIG_0)),
		hvgr_read_reg(
			gdev, gpu_control_reg(gdev, L2_MMU_CONFIG_0)));
	hvgr_debug(gdev, HVGR_PM, "  TILER_CONFIG=0x%08x    JM_CONFIG=0x%08x",
		hvgr_read_reg(gdev, gpu_control_reg(gdev, BTC_CONFIG_0)),
		hvgr_read_reg(gdev, gpu_control_reg(gdev, JM_CONFIG)));

	hvgr_debug(gdev, HVGR_PM, "  GPC pwr status = 0x%08x",
		hvgr_read_reg(gdev, gpu_control_reg(gdev, GPC_PWR_STATUS)));
	hvgr_debug(gdev, HVGR_PM, "  BL pwr status = 0x%08x",
		hvgr_read_reg(gdev, gpu_control_reg(gdev, BL_PWR_STATUS)));
#if hvgr_version_lt(HVGR_V350)
	hvgr_debug(gdev, HVGR_PM, "  AS state = 0x%08x.",
		hvgr_read_reg(gdev, mmu_control_reg(gdev, MMU_CMD_DONE)));
	hvgr_debug(gdev, HVGR_PM, "  MMU cmd warn = 0x%08x.",
		hvgr_read_reg(gdev, mmu_control_reg(gdev, MMU_CMD_CFG_WARN)));
	hvgr_debug(gdev, HVGR_PM, "  MMU invalid state = 0x%08x.",
		hvgr_read_reg(gdev, mmu_control_reg(gdev, MMU_INVALID_STATE)));
#else
	hvgr_err(gdev, HVGR_PM,
		"\t[MMU_CD_CFG][0x%08x]\n"
		"\t[MMU_CD_BASE_L][0x%08x]\n"
		"\t[MMU_CD_BASE_H][0x%08x]\n"
		"\t[MMU_CMDQ_BASE_L][0x%08x]\n"
		"\t[MMU_CMDQ_BASE_H][0x%08x]\n"
		"\t[MMU_EVENTQ_BASE_L][0x%08x]\n"
		"\t[MMU_EVENTQ_BASE_H][0x%08x]\n"
		"\t[MMU_CR1][0x%08x]\n"
		"\t[MMU_CR0][0x%08x]\n",
		hvgr_read_reg(gdev, mmu_control_reg(gdev, MMU_CD_CFG)),
		hvgr_read_reg(gdev, mmu_control_reg(gdev, MMU_CD_BASE_L)),
		hvgr_read_reg(gdev, mmu_control_reg(gdev, MMU_CD_BASE_H)),
		hvgr_read_reg(gdev, mmu_control_reg(gdev, MMU_CMDQ_BASE_L)),
		hvgr_read_reg(gdev, mmu_control_reg(gdev, MMU_CMDQ_BASE_H)),
		hvgr_read_reg(gdev, mmu_control_reg(gdev, MMU_EVENTQ_BASE_L)),
		hvgr_read_reg(gdev, mmu_control_reg(gdev, MMU_EVENTQ_BASE_H)),
		hvgr_read_reg(gdev, mmu_control_reg(gdev, MMU_CR1)),
		hvgr_read_reg(gdev, mmu_control_reg(gdev, MMU_CR0)));
#endif
	hvgr_sch_reset(gdev, HVGR_SCH_DUMP_REGISTER);
}

static bool hvgr_pm_is_core_ready(uint32_t cores, uint32_t core_status)
{
	return hvgr_pm_is_core_stable(cores, core_status, PM_CORE_POWERED_ON);
}

static bool hvgr_pm_is_core_off(uint32_t cores, uint32_t core_status)
{
	return hvgr_pm_is_core_stable(cores, core_status, PM_CORE_POWERED_OFF);
}

bool hvgr_pm_driver_gpc_pwr_on_by_sw(struct hvgr_device *gdev)
{
	uint32_t cores;
	struct hvgr_pm_pwr *pm_pwr = &gdev->pm_dev.pm_pwr;

	/* 1. Load jmcp firmeware */
	(void)hvgr_fcp_load_by_poll(gdev);
	/* 2. Un reset GPC */
	cores = pm_pwr->present_cores[HVGR_PM_CORE_GPC];
	cores &= pm_pwr->core_mask_cur;
	if (unlikely(!hvgr_pm_driver_update_gpc_status(gdev, cores,
		GPC_PWRON_CMD, hvgr_pm_is_core_ready))) {
			hvgr_err(gdev, HVGR_PM, "Unreset GPC failed.");
			return false;
		}

	hvgr_pm_start_cycle_count(gdev);
	hvgr_info(gdev, HVGR_PM, "Unreset GPC succeeded.");

	return true;
}

bool hvgr_pm_driver_gpc_pwr_off_by_sw(struct hvgr_device *gdev)
{
	uint32_t cores;
	struct hvgr_pm_pwr *pm_pwr = &gdev->pm_dev.pm_pwr;

	hvgr_pm_stop_cycle_count(gdev);
	/* 1. Reset GPC */
	cores = pm_pwr->present_cores[HVGR_PM_CORE_GPC];
	cores &= pm_pwr->core_mask_cur;
	if (hvgr_hw_has_issue(gdev, HVGR_HW_ISSUE_50210)) {
		if (unlikely(!hvgr_pm_driver_update_gpc_status(gdev, cores,
			GPC_PWROFF_CMD, hvgr_pm_is_core_off))) {
			hvgr_err(gdev, HVGR_PM, "Reset core failed.");
			return false;
		}
	} else {
		hvgr_write_reg(gdev, gpu_control_reg(gdev, GPC_PWROFF_CMD), cores);
	}

	hvgr_info(gdev, HVGR_PM, "Reset GPC ongoing, cores 0x%x", cores);
	return true;
}

bool hvgr_pm_driver_wait_gpc_pwr_off_by_sw(struct hvgr_device *gdev)
{
	uint32_t cores;
	struct hvgr_pm_pwr *pm_pwr = &gdev->pm_dev.pm_pwr;

	if (hvgr_hw_has_issue(gdev, HVGR_HW_ISSUE_50210))
		return true;

	cores = pm_pwr->present_cores[HVGR_PM_CORE_GPC];
	cores &= pm_pwr->core_mask_cur;
 
	if (unlikely(!hvgr_pm_driver_wait_core_trans_end(gdev, cores,
		GPC_PWR_STATUS, hvgr_pm_is_core_off))) {
		hvgr_err(gdev, HVGR_PM, "Reset GPC failed when powering off gpc.");
		return false;
	}

	hvgr_info(gdev, HVGR_PM, "Reset core succeeded.");
	return true;
}

static void hvgr_pm_wait_clean_flush_end(struct hvgr_device *gdev)
{
	unsigned long time_out;
	bool ret = false;
	uint32_t val;

	time_out = jiffies + msecs_to_jiffies(HVGR_PM_GPU_FLUSH_TIMEOUT);
	do {
		val = hvgr_read_reg(gdev, gpu_control_reg(gdev, GPU_IRQ_RAWSTAT));
		if ((val & CLEAN_CACHES_COMPLETED) != 0) {
			ret = true;
			break;
		}
		udelay(1); /* delay 1 us */
	} while (time_before(jiffies, time_out));

	if (ret)
		hvgr_info(gdev, HVGR_PM, "Gpu clean flush success!");
	else
		hvgr_err(gdev, HVGR_PM, "Gpu clean flush time out!");
}

static void hvgr_pm_clean_flush(struct hvgr_device *gdev)
{
	hvgr_write_reg(gdev, gpu_control_reg(gdev, GPU_COMMAND), GPU_COMMAND_CLEAN_CACHES);
	hvgr_pm_wait_clean_flush_end(gdev);
}

void hvgr_pm_driver_reset_gpc_and_bl(struct hvgr_device *gdev)
{
	uint32_t cores;
	struct hvgr_pm_pwr *pm_pwr = &gdev->pm_dev.pm_pwr;

	(void)hvgr_pm_driver_reset_cores(gdev);
	cores = pm_pwr->present_cores[HVGR_PM_CORE_GPC];
	cores &= pm_pwr->core_mask_cur;
	/* GPC is powering off */
	if (!hvgr_hw_has_issue(gdev, HVGR_HW_ISSUE_50210)) {
		if (unlikely(!hvgr_pm_driver_wait_core_trans_end(gdev, cores,
			GPC_PWR_STATUS, hvgr_pm_is_core_off))) {
			hvgr_err(gdev, HVGR_PM, "Reset GPC failed when reset gpc and bl.");
			return;
		}
	}

	if (hvgr_hw_has_issue(gdev, HVGR_HW_ISSUE_50210)) {
		hvgr_pm_clean_flush(gdev);
	} else {
		cores = pm_pwr->present_cores[HVGR_PM_CORE_BL];
		hvgr_write_reg(gdev, gpu_control_reg(gdev, BL_PWROFF_CMD), cores);
		if (unlikely(!hvgr_pm_driver_wait_core_trans_end(gdev, cores,
			BL_PWR_STATUS, hvgr_pm_is_core_off)))
			hvgr_err(gdev, HVGR_PM, "Reset BL failed.");
	}
}

void hvgr_pm_driver_cbit_config_by_sw(struct hvgr_device *gdev)
{
	uint32_t btc_cfg_4;
	uint32_t gpc_cfg_6;
	uint32_t cores;
	uint32_t gpc_cfg_4;
	uint32_t gpc_cfg_0;
	uint32_t gpc_cfg_2;
	uint32_t gpc_cfg_5;
	struct hvgr_pm_pwr *pm_pwr = &gdev->pm_dev.pm_pwr;

	btc_cfg_4 = hvgr_read_reg(gdev, gpu_control_reg(gdev, BTC_CONFIG_4));
	btc_cfg_4 = btc_cfg_4 | BTC_CFG4_TOP_MUX_SEL_2V;
	hvgr_write_reg(gdev, gpu_control_reg(gdev, BTC_CONFIG_4), btc_cfg_4);

	gpc_cfg_6 = hvgr_read_reg(gdev, gpu_control_reg(gdev, GPC_CONFIG_6));
	gpc_cfg_6 |= GPC_CFG6_LSC_NONGPC_STORE_MASK_DISABLE;

	gpc_cfg_6 |= GPC_CFG6_LSC_PUSH_VTC_GTC_PROP;
	hvgr_write_reg(gdev, gpu_control_reg(gdev, GPC_CONFIG_6), gpc_cfg_6);

	if (!hvgr_hw_has_issue(gdev, HVGR_HW_ISSUE_50300)) {
		if (hvgr_is_fpga(gdev)) {
			gpc_cfg_4 = hvgr_read_reg(gdev, gpu_control_reg(gdev, GPC_CONFIG_4));
			gpc_cfg_4 |= (0x2AU << 8); /* mask GPC0~2's SP1 */
			hvgr_write_reg(gdev, gpu_control_reg(gdev, GPC_CONFIG_4), gpc_cfg_4);
		}
	}

	if (hvgr_hw_has_issue(gdev, HVGR_HW_ISSUE_00031)) {
		gpc_cfg_0 = hvgr_read_reg(gdev, gpu_control_reg(gdev, GPC_CONFIG_0));
		gpc_cfg_0 |= SHADER_CONFIG_0_ICG;
		hvgr_write_reg(gdev, gpu_control_reg(gdev, GPC_CONFIG_0), gpc_cfg_0);

		gpc_cfg_2 = hvgr_read_reg(gdev, gpu_control_reg(gdev, GPC_CONFIG_2));
		gpc_cfg_2 |= SHADER_CONFIG_2_ICG;
		hvgr_write_reg(gdev, gpu_control_reg(gdev, GPC_CONFIG_2), gpc_cfg_2);

		gpc_cfg_5 = hvgr_read_reg(gdev, gpu_control_reg(gdev, GPC_CONFIG_5));
		gpc_cfg_5 |= SHADER_CONFIG_5_ICG;
		hvgr_write_reg(gdev, gpu_control_reg(gdev, GPC_CONFIG_5), gpc_cfg_5);
	}

	hvgr_datan_cbit_config(gdev);
	cores = pm_pwr->present_cores[HVGR_PM_CORE_GPC];
	cores &= pm_pwr->core_mask_cur;
	hvgr_pm_driver_job_rjd_os_config(gdev, cores);

	hvgr_mmu_cbit_config(gdev);

	hvgr_platform_timestamp_cbit_config(gdev);
}

int hvgr_pm_driver_clock_on_by_sw(struct hvgr_device *gdev)
{
	uint32_t cores;
	uint64_t start_time;
	uint64_t dfx_time;
	struct hvgr_pm_pwr *pm_pwr = &gdev->pm_dev.pm_pwr;

	dfx_time = hvgr_pm_dfx_get_curtime(gdev);
	start_time = hvgr_start_timing();
	hvgr_pm_driver_buck_on(gdev);
	hvgr_platform_enable_icg_clk(gdev);
#ifdef HVGR_FEATURE_SYSTEM_CACHE
	hvgr_sc_enable_scs(gdev);
	hvgr_sc_recover_remap_regs(gdev);
#endif
#if defined(HVGR_USE_PCIE) && HVGR_USE_PCIE
	(void)hvgr_crg_reset(gdev);
#endif

	cores = pm_pwr->present_cores[HVGR_PM_CORE_BL];
	hvgr_write_reg(gdev, gpu_control_reg(gdev, BL_PWRON_CMD), cores);
	if (unlikely(!hvgr_pm_driver_wait_core_trans_end(gdev, cores,
		BL_PWR_STATUS, hvgr_pm_is_core_ready))) {
		hvgr_err(gdev, HVGR_PM, "Unreset BL failed.");
		return HVGR_PM_CORE_FAIL;
	}
	hvgr_info(gdev, HVGR_PM, "Unreset BL succeeded.");
	hvgr_pm_driver_cbit_config(gdev);
	hvgr_sch_set_mode(gdev, HVGR_SCH_MODE_CQ, HVGR_SCH_SUBMIT_IGNORE);
	hvgr_enable_interrupts(gdev);

	hvgr_pm_dfx_buck_on_statistics(gdev, dfx_time);
	hvgr_end_timing_and_log(gdev, HVGR_PM, start_time, "Clock on");
	return HVGR_PM_OK;
}

bool hvgr_pm_driver_gpu_config_by_sw(struct hvgr_device *gdev)
{
	uint32_t cores;
	struct hvgr_pm_pwr *pm_pwr = &gdev->pm_dev.pm_pwr;

	hvgr_platform_enable_icg_clk(gdev);
	cores = pm_pwr->present_cores[HVGR_PM_CORE_BL];
	hvgr_write_reg(gdev, gpu_control_reg(gdev, BL_PWRON_CMD), cores);
	if (unlikely(!hvgr_pm_driver_wait_core_trans_end(gdev, cores,
		BL_PWR_STATUS, hvgr_pm_is_core_ready))) {
		hvgr_err(gdev, HVGR_PM, "Unreset BL failed.");
		return false;
	}
	hvgr_info(gdev, HVGR_PM, "Unreset BL succeeded.");
	hvgr_pm_driver_cbit_config(gdev);
	return true;
}

int hvgr_pm_driver_clock_off_by_sw(struct hvgr_device *gdev)
{
	uint32_t cores;
	uint64_t start_time;
	uint64_t dfx_time;
	struct hvgr_pm_pwr *pm_pwr = &gdev->pm_dev.pm_pwr;

	dfx_time = hvgr_pm_dfx_get_curtime(gdev);
	start_time = hvgr_start_timing();
	hvgr_disable_interrupts(gdev);
	hvgr_pm_driver_clock_off_pre_proc(gdev);

	if (hvgr_hw_has_issue(gdev, HVGR_HW_ISSUE_50210)) {
		hvgr_pm_clean_flush(gdev);
	} else {
		cores = pm_pwr->present_cores[HVGR_PM_CORE_BL];
		hvgr_write_reg(gdev, gpu_control_reg(gdev, BL_PWROFF_CMD), cores);
		if (unlikely(!hvgr_pm_driver_wait_core_trans_end(gdev, cores,
			BL_PWR_STATUS, hvgr_pm_is_core_off))) {
			hvgr_err(gdev, HVGR_PM, "Reset BL failed.");
			hvgr_sch_set_mode(gdev, HVGR_SCH_MODE_CQ, HVGR_SCH_SUBMIT_IGNORE);
			hvgr_enable_interrupts(gdev);
			return HVGR_PM_CORE_FAIL;
		}
		hvgr_info(gdev, HVGR_PM, "Reset BL succeeded.");
	}

#ifdef HVGR_FEATURE_SYSTEM_CACHE
	hvgr_sc_store_remap_regs(gdev);
#endif
	hvgr_pm_driver_buck_off(gdev);
	hvgr_pm_dfx_buck_off_statistics(gdev, dfx_time);
	hvgr_end_timing_and_log(gdev, HVGR_PM, start_time, "Clock off");
	return HVGR_PM_OK;
}

void hvgr_pm_driver_hw_init(struct hvgr_device *gdev)
{
	struct hvgr_pm_pwr *pm_pwr = &gdev->pm_dev.pm_pwr;

	pm_pwr->present_cores[HVGR_PM_CORE_BL] =
		hvgr_read_reg(gdev, gpu_control_reg(gdev, BL_PRESENT));
	pm_pwr->present_cores[HVGR_PM_CORE_GPC] =
		hvgr_read_reg(gdev, gpu_control_reg(gdev, GPC_PRESENT));

	if (pm_pwr->gpc_present_mask != 0)
		pm_pwr->present_cores[HVGR_PM_CORE_GPC] &= pm_pwr->gpc_present_mask;

	pm_pwr->core_mask_cur =
		pm_pwr->core_mask_desire =
		pm_pwr->present_cores[HVGR_PM_CORE_GPC];

	/* Register cc irq */
	if (hvgr_register_irq(gdev, CC_IRQ, hvgr_pm_cc_irq) != 0)
		hvgr_err(gdev, HVGR_PM, "Regiser cc irq failed.");
}

static uint32_t hvgr_pm_drier_get_ready_cores(struct hvgr_device *gdev)
{
	uint32_t status;
	uint32_t present;
	uint32_t ready = 0;
	uint32_t bit_pos;
	struct hvgr_pm_pwr *pm_pwr = &gdev->pm_dev.pm_pwr;

	present = pm_pwr->present_cores[HVGR_PM_CORE_GPC];
	status = hvgr_read_reg(gdev, gpu_control_reg(gdev, GPC_PWR_STATUS));

	while (present) {
		bit_pos = (uint32_t)ffs((int)present) - 1;
		if (((status >> (bit_pos * 2)) & PM_CORE_STATUS_MASK) ==
			PM_CORE_POWERED_ON)
			ready |= 1U << bit_pos;

		present &= ~(1U << bit_pos);
	}
	return ready;
}

void hvgr_pm_driver_set_core_mast_by_sw(struct hvgr_device *gdev)
{
	uint32_t desire_mask, ready, unreset_mask, reset_mask;
	struct hvgr_pm_pwr *pm_pwr = &gdev->pm_dev.pm_pwr;

	desire_mask = pm_pwr->core_mask_desire;
	ready = hvgr_pm_drier_get_ready_cores(gdev);
	unreset_mask = ~ready & desire_mask;
	reset_mask = ready & ~desire_mask;

	hvgr_pm_stop_cycle_count(gdev);
	if (unreset_mask)
		(void)hvgr_pm_driver_update_gpc_status(gdev, unreset_mask,
			GPC_PWRON_CMD, hvgr_pm_is_core_ready);

	if (reset_mask)
		(void)hvgr_pm_driver_update_gpc_status(gdev, reset_mask,
			GPC_PWROFF_CMD, hvgr_pm_is_core_off);

	hvgr_pm_start_cycle_count(gdev);
	ready = hvgr_pm_drier_get_ready_cores(gdev);
	hvgr_pm_driver_job_rjd_os_config(gdev, ready);
}

void hvgr_pm_driver_gpu_soft_reset(struct hvgr_device *gdev)
{
	hvgr_write_reg(gdev, gpu_control_reg(gdev, SOFT_RESET_CMD), GPU_COMMAND_SOFT_RESET);
	/* Unmask the reset complete interrupt only */
	hvgr_write_reg(gdev, gpu_control_reg(gdev, CC_IRQ_MASK), RESET_COMPLETED);
}

int hvgr_pm_driver_hw_access_enable(struct hvgr_device *gdev)
{
	uint32_t l2_present;

	hvgr_pm_driver_buck_on(gdev);
	hvgr_platform_enable_icg_clk(gdev);
	/* Init GPU control register baseaddr. */
	if (hvgr_init_control_baseaddr(gdev) != 0)
		return -1;
	if (hvgr_hw_set_issues_mask(gdev) != 0)
		return -1;
	l2_present = hvgr_read_reg(gdev, gpu_control_reg(gdev, BL_PRESENT));
	hvgr_write_reg(gdev, gpu_control_reg(gdev, BL_PWRON_CMD), l2_present);
	(void)hvgr_pm_driver_wait_core_trans_end(gdev, l2_present, BL_PWR_STATUS,
		hvgr_pm_is_core_ready);
	return 0;
}

void hvgr_pm_driver_hw_access_disable(struct hvgr_device *gdev)
{
	uint32_t l2_present;

	l2_present = hvgr_read_reg(gdev, gpu_control_reg(gdev, BL_PRESENT));

	hvgr_write_reg(gdev, gpu_control_reg(gdev, BL_PWROFF_CMD), l2_present);

	(void)hvgr_pm_driver_wait_core_trans_end(gdev, l2_present, BL_PWR_STATUS,
		hvgr_pm_is_core_off);

	hvgr_pm_driver_buck_off(gdev);

#ifdef CONFIG_HVGR_GPU_RT_PM
	hvgr_pm_driver_runtime_init(gdev);
#endif
}

void hvgr_pm_gate_icg_clk(struct hvgr_device *gdev)
{
	uint32_t val;

	if (!hvgr_hw_has_issue(gdev, HVGR_HW_ISSUE_00030))
		return;

	val = hvgr_read_reg(gdev, gpu_control_reg(gdev, PWR_OVERRIDE0));
	hvgr_write_reg(gdev, gpu_control_reg(gdev, PWR_KEY), HVGR_PWR_KEY_VALUE);
	hvgr_write_reg(gdev, gpu_control_reg(gdev, PWR_OVERRIDE0), val & (~HVGR_ICG_CLK_ON));
}
