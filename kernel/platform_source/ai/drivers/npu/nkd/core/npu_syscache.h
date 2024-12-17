/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 * Description:
 * Author:
 * Create: 2021-07-16
 */
#ifndef __NPU_SYSCACHE_H
#define __NPU_SYSCACHE_H

#include <linux/types.h>
#include <linux/timer.h>
#include "npu_proc_ctx.h"
#include "npu_common.h"

#define PGID_NPU 7

int npu_ioctl_attach_syscache(struct npu_proc_ctx *proc_ctx,
	unsigned long arg);
int npu_ioctl_detach_syscache(struct npu_proc_ctx *proc_ctx,
	unsigned long arg);

void npu_sc_timeout(struct timer_list *syscache_timer);
void npu_sc_enable(struct npu_dev_ctx *dev_ctx);
void npu_sc_enable_and_high(struct npu_dev_ctx *dev_ctx);
void npu_sc_low(struct npu_dev_ctx *dev_ctx);
void npu_sc_disable(struct npu_dev_ctx *dev_ctx);

#endif
