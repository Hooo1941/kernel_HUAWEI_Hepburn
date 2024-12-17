/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 * This file is the dvfs implementation of the PM module.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */
#ifndef HVGR_PM_DEFS_H
#define HVGR_PM_DEFS_H

#include <linux/atomic.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/mutex.h>
#include <linux/ktime.h>
#include <linux/hrtimer.h>
#include <linux/regulator/consumer.h>
#include <linux/workqueue.h>

#include "hvgr_pm_driver_adapt_defs.h"
#include "hvgr_version.h"

struct hvgr_device;

#define HVGR_CONVERT_TO_MILLISECOND      1000U

#define HVGR_PM_TIME_SHIFT               8

#define HVGR_PM_OK                       0
#define HVGR_PM_CORE_FAIL                (-1)
#define HVGR_PM_INVALID_STATUS           (-2)
#define HVGR_PM_POWER_OFF_TIMEOUT        (10 * HZ) /* 10s */

/*
 * @brief States to power state machine
 * processed by hvgr_pm_update_pwr_state()
 */
enum hvgr_pm_gpu_status {
	/* Core reset/unreset error, need to reset gpu. */
	HVGR_PM_CORE_ERROR = -1,
	/* The GPU is power off. */
	HVGR_PM_POWER_OFF,

	/*
	 * The GPU has been powered on and initialized. In this state,
	 * the GPU register can be accessed normally.
	 */
	HVGR_PM_POWER_ON,

	/*
	 * The GPU is ready. In this state, Jobs can be submitted to hardware.
	 */
	HVGR_PM_GPU_READY,

	/*
	 * In this state, only issue GPC poweroff cmd, but not wait it is ready
	 */
	HVGR_PM_GPC_POWERING_OFF,
};

enum hvgr_pm_policy {
	/* In this policy, the GPU is always powered on except suspended. */
	HVGR_PM_ALWAYS_ON,

	/*
	 * In this policy, if no job is running, the GPU will be powered
	 * off.
	 */
	HVGR_PM_JOB_DEMAND,

	HVGR_PM_POLICY_MAX,
};

/*
 * @brief Term Steps for the PM Module
 */
enum hvgr_pm_term_step {
	/* Release work queue, timer etc. */
	HVGR_PM_STEP_SOFT_TERM,

	/* Disable run time pm. */
	HVGR_PM_STEP_RUNTIME_TERM,
};

/*
 * @brief suspend/resume status
 */
enum hvgr_pm_sr_status {
	HVGR_PM_ACTIVE,
	HVGR_PM_SUSPENDING,
	HVGR_PM_SUSPENDED,
	HVGR_PM_RESUMEING,
};

/*
 * @brief gpu reset status
 */
enum hvgr_pm_gpu_reset_status {
	GPU_RESET_RESET_ERROR = -1,
	GPU_RESET_BEGIN,
	GPU_RESET_JOB_STOP,
	GPU_RESET_JOB_STOP_DONE,
	GPU_RESET_RESET_SUBMMIT,
	GPU_RESET_SOFT_RESET_DONE,
	GPU_RESET_RESET_DONE,
	GPU_RESET_END,
};

/*
 * @brief gpu reset type
 */
enum hvgr_pm_gpu_reset_type {
	/*
	 * Gpu reset normal: Issue the soft stop command. After 2.5 seconds,
	 * reset the GPU regardless of whether the soft stop is successful.
	 */
	GPU_RESET_TYPE_NORMAL,
	/*
	 * Gpu reset immediately: Reset the GPU immediately without delay and
	 * soft stop.
	 */
	GPU_RESET_TYPE_IMMEDIATELY,
	/*
	 * Gpu reset exit protect mode: The GPU reset is triggered by the deprotection
	 * mode. Debug information is not collected.
	 */
	GPU_RESET_TYPE_EXIT_PROTECT_MODE,
};

struct hvgr_pm_pwr {
	/* This lock must held whenever the GPU is being powered on or off. */
	struct mutex power_lock;

	/* This lock must held whenever core_mask is processing. */
	struct mutex core_mask_lock;

	/* This is the GPU user count */
	atomic_t user_ref;

