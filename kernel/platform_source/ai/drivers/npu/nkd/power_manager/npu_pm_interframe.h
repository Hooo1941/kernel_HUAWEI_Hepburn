/*
 * npu_pm_interframe.h
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
#ifndef _NPU_PM_INTERFRAME_H_
#define _NPU_PM_INTERFRAME_H_
#include "npu_proc_ctx.h"

int npu_enable_interframe_ilde(struct npu_proc_ctx *proc_ctx, uint32_t enable);

void npu_pm_interframe_delay_work_init(struct npu_interframe_idle_manager *idle_manager);
void npu_schedule_interframe_power_down(struct npu_interframe_idle_manager *idle_manager);
void npu_cancel_interframe_power_down(struct npu_interframe_idle_manager *idle_manager);

int npu_handle_interframe_powerup(struct npu_power_mgr *power_mgr, u32 workmode);

#endif // _NPU_PM_INTERFRAME_H_
