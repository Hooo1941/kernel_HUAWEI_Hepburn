/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: hw_stat iostat
 * Author: Wang Ke <wangke185@huawei.com>
 * Create: 2023-03-15
 */

#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/hw_stat.h>
#include <linux/blkdev.h>
#include <linux/blk_types.h>
#include <linux/magic.h>
#include <linux/hw_stat.h>
#include <linux/percpu-refcount.h>

#define IO_SIZE_4K	4
#define IO_SIZE_8K	8
#define IO_SIZE_16K	16
#define IO_SIZE_32K	32
#define IO_SIZE_64K	64
#define IO_SIZE_128K	128
#define IO_SIZE_256K	256
#define IO_SIZE_512K	512
#define IO_SIZE_1024K	1024

#define LEVEL0_NS	1000000
#define LEVEL1_NS	10000000
#define LEVEL2_NS	100000000

#define LOG_BTYE2KBYTE  10

enum hw_iostat_fs_type {
	HW_FS_TYPE_ALL,
	NR_HW_FS_BASE,
	HW_FS_TYPE_EXT4 = NR_HW_FS_BASE,
	HW_FS_TYPE_EROFS,
	HW_FS_TYPE_F2FS,
	NR_HW_FS_TYPE
};

enum hw_iostat_prio {
	HW_PRIO_LOW,
	HW_PRIO_HIGH,
	NR_HW_PRIO
};

/*
 * LE: less than or equal to
 * GT: greater than
 */
enum hw_iostat_size_group {
	HW_IO_LE_4K,
	HW_IO_LE_8K,
	HW_IO_LE_16K,
	HW_IO_LE_32K,
	HW_IO_LE_64K,
	HW_IO_LE_128K,
	HW_IO_LE_256K,
	HW_IO_LE_512K,
	HW_IO_LE_1024K,
	HW_IO_GT_1024K,
	NR_HW_IO_SIZE_GROUP
};

/*
 * LEVEL0: 0-1ms
 * LEVEL1: 1-10ms
 * LEVEL2: 10-100ms
 * LEVEL3: 100ms+
 */
enum hw_iostat_lat_level {
	HW_LAT_LEVEL0,
	HW_LAT_LEVEL1,
	HW_LAT_LEVEL2,
	HW_LAT_LEVEL3,
	NR_HW_LAT_LEVEL
};

struct hw_iostat_info {
	unsigned long size_cnt[NR_HW_FS_TYPE][NR_HW_OP_GROUP][NR_HW_PRIO][NR_HW_IO_SIZE_GROUP];
	unsigned long lat_cnt[NR_HW_FS_TYPE][NR_HW_OP_GROUP][NR_HW_PRIO][NR_HW_LAT_LEVEL];
};

DEFINE_PER_CPU(struct hw_iostat_info, hw_iostat) = {{0}};

static const char *fs_type_text[] = {
	"all",
	"ext4",
	"erofs",
	"f2fs"
};

static const char *prio_text[] = {
	"[prio_low]",
	"[prio_high]"
};

#ifdef CONFIG_MAS_BLK
static int io_is_high_prio(unsigned long flag, u64 mas_flag)
{
	if (flag & REQ_FG)
		return HW_PRIO_HIGH;
	if (mas_flag & REQ_CP)
		return HW_PRIO_HIGH;
	if (mas_flag & REQ_VIP)
		return HW_PRIO_HIGH;

	return HW_PRIO_LOW;
}
#else
static int io_is_high_prio(unsigned long flag)
{
	if (flag & REQ_FG)
		return HW_PRIO_HIGH;

	return HW_PRIO_LOW;
}
#endif

static inline void hw_iostat_size_cnt_inc(int fs_type, int op, int prio,
					  int lat_level)
{
	this_cpu_add(hw_iostat.size_cnt[fs_type][op][prio][lat_level], 1);
}

static inline void __hw_iostat_size_cnt_inc(int fs_type, int op, int prio,
					    int lat_level)
{
	raw_cpu_add(hw_iostat.size_cnt[fs_type][op][prio][lat_level], 1);
}

static inline void hw_iostat_lat_cnt_inc(int fs_type, int op, int prio,
					 int lat_level)
{
	this_cpu_add(hw_iostat.lat_cnt[fs_type][op][prio][lat_level], 1);
}

static inline void __hw_iostat_lat_cnt_inc(int fs_type, int op, int prio,
					   int lat_level)
{
	raw_cpu_add(hw_iostat.lat_cnt[fs_type][op][prio][lat_level], 1);
}

static inline int hw_iostat_get_fs_type(unsigned long fs_magic)
{
	switch (fs_magic) {
	case EXT4_SUPER_MAGIC:
		return HW_FS_TYPE_EXT4;
	case EROFS_SUPER_MAGIC_V1:
		return HW_FS_TYPE_EROFS;
	case F2FS_SUPER_MAGIC:
		return HW_FS_TYPE_F2FS;
	default:
		return 0;
	}
}

