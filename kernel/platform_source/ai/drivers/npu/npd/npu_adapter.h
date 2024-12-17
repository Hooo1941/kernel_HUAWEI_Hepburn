/*
 * npu_adapter.h
 *
 * about npu adapter
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
#ifndef __NPU_ADAPTER_INIT_H__
#define __NPU_ADAPTER_INIT_H__

#include <linux/types.h>

enum {
	NPU_INIT_BYPASS = 0,
	NPU_NON_BYPASS,
	NPU_BYPASS,
};

u32 npu_get_feature_bits(void);

int npu_plat_res_mailbox_send(void *mailbox, unsigned long mailbox_len,
	const void *message, unsigned long message_len);

bool npu_plat_is_support_sc(void);

int npu_plat_bypass_status(void);
#endif
