/*
 * mm/mmvip_scene.c
 *
 * Copyright (C) Huawei Technologies Co., Ltd. 2021. All rights reserved.
 *
 */
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/atomic.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/sched/mm.h>
#include <linux/sched/task.h>
#include <linux/oom.h>
#include <linux/signal.h>
#include <linux/sched.h> /* for clear pcp_vip: cond_resched() */

#include <platform_include/basicplatform/linux/hp/memcg_policy.h>
#include <platform_include/basicplatform/linux/mmvip_scene.h>

#define VIP_PAGE_NUM_MAX 180000

static atomic_t mm_vip_scene = ATOMIC_INIT(0);
static atomic_t mmvip_page_num = ATOMIC_INIT(0);

static struct cmp_name cam_table_p[MAX_CAM_NAME_P] = {
	{.name = "CameraDaemon"},
	{.name = "m.huawei.camera"},
	{.name = "cameraserver"},
	{.name = "allocator@2.0-s"},
};

bool is_mm_vip_scene(void)
{
	if (atomic_read(&mm_vip_scene))
		return true;

	return false;
}

void set_gfp_vip(gfp_t *gfp_mask)
{
	if (gfp_mask == NULL) {
		pr_err("set_gfp_vip is failed\n");
		return;
	}

	if (atomic_read(&mm_vip_scene) && current->mm && current->mm->mm_vip_tag)
		*gfp_mask |= ___GFP_VIP;
	else if (!atomic_read(&mm_vip_scene) && (*gfp_mask & ___GFP_VIP))
		*gfp_mask &= ~___GFP_VIP;
}

bool migrate_vip_check(struct page *page)
{
	if (page == NULL) {
		pr_err("%s: page is NULL\n", __func__);
		return false;
	}
	if (page_is_cma(page))
		return false;

	if (!current_is_kswapd() && (current->pid != get_zswapd_pid()))
		return false;

	if (atomic_read(&mmvip_page_num) >= VIP_PAGE_NUM_MAX)
		return false;

	if (atomic_read(&mm_vip_scene))
		return true;

	return false;
}

void set_migrate_vip(struct page *page, int *migratetype)
{
	if (page == NULL || migratetype == NULL) {
		pr_err("%s: set migrate is failed\n", __func__);
		return;
	}

	if (migrate_vip_check(page))
		*migratetype = MIGRATE_VIP;
}

void add_mmvip_page_num(struct page *page, int *migratetype,
			unsigned int order)
{
	if (page == NULL || migratetype == NULL) {
		pr_err("%s: add vip_page num is failed\n", __func__);
		return;
	}

	if (migrate_vip_check(page)) {
		*migratetype = MIGRATE_VIP;
		atomic_add(1 << order, &mmvip_page_num);
	}
}

void add_pcppages_count(struct per_cpu_pages *pcp,
			int migratetype, int count)
{
	if (pcp == NULL) {
		pr_err("%s: add cppage_count is failed\n", __func__);
		return;
	}

	if (migratetype == MIGRATE_VIP)
		pcp->vip_count += count;
	else
		pcp->count += count;
}

void sub_pcppages_count(struct per_cpu_pages *pcp, int migratetype)
{
	if (pcp == NULL) {
		pr_err("%s: sub pcppage_count is failed\n", __func__);
		return;
	}

	if (migratetype == MIGRATE_VIP)
		pcp->vip_count--;
	else
		pcp->count--;
}

/*
 * Frees pages from the vip migrate PCP lists
 */
static void __free_pcppages_vip(struct zone *zone, int count,
				struct per_cpu_pages *pcp, bool trigger)
{
	struct page *page = NULL;
	struct page *tmp = NULL;
	struct list_head *list = NULL;
	int prefetch_nr = 0;
	bool isolated_pageblocks;
	LIST_HEAD(head);

	if (count <= 0)
		return;

	list = &pcp->lists[MIGRATE_VIP];
	if (list_empty(list))
		return;