static inline int hw_iostat_get_prio()
{
	return current_is_high_prio();
}

static inline int hw_iostat_get_size_group(unsigned long size)
{
	if (size <= IO_SIZE_4K) {
		return HW_IO_LE_4K;
	} else if (size <= IO_SIZE_8K) {
		return HW_IO_LE_8K;
	} else if (size <= IO_SIZE_16K) {
		return HW_IO_LE_16K;
	} else if (size <= IO_SIZE_32K) {
		return HW_IO_LE_32K;
	} else if (size <= IO_SIZE_64K) {
		return HW_IO_LE_64K;
	} else if (size <= IO_SIZE_128K) {
		return HW_IO_LE_128K;
	} else if (size <= IO_SIZE_256K) {
		return HW_IO_LE_256K;
	} else if (size <= IO_SIZE_512K) {
		return HW_IO_LE_512K;
	} else if (size <= IO_SIZE_1024K) {
		return HW_IO_LE_1024K;
	} else {
		return HW_IO_GT_1024K;
	}
}

static inline int hw_iostat_get_lat_level(u64 duration)
{
	if (duration < LEVEL0_NS) {
		return HW_LAT_LEVEL0;
	} else if (duration < LEVEL1_NS) {
		return HW_LAT_LEVEL1;
	} else if (duration < LEVEL2_NS) {
		return HW_LAT_LEVEL2;
	} else {
		return HW_LAT_LEVEL3;
	}
}

void __hw_iostat_acct_size_blk(struct request *rq, unsigned long value)
{
	int op = op_stat_group(req_op(rq));
	int prio, size_group;

	if (op != STAT_READ && op != STAT_WRITE)
		return;

#ifdef CONFIG_MAS_BLK
	prio = io_is_high_prio(rq->cmd_flags, rq->mas_cmd_flags);
#else
	prio = io_is_high_prio(rq->cmd_flags);
#endif
	size_group = hw_iostat_get_size_group(value >> LOG_BTYE2KBYTE);

	__hw_iostat_size_cnt_inc(HW_FS_TYPE_ALL, op, prio, size_group);
}
EXPORT_SYMBOL(__hw_iostat_acct_size_blk);

void __hw_iostat_acct_lat_blk(struct request *rq, u64 value)
{
	int op = op_stat_group(req_op(rq));
	int prio, lat_level;

	if (op != STAT_READ && op != STAT_WRITE)
		return;

#ifdef CONFIG_MAS_BLK
	prio = io_is_high_prio(rq->cmd_flags, rq->mas_cmd_flags);
#else
	prio = io_is_high_prio(rq->cmd_flags);
#endif
	lat_level = hw_iostat_get_lat_level(value);

	__hw_iostat_lat_cnt_inc(HW_FS_TYPE_ALL, op, prio, lat_level);
}
EXPORT_SYMBOL(__hw_iostat_acct_lat_blk);

static void __hw_iostat_acct_size_vfs(struct file *file,
				      enum hw_iostat_op_gourp op,
				      unsigned long value)
{
	int fs_type, prio, size_group;

	fs_type = hw_iostat_get_fs_type(file->f_inode->i_sb->s_magic);
	prio = current_is_high_prio();
	size_group = hw_iostat_get_size_group(value >> LOG_BTYE2KBYTE);

	if (fs_type)
		__hw_iostat_size_cnt_inc(fs_type, op, prio, size_group);
	__hw_iostat_size_cnt_inc(HW_FS_TYPE_ALL, op, prio, size_group);
}

static void __hw_iostat_acct_lat_vfs(struct file *file,
				     enum hw_iostat_op_gourp op,
				     u64 value)
{
	int fs_type, prio, lat_level;

	fs_type = hw_iostat_get_fs_type(file->f_inode->i_sb->s_magic);
	prio = current_is_high_prio();
	lat_level = hw_iostat_get_lat_level(value);

	if (fs_type)
		__hw_iostat_lat_cnt_inc(fs_type, op, prio, lat_level);
	__hw_iostat_lat_cnt_inc(HW_FS_TYPE_ALL, op, prio, lat_level);
}

void __hw_iostat_acct_vfs(struct file *file, enum hw_iostat_op_gourp op,
			  ssize_t size, u64 duration)
{
	__hw_iostat_acct_size_vfs(file, op, size);
	__hw_iostat_acct_lat_vfs(file, op, duration);
}
EXPORT_SYMBOL(__hw_iostat_acct_vfs);

static inline void count_all_cpu_iostat_size(struct hw_iostat_info *stat,
					     struct hw_iostat_info *iostat,
					     int fs_type, int op, int prio)
{
	int i;

	for (i = 0; i < NR_HW_IO_SIZE_GROUP; i++)
		stat->size_cnt[fs_type][op][prio][i] += iostat->size_cnt[fs_type][op][prio][i];
}

