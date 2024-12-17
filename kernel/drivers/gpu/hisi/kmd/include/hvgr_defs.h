/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */


#ifndef HVGR_DEFS_H
#define HVGR_DEFS_H
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/version.h>
#include <linux/mutex.h>
#include <linux/kref.h>
#include <linux/rbtree.h>

#include "dm/hvgr_dm_defs.h"
#include "mem/hvgr_memory_data.h"
#include "hvgr_pm_defs.h"
#include "hvgr_msync_defs.h"
#include "sch/cq/hvgr_cq_data.h"
#include "apicmd/hvgr_softq_defs.h"
#include "datan/hvgr_datan.h"
#include "sch/softjob/hvgr_wq.h"
#include "datan/counter/hvgr_datan_ct.h"
#include "dm/driver/hvgr_dm_hwconfig_issues.h"
#include "external/hvgr_external_base.h"
#include "hvgr_schedule.h"
#include "hvgr_version.h"

#define HVGR_DEV_NAME_NUM   32

#define HVGR_DEV_ANY (-1)

#define ptr_to_u64(x) ((u64)(uintptr_t)(x))
#define u64_to_ptr(x) (void *)((uintptr_t)(x))

#define HVGR_MAX_NR_AS  8

#ifdef HVGR_TEST
#define HVGR_WEAK __attribute__((weak))
#define HVGR_STATIC
#else
#define HVGR_WEAK
#define HVGR_STATIC static
#endif

enum hvgr_device_type {
	HVGR_DEVICE_TYPE_SOC = 0,
	HVGR_DEVICE_TYPE_PCIE = 1
};

#if hvgr_version_ge(HVGR_V350)
struct hvgr_com_ctx {
	struct hvgr_ctx *ctx;
};
#endif


struct hvgr_device {
	/* Kernel data struct pointer. */
	struct device *dev;
	/* misc device, create /dev/newhvgr0 and we can open,close,ioctl and so on */
	struct miscdevice misc_dev;
	char devname[HVGR_DEV_NAME_NUM];
	/* create a debugfs dir name by devname in /sys/kernel/debug/ */
	struct dentry *root_debugfs_dir;
#ifdef CONFIG_DFX_DEBUG_FS
	/* create a debugfs dir name by ctx_id in /sys/kernel/debug/newhvgr0/ */
	struct dentry *ctx_debugfs_dir;
#endif

	/* Device manager data struct */
	struct hvgr_dm_dev dm_dev;
	/* memory manager data struct */
	struct hvgr_mem_dev mem_dev;
	/* Power Manager data struct */
	struct hvgr_pm_dev pm_dev;
	/* CQ Schedule data struct */
	struct hvgr_cq_dev cq_dev;
	struct hvgr_sch_dev sch_dev;
	struct hvgr_datan_dev datan_dev;
	struct hvgr_wq_dev wq_dev;

 	/* Bitpattern of free Address Spaces */
	u16 as_free;
	/* Mapping from active Addrxess Spaces to kmd_context */
	struct hvgr_ctx *as_to_ctx[HVGR_MAX_NR_AS];

	/* Total number of created contexts */
	atomic_t ctx_num;
	struct hvgr_ct_dev ct_dev;

#ifdef CONFIG_HISI_GPU_AI_FENCE_INFO
	/* GPU AI frequency schedule specific pid to collect fence info */
	pid_t ai_freq_game_pid;
	/* GPU AI frequency schedule fence trigger out counter */
	u64 ai_freq_signaled_seqno;
	u32 game_scene;
#endif
#ifdef HVGR_FEATURE_SYSTEM_CACHE
	u32 lb_enable;
#endif
	enum hvgr_device_type device_type;

