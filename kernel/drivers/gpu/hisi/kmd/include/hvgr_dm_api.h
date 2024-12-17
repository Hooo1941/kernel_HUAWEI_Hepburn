/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_DM_API_H
#define HVGR_DM_API_H

#include <linux/errno.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/kref.h>

#include "hvgr_ioctl.h"
#include "apicmd/hvgr_ioctl_dev.h"
#include "hvgr_dm_defs.h"

struct hvgr_ctx;
struct hvgr_device;

typedef enum hvgr_dmd_id {
	DMD_JOB_FAIL = 0,
	DMD_JOB_HANG,
	DMD_FCP_LOAD_FAIL,
	DMD_BUS_FAULT,
	DMD_PAGE_FAULT,
	DMD_PW_ONOFF_FAIL,
	DMD_HARD_RESET,
	DMD_SH_NV_MARK,
	DMD_SH_HANG_NV_MARK
} hvgr_gpu_fault_id;

#define hvgr_hw_has_issue(gdev, issue) \
	test_bit(issue, &(gdev)->hw_issues_mask[0])

#define gpu_control_reg(gdev, offset) \
	(((gdev)->dm_dev.reg_base.gpu_reg_base) + (uint64_t)(offset))

#define job_control_reg(gdev, offset) \
	(((gdev)->dm_dev.reg_base.job_reg_base) + (uint64_t)(offset))

#define mmu_control_reg(gdev, offset) \
	(((gdev)->dm_dev.reg_base.mmu_stage1_reg_base) + (uint64_t)(offset))

#define cq_control_reg(gdev, offset) \
	(((gdev)->dm_dev.reg_base.cq_reg_base) + (uint64_t)(offset))

#define u64_to_ptr_user(x) (void __user *)((uintptr_t)(x))

#define hvgr_ioctl_handle(_ctx, _cmd, _func) \
	do { \
		ret = _func((_ctx)); \
		goto exit; \
	} while (0)

#define hvgr_ioctl_handle_w(_ctx, _cmd, _func, _arg, _type) \
	do { \
		_type _para = {0}; \
		long _ret; \
		unsigned long cp_ret; \
		void __user *_upara = u64_to_ptr_user(_arg); \
		if (_upara == NULL) { \
			ret = -EINVAL; \
			goto exit; \
		} \
		cp_ret = \
			copy_from_user(&_para, _upara, sizeof(_para)); \
		if (cp_ret) { \
			ret = -EFAULT; \
			goto exit; \
		} \
		_ret = _func((_ctx), &_para); \
		ret = _ret; \
		goto exit; \
	} while (0)

#define hvgr_ioctl_handle_r(_ctx, _cmd, _func, _arg, _type) \
	do { \
		_type _para = {0}; \
		unsigned long cp_ret; \
		long _ret; \
		void __user *_upara = u64_to_ptr_user(_arg); \
		if (_upara == NULL)  { \
			ret = -EINVAL; \
			goto exit; \
		} \
		_ret = _func((_ctx), &_para); \
		if (_ret) { \
			ret = _ret; \
			goto exit; \
		} \
		cp_ret = \
			copy_to_user(_upara, &_para, sizeof(_para)); \
		if (cp_ret) { \
			ret = -EFAULT; \
			goto exit; \
		} \
		ret = _ret; \
		goto exit; \
	} while (0)

#define hvgr_ioctl_handle_wr(_ctx, _cmd, _func, _arg, _type) \
	do { \
		_type _para = {0}; \
		long _ret; \
		unsigned long cp_ret; \
		void __user *_upara = u64_to_ptr_user(_arg); \
		if (_upara == NULL) { \
			ret = -EINVAL; \
			goto exit; \
		} \
		cp_ret = \
			copy_from_user(&_para, _upara, sizeof(_para)); \
		if (cp_ret) { \
			ret = -EFAULT; \
			goto exit; \
		} \
		_ret = _func((_ctx), &_para); \
		if (_ret) { \
			ret = _ret; \
			goto exit; \
		} \
		cp_ret = copy_to_user(_upara, &_para, sizeof(_para)); \
		if (cp_ret) { \
			ret = -EFAULT; \
			goto exit; \
		} \
		ret = _ret; \
		goto exit; \
	} while (0)

/*
 * hvgr_ioctl_get_driver_info - Get driver info
 *
 * @ctx:     The hvgr context info
 * @para:    The driver info
 *
 * Return: 0 on success or error code
 */