static inline void count_all_cpu_iostat_lat(struct hw_iostat_info *stat,
					     struct hw_iostat_info *iostat,
					     int fs_type, int op, int prio)
{
	int i;

	for (i = 0; i < NR_HW_IO_SIZE_GROUP; i++)
		stat->lat_cnt[fs_type][op][prio][i] += iostat->lat_cnt[fs_type][op][prio][i];
}

static void count_all_cpu_iostat_info(struct hw_iostat_info *stat,
				      struct hw_iostat_info *iostat)
{
	int i, j, k;

	for (i = 0; i < NR_HW_FS_TYPE; i++) {
		for (j = 0; j < NR_HW_OP_GROUP; j++) {
			for (k = 0; k < NR_HW_PRIO; k++) {
				count_all_cpu_iostat_size(stat, iostat, i, j, k);
				count_all_cpu_iostat_lat(stat, iostat, i, j, k);
			}
		}
	}
}

static void hw_iostat_read_all(struct hw_iostat_info *stat)
{
	int cpu;

	for_each_possible_cpu(cpu) {
		struct hw_iostat_info *iostat = per_cpu_ptr(&hw_iostat, cpu);
		count_all_cpu_iostat_info(stat, iostat);
	}
}

#ifdef CONFIG_HW_STAT_IOSTAT_VFS
static void hw_iostat_vfs_show(struct seq_file *seq, struct hw_iostat_info *stat)
{
	int i, j, k;

	seq_printf(seq, "vfs_stat[size]\n");
	for (i = 0; i < NR_HW_FS_TYPE; i++) {
		for (j = 0; j < NR_HW_PRIO; j++) {
			seq_printf(seq, "%-10s%-15s", fs_type_text[i], prio_text[j]);
			for (k = 0; k < NR_HW_IO_SIZE_GROUP; k++)
				seq_printf(seq, " %10lu/%-10lu",
					stat->size_cnt[i][HW_OP_READ_VFS][j][k],
					stat->size_cnt[i][HW_OP_WRITE_VFS][j][k]);
			seq_printf(seq, "\n");
		}
	}
	seq_printf(seq, "\n");

	seq_printf(seq, "vfs_stat[lat]\n");
	for (i = 0; i < NR_HW_FS_TYPE; i++) {
		for (j = 0; j < NR_HW_PRIO; j++) {
			seq_printf(seq, "%-10s%-15s", fs_type_text[i], prio_text[j]);
			for (k = 0; k < NR_HW_LAT_LEVEL; k++)
				seq_printf(seq, " %20llu/%-20llu",
					stat->lat_cnt[i][HW_OP_READ_VFS][j][k],
					stat->lat_cnt[i][HW_OP_WRITE_VFS][j][k]);
			seq_printf(seq, "\n");
		}
	}
	seq_printf(seq, "\n");
}
#else
static void hw_iostat_vfs_show(struct seq_file *seq, struct hw_iostat_info *stat)
{
}
#endif

#ifdef CONFIG_HW_STAT_IOSTAT_BLK
static void hw_iostat_blk_show(struct seq_file *seq, struct hw_iostat_info *stat)
{
	int i, j, k;

	seq_printf(seq, "blk_stat[size]\n");
	for (i = 0; i < NR_HW_FS_BASE; i++) {
		for (j = 0; j < NR_HW_PRIO; j++) {
			seq_printf(seq, "%-10s%-15s", fs_type_text[i], prio_text[j]);
			for (k = 0; k < NR_HW_IO_SIZE_GROUP; k++)
				seq_printf(seq, " %10lu/%-10lu",
					stat->size_cnt[i][HW_OP_READ_BLK][j][k],
					stat->size_cnt[i][HW_OP_WRITE_BLK][j][k]);
			seq_printf(seq, "\n");
		}
	}
	seq_printf(seq, "\n");

	seq_printf(seq, "blk_stat[lat]\n");
	for (i = 0; i < NR_HW_FS_BASE; i++) {
		for (j = 0; j < NR_HW_PRIO; j++) {
			seq_printf(seq, "%-10s%-15s", fs_type_text[i], prio_text[j]);
			for (k = 0; k < NR_HW_LAT_LEVEL; k++)
				seq_printf(seq, " %20llu/%-20llu",
					stat->lat_cnt[i][HW_OP_READ_BLK][j][k],
					stat->lat_cnt[i][HW_OP_WRITE_BLK][j][k]);
			seq_printf(seq, "\n");
		}
	}
	seq_printf(seq, "\n");
}
#else
static void hw_iostat_blk_show(struct seq_file *seq, struct hw_iostat_info *stat)
{
}
#endif

static int hw_iostat_show(struct seq_file *seq, void *arg)
{
	struct hw_iostat_info stat = {{0}};

	hw_iostat_read_all(&stat);

	hw_iostat_vfs_show(seq, &stat);
	hw_iostat_blk_show(seq, &stat);

	return 0;
}

void hw_stat_init_iostat(struct proc_dir_entry *parent)
{
	proc_create_single("iostat", S_IRUSR, parent, hw_iostat_show);
}
