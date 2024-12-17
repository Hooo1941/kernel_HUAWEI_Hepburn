/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: memory managing for reserved memory with TEE.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include "reserved_mempool.h"
#include <linux/list.h>
#include <linux/sizes.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/debugfs.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>
#include <securec.h>
#include <linux/vmalloc.h>

#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <asm/io.h>

#include "teek_client_constants.h"
#include "tc_ns_log.h"
#include "smc_smp.h"

#define STATE_MODE 0440U
#define SLICE_RATE 4
#define MAX_SLICE  0x400000
#define MIN_RES_MEM_SIZE 0x400000

struct virt_page {
	unsigned long start;
};

struct reserved_page_t {
	struct list_head node;
	struct virt_page *page;
	int order;
	unsigned int count; /* whether be used */
};

struct reserved_free_area_t {
	struct list_head page_list;
	int order;
};

struct reserved_zone_t {
	struct virt_page *all_pages;
	struct reserved_page_t *pages;
	struct reserved_free_area_t free_areas[0];
};

static struct reserved_zone_t *g_res_zone;
static struct mutex g_res_lock;
static int g_res_max_order;
static unsigned long g_start_vaddr = 0;
static unsigned long g_start_paddr;
static struct dentry *g_res_mem_dbg_dentry;
static unsigned int g_res_mem_size = 0;

static unsigned int get_res_page_size(void)
{
	return g_res_mem_size >> PAGE_SHIFT;
}

static unsigned int calc_res_mem_size(unsigned int rsize)
{
	unsigned int size = rsize;
	unsigned int idx = 0;

	if (size == 0 || (size & (size - 1)) == 0)
		return size;

	while (size != 0) {
		size = size >> 1;
		idx++;
	}
	return (1 << (idx - 1));
}

unsigned int get_res_mem_slice_size(void)
{
	unsigned int size = (g_res_mem_size >> SLICE_RATE);
	return (size > MAX_SLICE) ? MAX_SLICE : size;
}

bool exist_res_mem(void)
{
	return (g_start_vaddr != 0) && (g_res_mem_size != 0);
}

unsigned long res_mem_virt_to_phys(unsigned long vaddr)
{
	return vaddr - g_start_vaddr + g_start_paddr;
}

int load_reserved_mem(void)
{
	struct device_node *np = NULL;
	struct resource r;
	unsigned int res_size;
	int rc;
	void *p = NULL;

	np = of_find_compatible_node(NULL, NULL, "tz_reserved");
	if (np == NULL) {
		tlogd("can not find reserved memory.\n");
		return 0;
	}

	rc = of_address_to_resource(np, 0, &r);
	if (rc != 0) {
		tloge("of_address_to_resource error\n");
		return -ENODEV;
	}

	res_size = (unsigned int)resource_size(&r);
	if (res_size < MIN_RES_MEM_SIZE) {
		tloge("reserved memory size is too small\n");
		return -EINVAL;
	}

	p = ioremap(r.start, res_size);
	if (p == NULL) {
		tloge("io remap for reserved memory failed\n");
		return -ENOMEM;
	}
	g_start_vaddr = (unsigned long)(uintptr_t)p;
	g_start_paddr = (unsigned long)r.start;
	g_res_mem_size = calc_res_mem_size(res_size);

	return 0;
}

void unmap_res_mem(void)
{
	if (exist_res_mem()) {
		iounmap((void __iomem *)g_start_vaddr);
		g_start_vaddr = 0;
		g_res_mem_size = 0;
	}
}

static int create_zone(void)
{
	size_t zone_len;
	g_res_max_order = get_order(g_res_mem_size);
	zone_len = sizeof(struct reserved_free_area_t) * (g_res_max_order + 1) + sizeof(*g_res_zone);

	g_res_zone = kzalloc(zone_len, GFP_KERNEL);
	if (g_res_zone == NULL) {
		tloge("fail to create zone\n");
		return -ENOMEM;
	}

	g_res_zone->pages = kzalloc(sizeof(struct reserved_page_t) * get_res_page_size(), GFP_KERNEL);
	if (g_res_zone->pages == NULL) {
		tloge("failed to alloc zone pages\n");
		kfree(g_res_zone);
		g_res_zone = NULL;
		return -ENOMEM;
	}
	return 0;
}

static struct virt_page *create_virt_pages(void)
{
	unsigned int i = 0;
	struct virt_page *pages = NULL;

	pages = kzalloc(get_res_page_size() * sizeof(struct virt_page), GFP_KERNEL);
	if (pages == NULL) {
		tloge("alloc pages failed\n");
		return NULL;
	}

	for (i = 0; i < get_res_page_size(); i++)
		pages[i].start = g_start_vaddr + i * PAGE_SIZE;
	return pages;
}

