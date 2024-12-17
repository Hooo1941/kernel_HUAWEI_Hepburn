/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_DM_DRIVER_ADAPT_DEFS_H
#define HVGR_DM_DRIVER_ADAPT_DEFS_H

#include <linux/types.h>

struct hvgr_dm_fault_msg {
	uint32_t fault_code;
	char *fault_name;
};

enum hvgr_irqs_type {
	JOB_IRQ,
	MMU_IRQ,
	GPU_IRQ,
	FCP_IRQ,
	CQ_IRQ,
	CC_IRQ,
#ifdef CONFIG_HVGR_VIRTUAL_GUEST
	TBU0_NS_IRQ,
	TBU1_NS_IRQ,
	TBU2_NS_IRQ,
	TBU3_NS_IRQ,
#endif
	GPU_IRQ_CNT
};
#endif
