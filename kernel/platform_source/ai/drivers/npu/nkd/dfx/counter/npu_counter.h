/*
 * npu_counter.h
 *
 * about npu counter
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
#ifndef __NPU_COUNTER_H
#define __NPU_COUNTER_H

#include "npu_platform.h"
#include <linux/atomic.h>
#include <linux/debugfs.h>
#include "npu_common.h"

typedef enum npu_counter_type {
	NPU_COUNTER_SEND_TASK_TO_TS = 0,
	NPU_COUNTER_RECV_RPT_FROM_TS,
	NPU_COUNTER_RECV_COMPUTE_CQ_INT,
	NPU_COUNTER_RECV_DFX_CQ_INT,
	NPU_COUNTER_FIND_CQ_INDEX,
	NPU_COUNTER_WAKE_UP_RESPONSE,
	NPU_COUNTER_SEND_MODEL_EXE_TO_TS,
	NPU_COUNTER_RECV_MODEL_RPT_FROM_TS,
	NPU_COUNTER_MAX,
} npu_counter_type_t;

struct npu_counter {
	atomic_t val;
	const char *name;
};

struct npu_counter* get_npu_ounnter_base(void);
void npu_counter_print(void);
void npu_counter_inc(npu_counter_type_t counter);
void npu_counter_add(int cnt, npu_counter_type_t counter);
void npu_counter_clear(void);

#endif /* __NPU_COUNTER_H */