	do {
		page = list_first_entry_or_null(list, struct page, lru);
		if (!page)
			continue;

		/* must delete to avoid corrupting pcp list */
		list_del(&page->lru);

		if (vip_bulkfree_pcp_prepare(page))
			continue;

		list_add_tail(&page->lru, &head);

		/*
		 * We are going to put the page back to the global pool,
		 * prefetch its buddy to speed up later access
		 * under zone->lock. It is believed the overhead of
		 * an additional test and calculating buddy_pfn here
		 * can be offset by reduced memory latency later.
		 * To avoid excessive prefetching due to large count,
		 * only prefetch buddy for the first pcp->batch nr of pages.
		 */
		if (prefetch_nr++ < pcp->batch)
			vip_prefetch_buddy(page);
	} while (--count && !list_empty(list));

	spin_lock(&zone->lock);
	isolated_pageblocks = has_isolate_pageblock(zone);

	/* Use safe version since after __free_one_page(),
	 * page->lru.next will not point to original list.
	 */
	list_for_each_entry_safe(page, tmp, &head, lru) {
		int mt = page->index;

		/* MIGRATE_ISOLATE page should not go to pcplists */
		VM_BUG_ON_PAGE(is_migrate_isolate(mt), page);
		/* Pageblock could have been isolated meanwhile */
		if (unlikely(isolated_pageblocks))
			mt = get_pageblock_migratetype(page);
		if (!trigger && is_migrate_vip(mt))
			atomic_add(1, &mmvip_page_num);
		if (trigger && is_migrate_vip(mt))
			mt = MIGRATE_MOVABLE;

		__free_vip_one_page(page, page_to_pfn(page), zone, 0, mt, FPI_NONE);
		trace_mm_page_pcpu_drain(page, 0, mt);
	}
	spin_unlock(&zone->lock);
}

void free_pcppages_vip(struct page *page, struct zone *zone,
			struct per_cpu_pages *pcp, int migratetype)
{
	bool trigger = false;
	int batch = 0;

	if (page == NULL || zone == NULL || pcp == NULL) {
		pr_err("%s: free pappages_vip is failed\n", __func__);
		return;
	}

	batch = READ_ONCE(pcp->batch);
	if (!atomic_read(&mm_vip_scene))
		trigger = true;

	list_add(&page->lru, &pcp->lists[migratetype]);
	if (migratetype == MIGRATE_VIP)
		pcp->vip_count++;
	else
		pcp->count++;
	if (pcp->vip_count && trigger) {
		__free_pcppages_vip(zone, pcp->vip_count, pcp, trigger);
		pcp->vip_count = 0;
	} else if (pcp->vip_count >= pcp->high) {
		__free_pcppages_vip(zone, batch, pcp, trigger);
		pcp->vip_count -= batch;
	}
}

static bool unreset_migvip_pageblock(bool force)
{
	unsigned long flags = 0;
	struct zone *zone = NULL;
	struct page *page = NULL;
	int order = 0;
	bool ret = true;
	int mt = 0;

	for_each_populated_zone(zone) {
		spin_lock_irqsave(&zone->lock, flags);
		for (order = 0; order < MAX_ORDER; order++) {
			struct free_area *area = &(zone->free_area[order]);
			struct list_head *list = &(area->free_list[MIGRATE_VIP]);
			while ((page = list_first_entry_or_null(list, struct page, lru))) {
				mt = get_pageblock_migratetype(page);
				if (is_migrate_vip(mt)) {
					mt = MIGRATE_MOVABLE;
					set_pageblock_migratetype(page, mt);
				}

				ret = move_freepages_block(zone, page, mt, NULL);
				if (!ret) {
					pr_err("mm_vip: %s some page_spanned\n", __func__);
					break;
				}
			}
		}
		spin_unlock_irqrestore(&zone->lock, flags);
	}

	return ret;
}

static int mm_vip_scene_proc_show(struct seq_file *m, void *v)
{
	int memstat = atomic_read(&mm_vip_scene);
	seq_printf(m, "%d\n", memstat);
	return 0;
}

