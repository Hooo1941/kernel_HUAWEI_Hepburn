/*
 * mem_ama.h
 *
 * Copyright(C) 2022 Hisilicon Technologies Co., Ltd. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _LINUX_MM_AMA_H
#define _LINUX_MM_AMA_H

#include <linux/device.h>
#include <linux/mm.h>
#include <linux/mmzone.h>
#include <linux/sizes.h>
#include <linux/hp/memcg_policy.h>
#include <internal.h>

#define AMA_MAX_KILL_NUM	(5)

typedef enum tag_ama_node_sum {
	LOCAL_NODE0 = 0,
	LOCAL_NODE1,
	LOCAL_NODE_SUM,
} ama_node_type;

typedef enum tag_ama_policy_name {
	AMA_FILE = 0,
	AMA_ANON,
	AMA_KALLOC,
	/* e.g. ion&gpu */
	AMA_MEDIA,
	AMA_TYPE_SUM,
} ama_policy_type;

extern unsigned int sysctl_ama_migrate_scan_size;
extern unsigned int sysctl_ama_migrate_scan_delay;
extern unsigned int sysctl_ama_shrink_watermark;
extern unsigned int sysctl_ama_kshrinkd_scan_delay;

static inline bool is_node_part0(int nid)
{
	return (nid == LOCAL_NODE0);
}

static inline bool is_node_part1(int nid)
{
	return (nid == LOCAL_NODE1);
}

struct page *ama_alloc_kernel_large(gfp_t gfp, unsigned order);
struct page *ama_alloc_page_cache(gfp_t gfp, unsigned order);
struct page *ama_alloc_anon_pages(gfp_t gfp, unsigned order);
struct page *ama_alloc_zram_page(gfp_t gfp);
struct page *ama_alloc_meida_pages(gfp_t gfp, unsigned order);
unsigned long ama_alloc_media_pages_bulk_list(gfp_t gfp,
		unsigned long nr_pages, struct list_head *page_list);
struct devres *ama_alloc_dr(dr_release_t release,
				size_t size, gfp_t gfp);

vm_fault_t do_ama_page(struct vm_fault *vmf);
void wake_ama_kworker(pg_data_t *pgdat);
void wakeup_kshrinkd(pg_data_t *pgdat);
void wake_all_kshrinkd(void);

extern void ama_shrink_node(pg_data_t *pgdat, struct scan_control *sc);
#endif
