/*
 * frame.h
 *
 * Frame declaration
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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

#ifndef FRAME_EXTERN_H
#define FRAME_EXTERN_H

#include <linux/sched.h>
#include <linux/bitmap.h>
#include <linux/spinlock.h>

/* FPS value : [1, 144] */
#define DEFAULT_FRAME_RATE 60
#define MIN_FRAME_RATE 1
#define MAX_FRAME_RATE 144

/* MARGIN value : [-100, 100] */
#define DEFAULT_VLOAD_MARGIN 16
#define MIN_VLOAD_MARGIN (-100)
#define MAX_VLOAD_MARGIN 0xffff

#define FRAME_MAX_VLOAD SCHED_CAPACITY_SCALE
#define FRAME_MAX_LOAD SCHED_CAPACITY_SCALE
#define FRAME_UTIL_INVALID_FACTOR 4
#define FRAME_DEFAULT_MAX_UTIL SCHED_CAPACITY_SCALE
#define FRAME_DEFAULT_MIN_PREV_UTIL 0
#define FRAME_DEFAULT_MAX_PREV_UTIL SCHED_CAPACITY_SCALE

#define INVALID_PREFERRED_CLUSTER 10

enum rtg_type {
	VIP = 0,
	TOP_TASK_KEY,
	TOP_TASK,
	NORMAL_TASK,
	RTG_TYPE_MAX,
};

#define FRAME_STATE		(1U << 0)
#define FRAME_ACT_STATE	(1U << 1)
#define FRAME_FREQ		(1U << 2)
#define MAX_TID_NUM 5
#define MAX_FRAME_CFS_THREADS 3

struct frame_thread_info {
	int prio;
	int pid;
	int tid;
	int thread[MAX_TID_NUM];
	int thread_num;
};

#ifdef CONFIG_HW_RTG_MULTI_FRAME
struct multi_frame_id_manager {
	DECLARE_BITMAP(id_map, MULTI_FRAME_NUM);
	unsigned int offset;
	rwlock_t lock;
};
struct frame_info *rtg_active_multi_frame_info(int id);
struct frame_info *rtg_multi_frame_info(int id);
int alloc_multi_frame_info(void);
void release_multi_frame_info(int id);
void clear_multi_frame_info(void);
#else
static inline int alloc_multi_frame_info(void)
{
	return -EFAULT;
}
static inline void release_multi_frame_info(int id __maybe_unused) {}
static inline void clear_multi_frame_info(void) {}
#endif

#ifdef CONFIG_FRAME_RTG
bool is_frame_rtg(int id);
struct frame_info *rtg_frame_info(int id);
bool is_frame_task(struct task_struct *task);
int set_frame_rate(struct frame_info *frame_info, int rate);
int get_frame_rate(struct frame_info *frame_info);
int set_frame_margin(struct frame_info *frame_info, int margin);
int set_frame_status(struct frame_info *frame_info, unsigned long status);
int set_frame_max_util(struct frame_info *frame_info, int max_util);
void set_frame_sched_state(struct frame_info *frame_info, bool enable);
int set_frame_timestamp(struct frame_info *frame_info, unsigned long timestamp);
int set_frame_min_util(struct frame_info *frame_info, int min_util, bool is_boost);
int set_frame_min_util_and_margin(struct frame_info *frame_info, int min_util, int margin);
void update_frame_thread(struct frame_info *frame_info, struct frame_thread_info *frame_thread_info);
void update_frame_cfs_thread(struct frame_info *frame_info, int tid, int index);
int update_frame_isolation(void);
struct frame_info *lookup_frame_info_by_task(struct task_struct *task);
struct frame_info *lookup_frame_info_by_pid(int pid);
int get_frame_prio_by_id(int rtgid);
void set_frame_prio(struct frame_info *frame_info, int prio);
#else
static inline bool is_frame_rtg(int id __maybe_unused)
{
	return false;
}

static inline struct frame_info *rtg_frame_info(int id __maybe_unused)
{
	return NULL;
}

static inline bool is_frame_task(struct task_struct *task __maybe_unused)
{
	return false;
}

static inline int set_frame_rate(struct frame_info *frame_info __maybe_unused,
				 int rate __maybe_unused)
{
	return 0;
}

static inline int get_frame_rate(struct frame_info *frame_info __maybe_unused)
{
	return -EINVAL;
}

static inline int set_frame_margin(struct frame_info *frame_info __maybe_unused,
				   int margin __maybe_unused)
{
	return 0;
}

static inline int set_frame_status(struct frame_info *frame_info __maybe_unused,
				   unsigned long status __maybe_unused)
{
	return 0;
}

static inline int set_frame_max_util(struct frame_info *frame_info __maybe_unused,
				     int max_util __maybe_unused)
{
	return 0;
}

static inline void set_frame_sched_state(struct frame_info *frame_info __maybe_unused,
					 bool enable __maybe_unused) {}

static inline int set_frame_timestamp(struct frame_info *frame_info __maybe_unused,
				      unsigned long timestamp __maybe_unused)
{
	return 0;
}

static inline int set_frame_min_util(struct frame_info *frame_info __maybe_unused,
				     int min_util __maybe_unused, bool is_boost __maybe_unused)
{
	return 0;
}

static inline int set_frame_min_util_and_margin(struct frame_info *frame_info __maybe_unused,
						int min_util __maybe_unused, int margin __maybe_unused)
{
	return 0;
}

static inline void update_frame_thread(struct frame_info *frame_info __maybe_unused,
				       struct frame_thread_info *frame_thread_info __maybe_unused)
{
}

static inline void update_frame_cfs_thread(struct frame_info *frame_info __maybe_unused,
					   int tid __maybe_unused, int index __maybe_unused) {}

static inline int update_frame_isolation(void)
{
	return 1;
}

static inline struct frame_info *lookup_frame_info_by_task(struct task_struct *task __maybe_unused)
{
	return NULL;
}

static inline struct frame_info *lookup_frame_info_by_pid(int pid __maybe_unused)
{
	return NULL;
}

static inline int get_frame_prio_by_id(int rtgid __maybe_unused)
{
	return -EINVAL;
}

static inline void set_frame_prio(struct frame_info *frame_info __maybe_unused,
				  int prio __maybe_unused) {}
#endif

#endif // FRAME_EXTERN_H
