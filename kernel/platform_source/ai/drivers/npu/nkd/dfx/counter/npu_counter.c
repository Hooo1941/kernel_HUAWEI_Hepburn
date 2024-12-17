/*
 * npu_counter.c
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
#include "npu_counter.h"
#include "npu_log.h"
#include <linux/debugfs.h>

static struct npu_counter g_npu_counters[NPU_COUNTER_MAX] = {
	[NPU_COUNTER_SEND_TASK_TO_TS] = {.val = {0}, .name = "send_task_to_ts"},
	[NPU_COUNTER_RECV_RPT_FROM_TS] = {.val = {0}, .name = "recv_rpt_from_ts"},
	[NPU_COUNTER_RECV_COMPUTE_CQ_INT] = {.val = {0}, .name = "recv_compute_cq_int"},
	[NPU_COUNTER_RECV_DFX_CQ_INT] = {.val = {0}, .name = "recv_dfx_cq_int"},
	[NPU_COUNTER_FIND_CQ_INDEX] = {.val = {0}, .name = "find_cq_index"},
	[NPU_COUNTER_WAKE_UP_RESPONSE] = {.val = {0}, .name = "wake_up_response"},
	[NPU_COUNTER_SEND_MODEL_EXE_TO_TS] = {.val = {0}, .name = "send_model_exe_to_ts"},
	[NPU_COUNTER_RECV_MODEL_RPT_FROM_TS] = {.val = {0}, .name = "recv_model_rpt_from_ts"},
};

void npu_counter_print(void)
{
	uint32_t i;
	for (i = 0; i < NPU_COUNTER_MAX; i++)
		npu_drv_warn("npu_counter[%u](%s) = %d\n", i, g_npu_counters[i].name, atomic_read(&g_npu_counters[i].val));
}

struct npu_counter* get_npu_ounnter_base(void)
{
	return g_npu_counters;
}

void npu_counter_inc(npu_counter_type_t counter)
{
	atomic_inc(&g_npu_counters[counter].val);
}

void npu_counter_add(int cnt, npu_counter_type_t counter)
{
	atomic_add(cnt, &g_npu_counters[counter].val);
}

void npu_counter_clear(void)
{
	uint32_t i;
	for (i = 0; i < NPU_COUNTER_MAX; i++)
		atomic_set(&g_npu_counters[i].val, 0);
}