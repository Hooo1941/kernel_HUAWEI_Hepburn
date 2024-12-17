/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */


#ifndef HVGR_DM_DRIVER_BASAE_H
#define HVGR_DM_DRIVER_BASAE_H

#include "hvgr_defs.h"


#define HW_PRODUCT_MAX_NUM    7

struct hvgr_hw_product {
	u32 product_model;
	struct {
		u32 version;
		const enum hvgr_hw_issue *issues;
	} map[HW_PRODUCT_MAX_NUM];
};

char *hvgr_get_irq_name(int i);
uint32_t hvgr_get_chipid_reg(void);
void hvgr_get_info_from_reg(struct hvgr_device *gdev);
uint32_t hvgr_get_gpu_id(struct hvgr_device *gdev);
bool hvgr_gpu_reset_needed_by_fault(uint32_t fault_code);
const char *hvgr_exception_name(uint32_t exception_code);
void hvgr_gpu_fault_proc(struct hvgr_device *gdev, uint32_t val);
int hvgr_hw_set_issues_mask(struct hvgr_device * const gdev);
void hvgr_gpu_fault_show_process(struct hvgr_device *gdev,
	char *process_info, uint32_t info_len);
#endif
