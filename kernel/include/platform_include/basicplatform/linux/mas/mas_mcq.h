/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 * Description: MAS MCQ header file
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __MAS_MCQ_H__
#define __MAS_MCQ_H__
#include <linux/blk_types.h>
#include <linux/blk-mq.h>
#include <platform_include/basicplatform/linux/hck/ufs/hck_ufs_mas_mcq.h>

extern struct blk_tagset_ops mas_mcq_tagset_ops;
extern struct blk_queue_ops mas_ufs_blk_queue_ops;

/*
 * This struct defines all the variable for ufs mcq io-scheduler per lld.
 */
struct mas_mcq_sched_ds_lld {
	struct blk_dev_lld *lld;

	atomic_t per_cpu_tag_used[NR_CPUS + 1];
	atomic_t per_cpu_hp_req_num[NR_CPUS + 1];

	atomic_t per_cpu_sync_req_inflt[NR_CPUS + 1];
	atomic_t per_cpu_vip_req_inflt[NR_CPUS + 1];
	atomic_t per_cpu_fg_req_inflt[NR_CPUS + 1];
	atomic_t per_cpu_cp_req_inflt[NR_CPUS + 1];

	atomic_t per_cpu_req_wait[NR_CPUS + 1];
	struct list_head per_cpu_req_wait_list[NR_CPUS + 1];
	spinlock_t per_cpu_req_wait_lock[NR_CPUS + 1];

	atomic_t async_io_inflt_cnt;
	atomic_t ref_cnt;
};

struct mas_mcq_tags {
	struct sbitmap_queue mcq_tags[NR_CPUS + 1];
	unsigned int cur_tags[NR_CPUS + 1];
	unsigned int sum_tags[NR_CPUS + 1];
};

/* mas_blk_iosched_ufs_mcq.c */
struct delayed_work *ufs_mcq_get_sync_work(
	const struct request_queue *q);
struct delayed_work *ufs_mcq_get_async_work(
	const struct request_queue *q);
struct workqueue_struct *ufs_mcq_get_sync_wq(void);
struct workqueue_struct *ufs_mcq_get_async_wq(void);

/* mas_mcq_iosched.c */
int ufs_mcq_sched_ds_lld_init(struct request_queue *q);
void ufs_mcq_sched_ds_lld_exit(struct request_queue *q);
struct mas_mcq_sched_ds_lld *mas_mcq_get_lld(const struct request_queue *q);
unsigned int ufs_mcq_get_cpu(struct request * req);
unsigned int ufs_mcq_tagset_bt_get(struct blk_mq_alloc_data *data,
	struct sbitmap_queue *bt, struct blk_mq_hw_ctx *hctx, unsigned int cpu);
int ufs_mcq_fs_io_limit(struct request *rq);
void ufs_mcq_rq_inflt_update(const struct request *rq);
void ufs_mcq_insert_sync_list(struct request *rq, bool head);
void ufs_mcq_insert_async_list(struct request *rq, bool head);
void ufs_mcq_sync_dispatch(const struct request_queue *q);
int ufs_mcq_sync_queue_rq(struct blk_mq_queue_data *bd,
	struct request *rq, const struct blk_mq_alloc_data *alloc_data);
void ufs_mcq_run_sync_list(const struct request_queue *q, int specific_cpu);
void ufs_mcq_requeue_sync_list(struct request *rq, struct request_queue *q);
void ufs_mcq_run_delay_async_list(
	const struct request_queue *q, unsigned long delay_jiffies);
void ufs_mcq_run_queue(const struct request_queue *q);
void ufs_mcq_exec_queue(const struct request_queue *q);
void ufs_mcq_io_guard_queue(const struct request_queue *q);
bool ufs_mcq_async_attempt_merge(struct bio *bio,
	const struct request_queue *q, unsigned int nr_segs);
bool ufs_mcq_list_is_empty(const struct request_queue *q, unsigned int cpu);
void ufs_mcq_async_dispatch(const struct request_queue *q);
void ufs_mcq_status_dump(const struct request_queue *q, enum blk_dump_scene s);
bool mas_mcq_dfa_enable(const struct request_queue *q);
void mas_mcq_dfa_set(struct blk_mq_tag_set *set, int enable);
void mas_mcq_dfa_doze_vote(const struct blk_dev_lld *lld, bool enable);

/* mas_mcq_tag.c */
int ufs_mcq_tag_wakeup_all(struct blk_mq_tags *tags);
unsigned int ufs_mcq_tag_get(const struct blk_mq_alloc_data *data);
int ufs_mcq_tag_put(const struct blk_mq_hw_ctx *hctx,
	unsigned int tag, const struct request *rq);
void blk_mcq_tag_wakeup_all(struct blk_mq_tags *tags);

/* mas_mcq_sysfs.c */
#ifdef CONFIG_MAS_DEBUG_FS
ssize_t mas_mcq_per_cpu_tags_show(struct request_queue *q, char *page);
ssize_t mas_mcq_dfa_enable_show(struct request_queue *q, char *page);
#endif
#endif