void free_reserved_mempool(void)
{
	if (!exist_res_mem())
		return;

	if (g_res_zone->all_pages != NULL) {
		kfree(g_res_zone->all_pages);
		g_res_zone->all_pages = NULL;
	}

	if (g_res_zone->pages != NULL) {
		kfree(g_res_zone->pages);
		g_res_zone->pages = NULL;
	}

	if (g_res_zone != NULL) {
		kfree(g_res_zone);
		g_res_zone = NULL;
	}

	if (!g_res_mem_dbg_dentry)
		return;
	debugfs_remove_recursive(g_res_mem_dbg_dentry);
	g_res_mem_dbg_dentry = NULL;
}

static void show_res_mem_info(void)
{
	unsigned int i;
	struct reserved_page_t *pos = NULL;
	struct list_head *head = NULL;
	unsigned int used = 0;

	if (g_res_zone == NULL) {
		tloge("res zone is NULL\n");
		return;
	}

	tloge("################## reserved memory info ######################\n");
	mutex_lock(&g_res_lock);
	for (i = 0; i < get_res_page_size(); i++) {
		if (g_res_zone->pages[i].count != 0) {
			tloge("page[%02d], order=%02d, count=%d\n",
				i, g_res_zone->pages[i].order,
				g_res_zone->pages[i].count);
			used += (1 << (uint32_t)g_res_zone->pages[i].order);
		}
	}
	tloge("reserved memory total usage:%u/%u\n", used, get_res_page_size());
	tloge("--------------------------------------------------------------\n");

	for (i = 0; i < (unsigned int)g_res_max_order; i++) {
		head = &g_res_zone->free_areas[i].page_list;
		if (list_empty(head) != 0) {
			tloge("order[%02d] is empty\n", i);
		} else {
			list_for_each_entry(pos, head, node)
				tloge("order[%02d]\n", i);
		}
	}
	mutex_unlock(&g_res_lock);

	tloge("#############################################################\n");
}

static ssize_t mb_res_mem_state_read(struct file *filp, char __user *ubuf,
	size_t cnt, loff_t *ppos)
{
	(void)(filp);
	(void)(ubuf);
	(void)cnt;
	(void)(ppos);
	show_res_mem_info();
	return 0;
}

static const struct file_operations g_res_mem_dbg_state_fops = {
	.owner = THIS_MODULE,
	.read = mb_res_mem_state_read,
};

static void init_res_mem_dentry(void)
{
}

static int res_mem_register(unsigned long paddr, unsigned int size)
{
	struct tc_ns_operation *operation = NULL;
	struct tc_ns_smc_cmd *smc_cmd = NULL;
	int ret = 0;

	smc_cmd = kzalloc(sizeof(*smc_cmd), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)smc_cmd)) {
		tloge("alloc smc_cmd failed\n");
		return -ENOMEM;
	}

	operation = kzalloc(sizeof(*operation), GFP_KERNEL);
	if (ZERO_OR_NULL_PTR((unsigned long)(uintptr_t)operation)) {
		tloge("alloc operation failed\n");
		ret = -ENOMEM;
		goto free_smc_cmd;
	}

	operation->paramtypes = TEE_PARAM_TYPE_VALUE_INPUT |
		(TEE_PARAM_TYPE_VALUE_INPUT << TEE_PARAM_NUM);
	operation->params[0].value.a = paddr;
	operation->params[0].value.b = paddr >> ADDR_TRANS_NUM;
	operation->params[1].value.a = size;

	smc_cmd->cmd_type = CMD_TYPE_GLOBAL;
	smc_cmd->cmd_id = GLOBAL_CMD_ID_REGISTER_RESMEM;
	smc_cmd->operation_phys = virt_to_phys(operation);
	smc_cmd->operation_h_phys = virt_to_phys(operation) >> ADDR_TRANS_NUM;

	if (tc_ns_smc(smc_cmd) != 0) {
		tloge("resigter res mem failed\n");
		ret = -EIO;
	}

	kfree(operation);
	operation = NULL;
free_smc_cmd:
	kfree(smc_cmd);
	smc_cmd = NULL;
	return ret;
}

static void zone_init(struct virt_page *all_pages)
{
	int i;
	struct reserved_free_area_t *area = NULL;
	int max_order_cnt;
	struct reserved_page_t *res_page = NULL;

	for (i = 0; i < (int)get_res_page_size(); i++) {
		g_res_zone->pages[i].order = -1;
		g_res_zone->pages[i].count = 0;
		g_res_zone->pages[i].page = &all_pages[i];
	}

	for (i = 0; i <= g_res_max_order; i++) {
		area = &g_res_zone->free_areas[i];
		INIT_LIST_HEAD(&area->page_list);
		area->order = i;
	}

	max_order_cnt = (int)(get_res_page_size() / (1 << (unsigned int)g_res_max_order));
	g_res_zone->all_pages = all_pages;
	for (i = 0; i < max_order_cnt; i++) {
		int idx = i * (1 << (unsigned int)g_res_max_order);
		g_res_zone->pages[idx].order = g_res_max_order;
		res_page = &g_res_zone->pages[idx];
		list_add_tail(&res_page->node, &area->page_list);
	}
}

