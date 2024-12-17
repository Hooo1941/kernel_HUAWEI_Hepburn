/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 * This file is the dvfs implementation of the PM module.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_PM_API_H
#define HVGR_PM_API_H
#include <linux/types.h>
#include <linux/device.h>
#include "hvgr_defs.h"
#include "hvgr_pm_defs.h"

#define HVGR_THRO_HIGH     (0U) /* Queue is run in high freq. */
#define HVGR_THRO_MEDIUM   (1U) /* Queue is run in med freq: auto dvfs freq. */
#define HVGR_THRO_LOW      (2U) /* Queue is run in low freq. */
#define HVGR_THRO_NORMAL   (3U) /* Queue is not cl job. */
#define HVGR_THRO_EMPTY    (4U) /* Queue is empty, but not unbind. */
#define HVGR_THRO_IDLE     (5U) /* Queue is idle, unbinded. */

struct hvgr_device;
/*
 * hvgr_pm_request_gpu - Request the gpu.
 * hvgr_pm_request_gpu and hvgr_pm_release_gpu must appear in pairs.
 *
 * @param gdev - The gdev object pointer.
 * @gpu_reset -GPU reset flag
 * @date - 2021-02-03
 *
 * @return: True if power on was successful
 */
bool hvgr_pm_request_gpu(struct hvgr_device *gdev, bool gpu_reset);

/*
 * hvgr_pm_release_gpu - Power off the gpu.
 * hvgr_pm_request_gpu and hvgr_pm_release_gpu must appear in pairs.,
 *
 * @param gdev - The gdev object pointer.
 *
 * @date - 2021-02-03
 *
 */
void hvgr_pm_release_gpu(struct hvgr_device *gdev);

/*
 * hvgr_pm_gpu_reset - Reset the gpu
 *
 * @param gdev - The gdev object pointer.
 *
 * @date - 2021-02-03
 */
void hvgr_pm_gpu_reset(struct hvgr_device *gdev, enum hvgr_pm_gpu_reset_type reset_type);

/*
 * hvgr_pm_get_gpu_status - Get the gpu status
 *
 * @param gdev - The gdev object pointer.
 *
 * Return: Current gpu status. For details see hvgr_pm_gpu_status.
 *
 * @date - 2021-02-03
 */
int hvgr_pm_get_gpu_status(struct hvgr_device *gdev);

/*
 * hvgr_pm_set_policy - Change the power policy.
 * This api is only called by sysfs.
 *
 * @param dev - The device object pointer.
 * @param attr - The device attribute pointer.
 * @param buf - New policy.
 * @param count - The number of bytes written to the sysfs file.
 *
 * Return: @count if the function succeeded. An error code on failure.
 *
 * @date - 2021-02-03
 */
ssize_t hvgr_pm_set_policy(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count);

/*
 * hvgr_pm_show_policy - Display the current power policy
 * This api is only called by sysfs.
 *
 * @param dev - The dev object pointer.
 * @param attr - The device attribute pointer.
 * @param buf - Out put buffer.
 *
 * Return: The number of bytes output to @buf.
 *
 * @date - 2021-02-03
 */
ssize_t hvgr_pm_show_policy(struct device *dev, struct device_attribute *attr, char *const buf);

/*
 * hvgr_pm_set_core_mask - Chage the available GPC bit map.
 * This api is only called by sysfs.
 *
 * @param gdev - The gdev object pointer.
 * @param buf - New GPC bit map.
 * @param count - The number of bytes written to the sysfs file.
 *
 * Return: @count if the function succeeded. An error code on failure.
 *
 * @date - 2021-02-03
 */
ssize_t hvgr_pm_set_core_mask(struct hvgr_device *gdev,
	const char *buf, size_t count);

/*
 * hvgr_pm_show_core_mask - Show the available GPC bit map.
 * This api is only called by sysfs.
 *
 * @param gdev - The gdev object pointer.
 * @param buf - Out put buffer.
 *
 * Return: The number of bytes output to @buf.
 *
 * @date - 2021-02-03
 */
ssize_t hvgr_pm_show_core_mask(void *data, char * const buf);

/*
 * hvgr_pm_init - PM init
 *
 * @param gdev - The kmd object pointer.
 *
 * Return: 0 if pm init succeeded. An error code on failure.
 *
 * @date - 2021-02-03
 */
int hvgr_pm_init(struct hvgr_device * const gdev);

/*
 * hvgr_pm_gpu_access_enable - Enable gpu access
 * This API is called only in the device probe phase.
 * @param gdev - The kmd object pointer.
 *
 * Return: 0 if succeeded. An error code on failure.
 *
 * @date - 2022-01-22
 */
int hvgr_pm_gpu_access_enable(struct hvgr_device * const gdev);

/*
 * hvgr_pm_gpu_access_disable - Disable gpu access
 * This API is called only in the device probe phase.
 * @param gdev - The kmd object pointer.
 *
 * Return: 0 if succeeded. An error code on failure.
 *
 * @date - 2022-01-22
 */
