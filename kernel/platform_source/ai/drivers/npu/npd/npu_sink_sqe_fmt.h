/*
 * npu_sink_sqe_fmt.h
 *
 * about npu hwts sqe fmt
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
#ifndef _NPU_SINK_SQE_FMT_H_
#define _NPU_SINK_SQE_FMT_H_

#include <linux/types.h>
#include "npu_user_common.h"
int npu_format_hw_sink_sqe(npu_model_desc_info_t *model_desc_info, void *stream_buf_addr,
	void *hwts_sq_addr, u64 ts_sq_len);
int npu_format_sw_sink_sqe(npu_model_desc_t *model_desc, void *stream_buf_addr,	u16 *first_task_id,
	u8 devid, int stream_idx);

#ifdef CONFIG_NPU_HWTS

#ifdef NPU_FPGA_PLAT
#define AIC_AIV_SQE_KERNEL_CREDIT 255
#else
#define AIC_AIV_SQE_KERNEL_CREDIT 2
#endif

int npu_hwts_query_sqe_info(u64 sqe);
#endif

#endif