long hvgr_ioctl_get_driver_info(struct hvgr_ctx * const ctx,
	hvgr_ioctl_para_version * const para);

/*
 * hvgr_ioctl_cfg_driver - Configure ctx resource and only this ioctl complete
 * then other ioctl could set.
 *
 * @ctx:     The hvgr context info
 * @para:    The driver ctx cfg info
 *
 * Return: 0 on success or error code
 */
long hvgr_ioctl_cfg_driver(struct hvgr_ctx * const ctx, hvgr_ioctl_para_drv_cfg * const para);

/*
 * hvgr_ioctl_decfg_driver - Deconfigure ctx resource and make sure release release ctx success.
 *
 * @ctx:     The hvgr context info
 * @para:    The driver ctx cfg info
 *
 * Return: 0 on success or error code
 */
long hvgr_ioctl_decfg_driver(struct hvgr_ctx * const ctx, hvgr_ioctl_para_drv_cfg * const para);

/*
 * hvgr_ioctl_dm - device manage ioctl
 *
 * @ctx:     The hvgr context info
 * @cmd:     cmdcode
 * @arg:     para from user mode
 *
 * Copy arg from user mode to kerenl mode and invoke function by cmd.
 * Return: 0 on success or error code
 */
long hvgr_ioctl_dm(struct hvgr_ctx * const ctx, unsigned int cmd, unsigned long arg);

/*
 * hvgr_crg_reset - Reset gpu region from soc register.
 *
 * @gdev:    The hvgr device
 *
 * Return: 0 on success or error code
 */
int hvgr_crg_reset(struct hvgr_device *gdev);

/*
 * hvgr_is_fpga - Gpu is fpga or soc.
 *
 * @gdev:    The hvgr device
 *
 */
bool hvgr_is_fpga(struct hvgr_device * const gdev);

/*
 * hvgr_synchronize_irqs - Sync irqs before the gpu powered off.
 *
 * @gdev:    The hvgr device
 *
 */
void hvgr_synchronize_irqs(struct hvgr_device *gdev);

/*
 * hvgr_register_irq - Register interrupt to kernel.
 *
 * @gdev:    The hvgr device
 * @type:    irqs type likes gpu, cq, memory, fcp and so on.
 * @handler  irq handle function.
 *
 * Return: 0 on success or error code
 */
int hvgr_register_irq(struct hvgr_device *gdev, enum hvgr_irqs_type type, irq_handler_t handler);

/*
 * hvgr_read_reg - Read gpu register
 *
 * @gdev:    The hvgr device
 * @addr:    The gpu register address
 *
 * Return: value of gpu register
 */
uint32_t hvgr_read_reg(struct hvgr_device * const gdev, uint64_t addr);

/*
 * hvgr_write_reg - Write gpu register
 *
 * @gdev:    The hvgr device
 * @addr:    The gpu register address
 * @value:   The value of gpu register
 *
 */
void hvgr_write_reg(struct hvgr_device * const gdev, uint64_t addr, uint32_t value);

/*
 * hvgr_get_device - Get hvgr_device pointer
 *
 *
 * Return: hvgr_device pointer
 */
struct hvgr_device *hvgr_get_device(void);

/*
 * hvgr_gpu_irq - gpu irq handler
 *
 * @irq:    The irq type
 * @data:   gdev
 *
 * Return: irq handle result
 */
irqreturn_t hvgr_gpu_irq(int irq, void *data);

/*
 * hvgr_device_probe - gpu device level init
 *
 * @gdev:    The hvgr device
 *
 * init gpu device level resource.
 *
 * Return: 0 on success or error code
 */
int hvgr_device_probe(struct hvgr_device *gdev);

/*
 * hvgr_device_remove - gpu device level uninit
 *
 * @gdev:    The hvgr device
 *
 * remove gpu device level resource.
 *
 * Return: 0 on success or error code
 */
int hvgr_device_remove(struct hvgr_device *gdev);

/*
 * hvgr_get_core_nums - Get gpc cores numbers
 *
 * @gdev:    The hvgr device
 *
 * Return: numbers of gpc cores
 */
uint32_t hvgr_get_core_nums(struct hvgr_device *gdev);

/*
 * hvgr_init_control_baseaddr - get baseaddr from different hw version
 *
 * @gdev:    The hvgr device
 *
 * Return: 0 on success or error code
 */
int hvgr_init_control_baseaddr(struct hvgr_device * const gdev);