	/*
	 * Current power policy. For details about the supported power
	 * policies, see hvgr_pm_policy.
	 */
	atomic_t cur_policy;

	/* Current SR status. For details see hvgr_pm_sr_status. */
	atomic_t sr_state;

	/*
	 * When the PM is performing suspend or resume processing,
	 * powered_off_directly is set to true.
	 * When powered_off_directly is set to true, the buck will be directly
	 * powered on and off without the run-time of the Linux.
	 */
	bool powered_off_directly;

	/* Wait queue set when GPU is power off. */
	wait_queue_head_t power_off_done_wait;
#ifdef HVGR_GPU_POWER_ASYNC
	/*
	 * When the power-off timer starts, power_off_processing is set to
	 * true.
	 * If power_off_processing is set to true, the power-off timer is not
	 * started repeatedly.
	 */
	bool power_off_processing;

	/* Power off workqueue and work struct. */
	struct workqueue_struct *poweroff_wq;

	/* Power off work struct. */
	struct work_struct poweroff_work;

	/* Power off timer */
	struct hrtimer poweroff_timer;

	/* Power off delay time, default is 1.8 ms */
	ktime_t delay_time;
#endif
	/* Current power on/off status. For details see hvgr_pm_gpu_status. */
	enum hvgr_pm_gpu_status cur_status;

	/* Bit masks identifying the available cores read from hardware. */
	uint32_t present_cores[HVGR_PM_CORE_MAX];

	/*
	 * Desired bit masks identifying the available GPCs that
	 * are specified via sysfs.
	 * When processing the core_mask, core_mask_desire is set first. After
	 * the core_mask is successful, core_mask_cur is set.
	 * If the values of core_mask_desire and core_mask_cur are different,
	 * core_mask is being performed.
	 */
	uint32_t core_mask_desire;

	/*
	 * Current bit masks identifying the available GPCs that
	 * are specified via sysfs.
	 * The actual GPCs that are powered on and off is
	 * core_mask_cur & present_cores[HVGR_PM_CORE_GPC].
	 */
	uint32_t core_mask_cur;

#ifdef CONFIG_REGULATOR
	struct regulator *regulator;
#endif

	/* available gpc mask identified in bootloader */
	u32 gpc_present_mask;
#ifdef CONFIG_HVGR_DFX_SH
	atomic_t gpu_hang_cnt;
	atomic_t hang_core_proc_flag;
	atomic_t core_mask_rd_flag;
	/* read from nvme */
	atomic_t core_mask_info;
	struct workqueue_struct *sh_hang_wq;
	struct work_struct sh_hang_work;
#endif
};

struct hvgr_pm_reset {
	/* Wait queue set when reset_done is true. */
	wait_queue_head_t reset_done_wait;

	atomic_t reset_status;
	atomic_t reset_type;
	struct hrtimer reset_timer;
	struct workqueue_struct *reset_workq;
	struct work_struct reset_work;
	uint32_t reset_delay_time;
};

struct hvgr_pm_dvfs_metrics {
	/* GPU busy time. */
	uint32_t time_busy;

	/* GPU idle time. */
	uint32_t time_idle;

	/* cl job run time. */
	uint32_t cl_time;

	/* other job run time. */
	uint32_t other_time;
};

struct hvgr_pm_dvfs {
	/* This lock must be held when accessing the dvfs data. */
	spinlock_t dvfs_lock;

	/* GPU utilisation. */
	int utilisation;

	/*
	 * CL boost flag. This flag is set to 1 when the cl_time/(cl_time +
	 * other_time) >= 1% in an adjustment period.
	 */
	int cl_boost;

	/*
	 * Throttle enable flag: 1 enable 0 disable
	 */
	atomic_t thro_enable;

	/*
	 * Throttle freq of the current frequency modulation period.
	 */
	unsigned long thro_freq;

	/*
	 * Throttle freq of the last frequency modulation period.
	 */
	unsigned long pre_thro_freq;

