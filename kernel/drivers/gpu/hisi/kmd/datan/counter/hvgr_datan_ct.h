/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_DATAN_CT_H
#define HVGR_DATAN_CT_H

#include <linux/atomic.h>
#include <linux/wait.h>
#include <linux/timer.h>
#include <linux/rtc.h>
#include <linux/list.h>
#include <linux/poll.h>
#include <linux/vmalloc.h>
#include <linux/kref.h>

#include "hvgr_datan_ct_time.h"
#include "hvgr_ioctl_datan.h"
#include "hvgr_datan_ct_config.h"

#define CT_DUMP_MIN_INTERVAL             70000ULL /* ns */
#define RING_BUFFER_MAX_SIZE             32
#define CT_DUMP_BY_MS                    1000000000ULL /* ns */
#define GROUP_SIZE_BY_UINT               64 /* every group has 64 uint */
#define HEADER_SIZE_BY_UINT              4  /* beginning of every group 4 uint called header */
#define BODY_SIZE_BY_UINT                60 /* rest of every group 60 uint called body */
#define CT_ONCE_DUMP_GROUP_NUM           8  /* fcm | mmu | btc | bvhg | gpc0 | gpc1 | gpc2 | gpc3 */
#define HEADER_SIZE_BY_BYTES             16 /* 4 * 4 */
#define GROUP_SIZE_BY_BYTES              256 /* 64 * 4 */
#define BITMASK_OFFSET_IN_HEADER         8
#define CT_DUMP_GROUP_ALL                (~0u)
#define DATAN_CT_DUMP_ASID               6U

#define datan_unused(x)                  ((void)(x))

struct hvgr_ctx;
struct hvgr_device;
struct hvgr_mem_area;

enum {
	CT_FCM_INDEX,
	CT_MMU_INDEX,
	CT_BTC_INDEX,
	CT_BVH_INDEX,
	CT_GPC_INDEX,
	CT_MAX_COUNT
};

#define CT_ENABLED    1
#define CT_CLOSED     0

struct hvgr_ct_cfg {
	u32 bitmap[CT_MAX_COUNT];
	u32 set_id;
	u32 ring_num;
};

struct hvgr_ct_context {
	struct mutex lock;
	struct hvgr_device *gdev;
	struct hvgr_ctx *ctx;

	u64 gpu_va;
	struct hvgr_mem_area *area;
	void *cpu_va;
	void *accu_cpu_va;
	uint32_t as_id;

	size_t dump_size;
	u32 bitmap[CT_MAX_COUNT];
	u32 set_id;
	bool reprogram;

	u32 client_num;
	struct list_head waiting_clients;
	struct list_head idle_clients;
	struct task_struct *thread;
	wait_queue_head_t waitq;
	atomic_t request_pending;
	bool accum_flag;
	atomic_t pm_ref_cnt;
};

struct hvgr_ct_dev {
	struct hvgr_ct_context *ct_ctx;
};

struct hvgr_ct_client {
	struct hvgr_ct_context *ct_ctx;
	struct list_head list;
	u32 event_mask;
	size_t dump_size;
	u32 bitmap[CT_MAX_COUNT];
	u32 set_id;
	void __user *legacy_buffer;
	void *accum_buffer;
	u64 dump_time;
	u32 dump_interval;
	wait_queue_head_t waitq;
	bool pending;
	bool accum_flag;

	struct hvgr_ctx *ctx;
	struct hvgr_ct_queue_ctrl *kmd_ctrl;
	struct hvgr_ct_queue_user_ctrl *user_ctrl;
	struct hvgr_ct_queue_data_ctrl *data_ctrl;
	struct hvgr_mem_area *kmd_ctrl_area;
	struct hvgr_mem_area *user_ctrl_area;
	struct hvgr_mem_area *data_ctrl_area;
	uint32_t ring_num;
};

struct hvgr_ct_single_dump_context {
	struct mutex ct_client_lock;
	struct hvgr_ct_client *single_dump_client;
};

void hvgr_api_ct_init_by_dev(struct hvgr_device * const gdev);

void hvgr_api_ct_term_by_dev(struct hvgr_ct_context *ct_ctx);

void hvgr_api_ct_collect_and_disable(struct hvgr_device *gdev, bool dump_ct);

int hvgr_api_ct_enable_and_go_on(struct hvgr_device *gdev);

long hvgr_api_ct_alloc(struct hvgr_ctx *ctx, union hvgr_ioctl_ct_alloc *const para);

int hvgr_api_ct_dump(struct hvgr_ctx *ctx);

int hvgr_api_ct_enable(struct hvgr_ctx *ctx, struct hvgr_ioctl_ct_single_dump_enable *enable);

int hvgr_api_ct_close_ctx(struct hvgr_ctx *ctx);

void hvgr_api_ct_open_ctx(struct hvgr_ctx *ctx);

void hvgr_ct_detach_client(struct hvgr_ct_client *client);

u64 hvgr_ct_get_dump_timestamp(void);

void hvgr_ct_add_client_to_wait_lists(struct hvgr_ct_client *client, struct list_head *waiting_clients);

unsigned int hvgr_ct_data_poll(struct file *filp, poll_table *wait);
long hvgr_ct_data_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
int hvgr_ct_data_release(struct inode *inode, struct file *filp);

#endif /* HVGR_DATAN_CT_H */
