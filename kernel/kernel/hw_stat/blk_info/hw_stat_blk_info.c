/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: get blk info
 * Author: xianghengliang
 * Create: 2023-04-02
 */
#include <linux/hw_stat.h>
#include <linux/proc_fs.h>
#include <linux/kernel.h>
#include <linux/ctype.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/genhd.h>
#include <linux/kdev_t.h>

#include "hw_stat_blk_info.h"

static u64 g_requeue_reason_stat[REQ_REQUEUE_MAX] = { 0 };
void hw_stat_requeue_reason(enum requeue_reason_enum type)
{
	if (type >= REQ_REQUEUE_MAX)
		return;
	g_requeue_reason_stat[type]++;
}

static void reset_hw_stat_blk_info(struct hw_stat_blk_info *blk_info)
{
	int i = 0;

	blk_info->r_inflight = 0;
	blk_info->r_inflight = 0;
	blk_info->bio_count = 0;
	blk_info->req_count = 0;
	blk_info->vip_wait_cnt = 0;
	blk_info->lrb_in_use = 0;
	blk_info->h_tag_used_cnt = 0;
	blk_info->tag_used_cnt = 0;
	blk_info->r_tag_used_cnt = 0;

	for (; i < REQ_REQUEUE_MAX; i++)
		blk_info->requeue_reason_cnt[i] = 0;
}

static int hw_stat_blk_info_show(struct seq_file *seqf, void *v)
{
	int i = 0;
	struct hw_stat_blk_info blk_info;
	reset_hw_stat_blk_info(&blk_info);
	hw_stat_get_blk_info(&blk_info);

	seq_printf(seqf, "nr_hw_queues: %d \n", blk_info.nr_hw_queues);
	seq_printf(seqf, "queue_depth: %d \n", blk_info.queue_depth);
	seq_printf(seqf, "nr_low_prio_tags: %d \n", blk_info.nr_low_prio_tags);
	seq_printf(seqf, "nr_normal_prio_tags: %d \n", blk_info.nr_normal_prio_tags);
	seq_printf(seqf, "nr_high_prio_tags: %d \n", blk_info.nr_high_prio_tags);

	seq_printf(seqf, "low_prio_tag_used_cnt: %d \n", blk_info.r_tag_used_cnt);
	seq_printf(seqf, "normal_prio_tag_used_cnt: %d \n", blk_info.tag_used_cnt);
	seq_printf(seqf, "high_prio_tag_used_cnt: %d \n", blk_info.h_tag_used_cnt);

	seq_printf(seqf, "bio_cnt: %llu \n", blk_info.bio_count);
	seq_printf(seqf, "req_count: %llu \n", blk_info.req_count);
	seq_printf(seqf, "vip_wait_cnt: %llu \n", blk_info.vip_wait_cnt);
	seq_printf(seqf, "r_inflight: %llu \n", blk_info.r_inflight);
	seq_printf(seqf, "w_inflight: %llu \n", blk_info.w_inflight);
	seq_printf(seqf, "ufs lrb_in_use: 0x%llx \n", blk_info.lrb_in_use);

	seq_printf(seqf, "requeue_reason_cnt: ");
	for (i = REQ_REQUEUE_IO_HW_LIMIT; i < REQ_REQUEUE_MAX; i++) {
		blk_info.requeue_reason_cnt[i] = g_requeue_reason_stat[i];
		seq_printf(seqf, " %lu ", blk_info.requeue_reason_cnt[i]);
	}
	seq_printf(seqf, "\n");
	return 0;
}

void hw_stat_init_blk_info(struct proc_dir_entry *parent)
{
	proc_create_single("blk_info", S_IRUSR, parent, hw_stat_blk_info_show);
}