int reserved_mempool_init(void)
{
	struct virt_page *all_pages = NULL;
	int ret = 0;
	unsigned long paddr;

	if (!exist_res_mem())
		return 0;

	ret = create_zone();
	if (ret != 0)
		return ret;

	all_pages = create_virt_pages();
	if (all_pages == NULL) {
		kfree(g_res_zone->pages);
		g_res_zone->pages = NULL;
		kfree(g_res_zone);
		g_res_zone = NULL;
		return -ENOMEM;
	}

	paddr = g_start_paddr;
	ret = res_mem_register(paddr, g_res_mem_size);
	if (ret != 0) {
		kfree(all_pages);
		all_pages = NULL;
		kfree(g_res_zone->pages);
		g_res_zone->pages = NULL;
		kfree(g_res_zone);
		g_res_zone = NULL;
		return -EIO;
	}

	zone_init(all_pages);

	mutex_init(&g_res_lock);
	init_res_mem_dentry();
	return 0;
}

void *reserved_mem_alloc(size_t size)
{
	int i, j;
	struct reserved_page_t *pos = NULL;
	struct list_head *head = NULL;
	int order = get_order(ALIGN(size, SZ_4K));
	unsigned long addr = 0;

	bool valid_param = (size > 0 && order <= g_res_max_order && order >= 0);
	if (!valid_param) {
		tloge("invalid alloc param, size %d, order %d, max %d\n",(int)size, order, g_res_max_order);
		return NULL;
	}
	mutex_lock(&g_res_lock);
	for (i = order; i <= g_res_max_order; i++) {
		head = &g_res_zone->free_areas[i].page_list;
		if (list_empty(head) != 0)
			continue;

		pos = list_first_entry(head, struct reserved_page_t, node);
		pos->count = 1;
		pos->order = order;

		for (j = order; j < i; j++) {
			struct reserved_page_t *new_page = NULL;
			new_page = pos + (1 << (unsigned int)j);
			new_page->count = 0;
			new_page->order = j;
			list_add_tail(&new_page->node, &g_res_zone->free_areas[j].page_list);
		}
		list_del(&pos->node);
		addr = pos->page->start;
		break;
	}
	mutex_unlock(&g_res_lock);
	return (void *)(uintptr_t)addr;
}

static int get_virt_page_index(const void *ptr)
{
	unsigned long vaddr = (unsigned long)(uintptr_t)ptr;
	unsigned long offset = vaddr - g_start_vaddr;
	int pg_idx = offset / (1 << PAGE_SHIFT);
	if ((unsigned int)pg_idx >= get_res_page_size() || pg_idx < 0)
		return -1;
	return pg_idx;
}

static int buddy_merge(struct virt_page *vpage, int order, unsigned int *page_index)
{
	int i;
	unsigned int cur_idx;
	unsigned int buddy_idx;
	struct reserved_page_t *self = NULL;
	struct reserved_page_t *buddy = NULL;

	for (i = order; i < g_res_max_order; i++) {
		cur_idx = vpage - g_res_zone->all_pages;
		buddy_idx = cur_idx ^ (1 << (unsigned int)i);
		self = &g_res_zone->pages[cur_idx];
		buddy = &g_res_zone->pages[buddy_idx];
		self->count = 0;
		/* is buddy free  */
		if (buddy->order == i && buddy->count == 0) {
			/* release buddy */
			list_del(&buddy->node);
			/* combine self and buddy */
			if (cur_idx > buddy_idx) {
				vpage = buddy->page;
				buddy->order = i + 1;
				self->order = -1;
			} else {
				self->order = i + 1;
				buddy->order = -1;
			}
		} else {
			/* release self */
			list_add_tail(&self->node,
				&g_res_zone->free_areas[i].page_list);
			return -1;
		}
	}

	if (order == g_res_max_order) {
		cur_idx = vpage - g_res_zone->all_pages;
		tlogd("no need to find buddy, cur is %u\n", cur_idx);
		*page_index = cur_idx;
		return 0;
	}
	*page_index = (cur_idx > buddy_idx) ? buddy_idx : cur_idx;
	return 0;
}

void reserved_mem_free(const void *ptr)
{
	struct reserved_page_t *self = NULL;
	int self_idx;
	unsigned int page_index;
	struct reserved_page_t *max_order_page = NULL;

	if (ptr == NULL) {
		tloge("invalid ptr\n");
		return;
	}

	mutex_lock(&g_res_lock);
	self_idx = get_virt_page_index(ptr);
	if (self_idx < 0) {
		mutex_unlock(&g_res_lock);
		tloge("invalid page\n");
		return;
	}
	self = &g_res_zone->pages[self_idx];
	if (self->count == 0) {
		tloge("already free in reserved mempool\n");
		mutex_unlock(&g_res_lock);
		return;
	}

	if (buddy_merge(self->page, self->order, &page_index) < 0) {
		mutex_unlock(&g_res_lock);
		return;
	}

	max_order_page = &g_res_zone->pages[page_index];
	list_add_tail(&max_order_page->node,
		&g_res_zone->free_areas[g_res_max_order].page_list);
	mutex_unlock(&g_res_lock);
}
