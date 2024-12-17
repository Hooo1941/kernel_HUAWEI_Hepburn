/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: hw_stat's global header
 * Author: Qin Hongwei <qinhongwei7@huawei.com>
 * Create: 2023-02-17
 */
#ifndef __HW_STAT_H
#define __HW_STAT_H

#include <linux/fs.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#if defined(CONFIG_HW_STAT_IOSTAT) || defined(CONFIG_HW_STAT_BLK_INFO)
#include <linux/blkdev.h>
#endif

int current_is_high_prio(void);

#ifdef CONFIG_HW_STAT_TRACING_MARK_WRITE
#define HW_STAT_TRACE_START (1)
#define HW_STAT_TRACE_END (0)
#define HW_STAT_MAX_BUF_LEN (128)

struct hw_stat_trace_param {
	int ignore_qos; // Whether we want to forcefully print regardless of qos
	const char *note; // The first string to print in tracing mark write
	const char *note2; // The second string to print in tracing mark write
	unsigned long value1; // The first value to print in tracing mark write
	unsigned long value2; // The second value to print in tracing mark write
	unsigned long value3; // The third value to print in tracing mark write
};

// Caller ensures all params not null
void hw_stat_trace_fill_name(struct hw_stat_trace_param *p_param, char **p_path,
	struct inode *inode, char *name_buf, unsigned int name_buf_len);
// Caller ensures @param not null.
int hw_stat_trace(int is_start, const struct hw_stat_trace_param *param);
int hw_stat_trace_enabled(void);
void hw_stat_init_trace_marker(struct proc_dir_entry *parent);
#endif /* CONFIG_HW_STAT_TRACING_MARK_WRITE */

#ifdef CONFIG_HW_STAT_FAULT
enum pf_dimension {
	FILE_ADD_PAGES,                 /* # of pages added to page cache since bootup */
	FILE_ADD_PAGES_HIGH_PRIO,       /* # of high priority pages added to page cache since bootup */
	MINOR_FAULT,                    /* # of minor fault since bootup */
	MINOR_FAULT_HIGH_PRIO,          /* # of high priority minor fault since bootup */
	MINOR_FAULT_LAT,                /* minor fault latency since bootup */
	MINOR_FAULT_LAT_HIGH_PRIO,      /* high priority minor fault latency since bootup */
	MAJOR_FAULT,                    /* # of major fault since bootup */
	MAJOR_FAULT_HIGH_PRIO,          /* # of high priority major fault since bootup */
	MAJOR_FAULT_LAT,                /* major fault latency since bootup */
	MAJOR_FAULT_LAT_HIGH_PRIO,      /* high priority major fault latency since bootup */
	NR_PF_DIM
};

enum pf_fs_type {
	PF_TYPE_ALL,
	PF_TYPE_EXT4,
	PF_TYPE_EROFS,
	PF_TYPE_F2FS,
	NR_PF_FS_TYPE
};

struct fault_stat_info {
	u64 event[NR_PF_DIM][NR_PF_FS_TYPE];
};

void __fault_stat_acct(struct inode *inode, enum pf_dimension pf_dim, u64 value);
bool filemap_fault_acct_start(u64 *start_time);
void filemap_fault_acct_done(struct inode *inode, int fault_type, u64 start_time);
void hw_stat_init_faultstat(struct proc_dir_entry *parent);
#endif /* CONFIG_HW_STAT_FAULT */

#ifdef CONFIG_HW_STAT_IOSTAT
enum hw_iostat_op_gourp {
	HW_OP_READ_BLK = STAT_READ,
	HW_OP_WRITE_BLK = STAT_WRITE,
	HW_OP_READ_VFS,
	HW_OP_WRITE_VFS,
	NR_HW_OP_GROUP
};

void __hw_iostat_acct_size_blk(struct request *rq, unsigned long value);
void __hw_iostat_acct_lat_blk(struct request *rq, u64 value);
void __hw_iostat_acct_vfs(struct file *file, enum hw_iostat_op_gourp op,
			  ssize_t size, u64 duration);
void hw_stat_init_iostat(struct proc_dir_entry *parent);
#endif /* CONFIG_HW_STAT_IOSTAT */

#ifdef CONFIG_HW_STAT_PSI_LRB
/*
 * when ufs driver set_bit lrb_in_use, please call the func.
 * restrict: Caller must ensure that the func is not executed concurrently.
 */
void hw_stat_lrb_in_use_changed(unsigned long lrb_in_use);
#endif

#ifdef CONFIG_HW_STAT_PSI
void hw_stat_update_psi(void);
#endif

#ifdef CONFIG_HW_STAT_BLK_INFO
struct hw_stat_blk_info {
	unsigned int nr_hw_queues; /* hw queue cnt */
	unsigned int queue_depth; /* total nums of soft queue tags */
	unsigned int nr_low_prio_tags; /* low priority tags cnt */
	unsigned int nr_normal_prio_tags; /* normal priority tags cnt */
	unsigned int nr_high_prio_tags; /* high priority tags cnt */
	unsigned int r_inflight; /* read inflight cnt */
	unsigned int w_inflight; /* write inflight cnt */
	u64 bio_count;
	u64 req_count;
	int vip_wait_cnt;
	int h_tag_used_cnt; /* high priority tags used cnt */
	int tag_used_cnt; /* normal priority tags used cnt */
	int r_tag_used_cnt; /* low priority tags used cnt */
	int requeue_reason_cnt[REQ_REQUEUE_MAX]; /* statistic every requeue reason cnt */
	unsigned long lrb_in_use; /* statistic hw queue using */
};
void hw_stat_get_blk_info(struct hw_stat_blk_info *blk_info);
void hw_stat_requeue_reason(enum requeue_reason_enum type);
void hw_stat_get_ufs_mq_info(const struct request_queue *q, struct hw_stat_blk_info *blk_info);
#endif /* CONFIG_HW_STAT_BLK_INFO */
#endif /* __HW_STAT_H */
