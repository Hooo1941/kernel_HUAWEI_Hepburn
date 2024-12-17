/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 * This file is the dvfs implementation of the PM module.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */
#ifndef HVGR_PM_DRIVER_H
#define HVGR_PM_DRIVER_H
#include <linux/types.h>
#include <linux/device.h>
#include "hvgr_pm_defs.h"

/*
 * hvgr_pm_driver_reset_cores - Reset the GPU Core.
 *
 * @param gdev - The gdev object pointer.
 *
 * @return: True if the GPU core is reset successfully.
 * @date - 2021-02-03
 */
bool hvgr_pm_driver_reset_cores(struct hvgr_device *gdev);

/*
 * hvgr_pm_driver_unreset_cores - Unreset the GPU Core.
 *
 * @param gdev - The gdev object pointer.
 *
 * @return: True if the GPU core is unreset successfully.
 * @date - 2021-02-03
 */
bool hvgr_pm_driver_unreset_cores(struct hvgr_device *gdev);

/*
 * hvgr_pm_wait_gpc_poweroff_ready - wait power off GPC ready.
 *
 * @param gdev - The gdev object pointer.
 *
 * @return: True if the GPC is powered off successfully.
 * @date - 2021-02-03
 */
bool hvgr_pm_wait_gpc_poweroff_ready(struct hvgr_device *gdev);

/*
 * hvgr_pm_driver_clock_on - Colck on
 * Powers on the buck, enables interrupts, and refreshes the MMU.
 *
 * @param gdev - The gdev object pointer.
 *
 * @return: 0 if the GPU clock on successfully.
 * @date - 2021-02-03
 */
int hvgr_pm_driver_clock_on(struct hvgr_device *gdev);

/*
 * hvgr_pm_driver_clock_off - Colck off
 * disable interrupts, process unfinished page faults/bus faults
 * and to power off the buck.
 *
 * @param gdev - The gdev object pointer.
 *
 * @return: 0 if the clock off successfully.
 * @date - 2021-02-03
 */
int hvgr_pm_driver_clock_off(struct hvgr_device *gdev);

/*
 * hvgr_pm_driver_hw_init - Initialize software configurations based on
 * the hardware version.
 *
 * @param gdev - The gdev object pointer.
 *
 * @date - 2021-02-03
 */
void hvgr_pm_driver_hw_init(struct hvgr_device *gdev);

/*
 * hvgr_pm_driver_set_core_mask - Inserting and removing cores.
 * Inserting cores: reset gpc. Removing cores: unreset gpc.
 *
 * @param gdev - The gdev object pointer.
 *
 * @date - 2021-02-03
 */
void hvgr_pm_driver_set_core_mask(struct hvgr_device *gdev);

/*
 * hvgr_pm_driver_soft_init - Software initialization at the driver layer.
 * Currently, only the regulator needs to be initialized.
 *
 * @param gdev - The gdev object pointer.
 *
 * @return: 0 if Initialization succeeded.
 * @date - 2021-02-03
 */
int hvgr_pm_driver_soft_init(struct hvgr_device *gdev);

/*
 * hvgr_pm_driver_soft_term - Software termination at the driver layer.
 * Currently, only the regulator needs to be released.
 *
 * @param gdev - The gdev object pointer.
 *
 * @date - 2021-02-03
 */
void hvgr_pm_driver_soft_term(struct hvgr_device *gdev);

/*
 * hvgr_pm_driver_gpu_soft_reset - Soft reset the GPU.
 * This interface only delivers the soft reset command.
 * The soft reset register addresses vary according to hardware versions.
 * Therefore, the soft reset is performed at the driver layer to mask
 * hardware differences. Other GPU reset processes are completed in the
 * gpu_reset module.
 *
 * @param gdev - The gdev object pointer.
 *
 * @date - 2021-02-03
 */
void hvgr_pm_driver_gpu_soft_reset(struct hvgr_device *gdev);

/*
 * hvgr_pm_driver_hw_access_enable - Enable hw access.
 * In the device probe phase, some system statuses, such as address mapping
 * and hardware specifications, need to be initialized based on GPU_ID.
 * Therefore, we need to ensure that the GPU registers can be accessed. After
 * this interface is executed, the GPU registers can be accessed.
 * This interface is called only in the device probe phase.
 *
 * @param gdev - The gdev object pointer.
 *
 * @return: 0 if gpu subsystem power up succeeded.
 * @date - 2021-02-03
 */
int hvgr_pm_driver_hw_access_enable(struct hvgr_device *gdev);

