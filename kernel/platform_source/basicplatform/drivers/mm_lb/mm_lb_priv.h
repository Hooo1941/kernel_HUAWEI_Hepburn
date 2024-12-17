/*
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
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

#ifndef __MM_LB_PRIV_H
#define __MM_LB_PRIV_H

#include <linux/platform_device.h>

#ifndef BIT
#define BIT(n)                   (0x1U << (n))
#endif

#define GID_EN_SHIFT        11

#define GID_STATIC          BIT(0)
#define GID_DYNAMIC         BIT(1)
#define GID_NON_SEC         BIT(2)
#define GID_SEC             BIT(3)
#define GID_VM_STATE        BIT(4)
#define GID_BYPASS_STATE    BIT(5)
#define GID_PAGES_REMAP     BIT(6)
#define GID_SUSPEND         BIT(7)
#define GID_ATF_CONFIG      BIT(8)

#define MAX_JOB_CNT   16
#define CLASS_CNT     2

#define LB_ERROR  1
#define LB_WARN   3
#define LB_INFO   5

static int lb_d_lvl = LB_WARN;

#define lb_print(level, x...)    \
	do {                         \
		if (lb_d_lvl >= (level)) \
			pr_err(x);           \
	} while (0)

enum {
	POWERDOWN,
	POWERUP,
	PARTPOWERDOWN,
	PARTPOWERUP,
	GID_ENABLE,
	GID_BYPASS,
	SEC_GID_REQUEST,
	SEC_GID_RELEASE,
	SEC_GID_CONFIG,
};
typedef union {
	u32 value;
	union {
		struct {
			u32 mid_flt0 : 7;
			u32 res0 : 9;
			u32 mid_flt0_msk : 7;
			u32 res1 : 8;
			u32 mid_flt0_en : 1;
		} gid_mid_flt0;

		struct {
			u32 mid_flt1 : 7;
			u32 res0 : 9;
			u32 mid_flt1_msk : 7;
			u32 res1 : 8;
			u32 mid_flt1_en : 1;
		} gid_mid_flt1;

		struct {
			u32 nor_r_plc : 2;
			u32 nor_w_plc : 2;
			u32 flt_r_plc : 2;
			u32 flt_w_plc : 2;
			u32 sam_rplc  : 1;
			u32 gid_prpty : 2;
			u32 gid_en : 1;
			u32 res : 20;
		} gid_allc_plc;

		struct {
			u32 gid_way_en : 16;
			u32 gid_way_srch : 16;
		} gid_way_allc;

		struct {
			u32 quota_l : 16;
			u32 quota_h : 16;
		} gid_quota;
	} reg;
} gid;

struct lb_plc_info {
	const char *name;
	unsigned int prio;
	unsigned int r_prio;
	unsigned int pid;
	unsigned int stat;
	unsigned int gid;
	unsigned int quota;
	unsigned int r_quota;
	unsigned int flowid;
	unsigned int alloc;
	unsigned int plc;
	unsigned int enabled;
	unsigned long page_count;
	struct vm_info *vm;
	spinlock_t lock;
};

struct gid_policy {
	int nr;
	struct lb_plc_info *ptbl;

	unsigned int gidmap;
	unsigned int max_quota;
	unsigned int max_size;
	unsigned int is_support_bypass;
	unsigned int job_queue[MAX_JOB_CNT][CLASS_CNT];
	int job_cnt;
	spinlock_t lock;
};

struct dsm_info {
	struct dsm_client *ai_client;
	struct work_struct dsm_work;
};

struct lb_device {
	struct device *dev;
	void __iomem  *base;
	int dfx_irq;
	int cmd_irq;
	unsigned int power_state;
	unsigned int is_available;
	unsigned int way_efuse;
	unsigned int way_en;
	unsigned int slc_idx;
	struct gid_policy gdplc;
	struct dsm_info dsm;
};

#define DEFINE_INIT_QUOTA(quota, l, h)                                         \
	do {                                                                   \
		(quota).value = 0;                                             \
		(quota).reg.gid_quota.quota_l = (l);                           \
		(quota).reg.gid_quota.quota_h = (h);                           \
	} while (0)

#define DEFINE_INIT_ALLOC(allc, way_en, way_srch_en)                           \
	do {                                                                   \
		(allc).value = 0;                                              \
		(allc).reg.gid_way_allc.gid_way_en = (way_en);                 \
		(allc).reg.gid_way_allc.gid_way_srch = (way_srch_en);          \
	} while (0)

#define set_bits(mask, reg) writel(((mask) | (readl(reg))), (reg))
#define clr_bits(mask, reg) writel(((~(mask)) & (readl(reg))), (reg))

extern u32 cmo_dummy_pa;
extern struct lb_device *lbdev;

#endif