int hvgr_pm_gpu_access_disable(struct hvgr_device * const gdev);

/*
 * hvgr_pm_gpu_access_term - Term gpu access
 * This API is called only in the device probe phase.
 * @param gdev - The kmd object pointer.
 *
 * @date - 2022-01-22
 */
void hvgr_pm_gpu_access_term(struct hvgr_device * const gdev);

/*
 * hvgr_pm_term - PM term
 *
 * @param gdev - The gdev object pointer.
 *
 * @date - 2021-02-03
 */
void hvgr_pm_term(struct hvgr_device *gdev);

/*
 * hvgr_pm_request_cycle_counter - Enable cycle counter.
 *
 * @param gdev - The gdev object pointer.
 *
 * @date - 2021-02-03
 */
void hvgr_pm_request_cycle_counter(struct hvgr_device *gdev);

/*
 * hvgr_pm_release_cycle_counter - Disable cycle counter.
 *
 * @param gdev - The gdev object pointer.
 *
 * @date - 2021-02-03
 */
void hvgr_pm_release_cycle_counter(struct hvgr_device *gdev);

/*
 * hvgr_pm_get_availiable_cores - Get availiable GPC bit map
 *
 * @param gdev - The gdev object pointer.
 *
 * Return: Availiable GPC bit map.
 *
 * @date - 2021-02-03
 */
uint32_t hvgr_pm_get_availiable_cores(const struct hvgr_device *gdev);

/*
 * hvgr_pm_reset_done - Notify the waiting thread that the GPU has
 *                             completed a soft reset.
 * This function will be called when a RESET_COMPLETED interrupt occurs.
 *
 * @param gdev - The gdev object pointer.
 *
 * @date - 2021-02-03
 */
void hvgr_pm_reset_done(struct hvgr_device *gdev);

/*
 * hvgr_pm_is_suspending - Check whether the GPU is suspending or
 *                         has been suspended.
 *
 * @param gdev - The gdev object pointer.
 *
 * Return : True if the GPU is suspending or has been suspended.
 * @date - 2021-02-03
 */
bool hvgr_pm_is_suspending(const struct hvgr_device *gdev);

/*
 * hvgr_pm_suspend - Suspend the GPU
 * This api suspends the JS module and then calls hvgr_pm_release_gpu
 * to power off the GPU.
 *
 * This is called when the device should suspend.
 *
 * @param gdev - The gdev object pointer.
 *
 * Return: A standard Linux error code
 *
 * @date - 2021-02-03
 */
int hvgr_pm_suspend(struct hvgr_device *gdev);

/*
 * hvgr_pm_resume - Resume the GPU
 *
 * This api calls hvgr_pm_request_gpu to power on the GPU and resumes
 * the JS module.
 *
 * This is called when the device should resume from suspension.
 *
 * @param gdev - The gdev object pointer.
 *
 * @date - 2021-02-03
 */
void hvgr_pm_resume(struct hvgr_device *gdev);

#ifdef CONFIG_HVGR_GPU_RT_PM
/*
 * hvgr_pm_runtime_suspend - Runtime suspend
 * Different from hvgr_pm_suspend, this api only powers off
 * the BUCK.
 *
 * This api is called only by Linux. If this api is invoked, the GPU has
 * completed the preparations for power-off and can be safely powered off.
 *
 * @param gdev - The gdev object pointer.
 *
 * Return: A standard Linux error code
 *
 * @date - 2021-02-03
 */
int hvgr_pm_runtime_suspend(struct hvgr_device *gdev);

/*
 * hvgr_pm_runtime_resume - Runtime resume
 * Different from hvgr_pm_resume, this api only powers on
 * the BUCK.
 *
 * This api is called only by Linux.
 *
 * Return: A standard Linux error code
 *
 * @param gdev - The gdev object pointer.
 * @date - 2021-02-03
 */
int hvgr_pm_runtime_resume(struct hvgr_device *gdev);

/*
 * hvgr_pm_runtime_idle - Runtime idle
 * This is called by Linux when the device appears to be inactive and it might
 * be placed into a low power state.
 *
 * @param gdev - The gdev object pointer.
 *
 * Return: 0 if device can be suspended, non-zero to avoid runtime autosuspend,
 * otherwise a standard Linux error code.
 *
 * @date - 2021-02-03
 */
int hvgr_pm_runtime_idle(struct hvgr_device *gdev);
#endif

#ifdef CONFIG_HVGR_GPU_THROTTLE_DEVFREQ

/*
 * hvgr_pm_set_force_freq - Set the forced GPU frequency.
 *                          In the same frequency modulation period, if the
 *                          later frequency is lower than the earlier frequency,
 *                          the later frequency is ignored.
 *
 * @param gdev - The gdev object pointer.
 * @param freq - Frequency
 * @param low - If low is true, the min frequency is forcibly set.
 *
 * @date - 2021-07-06
 */