	unsigned long max_freq;
	unsigned long min_freq;
	/*
	 * If a job starts or ends in the current sampling period, the value of
	 * thro_freq_flag is true. If thro_freq_flag is true, Throttle freq is
	 * thro_freq. Otherwise, Throttle freq is pre_thro_freq.
	 */
	bool thro_freq_flag;
	/* Sampling timestamp, which is updated after sampling is complete.
	 * Subtract time_stamp from the current time to obtain a sampling period,
	 * and then calculate the GPU duty cycle in a sampling period.
	 */
	ktime_t time_stamp;

	/* GPU usage sampling timer */
	struct hrtimer util_timer;

	/* GPU usage sampling time: 20ms */
	ktime_t delay_time;

	/* GPU usage sampling timer running flag */
	bool timer_running;

	/* Gpu busy/idle status, True: gpu is busy, false: gpu is idle */
	bool gpu_active;

	/*
	 * CL job status, True: cl job is running, False: Other job is running or no job
	 * is running.
	 */
	bool cl_job_flag;
	struct hvgr_pm_dvfs_metrics cur;
	struct hvgr_pm_dvfs_metrics last;
};

struct hvgr_pm_dfx {
	/* PM statistics enable flag */
	bool is_enable;

	/*
	 * Number of GPU power-on and power-off times, that is, the number of times the PM state
	 * machine switches from HVGR_PM_POWER_OFF to HVGR_PM_GPU_READY or from HVGR_PM_GPU_READY
	 * to HVGR_PM_POWER_OFF
	 */
	uint32_t pm_gpu_on_times;
	uint32_t pm_gpu_off_times;

	/* Number of BUCK power-on and power-off times. */
	uint32_t pm_buck_on_times;
	uint32_t pm_buck_off_times;

	/* Number of core power-on and power-off times, including GPC, BTC, and L2. */
	uint32_t pm_core_on_times;
	uint32_t pm_core_off_times;

	/* Number of gpu reset times */
	uint32_t pm_gpu_reset_times;
	/*
	 * Total GPU power-on/off time, The total power-on/off time divided by the number
	 * of power-off times is the average power-on/off time.
	 */
	uint64_t pm_total_gpu_on_time;
	uint64_t pm_total_gpu_off_time;
	uint64_t pm_gpu_on_max_time;
	uint64_t pm_gpu_off_max_time;
	uint64_t pm_gpu_on_min_time;
	uint64_t pm_gpu_off_min_time;
	/*
	 * Total BUCK power-on/off time, The total power-on/off time divided by the number
	 * of power-off times is the average power-on/off time.
	 */
	uint64_t pm_total_buck_on_time;
	uint64_t pm_total_buck_off_time;
	uint64_t pm_buck_on_max_time;
	uint64_t pm_buck_off_max_time;
	uint64_t pm_buck_on_min_time;
	uint64_t pm_buck_off_min_time;
	/*
	 * Total core power-on/off time, The total power-on/off time divided by the number
	 * of power-off times is the average power-on/off time.
	 */
	uint64_t pm_total_core_off_time;
	uint64_t pm_total_core_on_time;
	uint64_t pm_core_on_max_time;
	uint64_t pm_core_off_max_time;
	uint64_t pm_core_on_min_time;
	uint64_t pm_core_off_min_time;

	/*
	 * Total gpu reset The total gpu resed time divided by the number
	 * of gpu reset times is the average gpu reset time.
	 *
	 */
	uint64_t pm_total_gpu_reset_time;
	uint64_t pm_gpu_reset_max_time;
	uint64_t pm_gpu_reset_min_time;
};

struct hvgr_pm_dev {
	/*
	 * A pointer to kmd_device. The kmd_device data is required
	 * when the GPU register is accessed.
	 */
	struct hvgr_device *gdev;

	struct dentry *pm_dir;
	/* Power Management Data */
	struct hvgr_pm_pwr pm_pwr;

	/* The reference count of gpu cycle counter users */
	atomic_t cycle_counter_ref;

	/* GPU reset data Data */
	struct hvgr_pm_reset pm_reset;
	/* GPU dvfs data */
	struct hvgr_pm_dvfs pm_dvfs;
	/* GPU dfx data */
	struct hvgr_pm_dfx pm_dfx;
};

#endif
