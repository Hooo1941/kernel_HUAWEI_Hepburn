/*
 * npu_ioctl_services.h
 *
 * about npu ioctl services
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
#ifndef __NPU_DIRECT_IOCTL_SERVICE_H
#define __NPU_DIRECT_IOCTL_SERVICE_H
#include <linux/cdev.h>

#include "npu_proc_ctx.h"

#define NPU_DIRECT_MAGIC   'S'
#define NPU_DIRECT_MODEL_EXECUTE      _IO(NPU_DIRECT_MAGIC, 1)
#define NPU_DIRECT_RESPONSE_RECEIVE   _IO(NPU_DIRECT_MAGIC, 2)
#define NPU_DIRECT_MODEL_STOP_LOOP_EXECUTE      _IO(NPU_DIRECT_MAGIC, 3)

#define NPU_MAX_DIRECT_CMD  3
#define NPU_NON_SINK_START_TASK_ID      15000

long npu_direct_ioctl(struct file *filep, unsigned int cmd, unsigned long arg);

int npu_ioctl_direct_send_request(struct npu_proc_ctx *proc_ctx,
	unsigned long arg);

#endif /* __NPU_MANAGER_H */
