/*
 * memcheck_account.c
 *
 * Get account memory infomation
 *
 * Copyright (c) 2022 Huawei Technologies Co., Ltd.
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

#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/seq_file.h>
#include <linux/sched.h>
#include <linux/sched/task.h>
#include <linux/sched/signal.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
#include <linux/sched/mm.h>
#endif
#if (KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE)
#include <linux/signal_types.h>
#include <linux/pagewalk.h>
#endif
#include <linux/ratelimit.h>
#include <log/log_usertype.h>
#ifdef CONFIG_DFX_OHOS
#include <dfx/hiview_hisysevent.h>
#else
#include <log/hiview_hievent.h>
#endif
#include <platform/linux/memcheck.h>
#include <platform/trace/hooks/memcheck.h>

#include "slab.h"
#include "memcheck_ioctl.h"
#include "memcheck_log_ashmem.h"
#include "memcheck_log_cma.h"
#include "memcheck_log_gpumem.h"
#ifdef CONFIG_HISI_GPU
#include "memcheck_log_gpumem_hvgr.h"
#elif defined(CONFIG_DFX_OHOS) && defined(CONFIG_MALI_MIDGARD)
#include "memcheck_log_gpumem_mali.h"
#endif
#include "memcheck_log_ion.h"
#include "memcheck_log_vmalloc.h"
#include "memcheck_fd_socket.h"
#include "memcheck_fd_pipe.h"
#include "memcheck_fd_fence.h"

#define MAX_TOP_NUM 10
#define TOPN_BUF_LEN 4
#define DEFAULT_TOPN 3
#define RSS_TOP_BATCHING 50
#define MAX_MSG_LEN 256
#define MAX_SLUB_OBJ_SIZE 800
#define SLUB_OBJ_DOMAIN "KERNEL_VENDOR"

#ifdef CONFIG_DFX_LIBLINUX
#define SLUB_OBJ_STRING "LDK_SLUB_OBJ"
#else
#define SLUB_OBJ_STRING "SLUB_OBJ"
#endif

#define LOWMEM_STRING "LOWMEM"

static const char *const slubobj_ignore_list[] = {
	"f2fs_inode_cache",
};

struct event_info {
	pid_t pid;
	pid_t tgid;
	char comm[TASK_COMM_LEN];
	char *name;
	unsigned long total;
	unsigned long num;
};

static int cur_topn = DEFAULT_TOPN;
static struct event_info event;
static struct work_struct report_work;
static struct work_struct show_stats_work;

#if (KERNEL_VERSION(5, 4, 0) >= LINUX_VERSION_CODE)
extern void show_stack(struct task_struct *task, unsigned long *sp);
#else
extern void show_stack(struct task_struct *tsk, unsigned long *sp, const char *loglvl);
static inline const char *cache_name(struct kmem_cache *s)
{
#if defined(CONFIG_MEMCG_KMEM) && defined(CONFIG_MEMCG) && defined(CONFIG_ARCH_QCOM)
	if (s->memcg_params.root_cache)
		s = s->memcg_params.root_cache;
#endif
	return s->name;
}
#endif

#define MAX_MSG_LEN 256

struct lowmem_info {
	pid_t pid;
	pid_t tgid;
	char comm[TASK_COMM_LEN];
	unsigned long points;
	unsigned long total_vm;
};

static struct work_struct g_lowmem_report_work;
static struct lowmem_info g_lowmem_info;

#ifdef CONFIG_DFX_OHOS
static void lowmem_report_work_func(struct work_struct *work)
{
	int ret;
	char msg[MAX_MSG_LEN] = { 0 };
	struct hiview_hisysevent *lowmem_event = NULL;
	ret = snprintf(msg, MAX_MSG_LEN, "lowmem name:%s points=%ld num=%ld",
		       g_lowmem_info.comm, g_lowmem_info.points,
		       g_lowmem_info.total_vm);
	if (ret <= 0)
		return;

	memcheck_info("domain: KERNEL_VENDOR, stringid: %s, pid: %d, tgid: %d, name: %s",
		      LOWMEM_STRING, g_lowmem_info.pid, g_lowmem_info.tgid,
		      g_lowmem_info.comm);
	lowmem_event = hisysevent_create(SLUB_OBJ_DOMAIN, LOWMEM_STRING, FAULT);
	if (!lowmem_event) {
		memcheck_err("failed to create lowmem_event");
		goto hisysevent_end;
	}
	ret = hisysevent_put_integer(lowmem_event, "PID", g_lowmem_info.pid);
	ret += hisysevent_put_integer(lowmem_event, "UID", g_lowmem_info.tgid);
	ret += hisysevent_put_string(lowmem_event, "PACKAGE_NAME", g_lowmem_info.comm);
	ret += hisysevent_put_string(lowmem_event, "PROCESS_NAME", g_lowmem_info.comm);
	ret += hisysevent_put_string(lowmem_event, "MSG", msg);
	if (ret != 0) {
		memcheck_err("add info to lowmem_event failed, ret=%d", ret);
		goto hisysevent_end;
	}
	ret = hisysevent_write(lowmem_event);
	if (ret < 0)
		memcheck_err("send hisysevent fail, domain: KERNEL_VENDOR, stringid:%s",
			     LOWMEM_STRING);

hisysevent_end:
	hisysevent_destroy(&lowmem_event);
}
#else
static void lowmem_report_work_func(struct work_struct *work)
{
	int ret;
	char msg[MAX_MSG_LEN] = { 0 };

	ret = snprintf(msg, MAX_MSG_LEN, "lowmem name:%s points=%lu vm=%lu",
		       g_lowmem_info.comm, g_lowmem_info.points,
		       g_lowmem_info.total_vm);
	if (ret <= 0)
		return;

	memcheck_info("domain: KERNEL_VENDOR, stringid: %s, pid: %d, tgid: %d, name: %s",
		      LOWMEM_STRING, g_lowmem_info.pid, g_lowmem_info.tgid,
		      g_lowmem_info.comm);

	ret = hiview_send_hisysevent(SLUB_OBJ_DOMAIN, LOWMEM_STRING, FAULT,
				     "%s,%d,%s,%d,%s,%s,%s,%s,%s,%s",
				     "PID", g_lowmem_info.pid,
				     "UID", g_lowmem_info.tgid,
				     "PACKAGE_NAME", g_lowmem_info.comm,
				     "PROCESS_NAME", g_lowmem_info.comm,
				     "MSG", msg);
	if (ret < 0)
		memcheck_err("send hisysevent fail, domain: KERNEL_VENDOR, stringid:%s",
			     LOWMEM_STRING);
}
#endif

void memcheck_lowmem_report(struct task_struct *p, unsigned long points)
{
	static DEFINE_RATELIMIT_STATE(ratelimit_lowmem_report, 2 * 60 * HZ, 1); // report once maximum in 2 mins

	if (!__ratelimit(&ratelimit_lowmem_report))
		return;

	task_lock(p);
	g_lowmem_info.pid = p->pid;
	g_lowmem_info.tgid = p->tgid;
	memcpy(g_lowmem_info.comm, p->comm, TASK_COMM_LEN);
	g_lowmem_info.points = points;
	g_lowmem_info.total_vm = p->mm->total_vm;
	task_unlock(p);

	schedule_work(&g_lowmem_report_work);
}

#ifdef CONFIG_DFX_OHOS
static void report_work_func(struct work_struct *work)
{
	int ret;
	char msg[MAX_MSG_LEN] = { 0 };
	struct hiview_hisysevent *slub_event = NULL;
	ret = snprintf(msg, MAX_MSG_LEN, "cache_name:%s total=%ld num=%ld",
		       event.name, event.total, event.num);
	if (ret <= 0)
		return;

	memcheck_info("domain: %s, stringid: %s, pid: %d, tgid: %d, name: %s",
		      SLUB_OBJ_DOMAIN, SLUB_OBJ_STRING, event.pid, event.tgid,
		      event.comm);
	slub_event = hisysevent_create(SLUB_OBJ_DOMAIN, SLUB_OBJ_STRING, FAULT);
	if (!slub_event) {
		memcheck_err("failed to create slub_event");
		goto hisysevent_end;
	}
	ret = hisysevent_put_integer(slub_event, "PID", event.pid);
	ret += hisysevent_put_integer(slub_event, "UID", event.tgid);
	ret += hisysevent_put_string(slub_event, "PACKAGE_NAME", event.comm);
	ret += hisysevent_put_string(slub_event, "PROCESS_NAME", event.comm);
	ret += hisysevent_put_string(slub_event, "MSG", msg);
	if (ret != 0) {
		memcheck_err("add info to slub_event failed, ret=%d", ret);
		goto hisysevent_end;
	}
	ret = hisysevent_write(slub_event);
	if (ret < 0)
		memcheck_err("send hisysevent fail, domain:%s, stringid:%s",
			     SLUB_OBJ_DOMAIN, SLUB_OBJ_STRING);

hisysevent_end:
	hisysevent_destroy(&slub_event);
}
#else
static void report_work_func(struct work_struct *work)
{
	int ret;
	char msg[MAX_MSG_LEN] = { 0 };

	ret = snprintf(msg, MAX_MSG_LEN, "cache_name:%s total=%ld num=%ld",
		       event.name, event.total, event.num);
	if (ret <= 0)
		return;

	memcheck_info("domain: %s, stringid: %s, pid: %d, tgid: %d, name: %s",
		      SLUB_OBJ_DOMAIN, SLUB_OBJ_STRING, event.pid, event.tgid,
		      event.comm);
	ret = hiview_send_hisysevent(SLUB_OBJ_DOMAIN, SLUB_OBJ_STRING, FAULT,
				     "%s,%d,%s,%d,%s,%s,%s,%s,%s,%s",
				     "PID", event.pid,
				     "UID", event.tgid,
				     "PACKAGE_NAME", event.comm,
				     "PROCESS_NAME", event.comm,
				     "MSG", msg);
	if (ret < 0)
		memcheck_err("send hisysevent fail, domain:%s, stringid:%s",
			     SLUB_OBJ_DOMAIN, SLUB_OBJ_STRING);
}
#endif

static inline unsigned long node_nr_objs(struct kmem_cache_node *n)
{
#ifdef CONFIG_SLUB_DEBUG
	return atomic_long_read(&n->total_objects);
#else
	return 0;
#endif
}

static void get_slub_objs(struct kmem_cache *s, struct slabinfo *sinfo)
{
	unsigned long nr_objs = 0;
	int node;
	struct kmem_cache_node *n;

	for_each_kmem_cache_node(s, node, n) {
		nr_objs += node_nr_objs(n);
	}

	sinfo->num_objs = nr_objs;
}

void memcheck_slub_obj_report(struct kmem_cache *s)
{
	int i;
	struct slabinfo sinfo;
	unsigned long size = 0;
	const char *comm_name = "hilogcat";
	static DEFINE_RATELIMIT_STATE(ratelimit_stack, 5 * HZ, 2); // print stack twice for 5 second maximum
	static DEFINE_RATELIMIT_STATE(ratelimit_report, 60 * 60 * HZ, 1); // report once for one hour maximum

#ifndef CONFIG_DFX_OHOS
	if (get_logusertype_flag() != BETA_USER)
		return;
#endif

	for (i = 0; i < ARRAY_SIZE(slubobj_ignore_list); i++) {
		if (strstr(slubobj_ignore_list[i], cache_name(s)))
			return;
	}

	get_slub_objs(s, &sinfo);
	size = sinfo.num_objs * s->size / 1024 / 1024;
	if (size <= MAX_SLUB_OBJ_SIZE || strstr(current->comm, comm_name))
		return;

	if (__ratelimit(&ratelimit_stack)) {
		memcheck_err("slub may leak, cache_name=%s, cache_size=%lu",
			     cache_name(s), sinfo.num_objs * s->size);
#if (KERNEL_VERSION(5, 4, 0) >= LINUX_VERSION_CODE)
		show_stack(current, NULL);
#else
		show_stack(current, NULL, KERN_DEFAULT);
#endif
	}
	if (__ratelimit(&ratelimit_report)) {
		event.pid = current->pid;
		event.tgid = current->tgid;
		memcpy(event.comm, current->comm, TASK_COMM_LEN);
		event.name = (char *)cache_name(s);
		event.total = size;
		event.num = sinfo.num_objs;
		schedule_work(&report_work);
	}
}

static void memcheck_stats_show_func(struct work_struct *work)
{
	memcheck_ion_info_show();
	memcheck_ashmem_info_show();
#if defined(CONFIG_HISI_GPU) || (defined(CONFIG_DFX_OHOS) && defined(CONFIG_MALI_MIDGARD))
	memcheck_gpumem_info_show();
#else
#ifdef CONFIG_ANDROID_VENDOR_HOOKS
	trace_android_mm_memcheck_gpumem_info_show(0);
#endif
#endif
	memcheck_cma_info_show();
	memcheck_vmalloc_info_show();
}

void memcheck_stats_show(void)
{
	schedule_work(&show_stats_work);
}

static int rss_topn_info_show(struct seq_file *s, void *ptr)
{
	int i;
	struct task_struct *tsk = NULL;
	unsigned long rss[MAX_TOP_NUM] = { 0 };
	int tgid[MAX_TOP_NUM] = { 0 };
	char name[MAX_TOP_NUM][TASK_COMM_LEN] = { 0 };

	seq_printf(s, "%16s %6s %16s\n", "task", "tgid", "rss");

	rcu_read_lock();
	for_each_process(tsk) {
		int index = 0;
		unsigned long cur_rss;
		struct mm_struct *mm = NULL;

		if (tsk->flags & PF_KTHREAD)
			continue;
		mm = get_task_mm(tsk);
		if (!mm)
			continue;
		cur_rss = get_mm_rss(mm) + get_mm_counter(mm, MM_SWAPENTS);
		for (i = 0; i < cur_topn; i++) {
			if (cur_rss > rss[i]) {
				index = i;
				break;
			}
		}
		if (i >= cur_topn) {
			mmput(mm);
			continue;
		}
		rss[index] = cur_rss;
		tgid[index] = tsk->tgid;
		memcpy(name[index], tsk->comm, TASK_COMM_LEN);
		mmput(mm);
	}
	rcu_read_unlock();

	for (i = 0; i < cur_topn; i++)
		seq_printf(s, "%16s %6d %16lu\n", name[i], tgid[i],
			   rss[i] * PAGE_SIZE);

	return 0;
}

static ssize_t rss_topn_info_write(struct file *file, const char __user *buf,
				   size_t count, loff_t *ppos)
{
	char buffer[TOPN_BUF_LEN];
	int topn;
	int err = 0;

	memset(buffer, 0, sizeof(buffer));
	if (count > sizeof(buffer) - 1)
		count = sizeof(buffer) - 1;
	if (copy_from_user(buffer, buf, count)) {
		err = -EFAULT;
		goto err;
	}
	err = kstrtoint(strstrip(buffer), 0, &topn);
	if (err)
		goto err;

	if (topn <= 0 || topn > MAX_TOP_NUM) {
		err = -EINVAL;
		goto err;
	}
	cur_topn = topn;

err:
	return err < 0 ? err : count;
}

static int rss_topn_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, rss_topn_info_show, PDE_DATA(inode));
}

#if (KERNEL_VERSION(5, 10, 0) <= LINUX_VERSION_CODE)
static const struct proc_ops rss_topn_info_fops = {
	.proc_open = rss_topn_info_open,
	.proc_read = seq_read,
	.proc_write = rss_topn_info_write,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};
#else
static const struct file_operations rss_topn_info_fops = {
	.open = rss_topn_info_open,
	.read = seq_read,
	.write = rss_topn_info_write,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif

int memcheck_createfs(void)
{
	proc_create_data("rss_topn", 0660, NULL,
			 &rss_topn_info_fops, NULL);
	memcheck_ion_createfs();
	memcheck_ashmem_createfs();
#if defined(CONFIG_HISI_GPU) || (defined(CONFIG_DFX_OHOS) && defined(CONFIG_MALI_MIDGARD))
	memcheck_gpumem_createfs();
#endif
	memcheck_cma_createfs();
	memcheck_vmalloc_createfs();
	memcheck_fd_socket_createfs();
	memcheck_fd_pipe_createfs();
	memcheck_fd_fence_createfs();
	INIT_WORK(&report_work, report_work_func);
	INIT_WORK(&g_lowmem_report_work, lowmem_report_work_func);
	INIT_WORK(&show_stats_work, memcheck_stats_show_func);

	return 0;
}