void hvgr_pm_set_force_freq(struct hvgr_device *gdev, unsigned long freq, bool low);

/*
 * hvgr_pm_trans_thro_hint_to_freq - Converts thro_hint to frequencies.
 *
 * @param gdev - The gdev object pointer.
 * @param thro_hint - throttle hint.
 *
 * Return: frequency
 * @date - 2021-07-06
 */
unsigned long hvgr_pm_trans_thro_hint_to_freq(struct hvgr_device *gdev, uint32_t thro_hint);

/*
 * hvgr_pm_show_thro_freq - Show throttle freq.
 *
 * @param gdev - The gdev object pointer.
 * @param buf - Out put buffer.
 *
 * Return: The number of bytes output to @buf.
 * @date - 2021-07-06
 */
ssize_t hvgr_pm_show_thro_freq(struct hvgr_device *gdev, char * const buf);

/*
 * hvgr_pm_show_thro_enable - Show throttle enable flag.
 *
 * @param gdev - The gdev object pointer.
 * @param buf - Out put buffer.
 *
 * Return: The number of bytes output to @buf.
 * @date - 2021-07-06
 */
ssize_t hvgr_pm_show_thro_enable(struct hvgr_device *gdev,
	char * const buf);

/*
 * hvgr_pm_set_thro_enable - Set throttle enable flag
 * This api is only called by sysfs.
 *
 * @param gdev - The gdev object pointer.
 * @param buf - Input param
 * @param count - The number of bytes written to the sysfs file.
 *
 * Return: @count if the function succeeded. An error code on failure.
 *
 * @date - 2021-02-03
 */
ssize_t hvgr_pm_set_thro_enable(struct hvgr_device *gdev, char * const buf,
	size_t count);
#endif

/*
 * hvgr_pm_set_statistics - Set pm statistics enable or disable
 * This api is only called by sysfs.
 *
 * @param gdev - The gdev object pointer.
 * @param buf - Input param
 * @param count - The number of bytes written to the sysfs file.
 *
 * Return: @count if the function succeeded. An error code on failure.
 *
 * @date - 2021-02-03
 */
ssize_t hvgr_pm_set_statistics(struct hvgr_device *gdev,
	const char *buf, size_t count);

/*
 * hvgr_pm_show_statistics - Show pm statistics
 *
 * @param gdev - The gdev object pointer.
 * @param buf - Out put buffer.
 *
 * Return: The number of bytes output to @buf.
 * @date - 2021-07-06
 */
ssize_t hvgr_pm_show_statistics(struct hvgr_device *gdev,
	char *const buf);

/*
 * hvgr_pm_gpu_thermal_cores_control - Temperature-controlled core
 *                                     plug and unplug
 * This API is called when the temperature is too high or when the temperature
 * recovers.
 *
 * @param gdev - The gdev object pointer.
 * @param core_num - Number of GPCs that need to be powered on.
 * @date - 2021-02-03
 */
void hvgr_pm_gpu_thermal_cores_control(u64 core_num);

/*
 * hvgr_pm_change_gpu_active_status - Update gpu status(busy/idle) and calculate busy/idle time.
 *
 * @param gdev - The gdev object pointer.
 * @param now - Current time stamp.
 * @param status - True: GPU is busy. False: GPU is idle.
 *
 * @date - 2022-06-29
 */
void hvgr_pm_change_gpu_active_status(struct hvgr_device *gdev, ktime_t now, bool status);

/*
 * hvgr_pm_set_cl_job_flag - Update cl job flag and calculate cl job time.
 *
 * @param gdev - The gdev object pointer.
 * @param now - Current time stamp.
 * @param status - True: CL jobs are running. False: Other jobs are running or no jobs are running.
 *
 * @date - 2022-06-29
 */
void hvgr_pm_set_cl_job_flag(struct hvgr_device *gdev, ktime_t now, bool status);

/*
 * hvgr_is_always_on - Check whether the GPU is always_on mode
 *
 * @param gdev - The gdev object pointer.
 *
 * Return : True if the GPU is always_on mode.
 * @date - 2022-09-29
 */
bool hvgr_is_always_on(struct hvgr_device *gdev);

/*
 * hvgr_pm_gate_icg_clk - gate icg clk
 *
 * @param gdev - The gdev object pointer.
 */
void hvgr_pm_gate_icg_clk(struct hvgr_device *gdev);

/*
 * hvgr_pm_gpu_hang_cores_control - Gpu hang-controlled core
 *
 * @param gdev - The gdev object pointer.
 * @param core_mask - Bit masp of GPCs that need to be powered on.
 *
 * Return: @count if the function succeeded. An error code on failure.
 *
 * @date - 2023-12-16
 */
ssize_t hvgr_pm_gpu_hang_cores_control(struct hvgr_device *gdev, uint32_t core_mask);
#endif
