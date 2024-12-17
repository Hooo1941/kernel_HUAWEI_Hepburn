/*
 * npu_dfx_profiling.h
 *
 * about npu dfx profiling
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
#ifndef _PROF_DRV_DEV_H_
#define _PROF_DRV_DEV_H_

#include <linux/cdev.h>
#include "npu_user_common.h"

#ifdef NPU_ARCH_V100
#include "npu_dfx_profiling_bs_adapter.h"
#else
#include "npu_dfx_profiling_hwts_adapter.h"
#endif

#ifndef STATIC
#define STATIC
#endif

#define DRV_MODE_VERSION  "NPU 1.01"

#define PROF_OK                         0
#define PROF_ERROR                      (-1)

struct prof_read_config {
	uint64_t dest_buff;
	uint32_t dest_size;
	uint32_t src_size;
	uint32_t channel;
};

/* profiling command */
enum prof_cmd_type {
	PROF_SETTING = 201,
	PROF_START,
	PROF_READ,
	PROF_STOP,
	PROF_CMD_MAX
};

enum profiling_report_switch {
	PROFILING_REPORT_DISABLE = 0,
	PROFILING_REPORT_ENABLE = 1,
};

typedef struct profiling_device {
	struct class *dev_class;
	struct cdev   cdev;
	dev_t devno;
	struct device *device;
} profiling_device_t;

uint64_t get_prof_channel_data_addr(struct prof_buff_desc *prof_buff, u32 channel);
u32 get_prof_channel_data_size(struct prof_buff_desc *prof_buff, u32 channel);

int prof_buffer_init(struct prof_buff_desc *prof_buff);
int prof_buffer_release(struct prof_buff_desc *prof_buff);

struct prof_buff_desc *npu_prof_get_mem(void);

void profiling_clear_mem_ptr(void);

int npu_prof_module_init(void);

void npu_prof_module_exit(void);

#endif /* _PROF_DRV_DEV_H_ */
