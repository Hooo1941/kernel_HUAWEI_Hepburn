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
#ifndef __NPU_IOCTL_SERVICE_H
#define __NPU_IOCTL_SERVICE_H
#include <linux/cdev.h>

#include "npu_proc_ctx.h"

#define NPU_ID_MAGIC    'D'
#define MAX_NODE_NUM        4
#define MAX_NODE_NUM_OF_FRAME    8
#define MAX_PRIORITY_LEVEL    7

#define NPU_ALLOC_STREAM_ID _IO(NPU_ID_MAGIC, 1)
#define NPU_FREE_STREAM_ID _IO(NPU_ID_MAGIC, 2)
#define NPU_ALLOC_EVENT_ID _IO(NPU_ID_MAGIC, 3)
#define NPU_FREE_EVENT_ID _IO(NPU_ID_MAGIC, 4)
#define NPU_GET_SVM_SSID _IO(NPU_ID_MAGIC, 5)
#define NPU_GET_CHIP_INFO _IO(NPU_ID_MAGIC, 6)
#define NPU_LOAD_MODEL_BUFF _IO(NPU_ID_MAGIC, 7)
#define NPU_ALLOC_MODEL_ID _IO(NPU_ID_MAGIC, 11)
#define NPU_FREE_MODEL_ID _IO(NPU_ID_MAGIC, 12)
#define NPU_REQUEST_SEND _IO(NPU_ID_MAGIC, 13)
#define NPU_RESPONSE_RECEIVE _IO(NPU_ID_MAGIC, 14)

#define NPU_ENTER_WORKMODE            _IO(NPU_ID_MAGIC, 16)
#define NPU_EXIT_WORKMODE             _IO(NPU_ID_MAGIC, 17)
#define NPU_SET_LIMIT                 _IO(NPU_ID_MAGIC, 18)
#define NPU_ENABLE_FEATURE            _IO(NPU_ID_MAGIC, 19)
#define NPU_DISABLE_FEATURE           _IO(NPU_ID_MAGIC, 20)

#define NPU_ALLOC_TASK_ID             _IO(NPU_ID_MAGIC, 27)
#define NPU_FREE_TASK_ID              _IO(NPU_ID_MAGIC, 28)
#define NPU_GET_TS_TIMEOUT_ID         _IO(NPU_ID_MAGIC, 30)
#define NPU_ATTACH_SYSCACHE           _IO(NPU_ID_MAGIC, 31)
#define NPU_DETACH_SYSCACHE           _IO(NPU_ID_MAGIC, 32)

#define NPU_SVM_BIND_PID              _IO(NPU_ID_MAGIC, 35)
#define NPU_SVM_UNBIND_PID            _IO(NPU_ID_MAGIC, 36)

#define NPU_MODEL_PRIORITY_SET        _IO(NPU_ID_MAGIC, 42)
#define NPU_MODEL_PROFILING_SET       _IO(NPU_ID_MAGIC, 43)
#define NPU_MODEL_GET_EXECUTE_INFO    _IO(NPU_ID_MAGIC, 44)
#define NPU_MODEL_SET_EXECUTE_FLAG    _IO(NPU_ID_MAGIC, 45)

#define NPU_BBOX_RECORD_ERROR_TASK    _IO(NPU_ID_MAGIC, 46)
#define NPU_ALLOC_NOTIFY_ID           _IO(NPU_ID_MAGIC, 47)
#define NPU_FREE_NOTIFY_ID            _IO(NPU_ID_MAGIC, 48)

#define NPU_MAX_CMD 65

#define NPU_INTERFRAME_FEATURE_ID     0
#define NPU_MAX_FEATURE_ID            10

long npu_ioctl(struct file *filep, unsigned int cmd, unsigned long arg);

int npu_ioctl_alloc_stream(struct npu_proc_ctx *proc_ctx, unsigned long arg);

int npu_ioctl_alloc_event(struct npu_proc_ctx *proc_ctx, unsigned long arg);

int npu_ioctl_alloc_model(struct npu_proc_ctx *proc_ctx, unsigned long arg);

int npu_ioctl_alloc_task(struct npu_proc_ctx *proc_ctx, unsigned long arg);

int npu_ioctl_alloc_notify(struct npu_proc_ctx *proc_ctx, unsigned long arg);

int npu_ioctl_send_request(struct npu_proc_ctx *proc_ctx, unsigned long arg);

int npu_ioctl_free_stream(struct npu_proc_ctx *proc_ctx, unsigned long arg);

int npu_ioctl_free_event(struct npu_proc_ctx *proc_ctx, unsigned long arg);

int npu_ioctl_free_model(struct npu_proc_ctx *proc_ctx, unsigned long arg);

int npu_ioctl_free_task(struct npu_proc_ctx *proc_ctx, unsigned long arg);

int npu_ioctl_free_notify(struct npu_proc_ctx *proc_ctx, unsigned long arg);

int npu_ioctl_receive_response(struct npu_proc_ctx *proc_ctx, unsigned long arg);

int npu_ioctl_get_chip_info(struct npu_proc_ctx *proc_ctx, unsigned long arg);

int npu_ioctl_get_svm_ssid(struct npu_proc_ctx *proc_ctx, unsigned long arg);

int npu_ioctl_load_model(struct npu_proc_ctx *proc_ctx, unsigned long arg);

int npu_ioctl_get_ts_timeout(struct npu_proc_ctx *proc_ctx, unsigned long arg);

int npu_ioctl_enter_workmode(struct npu_proc_ctx *proc_ctx, unsigned long arg);

int npu_ioctl_exit_workmode(struct npu_proc_ctx *proc_ctx, unsigned long arg);

int npu_ioctl_set_limit(struct npu_proc_ctx *proc_ctx, unsigned long arg);

int npu_ioctl_check_ion(struct npu_proc_ctx *proc_ctx, unsigned long arg);

int npu_ioctl_svm_bind_pid(struct npu_proc_ctx *proc_ctx, unsigned long arg);

int npu_ioctl_svm_unbind_pid(struct npu_proc_ctx *proc_ctx, unsigned long arg);

extern int npu_feature_enable(struct npu_proc_ctx *proc_ctx, uint32_t feature_id, uint32_t enable);

int npu_ioctl_enable_feature(struct npu_proc_ctx *proc_ctx, unsigned long arg);

int npu_ioctl_disable_feature(struct npu_proc_ctx *proc_ctx, unsigned long arg);

int npu_ioctl_set_model_priority(struct npu_proc_ctx *proc_ctx, unsigned long arg);

int npu_ioctl_set_model_profiling(struct npu_proc_ctx *proc_ctx, unsigned long arg);
int npu_ioctl_get_model_execute_info(struct npu_proc_ctx *proc_ctx,
	unsigned long arg);
int npu_ioctl_set_model_execute_enable(struct npu_proc_ctx *proc_ctx,
	unsigned long arg);
int npu_ioctl_bbox_record_err_task(struct npu_proc_ctx *proc_ctx,
	unsigned long arg);

#endif /* __NPU_MANAGER_H */
