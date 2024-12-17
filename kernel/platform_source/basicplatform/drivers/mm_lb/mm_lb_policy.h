/*
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __MM_LB_POLICY_H
#define __MM_LB_POLICY_H

#include "mm_lb_priv.h"

#define LB_PRIO_HALF_LVL 0x5

enum lb_prio {
	LB_CACHE_LOW_PRIO = 0x99,
	LB_CACHE_HIGH_PRIO = 0x50,
	LB_BUF_LOW_PRIO = 0x40,
	LB_BUF_HIGH_PRIO = 0x20,
};

enum {
	STATUS_DISABLE,
	STATUS_ENABLE,
	STATUS_BYPASS,
};

unsigned int __is_lb_available(void);
unsigned int is_support_bypass(void);
bool policy_id_valid(unsigned int policy_id, unsigned int mask);
int lb_pfn_valid_within(phys_addr_t phys);
int set_gid(struct lb_plc_info *policy, unsigned int way_alloc,
		unsigned int quota_set, unsigned int plc_set);
int reset_gid(struct lb_plc_info *policy);
unsigned int alloc_gid(struct lb_plc_info *policy);
void free_gid(struct lb_plc_info *policy);
int gid_enable(struct lb_plc_info *policy);
int gid_bypass(struct lb_plc_info *policy);
int sec_gid_enable(struct lb_plc_info *policy);
int sec_gid_disable(struct lb_plc_info *policy);
unsigned int alloc_quota(struct lb_plc_info *policy, unsigned int quota);
void free_quota(struct lb_plc_info *policy, unsigned int quota);
unsigned int get_max_quota(void);
int lb_set_master_policy(unsigned int pid, unsigned int quota,
		unsigned int prio);
int lb_reset_master_policy(unsigned int pid);
int set_page(struct lb_plc_info *policy,
		struct page *page_alloc, size_t count);
void reset_page(struct lb_plc_info *policy,
		struct page *page, size_t count);
#if defined(CONFIG_MM_LB_PMU) || defined(CONFIG_MM_LB_PMU_V500)
int lb_pmu_init(struct platform_device *pdev, struct lb_device *lbd);
#else
static inline int lb_pmu_init(
	struct platform_device *pdev, struct lb_device *lbd) { return 0; }
#endif

#endif
