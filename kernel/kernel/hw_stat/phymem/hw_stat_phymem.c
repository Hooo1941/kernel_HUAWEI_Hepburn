/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: get physical memory info
 * Author: Qin Hongwei <qinhongwei7@huawei.com>
 * Create: 2023-02-18
 */
#include <linux/hw_stat.h>
#include <linux/proc_fs.h>
#include <linux/mmzone.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/trace_events.h>
#include <linux/memcontrol.h>
#include <trace/events/mmflags.h>
#include <linux/migrate.h>
#include <linux/page_owner.h>
#include <linux/page_pinner.h>
#include <linux/ctype.h>
#include <linux/magic.h>

#include "hw_stat_phymem.h"

// Caller ensures @pstat and @page not null
static void check_one_page(struct physical_mem_stat *pstat, struct page *page)
{
	struct page *head = compound_head(page);
	struct address_space *mapping = NULL;
	struct inode *host = NULL;
	struct super_block *sb = NULL;
	unsigned long s_magic;

	if (PagePoisoned(page)) {
		pstat->poisoned++;
		return;
	}

	if ((page < head) || (page >= head + MAX_ORDER_NR_PAGES)) {
		pstat->corrupted++;
		return;
	}

	if (PageKsm(page)) {
		pstat->ksm++;
		return;
	}
	if (PageAnon(page)) {
		pstat->anon++;
		return;
	}

	mapping = page_mapping(page);
	if (!mapping) {
		pstat->null_mapping++;
		return;
	}

	pstat->file++;

	if (get_kernel_nofault(host, &mapping->host))
		return;

	if (!host)
		return;

	if (get_kernel_nofault(sb, &host->i_sb) ||
	    get_kernel_nofault(s_magic, &sb->s_magic))
		return;

	if (s_magic == F2FS_SUPER_MAGIC)
		pstat->f2fs_file++;
	else if (s_magic == EXT4_SUPER_MAGIC)
		pstat->ext4_file++;
	else if (s_magic == EROFS_SUPER_MAGIC_V1)
		pstat->erofs_file++;
}

static void show_one_zone(struct seq_file *seq, int nodeid,
	struct zone *zone, int zoneid)
{
	unsigned long start_pfn = zone->zone_start_pfn;
	unsigned long end_pfn = zone_end_pfn(zone);
	unsigned long pfn;
	struct physical_mem_stat local_stat = {};

	for (pfn = start_pfn; pfn < end_pfn; pfn++) {
		struct page *page;

		if (!pfn_valid(pfn))
			continue;
		page = pfn_to_page(pfn);
		// pfn_to_page makes sure (page != NULL)
		check_one_page(&local_stat, page);
	}
	pr_notice("hw_stat print zone %d\n", zoneid);

	seq_printf(seq, "%d\t%d\t%lu\t%lu\t%lu\t%lu\t%lu\t%lu\t%lu\t%lu\t%lu\n",
		nodeid, zoneid, local_stat.anon,
		local_stat.file, local_stat.f2fs_file, local_stat.ext4_file, local_stat.erofs_file,
		local_stat.poisoned, local_stat.corrupted, local_stat.ksm,
		local_stat.null_mapping);
}

static int hw_stat_phymem_show(struct seq_file *seq, void *offset)
{
	pg_data_t *pgdat;
	for_each_online_pgdat(pgdat) {
		int zoneid;

		for (zoneid = 0; zoneid < MAX_NR_ZONES; zoneid++) {
			struct zone *zone = &pgdat->node_zones[zoneid];

			if (!populated_zone(zone))
				continue;
			show_one_zone(seq, pgdat->node_id, zone, zoneid);
		}
	}
	return 0;
}

void hw_stat_init_phymem(struct proc_dir_entry *dir)
{
	proc_create_single("hw_stat_phymem", S_IRUSR, dir, hw_stat_phymem_show);
}
