/*
 * npu_pm_framework.h
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
#ifndef __NPU_PM_FRAMEWORK_H
#define __NPU_PM_FRAMEWORK_H

#include "npu_proc_ctx.h"
#include "npu_pm_interframe.h"
#include "npu_except_mgr.h"

typedef struct {
	u32 data;
} npu_work_mode_set;

typedef struct {
	u32 data;
} npu_subip_set;

struct npu_pm_work_strategy {
	enum npu_workmode work_mode;        /* curr_work_mode */
	npu_work_mode_set work_mode_set;    /* curr_work_mode is mutually exclusive with work_mode in work_mode_set */
	bool is_idle_support;               /* is curr_work_mode support idle timer */
	npu_subip_set subip_set;            /* curr_work_mode need these subip */
};

enum npu_pm_ops {
	POWER_UP    = 0,
	POWER_DOWN  = 1,
	OPS_MAX,
};

struct npu_pm_subip_action {
	u32 type;
	int (*power_up)(u32, u32);
	int (*power_down)(u32, u32);
};

enum npu_power_stage {
	NPU_PM_DOWN,
	NPU_PM_NPUCPU,
	NPU_PM_SMMU,
	NPU_PM_TS,
	NPU_PM_UP
};

enum npu_work_status {
	WORK_IDLE,
	WORK_ADDING,
	WORK_CANCELING,
	WORK_ADDED
};

#define npu_pm_safe_call(dev_ctx, subip, expr) do { \
	mutex_lock(&((dev_ctx)->pm.npu_power_mutex)); \
	if (((((dev_ctx)->pm.work_mode >> NPU_SEC) & 0x1) == 0) && \
		(((((dev_ctx)->pm.cur_subip_set) >> (subip)) & 0x1) != 0)) { \
		(expr); \
	} \
	mutex_unlock(&((dev_ctx)->pm.npu_power_mutex)); \
} while (0)

#define npu_pm_safe_call_with_return(dev_ctx, subip, expr, ret) do { \
	mutex_lock(&((dev_ctx)->pm.npu_power_mutex)); \
	if (((((dev_ctx)->pm.work_mode >> NPU_SEC) & 0x1) == 0) && \
		(((((dev_ctx)->pm.cur_subip_set) >> (subip)) & 0x1) != 0)) { \
		(ret) = (expr); \
	} \
	mutex_unlock(&((dev_ctx)->pm.npu_power_mutex)); \
} while (0)

int npu_pm_proc_ioctl_enter_wm(struct npu_proc_ctx *proc_ctx,
	struct npu_dev_ctx *dev_ctx, npu_work_mode_info_t *wm_info);

int npu_pm_proc_ioctl_exit_wm(struct npu_proc_ctx *proc_ctx,
	struct npu_dev_ctx *dev_ctx, u32 work_mode);

int npu_pm_proc_send_task_enter_wm(struct npu_proc_ctx *proc_ctx,
	struct npu_dev_ctx *dev_ctx, uint32_t work_mode);

int npu_pm_proc_release_task_exit_wm(struct npu_proc_ctx *proc_ctx,
	struct npu_dev_ctx *dev_ctx, u32 num, u32 work_mode);

int npu_pm_proc_ffrt_enter_wm(struct npu_proc_ctx *proc_ctx,
	struct npu_dev_ctx *dev_ctx, u32 work_mode);

int npu_pm_proc_ffrt_exit_wm(struct npu_proc_ctx *proc_ctx,
	struct npu_dev_ctx *dev_ctx, u32 work_mode);

int npu_pm_proc_release_exit_wm(struct npu_proc_ctx *proc_ctx,
	struct npu_dev_ctx *dev_ctx);

int npu_pm_handle_exception(struct npu_dev_ctx *dev_ctx, struct npu_exception_record* er);

int npu_pm_handle_power_up(struct npu_dev_ctx *dev_ctx);

void npu_pm_adapt_init(struct npu_dev_ctx *dev_ctx);

int npu_ctrl_core(u8 dev_id, u32 core_num);

int npu_pm_powerup_proc(struct npu_power_mgr *power_mgr,
	u32 work_mode, u32 delta_subip);

int npu_pm_powerdown_proc(struct npu_power_mgr *power_mgr,
	u32 work_mode, u32 delta_subip);

int npu_suspend_powerdown(struct npu_dev_ctx *dev_ctx);

#endif