	/* List of SW workarounds for HW issues */
	unsigned long hw_issues_mask[
		((char)HVGR_HW_ISSUE_END + BITS_PER_LONG - 1) / BITS_PER_LONG];
	struct hvgr_platform_subsys_ops *subsys_ops;

#if hvgr_version_ge(HVGR_V350)
	struct hvgr_com_ctx com_ctx;
#endif

#ifdef CONFIG_HW_LINUX_KERNEL_MM
	struct notifier_block oom_notifier_block;
#endif
};

#define HVGR_GPU_HANG_THRESHOLD   (3)

#define HVGR_CTX_FLAG_ENABLE                (1u << 0)
#define HVGR_CTX_FLAG_COMPAT                (1u << 1)
#define HVGR_CTX_FLAG_CFGED                 (1u << 2)
#define HVGR_CTX_FLAG_DYING                 (1u << 3)
#define HVGR_CTX_FLAG_MEM_PROFILE_ADDED     (1u << 4)

#define hvgr_ctx_is_compat(flags)    (((flags) & HVGR_CTX_FLAG_COMPAT) != 0)

#define HVGR_INVALID_SOFTQ_ID   (-1)
#define HVGR_MAX_SOFTQ_BITMAP   (0x1FF)
#define COMMON_USE_SSID_VAL     (0)
#define COMMON_USE_SSID_EN      (1)

#define HVGR_ENABLE     1
#define HVGR_DISABLE    0

struct hvgr_ctx {
	struct file *kfile;
	struct hvgr_device *gdev;
	struct mutex ctx_lock;
	struct kref ctx_kref;
#ifdef CONFIG_DFX_DEBUG_FS
	struct dentry *debugfs_dir;
#endif
	char *mem_profile_buf;
	uint64_t mem_profile_size;

	atomic_t flags;
	pid_t tgid;
	pid_t pid;
	char process_name[TASK_COMM_LEN];
	char group_name[TASK_COMM_LEN];
	struct task_struct *leader;

	struct list_head dev_link;

	/* Memory data of this context */
	struct hvgr_mem_ctx mem_ctx;
	struct hvgr_datan_ctx datan_ctx;
	struct hvgr_ct_single_dump_context counter_ctx;
#ifdef CONFIG_HVGR_SAMEVA_CACHE
	struct rb_root root;
	struct mutex root_mutex;
	unsigned long root_size;
	unsigned long root_pages;
	bool cache_enabled;
	struct list_head lru_node;
#endif

	uint32_t same_va_4g_base_page;
	u32 id;
	uint32_t asid;
#if hvgr_version_ge(HVGR_V350)
	uint32_t ssid;
#endif
	atomic_t refcount;

	struct hvgr_cq_ctx_data ctx_cq;
	wait_queue_head_t softq_wait;

	struct hvgr_soft_q *sq_ctx;
	struct mutex softq_lock;
	atomic_t softq_closed;
	uint16_t softq_unused;
	struct hvgr_soft_q *id_to_qctx[HVGR_MAX_NR_SOFTQ];

	uint32_t wq_wait_max_cnt;

	/* jcd */
	u64  jsx_jcd;
	void *jcd_addr;
	struct hvgr_mem_area *jcd_area;
	u32 jcd_offset[BASE_JM_MAX_NR_SLOTS];
#ifdef CONFIG_HVGR_DFX_SH
	u32 gaf_flag;
#endif
};

struct hvgr_ctx_list_element {
	struct list_head link;
	struct hvgr_ctx *ctx;
};

static inline bool hvgr_ctx_flag(const struct hvgr_ctx *ctx, uint32_t flag)
{
	return (((uint32_t)atomic_read(&ctx->flags)) & flag) != 0u;
}

static inline void hvgr_ctx_flag_clear(struct hvgr_ctx *ctx, uint32_t flag)
{
	atomic_andnot((int)flag, &ctx->flags);
}

static inline void hvgr_ctx_flag_set(struct hvgr_ctx *ctx, uint32_t flag)
{
	atomic_or((int)flag, &ctx->flags);
}

#endif
