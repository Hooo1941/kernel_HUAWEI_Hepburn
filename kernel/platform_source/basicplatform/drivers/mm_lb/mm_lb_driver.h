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

#ifndef __MM_LB_DRIVER_H
#define __MM_LB_DRIVER_H

#include "mm_lb_priv.h"

#if defined(CONFIG_MM_LB_V500)
#include "mm_lb_driver_v500.h"
#elif defined(CONFIG_MM_LB_V400)
#include "mm_lb_driver_v400.h"
#elif defined(CONFIG_MM_LB_V300)
#include "mm_lb_driver_v300.h"
#else
#ifndef CONFIG_MM_LB_V100
#include "mm_lb_driver_v200.h"
#else
#include "mm_lb_driver_v100.h"
#endif
#endif

#define SLC_ADDR2000(addr, slc_id) \
	((addr) + (slc_id) * 0x2000)
#define SLC_ADDR400(addr, slc_id) \
	((addr) + (slc_id) * 0x400)
#define CMO_ADDR(addr, slc_id) \
	((addr) + (slc_id) * 0x4)

#define QUOTA_L_SHIFT 16
#define QUOTA_L_MASK  ((1U << QUOTA_L_SHIFT) - 1)
#define QUOTA_H_MASK  (~QUOTA_L_MASK)

#define GID_IDX   16

#if defined(CONFIG_MM_LB_V300) && defined(CONFIG_HISI_LB_SCB_ENABLE)
#define SCB_BIT12 12
#define SCB_BIT13 13
#define SCB_BIT14 14
#define SCB_BIT21 21
#define SCB_BIT22 22
#define SCB_BIT23 23
#endif

typedef enum {
	EVENT = 0,
	INTR = 1,
} sync_type;

typedef enum {
	INV = 0,
	CLEAN = 2,
	CI = 4,
} ops_type;

typedef enum {
	CMO_BY_WAY = 0,
	CMO_BY_GID,
	CMO_BY_64PA,
	CMO_BY_128PA,
	CMO_BY_4XKPA,
} cmo_type;

typedef enum {
	ERR_NON,
	SINGLE_BIT_ERR,
	MULTI_BIT_ERR,
	OTHER_ERR,
} bit_err_type;

void set_quota(struct lb_plc_info *policy, unsigned int quota_value);
void clr_bypass_status(struct lb_plc_info *policy);
void lb_invalid_cache(u32 gid, u64 pa, size_t sz);
int reset_ip(struct lb_device *lbd);
void lb_cmo_sync_cmd_by_event(void);
int lb_ops_cache(sync_type synct, ops_type ops, cmo_type by_x,
	u32 bitmap, u64 pa, size_t sz);
unsigned int clear_gid_quota(unsigned int gid);
void set_gid_quota(unsigned int gid, unsigned int quota);
void set_gid_config(unsigned int gid, unsigned int plc,
	unsigned int way, unsigned int quota);
void gid_suspend_enable(unsigned int gid);
void lb_ecc_info_notify(u32 bit_err);

#if defined(CONFIG_MM_LB_V200) || defined(CONFIG_MM_LB_V100) || defined(CONFIG_MM_LB_V400)
static inline unsigned int get_flow(unsigned int id, unsigned int dir)
{
	return 0;
}
static inline void start_flow(unsigned int gid, unsigned int id) { }
static inline void end_flow(unsigned int id) { }
#else
unsigned int get_flow(unsigned int id, unsigned int dir);
void start_flow(unsigned int gid, unsigned int id);
void end_flow(unsigned int id);
#endif

#endif
