/*
 * dsp msg.h
 *
 * head of dsp msg.c
 *
 * Copyright (c) 2015-2020 Huawei Technologies Co., Ltd.
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

#ifndef __DSP_MSG_H__
#define __DSP_MSG_H__

#include <linux/list.h>
#include "dsp_misc.h"

void dsp_msg_init(struct dsp_misc_proc *misc_proc, unsigned char *base_virt,
	unsigned char *base_phy);
void dsp_msg_deinit(void);
int rcv_dsp_sync_msg(struct list_head *node, unsigned int sn);
int sync_cmd_from_kernel(void *para_kernel_in, unsigned int para_kernel_size_in,
	void *para_kernel_out, unsigned int para_kernel_size_out);
#endif /* __DSP_MSG_H__ */

