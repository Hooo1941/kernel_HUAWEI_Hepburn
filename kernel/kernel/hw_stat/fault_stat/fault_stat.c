/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: hw_stat faultstat
 * Author: Wang Ke <wangke185@huawei.com>
 * Create: 2023-03-18
 */
#include <linux/mm.h>
#include <linux/vmstat.h>
#include <linux/hw_stat.h>
#include <linux/seq_file.h>

#ifdef CONFIG_HUAWEI_QOS_BLKIO
#include <chipset_common/hwqos/hwqos_common.h>
#endif

#define PF_DIM_HIGH_PRIO_OFFSET	(FILE_ADD_PAGES_HIGH_PRIO - FILE_ADD_PAGES)
#define PF_DIM_LAT_OFFSET	(MINOR_FAULT_LAT - MINOR_FAULT)

DEFINE_PER_CPU(struct fault_stat_info, fault_stat) = {{0}};

static inline void __fault_stat_add(enum pf_dimension pf_dim,
				    enum pf_fs_type pf_fs, u64 value)
{
	raw_cpu_add(fault_stat.event[pf_dim][pf_fs], value);
}

static inline void fault_stat_add(enum pf_dimension pf_dim,
				  enum pf_fs_type pf_fs, u64 value)
{
	this_cpu_add(fault_stat.event[pf_dim][pf_fs], value);
}

static int get_pf_fs_type(struct inode *inode)
{
	switch (inode->i_sb->s_magic) {
	case EXT4_SUPER_MAGIC:
		return PF_TYPE_EXT4;
	case EROFS_SUPER_MAGIC_V1:
		return PF_TYPE_EROFS;
	case F2FS_SUPER_MAGIC:
		return PF_TYPE_F2FS;
	default:
		return 0;
	}
}

static void __do_fault_stat_acct(struct inode *inode, enum pf_dimension pf_dim,
				 u64 value)
{
	int pf_fs = get_pf_fs_type(inode);

	__fault_stat_add(pf_dim, PF_TYPE_ALL, value);
	if (pf_fs)
		__fault_stat_add(pf_dim, pf_fs, value);
}

void __fault_stat_acct(struct inode *inode, enum pf_dimension pf_dim, u64 value)
{
	__do_fault_stat_acct(inode, pf_dim, value);
	if (current_is_high_prio())
		__do_fault_stat_acct(inode, pf_dim + PF_DIM_HIGH_PRIO_OFFSET, value);
}
EXPORT_SYMBOL(__fault_stat_acct);

bool filemap_fault_acct_start(u64 *start_time)
{
	*start_time = ktime_get_ns();
	return true;
}
EXPORT_SYMBOL(filemap_fault_acct_start);

void filemap_fault_acct_done(struct inode *inode, int fault_type, u64 start_time)
{
	__fault_stat_acct(inode, fault_type, 1);
	__fault_stat_acct(inode, fault_type + PF_DIM_LAT_OFFSET,
			  ktime_get_ns() - start_time);
}
EXPORT_SYMBOL(filemap_fault_acct_done);

static void fault_stat_read_all(struct fault_stat_info *stat)
{
	int cpu;
	int i, j;

	for_each_online_cpu(cpu) {
		struct fault_stat_info *this = &per_cpu(fault_stat, cpu);

		for (i = 0; i < NR_PF_DIM; i++)
			for (j = 0; j < NR_PF_FS_TYPE; j++)
				stat->event[i][j] += this->event[i][j];
	}
}

static inline void seq_printf_fault_stat(struct seq_file *seq,
					 struct fault_stat_info *stat,
					 char *item, enum pf_dimension pf_dim)
{
	seq_printf(seq, "%-20s %20llu %20llu %20llu %20llu\n",
		   item, stat->event[pf_dim][PF_TYPE_ALL],
		   stat->event[pf_dim][PF_TYPE_EXT4],
		   stat->event[pf_dim][PF_TYPE_EROFS],
		   stat->event[pf_dim][PF_TYPE_F2FS]);
}

static int faultstat_show(struct seq_file *seq, void *arg)
{
	int nid;
	unsigned long file = 0;
	pg_data_t *pgdat = NULL;
	struct fault_stat_info stat = {{0}};

	fault_stat_read_all(&stat);
	for_each_online_node(nid) {
		struct lruvec *lruvec;

		pgdat = NODE_DATA(nid);
		lruvec = node_lruvec(pgdat);
		file += lruvec_lru_size(lruvec, LRU_ACTIVE_FILE, MAX_NR_ZONES) +
					lruvec_lru_size(lruvec,
					LRU_INACTIVE_FILE, MAX_NR_ZONES);
	}

	seq_printf(seq, "MemAvailable: %lu kB\n",
		si_mem_available() * PAGE_SIZE / SZ_1K);
	seq_printf(seq, "MemFree: %lu kB\n",
		global_zone_page_state(NR_FREE_PAGES) * PAGE_SIZE / SZ_1K);
	seq_printf(seq, "Inactive_file: %lu kB\n",
		global_node_page_state_pages(NR_INACTIVE_FILE) * PAGE_SIZE / SZ_1K);
	seq_printf(seq, "active_file: %lu kB\n",
		global_node_page_state_pages(NR_ACTIVE_FILE) * PAGE_SIZE / SZ_1K);
	seq_printf(seq, "FileLru: %lu kB\n", file * PAGE_SIZE / SZ_1K);
	seq_printf(seq, "workingset_refault_file: %lu\n",
		global_node_page_state_pages(WORKINGSET_REFAULT_FILE));

	/* print header */
	seq_printf(seq, "%-20s %20s %20s %20s %20s\n", " ", "all", "ext4", "erofs", "f2fs");

	seq_printf_fault_stat(seq, &stat, "add_pages:", FILE_ADD_PAGES);
	seq_printf_fault_stat(seq, &stat, "add_pages_hp:", FILE_ADD_PAGES_HIGH_PRIO);
	seq_printf_fault_stat(seq, &stat, "major_fault:", MAJOR_FAULT);
	seq_printf_fault_stat(seq, &stat, "major_fault_lat:", MAJOR_FAULT_LAT);
	seq_printf_fault_stat(seq, &stat, "major_fault_hp:", MAJOR_FAULT_HIGH_PRIO);
	seq_printf_fault_stat(seq, &stat, "major_fault_lat_hp:", MAJOR_FAULT_LAT_HIGH_PRIO);
	seq_printf_fault_stat(seq, &stat, "minor_fault:", MINOR_FAULT);
	seq_printf_fault_stat(seq, &stat, "minor_fault_lat:", MINOR_FAULT_LAT);
	seq_printf_fault_stat(seq, &stat, "minor_fault_hp:", MINOR_FAULT_HIGH_PRIO);
	seq_printf_fault_stat(seq, &stat, "minor_fault_lat_hp:", MINOR_FAULT_LAT_HIGH_PRIO);

	return 0;
}

void hw_stat_init_faultstat(struct proc_dir_entry *parent)
{
	proc_create_single("faultstat", S_IRUSR, parent, faultstat_show);
}