static int mm_vip_scene_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, mm_vip_scene_proc_show, NULL);
}

static ssize_t mm_vip_scene_proc_write(struct file *file, const char __user *buffer,
					size_t count, loff_t *ppos)
{
	unsigned int val = 0;
	int ret = 0;

	ret = kstrtouint_from_user(buffer, count, 0, &val);
	if (ret < 0)
		return ret;

	atomic_set(&mm_vip_scene, val);

	/* clear mt_vip and pcp_vip */
	if(val == 0) {
		atomic_set(&mmvip_page_num, 0);;
		unreset_migvip_pageblock(true);
	}

	return count;
}

static const struct proc_ops mm_vip_scene_proc_fops = {
	.proc_open		= mm_vip_scene_proc_open,
	.proc_read		= seq_read,
	.proc_write		= mm_vip_scene_proc_write,
	.proc_lseek		= seq_lseek,
	.proc_release	= single_release,
};

static void mmvip_cam_setpid(void)
{
	struct task_struct *p = NULL;
	struct task_struct *tsk = NULL;
	struct mm_struct *mm = NULL;
	int i;

	rcu_read_lock();
	for (i = 0; i < MAX_CAM_NAME_P; i++) {
		p = NULL;
		tsk = NULL;
		mm = NULL;

		for_each_process(tsk) {
			if ((tsk->flags & PF_KTHREAD) || is_global_init(tsk))
				continue;
			if (strncmp(cam_table_p[i].name, tsk->comm, strlen(tsk->comm)))
				continue;

			pr_err("%s: task->pid %d name %s\n", __func__, tsk->pid, tsk->comm);
			p = find_lock_task_mm(tsk);
			if (!p)
				continue;
			task_unlock(p);

			mm = get_task_mm(p);
			if (!mm)
				continue;

			if (atomic_read(&mm_vip_scene))
				mm->mm_vip_tag = 1;
			else
				mm->mm_vip_tag = 0;

			mmput(mm);
			break;
		}
	}
	rcu_read_unlock();
}

static ssize_t mm_vip_setpid_proc_write(struct file *filp,
		const char __user *user_buf, size_t cnt, loff_t *ppos)
{
	ssize_t ret = cnt;
	pid_t upid = 0;
	struct pid *pid = NULL;
	struct task_struct *tsk = NULL;
	struct mm_struct *mm = NULL;

	if (kstrtoint_from_user(user_buf, cnt, 0, &upid))
		return  -EINVAL;

	if (upid == MMVIP_CAM_VAL) {
		mmvip_cam_setpid();
		return ret;
	}

	pid = find_get_pid(upid);
	if (!pid) {
		ret =  -EINVAL;
		goto out;
	}

	tsk = get_pid_task(pid, PIDTYPE_PID);
	if (!tsk) {
		ret =  -ESRCH;
		goto pid_put;
	}

	mm = get_task_mm(tsk);
	if (!mm) {
		ret =  -EINVAL;
		goto tsk_put;
	}

	if (atomic_read(&mm_vip_scene))
		mm->mm_vip_tag = 1;
	else
		mm->mm_vip_tag = 0;

	mmput(mm);
tsk_put:
	put_task_struct(tsk);
pid_put:
	put_pid(pid);
out:
	return ret;
}

static int mm_vip_setpid_proc_open(struct inode *inode, struct file *file)
{
	return 0;
}

static const struct proc_ops mm_vip_setpid_proc_fops = {
	.proc_open		= mm_vip_setpid_proc_open,
	.proc_write		= mm_vip_setpid_proc_write,
};

static int __init proc_mm_vip_scene_init(void)
{
	proc_create("mm_vip_scene", 0660, NULL, &mm_vip_scene_proc_fops);
	proc_create("mm_vip_pid", 0220, NULL, &mm_vip_setpid_proc_fops);
	return 0;
}

fs_initcall(proc_mm_vip_scene_init);
