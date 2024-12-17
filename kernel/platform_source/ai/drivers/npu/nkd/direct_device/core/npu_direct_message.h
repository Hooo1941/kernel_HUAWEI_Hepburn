/*
 * npu_message.h
 *
 * about npu sq/cq msg
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

#ifndef __NPU_DIRECT_MESSAGE_H
#define __NPU_DIRECT_MESSAGE_H

#include "npu_proc_ctx.h"
#include "npu_rt_task.h"

int npu_pm_proc_hts_enter_wm(struct npu_proc_ctx *proc_ctx, u32 model_id);
int npu_pm_proc_hts_exit_wm(struct npu_proc_ctx *proc_ctx, u32 model_id);
int npu_compose_task_for_request(struct npu_proc_ctx *proc_ctx, u32 model_id,
	npu_rt_task_t *task);
int npu_compose_stop_task(struct npu_proc_ctx *proc_ctx, u32 model_id,
	npu_rt_task_t *task);
#endif