/*
 * hvgr_fcp_load_by_poll - Load fcp firmware by polling INT status
 *
 * @gdev:    The hvgr device
 *
 * Return: 0 on success or error code
 */
int hvgr_fcp_load_by_poll(struct hvgr_device *gdev);

/*
 * hvgr_disable_interrupts - Disable gpu interrupts
 *
 * @gdev:    The hvgr device
 *
 * mask off gpu interrupts and clear it.
 */
void hvgr_disable_interrupts(struct hvgr_device *gdev);

/*
 * hvgr_enable_interrupts - Enable gpu interrupts
 *
 * @gdev:    The hvgr device
 *
 * clear gpu interrupts and mask it.
 *
 */
void hvgr_enable_interrupts(struct hvgr_device *gdev);

/*
 * hvgr_fcp_loader_entry - Trigger load fcp firmware
 *
 * @gdev:    The hvgr device
 */
void hvgr_fcp_loader_entry(struct hvgr_device *gdev);

/*
 * hvgr_dmd_msg_init - init dmd message
 *
 * @gdev:    The hvgr device
 *
 * Return: 0 on success or error code
 */
int hvgr_dmd_msg_init(struct hvgr_device *gdev);

/*
 * hvgr_dmd_msg_term - terminate dmd message
 *
 * @gdev:    The hvgr device
 */
void hvgr_dmd_msg_term(struct hvgr_device *gdev);

/*
 * hvgr_dmd_msg_set - set dmd message
 *
 * @gdev:    The hvgr device
 * @dmd_id:  The dmd id
 * @dmd_log: The dmd log
 *
 */
void hvgr_dmd_msg_set(struct hvgr_device *gdev,  enum hvgr_dmd_id dmd_id,
	const char *dmd_log);

/*
 * hvgr_dmd_msg_report - report dmd message
 *
 * @gdev:    The hvgr device
 */
void hvgr_dmd_msg_report(struct hvgr_device *gdev);

/*
 * hvgr_dmd_id_check - check weather dmd id is valid
 *
 * @gdev:    The hvgr device
 */
bool hvgr_dmd_id_check(struct hvgr_device *gdev);

/*
 * hvgr_dmd_msg_clear - Clear dmd msg after gpu reset done if dmdmsg is not null.
 *
 * @gdev:    The hvgr device
 */
void hvgr_dmd_msg_clear(struct hvgr_device *gdev);

/*
 * hvgr_get_ctx_compat - get ctx 32bit or 64bit
 *
 * @ctx:    The hvgr context info
 */
int hvgr_get_ctx_compat(struct hvgr_ctx *ctx);

/*
 * hvgr_get_main_process_name - get ctx tgid process name
 *
 * @ctx:    The hvgr context info
 */
char *hvgr_get_main_process_name(struct hvgr_ctx *ctx);

/*
 * hvgr_hw_set_issues_mask - Set the hardware issues mask based on the GPU ID
 *
 * @gdev:    The hvgr device
 */
int hvgr_hw_set_issues_mask(struct hvgr_device * const gdev);

/*
 * hvgr_dm_get_soc_timestamp - get soc timestamp
 *
 * @gdev:    The hvgr device
 */
uint64_t hvgr_dm_get_soc_timestamp(struct hvgr_device * const gdev);

/*
 * hvgr_ctx_kref_release - hvgr_ctx
 *
 * @ctx_kref:    The hvgr_ctx ref count
 */
void hvgr_ctx_kref_release(struct kref *ctx_kref);

#ifdef CONFIG_HVGR_DFX_SH
void hvgr_dmd_set_sid(struct hvgr_ctx *ctx, uint32_t sid);

void hvgr_dmd_config_ctx_gaf(struct hvgr_ctx *ctx, uint32_t flag);

uint32_t hvgr_dmd_read_gaf(struct hvgr_device *gdev);

void hvgr_dmd_sr_awake(struct hvgr_device *gdev);

void hvgr_dmd_sr_relax(struct hvgr_device *gdev);

int hvgr_dmd_nv_read(struct hvgr_device *gdev, struct hvgr_dmd_nv_data *data);

int hvgr_dmd_nv_write(struct hvgr_device *gdev, struct hvgr_dmd_nv_data *data);

void hvgr_updata_core_mask_info(struct hvgr_device *gdev, uint32_t cores);

uint32_t hvgr_dmd_get_core_mask_info(struct hvgr_device *gdev);
#endif

#endif