/*
 * hvgr_pm_driver_hw_access_disable - Disable hw access.
 * After this interface is executed, the GPU registers can not be accessed.
 * This interface is called only in the device probe phase.
 *
 * @param gdev - The gdev object pointer.
 *
 * @date - 2021-02-03
 */
void hvgr_pm_driver_hw_access_disable(struct hvgr_device *gdev);

/*
 * hvgr_pm_driver_gpu_config - GPU Hardware Configuration.
 * After the GPU is reset, the hardware status needs to be reconfigured,
 * includes the cbit register and override register.
 *
 * @param gdev - The gdev object pointer.
 *
 * @return: True if config gpu succeeded.
 * @date - 2021-02-03
 */
bool hvgr_pm_driver_gpu_config(struct hvgr_device *gdev);

/*
 * hvgr_pm_driver_request_cycle_counter - Request cycle counter.
 * Before enabling the cycle counter, increase the value of reference counting.
 * If the reference count is 1, the cycle counter is used for the first time.
 * In this case, write to the register to enable the cycle counter.
 *
 * @param gdev - The gdev object pointer.
 *
 * @date - 2021-02-03
 */
void hvgr_pm_driver_request_cycle_counter(struct hvgr_device *gdev);

/*
 * hvgr_pm_driver_release_cycle_counter - Release cycle counter.
 * Before disabling the cycle counter, decrease the value of reference
 * counting. If the reference count is 0, write to the register to
 * disable the cycle counter.
 *
 * @param gdev - The gdev object pointer.
 *
 * @date - 2021-02-03
 */
void hvgr_pm_driver_release_cycle_counter(struct hvgr_device *gdev);

/*
 * hvgr_pm_driver_re_enable_cycle_counter - Enable cycle counter again.
 * After the GPU is reset, the cycle counter is disabled. If the reference
 * counting value of the cycle counter is not 0, we need to rewrite the
 * register to enable the cycle counter again.
 *
 * @param gdev - The gdev object pointer.
 *
 * @date - 2021-02-03
 */
void hvgr_pm_driver_re_enable_cycle_counter(struct hvgr_device *gdev);

/*
 * hvgr_pm_driver_gpu_reset_timeout_proc - GPU reset timeout process.
 *
 * @param gdev - The gdev object pointer.
 *
 * @date - 2021-02-03
 */
void hvgr_pm_driver_gpu_reset_timeout_proc(struct hvgr_device *gdev);

#ifdef CONFIG_HVGR_GPU_RT_PM
/*
 * hvgr_pm_driver_runtime_suspend - Runtime suspend
 * Suspend the dvfs and power off the BUCK.
 *
 * @param gdev - The gdev object pointer.
 *
 * @date - 2021-02-03
 */
int hvgr_pm_driver_runtime_suspend(struct hvgr_device *gdev);

/*
 * hvgr_pm_driver_runtime_resume - Runtime resume
 * Power on the BUCK and resume the dvfs.
 *
 * @param gdev - The gdev object pointer.
 *
 * @date - 2021-02-03
 */
int hvgr_pm_driver_runtime_resume(struct hvgr_device *gdev);

/*
 * hvgr_pm_driver_runtime_idle - Runtime idle
 * on.
 *
 * @param gdev - The gdev object pointer.
 *
 * @date - 2021-02-03
 */
int hvgr_pm_driver_runtime_idle(struct hvgr_device *gdev);

/*
 * hvgr_pm_driver_runtime_init - Runtime init
 *
 * @param gdev - The gdev object pointer.
 *
 * @date - 2021-02-03
 */
void hvgr_pm_driver_runtime_init(struct hvgr_device *gdev);

/*
 * hvgr_pm_driver_runtime_term - Runtime term
 *
 * @param gdev - The gdev object pointer.
 *
 * @date - 2021-02-03
 */
void hvgr_pm_driver_runtime_term(struct hvgr_device *gdev);
#endif
void hvgr_pm_driver_dump_power_reg(struct hvgr_device *gdev);

/*
 * hvgr_dump_registers - Rump register when gpu reset.
 *
 * @param gdev - The gdev object pointer.
 *
 * @date - 2021-02-03
 */
void hvgr_dump_registers(struct hvgr_device *gdev);

/*
 * hvgr_pm_driver_reset_gpc_and_bl - Reset gpc and bl when exit protect mode.
 *
 * @param gdev - The gdev object pointer.
 *
 * @date - 2021-02-03
 */
void hvgr_pm_driver_reset_gpc_and_bl(struct hvgr_device *gdev);

void hvgr_pm_driver_cbit_config(struct hvgr_device *gdev);

#endif
