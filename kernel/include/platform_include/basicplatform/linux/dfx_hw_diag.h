/*
 *
 * hardware diaginfo record module.
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
#ifndef __DFX_HW_DIAG_H__
#define __DFX_HW_DIAG_H__

#include <platform_include/basicplatform/linux/dfx_bbox_diaginfo.h>
#include <linux/spinlock_types.h>

#ifdef CONFIG_DFX_NOC
struct dfx_diag_noc_info {
	char *init_flow;
	char *target_flow;
};
#endif

#ifdef CONFIG_DFX_SEC_QIC 
struct dfx_diag_qic_info {
	unsigned int buskey;
};
#endif

struct dfx_diag_panic_info {
	unsigned int cpu_num;
};

union dfx_hw_diag_info {
#ifdef CONFIG_DFX_NOC
	struct dfx_diag_noc_info noc_info;
#endif
#ifdef CONFIG_DFX_SEC_QIC
	struct dfx_diag_qic_info qic_info;
#endif
	struct dfx_diag_panic_info cpu_info;
};

struct dfx_hw_diag_dev {
	spinlock_t record_lock;
	struct dfx_hw_diag_trace *trace_addr;
	unsigned int trace_size;
	unsigned int trace_max_num;
};

#ifdef CONFIG_DFX_HW_DIAG
void dfx_hw_diaginfo_trace(unsigned int err_id, const union dfx_hw_diag_info *diaginfo);
void dfx_hw_diaginfo_record(const char *date);
void dfx_hw_diag_init(void);
int dfx_hw_diag_get_panic_cpunum(void);
#ifdef CONFIG_DFX_SEC_QIC
unsigned int dfx_hw_diag_get_qic_buskey(void);
#endif
#else
static inline void dfx_hw_diaginfo_trace(unsigned int err_id, const union dfx_hw_diag_info *diaginfo) { return; }
static inline void dfx_hw_diaginfo_record(const char *date) { return; }
static inline void dfx_hw_diag_init(void) { return; }
static inline int dfx_hw_diag_get_panic_cpunum(void) {return -1;}
#ifdef CONFIG_DFX_SEC_QIC
static inline unsigned int dfx_hw_diag_get_qic_buskey(void) {return 0;}
#endif
#endif

#endif
