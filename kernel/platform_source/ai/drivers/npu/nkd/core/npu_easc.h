/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2021. All rights reserved.
 * Description:
 * Author:
 * Create: 2021-03-22
 */
#ifndef __NPU_EASC_H
#define __NPU_EASC_H

#include <linux/workqueue.h>
#include <linux/atomic.h>

#include "npu_common.h"

#define NPU_EASC_EXCP_COUNT_MAX  1024

struct npu_easc_exception_info {
	struct work_struct easc_excp_work;
	struct workqueue_struct *easc_excp_wq;
	atomic_t excp_total_count;
};

int npu_easc_init(struct npu_dev_ctx *cur_dev_ctx);

int npu_easc_deinit(void);

#endif
