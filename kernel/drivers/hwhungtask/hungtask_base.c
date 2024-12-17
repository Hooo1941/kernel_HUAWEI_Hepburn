/*
 * hungtask_base.c
 *
 * Detect Hung Task Base Implementation
 *
 * Copyright (c) 2017-2021 Huawei Technologies Co., Ltd.
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

#define pr_fmt(fmt)	"hungtask_base " fmt

#include <linux/nmi.h>
#include <linux/delay.h>
#include <linux/freezer.h>
#include <linux/utsname.h>
#include <trace/events/sched.h>
#include <linux/slab.h>
#include <linux/version.h>
#if !defined(KERNEL_VERSION) || !defined(LINUX_VERSION_CODE)
#include <linux/sched/debug.h>
#else
#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
#include <linux/sched/debug.h>
#endif
#endif
#include <linux/suspend.h>
#include <linux/spinlock.h>
#include <platform/linux/zrhung.h>

#ifdef CONFIG_BLACKBOX
#include <platform/trace/events/rainbow.h>
#include <platform/linux/rainbow.h>
#endif
#ifdef CONFIG_TZDRIVER
#include <chipset_common/security/itrustee.h>
#endif

#include "securec.h"
#include "hungtask_base.h"
#include "hungtask_ext.h"
#include "hungtask_user.h"
#include "hungtask_mmap_sem.h"
#ifdef CONFIG_LOCK_DETECTOR
#include "../lock_detector/lock_detector.h"
#endif

#ifdef CONFIG_SCHED_CFS_WORKAROUND
#define SCHED_HUNGTASK_WA_TIME 300
#endif

#ifndef CONFIG_CHIP_HONGHU
#include <linux/vmstat.h>
#include <linux/mmzone.h>

#ifdef CONFIG_TOPN_MEMINFO_DUMP
#include <huawei_platform/pmeminfo/process_mem.h>
#endif

#ifdef CONFIG_HUAWEI_MEMINFO_DUMP
#include <proc_mem/process_mem.h>
#endif
#endif

static struct rb_root list_tasks = RB_ROOT;
static DEFINE_SPINLOCK(list_tasks_lock);
static struct hlist_head whitelist[WHITELIST_LEN];
static struct whitelist_item whitetmplist[WHITELIST_LEN];
static bool whitelist_empty = true;
static int remove_cnt;
static struct task_item *remove_list[MAX_REMOVE_LIST_NUM + 1];
/* zero means infinite timeout - no checking done */
static u64 __read_mostly hungtask_timeout_secs =
	CONFIG_DEFAULT_HUNG_TASK_TIMEOUT;
static int did_panic;
static u32 hungtask_enable = HT_DISABLE;
static u32 whitelist_type = WHITE_LIST;
static int whitelist_dump_cnt = DEFAULT_WHITE_DUMP_CNT;
static int whitelist_panic_cnt = DEFAULT_WHITE_PANIC_CNT;
/* record critical process pid used to judge task type */
static int zygote64_pid;
static int zygote_pid;
static int maple_zygote64_pid;
static bool vmboot_flag;
static int systemserver_pid;
static int last_systemserver_pid;
static int dump_and_upload;
static int time_since_upload;
static int hung_task_must_panic;
static int report_zrhung_id;
static struct task_hung_upload upload;
static int do_refresh;
static char frozen_buf[FROZEN_BUF_LEN];
static int frozen_used;
static bool frozed_head;
static u64 cur_heartbeat;
static struct work_struct send_work;
static char report_buf_tag_cmd[REPORT_MSGLENGTH + 2]; /* 2: R= */
static char report_buf_text[REPORT_MSGLENGTH];

unsigned long hungtask_get_timeout(void);
unsigned int hungtask_get_panic(void);

bool hashlist_find(struct hlist_head *head, int count, pid_t tgid)
{
	struct hashlist_node *hnode = NULL;

	if (count <= 0)
		return false;
	if (hlist_empty(&head[tgid % count]))
		return false;
	hlist_for_each_entry(hnode, &head[tgid % count], list) {
		if (hnode->pid == tgid)
			return true;
	}
	return false;
}

void hashlist_clear(struct hlist_head *head, int count)
{
	int i;
	struct hlist_node *n = NULL;
	struct hashlist_node *hnode = NULL;

	for (i = 0; i < count; i++) {
		hlist_for_each_entry_safe(hnode, n, &head[i], list) {
			hlist_del(&hnode->list);
			kfree(hnode);
			hnode = NULL;
		}
	}
	for (i = 0; i < count; i++)
		INIT_HLIST_HEAD(&head[i]);
}

bool hashlist_insert(struct hlist_head *head, int count, pid_t tgid)
{
	struct hashlist_node *hnode = NULL;

	if (hashlist_find(head, count, tgid))
		return false;
	hnode = kmalloc(sizeof(struct hashlist_node), GFP_ATOMIC);
	if (!hnode)
		return false;
	INIT_HLIST_NODE(&hnode->list);
	hnode->pid = tgid;
	hlist_add_head(&hnode->list, &head[tgid % count]);
	return true;
}

static bool rcu_lock_break(struct task_struct *g, struct task_struct *t)
{
	bool can_cont = false;

	get_task_struct(g);
	get_task_struct(t);
	rcu_read_unlock();
	cond_resched();
	rcu_read_lock();
	can_cont = pid_alive(g) && pid_alive(t);
	put_task_struct(t);
	put_task_struct(g);
	return can_cont;
}

static bool rcu_break(int *max_count, int *batch_count,
		      struct task_struct *g,
		      struct task_struct *t)
{
	if (!(*max_count)--)
		return true;
	if (!--(*batch_count)) {
		*batch_count = HUNG_TASK_BATCHING;
		if (!rcu_lock_break(g, t))
			return true;
	}
	return false;
}

static pid_t get_pid_by_name(const char *name)
{
	int max_count = PID_MAX_LIMIT;
	int batch_count = HUNG_TASK_BATCHING;
	struct task_struct *g = NULL;
	struct task_struct *t = NULL;
	int pid = 0;

	/*
	 * Try to find system_server's pid by name "PowerManagerSer" first.
	 * If fail, then retry with name "system_server".
	 */
	if (!strcmp("system_server", name)) {
		pid = get_pid_by_name("PowerManagerSer");
		if (pid)
			return pid;
		pr_info("retry with %s\n", name);
	}

	rcu_read_lock();
	do_each_thread(g, t) {
		if (rcu_break(&max_count, &batch_count, g, t))
			goto unlock;
		if (!strncmp(t->comm, name, TASK_COMM_LEN)) {
			pid = t->tgid;
			goto unlock;
		}
	} while_each_thread(g, t);

unlock:
	rcu_read_unlock();
	return pid;
}

static u32 get_task_type(pid_t pid, pid_t tgid, struct task_struct *parent)
{
	u32 flag = TASK_TYPE_IGNORE;

	if (parent) {
		pid_t ppid = parent->tgid;

		if (ppid == PID_KTHREAD)
			flag |= TASK_TYPE_KERNEL;
		else if (((ppid == zygote_pid) || (ppid == zygote64_pid) ||
			 (ppid == maple_zygote64_pid)) &&
			 (tgid != systemserver_pid))
			flag |= TASK_TYPE_APP;
		else if (ppid == PID_INIT)
			flag |= TASK_TYPE_NATIVE;
	}
	if (!whitelist_empty && hashlist_find(whitelist, WHITELIST_LEN, tgid))
		flag |= TASK_TYPE_WHITE | TASK_TYPE_JANK;

	return flag;
}

static void refresh_zygote_pids(void)
{
	int max_count = PID_MAX_LIMIT;
	int batch_count = HUNG_TASK_BATCHING;
	struct task_struct *g = NULL;
	struct task_struct *t = NULL;
#ifdef CONFIG_DFX_DIE_CATCH
	unsigned short vipthread_flag;
#endif

	rcu_read_lock();
	do_each_thread(g, t) {
		if (rcu_break(&max_count, &batch_count, g, t))
			goto unlock;
#ifdef CONFIG_DFX_DIE_CATCH
		if (t->signal)
			vipthread_flag = t->signal->unexpected_die_catch_flags;
		else
			vipthread_flag = 0;
#endif
#ifdef CONFIG_DFX_OHOS
		if (!strncmp(t->comm, "appspawn", TASK_COMM_LEN)) {
#else
		if (!strncmp(t->comm, "main", TASK_COMM_LEN)) {
#endif
#ifdef CONFIG_DFX_DIE_CATCH
			if (vipthread_flag == 0)
				continue;
#endif
			if ((t->tgid < systemserver_pid) && vmboot_flag) {
				if (zygote64_pid == 0) {
					zygote64_pid = t->tgid;
					pr_info("zygote64-%d\n", zygote64_pid);
				} else {
					zygote_pid = t->tgid;
					pr_info("zygote-%d\n", zygote_pid);
					vmboot_flag = false;
				}
			}
		} else if (!strncmp(t->comm, "system_server", TASK_COMM_LEN)) {
			systemserver_pid = t->tgid;
			if (systemserver_pid != last_systemserver_pid) {
				vmboot_flag = true;
				last_systemserver_pid = systemserver_pid;
			}
			if (t->pid == t->tgid) {
				maple_zygote64_pid = t->real_parent->tgid;
				pr_info("maple_zygote64-%d, system_server-%d\n",
				       maple_zygote64_pid, systemserver_pid);
			}
		}
	} while_each_thread(g, t);
unlock:
	rcu_read_unlock();
}

pid_t get_system_server_pid(void)
{
	return systemserver_pid;
}

static void refresh_task_type(pid_t pid, int task_type)
{
	struct task_item *item = NULL;
	struct rb_node *p = NULL;

	spin_lock(&list_tasks_lock);
	for (p = rb_first(&list_tasks); p; p = rb_next(p)) {
		item = rb_entry(p, struct task_item, node);
		if (item->tgid == pid)
			item->task_type = task_type;
	}
	spin_unlock(&list_tasks_lock);
}

static void refresh_whitelist_pids(void)
{
	int i;

	hashlist_clear(whitelist, WHITELIST_LEN);
	for (i = 0; i < WHITELIST_LEN; i++) {
		if (!strlen(whitetmplist[i].name))
			continue;
		whitetmplist[i].pid =
			get_pid_by_name(whitetmplist[i].name);
		if (!whitetmplist[i].pid)
			continue;
		refresh_task_type(whitetmplist[i].pid,
				  TASK_TYPE_WHITE | TASK_TYPE_JANK);
		if (hashlist_insert(whitelist, WHITELIST_LEN,
				    whitetmplist[i].pid))
			pr_info("whitelist[%d]-%s-%d\n", i,
				whitetmplist[i].name, whitetmplist[i].pid);
		else
			pr_info("can't find %s\n", whitetmplist[i].name);
	}
	refresh_zygote_pids();
}

static struct task_item *find_task(pid_t pid, struct rb_root *root)
{
	struct rb_node **p = &root->rb_node;
	struct task_item *cur = NULL;
	struct rb_node *parent = NULL;

	while (*p) {
		parent = *p;
		cur = rb_entry(parent, struct task_item, node);
		if (!cur)
			return NULL;
		if (pid < cur->pid)
			p = &(*p)->rb_left;
		else if (pid > cur->pid)
			p = &(*p)->rb_right;
		else
			return cur;
	}
	return NULL;
}

static bool insert_task(struct task_item *item, struct rb_root *root)
{
	struct rb_node **p = &root->rb_node;
	struct rb_node *parent = NULL;
	struct task_item *cur = NULL;

	while (*p) {
		parent = *p;

		cur = rb_entry(parent, struct task_item, node);
		if (!cur)
			return false;
		if (item->pid < cur->pid) {
			p = &(*p)->rb_left;
		} else if (item->pid > cur->pid) {
			p = &(*p)->rb_right;
		} else {
			pr_info("insert pid=%d,tgid=%d,name=%s,type=%d fail\n",
				item->pid, item->tgid,
				item->name, item->task_type);
			return false;
		}
	}
	rb_link_node(&item->node, parent, p);
	rb_insert_color(&item->node, root);
	return true;
}

static void show_block_task(struct task_item *taskitem, struct task_struct *p)
{
	u64 last_arrival;
	u64 last_queued;

#ifdef CONFIG_SCHED_INFO
	last_arrival = p->sched_info.last_arrival;
	last_queued = p->sched_info.last_queued;
#else
	last_arrival = 0;
	last_queued = 0;
#endif
	if (unlikely(p->flags & PF_FROZEN)) {
#ifdef CONFIG_DFX_OHOS
		if (taskitem)
			pr_err("name=%s,PID=%d,tgid=%d,tgname=%s,stat_frozen_by=%d,"
			       "procs_frozen_by=%d, FROZEN for %ds,type=%d,SP=0x%08lx,la%llu/lq%llu,%d\n",
			       p->comm, p->pid, p->tgid,
			       p->group_leader->comm, p->frozen_stat_by, p->frozen_procs_by,
			       taskitem->d_state_time * HEARTBEAT_TIME,
			       taskitem->task_type, p->thread.cpu_context.sp,
			       last_arrival, last_queued, cgroup_state(p));
		else
			pr_err("name=%s,PID=%d,tgid=%d,tgname=%s,stat_frozen_by=%d,"
			       "procs_frozen_by=%d, just FROZE,SP=0x%08lx,la%llu/lq%llu,%d\n",
			       p->comm, p->pid, p->tgid, p->group_leader->comm,
			       p->frozen_stat_by, p->frozen_procs_by, p->thread.cpu_context.sp,
			       last_arrival, last_queued, cgroup_state(p));
#else
		if (taskitem)
			pr_err("name=%s,PID=%d,tgid=%d,tgname=%s,frozen_by=%d,"
			       "FROZEN for %ds,type=%d,SP=0x%08lx,la%llu/lq%llu,%d\n",
			       p->comm, p->pid, p->tgid,
			       p->group_leader->comm, p->frozen_by,
			       taskitem->d_state_time * HEARTBEAT_TIME,
			       taskitem->task_type, p->thread.cpu_context.sp,
			       last_arrival, last_queued, cgroup_state(p));
		else
			pr_err("name=%s,PID=%d,tgid=%d,tgname=%s,frozen_by=%d,"
			       "just FROZE,SP=0x%08lx,la%llu/lq%llu,%d\n",
			       p->comm, p->pid, p->tgid, p->group_leader->comm,
			       p->frozen_by, p->thread.cpu_context.sp,
			       last_arrival, last_queued, cgroup_state(p));
#endif
	} else {
		if (taskitem)
			pr_err("name=%s,PID=%d,tgid=%d,prio=%d,cpu=%d,tgname=%s,"
			       "type=%d,blocked for %ds,SP=0x%08lx,la%llu/lq%llu\n",
			       taskitem->name, taskitem->pid, p->tgid, p->prio,
			       task_cpu(p), p->group_leader->comm, taskitem->task_type,
			       taskitem->d_state_time * HEARTBEAT_TIME,
			       p->thread.cpu_context.sp, last_arrival, last_queued);
		else
			pr_err("name=%s,PID=%d,tgid=%d,prio=%d,cpu=%d,"
			       "tgname=%s,SP=0x%08lx,la%llu/lq%llu\n",
			       p->comm, p->pid, p->tgid, p->prio, task_cpu(p),
			       p->group_leader->comm, p->thread.cpu_context.sp,
			       last_arrival, last_queued);

		sched_show_task(p);
	}
}

void htbase_show_state_filter(u64 state_filter)
{
	struct task_struct *g = NULL;
	struct task_struct *p = NULL;
	struct task_item *taskitem = NULL;

#if BITS_PER_LONG == 32
	pr_info("  task                PC stack   pid father\n");
#else
	pr_info("  task                        PC stack   pid father\n");
#endif
	rcu_read_lock();
	for_each_process_thread(g, p) {
		touch_nmi_watchdog();
		if (((p->state == TASK_RUNNING) || (p->state & state_filter)) &&
		    !htext_find_ignorelist(p->pid)) {
			spin_lock(&list_tasks_lock);
			taskitem = find_task(p->pid, &list_tasks);
			spin_unlock(&list_tasks_lock);
			show_block_task(taskitem, p);
		}
	}
	touch_all_softlockup_watchdogs();
	rcu_read_unlock();

	if ((state_filter == TASK_UNINTERRUPTIBLE) || !state_filter)
		debug_show_all_locks();
}

void hungtask_show_state_filter(u64 state_filter)
{
	pr_err("BinderChain_SysRq start\n");
	htbase_show_state_filter(state_filter);
	pr_err("BinderChain_SysRq end\n");
}
EXPORT_SYMBOL(hungtask_show_state_filter);

void do_dump_task(struct task_struct *task)
{
	sched_show_task(task);
	debug_show_held_locks(task);
}

void do_show_task(struct task_struct *task, u32 flag, int d_state_time)
{
	rcu_read_lock();
	if (!pid_alive(task)) {
		rcu_read_unlock();
		return;
	}
	if (flag & (FLAG_DUMP_WHITE | FLAG_DUMP_APP)) {
		int cnt = 0;

		trace_sched_process_hang(task);
		cnt = d_state_time;
		pr_err("INFO: task %s:%d tgid:%d blocked for %ds in %s\n",
			   task->comm, task->pid, task->tgid,
			   (HEARTBEAT_TIME * cnt),
			   (flag & FLAG_DUMP_WHITE) ? "whitelist" : "applist");
		pr_err("      %s %s %.*s\n",
			   print_tainted(), init_utsname()->release,
			   (int)strcspn(init_utsname()->version, " "),
			   init_utsname()->version);
		do_dump_task(task);
		touch_nmi_watchdog();
		if ((flag & FLAG_DUMP_WHITE) && (!dump_and_upload)) {
			dump_and_upload++;
			upload.pid = task->pid;
			upload.tgid = task->tgid;
			upload.duration = d_state_time;
			memset(upload.name, 0, sizeof(upload.name));
			if (strncpy_s(upload.name, sizeof(upload.name),
				      task->comm, sizeof(task->comm)) != EOK)
				pr_err("failed to copy upload comm name\n");
			upload.flag = flag;
			if (task->flags & PF_FROZEN)
				upload.flag = upload.flag | FLAG_PF_FROZEN;
		}
	}
	htext_do_dump_jank(task, flag, d_state_time);
	rcu_read_unlock();
}

static void do_panic(void)
{
	if (hungtask_get_panic()) {
		trigger_all_cpu_backtrace();
#if defined (CONFIG_BLACKBOX) && defined(CONFIG_QCOM_ARCH)
		rb_sreason_set("hungtask");
#endif
		panic("hungtask: blocked tasks");
	}
}

static void create_taskitem(struct task_item *taskitem,
			    struct task_struct *task)
{
	taskitem->pid = task->pid;
	taskitem->tgid = task->tgid;
	taskitem->name[0] = 0;
	taskitem->switch_count = task->nvcsw + task->nivcsw;
	taskitem->dump_wa = 0;
	taskitem->panic_wa = 0;
	taskitem->d_state_time = -1;
	taskitem->isdone_wa = true;
#ifdef CONFIG_DFX_HUNGTASK_EXT
	taskitem->dump_jank = 0;
	taskitem->isdone_jank = true;
	taskitem->isreport_jank = false;
#endif
	if (strncpy_s(taskitem->name, sizeof(taskitem->name),
		      task->comm, sizeof(task->comm)) != EOK)
		pr_err("create taskitem failed\n");
}

static bool refresh_task(struct task_item *taskitem, struct task_struct *task)
{
	bool is_called = false;

	if (taskitem->switch_count != (task->nvcsw + task->nivcsw)) {
		taskitem->switch_count = task->nvcsw + task->nivcsw;
		is_called = true;
		return is_called;
	}
	if (taskitem->task_type & TASK_TYPE_WHITE) {
		taskitem->isdone_wa = false;
		taskitem->dump_wa++;
		taskitem->panic_wa++;
	}
	htext_refresh_task_app(taskitem, task);
	taskitem->d_state_time++;
	if (task->flags & PF_FROZEN)
		taskitem->task_type |= TASK_TYPE_FROZEN;
	return is_called;
}

static void remove_list_tasks(struct task_item *item)
{
	rb_erase(&item->node, &list_tasks);
	kfree(item);
}

static void shrink_process_item(struct task_item *item, bool *is_finish)
{
	if (remove_cnt >= MAX_REMOVE_LIST_NUM) {
		int i;

		remove_list[remove_cnt++] = item;
		for (i = 0; i < remove_cnt; i++)
			remove_list_tasks(remove_list[i]);
		remove_cnt = 0;
		*is_finish = false;
	} else {
		remove_list[remove_cnt++] = item;
	}
}

static void shrink_list_tasks(void)
{
	int i;
	bool is_finish = false;
	struct rb_node *n = NULL;
	struct task_item *item = NULL;

	spin_lock(&list_tasks_lock);
	while (!is_finish) {
		is_finish = true;
		for (n = rb_first(&list_tasks); n != NULL; n = rb_next(n)) {
			item = rb_entry(n, struct task_item, node);
			if (!item)
				continue;
			if (item->isdone_wa) {
				shrink_process_item(item, &is_finish);
				if (!is_finish)
					break;
			}
		}
	}
	for (i = 0; i < remove_cnt; i++)
		remove_list_tasks(remove_list[i]);
	remove_cnt = 0;
	spin_unlock(&list_tasks_lock);
}

static void check_parameters(void)
{
	if ((whitelist_dump_cnt < 0) ||
		(whitelist_dump_cnt > DEFAULT_WHITE_DUMP_CNT))
		whitelist_dump_cnt = DEFAULT_WHITE_DUMP_CNT;
	if ((whitelist_panic_cnt <= 0) ||
		(whitelist_panic_cnt > DEFAULT_WHITE_PANIC_CNT))
		whitelist_panic_cnt = DEFAULT_WHITE_PANIC_CNT;
	htext_check_parameters_app();
}

static void send_work_handler(struct work_struct *data)
{
	zrhung_send_event(ZRHUNG_WP_HUNGTASK, report_buf_tag_cmd,
			  report_buf_text);
}

static void htbase_report_zrhung_event(const char *report_buf_tag)
{
	htbase_show_state_filter(TASK_UNINTERRUPTIBLE);
#if defined(CONFIG_HW_EAS_SCHED) || defined(CONFIG_HISI_EAS_SCHED)
	print_cpu_rq_info();
#endif
	pr_err(" %s end\n", report_buf_tag);
	if (snprintf_s(report_buf_tag_cmd, sizeof(report_buf_tag_cmd),
		       sizeof(report_buf_tag_cmd) - 1,
		       "R=%s", report_buf_tag) <= 0)
		pr_err("failed to snprintf report_buf_tag_cmd\n");
	schedule_work(&send_work);
	report_zrhung_id++;
#ifdef CONFIG_SCHED_CFS_WORKAROUND
	enable_sched_hungtask_workaround_timeout(SCHED_HUNGTASK_WA_TIME);
#endif
}

static void htbase_report_zrhung(u32 event)
{
	bool report_load = false;
	char report_buf_tag[REPORT_MSGLENGTH] = {0};
	char report_name[TASK_COMM_LEN + 1] = {0};
	int report_pid = 0;
	int report_hungtime = 0;
	int report_tasktype = 0;
	int ret;

	if (!event)
		return;
	if (event & HUNGTASK_EVENT_WHITELIST) {
		if (snprintf_s(report_buf_tag, sizeof(report_buf_tag),
			       sizeof(report_buf_tag) - 1, "hungtask_whitelist_%d",
			       report_zrhung_id) <= 0)
			pr_err("failed to snprintf report_buf_tag\n");
		if (strncpy_s(report_name, sizeof(report_name), upload.name,
			      TASK_COMM_LEN) != EOK)
			pr_err("failed to copy report name\n");
		report_pid = upload.pid;
		report_tasktype = TASK_TYPE_WHITE;
		report_hungtime = whitelist_dump_cnt * HEARTBEAT_TIME;
		report_load = true;
	} else {
		pr_err("No such event report to zerohung!\n");
	}
	pr_err(" %s start\n", report_buf_tag);
	if (event & HUNGTASK_EVENT_WHITELIST)
		pr_err("report HUNGTASK_EVENT_WHITELIST to zrhung\n");
	if (upload.flag & FLAG_PF_FROZEN)
		ret = snprintf_s(report_buf_text, sizeof(report_buf_text),
				 sizeof(report_buf_text) - 1,
				 "Task %s(%s) pid %d type %d blocked %ds.",
				 report_name, "FROZEN", report_pid,
				 report_tasktype, report_hungtime);
	else
		ret = snprintf_s(report_buf_text, sizeof(report_buf_text),
				 sizeof(report_buf_text) - 1,
				 "Task %s pid %d type %d blocked %ds.",
				 report_name, report_pid, report_tasktype,
				 report_hungtime);
	if (ret <= 0)
		pr_err("failed to snprintf report_buf_text\n");
	if (report_load)
		htbase_report_zrhung_event(report_buf_tag);
}

#ifndef CONFIG_FINAL_RELEASE
static int print_frozen_list_item(int pid)
{
	int tmp;
	int left;

	if (!frozed_head) {
		tmp = snprintf_s(frozen_buf, FROZEN_BUF_LEN, FROZEN_BUF_LEN - 1,
				 "%s", "FROZEN Pid:");
		if (tmp < 0)
			return -EINVAL;
		frozen_used += min(tmp, FROZEN_BUF_LEN - 1);
		frozed_head = true;
	}
	left = FROZEN_BUF_LEN - frozen_used;
	tmp = snprintf_s(frozen_buf + frozen_used, left,
			 (left >= 1) ? left - 1 : 0,
			 "%d,", pid);
	if (tmp < 0)
		return -EINVAL;
	frozen_used += min(tmp, FROZEN_BUF_LEN - frozen_used - 1);
	return frozen_used;
}
#endif

int dump_task_wa(struct task_item *item, int dump_cnt,
		 struct task_struct *task, u32 flag)
{
	int ret = 0;
#ifndef CONFIG_FINAL_RELEASE
	int tmp;
#endif

	if ((item->d_state_time > TWO_MINUTES) &&
		(item->d_state_time % TWO_MINUTES != 0))
		return ret;
	if ((item->d_state_time > HUNG_TEN_MINUTES) &&
		(item->d_state_time % HUNG_TEN_MINUTES != 0))
		return ret;
	if ((item->d_state_time > HUNG_ONE_HOUR) &&
		(item->d_state_time % HUNG_ONE_HOUR != 0))
		return ret;
	if (dump_cnt && (item->dump_wa > dump_cnt)) {
		item->dump_wa = 1;
		if (!dump_and_upload && task->flags & PF_FROZEN) {
#ifndef CONFIG_FINAL_RELEASE
			tmp = print_frozen_list_item(item->pid);
			if (tmp < 0)
				return ret;
			if (tmp >= FROZEN_BUF_LEN - 1) {
				pr_err("%s", frozen_buf);
				memset(frozen_buf, 0, sizeof(frozen_buf));
				frozen_used = 0;
				frozed_head = false;
				print_frozen_list_item(item->pid);
			}
#endif
		} else if (!dump_and_upload) {
			pr_err("Ready to dump a task %s\n", item->name);
			do_show_task(task, flag, item->d_state_time);
			ret++;
		}
	}
	return ret;
}

static void update_panic_task(const struct task_item *item)
{
	if (upload.pid != 0)
		return;

	upload.pid = item->pid;
	upload.tgid = item->tgid;
	if (memcpy_s(upload.name, sizeof(upload.name),
	    item->name, strlen(item->name) != EOK))
		pr_err("failed to copy upload name");
}

static void deal_task(struct task_item *item, struct task_struct *task,
		      bool is_called)
{
	int any_dumped_num = 0;

	if (is_called) {
		item->dump_wa = 1;
		item->panic_wa = 1;
#ifdef CONFIG_DFX_HUNGTASK_EXT
		item->dump_jank = 1;
#endif
		item->d_state_time = 0;
		return;
	}
	if (item->task_type & TASK_TYPE_WHITE)
		any_dumped_num = dump_task_wa(item, whitelist_dump_cnt, task,
									  FLAG_DUMP_WHITE);
	if (!is_called && (item->task_type & TASK_TYPE_WHITE)) {
		if (whitelist_panic_cnt && item->panic_wa > whitelist_panic_cnt) {
			pr_err("Task %s is causing panic\n", item->name);
			update_panic_task(item);
			item->panic_wa = 0;
			hung_task_must_panic++;
		} else {
			item->isdone_wa = false;
		}
	}
	htext_deal_task_app(item, task, is_called, &any_dumped_num,
						dump_and_upload);
#ifdef CONFIG_DFX_HUNGTASK_EXT
	if (item->isdone_wa && item->isdone_jank)
#else
	if (item->isdone_wa)
#endif
		remove_list_tasks(item);
}

static bool check_conditions(struct task_struct *task, u32 task_type)
{
	bool no_check = true;

	if ((task->flags & PF_FROZEN))
		return no_check;
	if ((task_type & TASK_TYPE_WHITE) &&
		(whitelist_dump_cnt || whitelist_panic_cnt))
		no_check = false;
	if (!htext_check_conditions_app(task, task_type))
		no_check = false;
	return no_check;
}

static void htbase_check_one_task(struct task_struct *t)
{
	u32 task_type = TASK_TYPE_IGNORE;
	u64 switch_count = t->nvcsw + t->nivcsw;
	struct task_item *taskitem = NULL;
	bool is_called = false;

	if (unlikely(!switch_count)) {
		pr_info("skip task switch_count is zero\n");
		return;
	}

	if (htext_check_one_task_ignore(t, cur_heartbeat))
		return;

	taskitem = find_task(t->pid, &list_tasks);
	if (taskitem) {
		if (check_conditions(t, taskitem->task_type))
			return;
		is_called = refresh_task(taskitem, t);
	} else {
		task_type = get_task_type(t->pid, t->tgid, t->real_parent);
		if (check_conditions(t, task_type))
			return;
		taskitem = kmalloc(sizeof(*taskitem), GFP_ATOMIC);
		if (!taskitem) {
			pr_err("kmalloc failed\n");
			return;
		}
		memset(taskitem, 0, sizeof(*taskitem));
		taskitem->task_type = task_type;
		create_taskitem(taskitem, t);
		is_called = refresh_task(taskitem, t);
		insert_task(taskitem, &list_tasks);
	}
	deal_task(taskitem, t, is_called);
}

static void htbase_pre_process(void)
{
	htbase_set_timeout_secs(hungtask_get_timeout());
	cur_heartbeat++;
	if ((cur_heartbeat % REFRESH_INTERVAL) == 0)
		do_refresh = 1;
	else
		do_refresh = 0;
	if (do_refresh || (cur_heartbeat < TIME_REFRESH_PIDS)) {
		refresh_whitelist_pids();
		check_parameters();
	}
}

#ifndef CONFIG_CHIP_HONGHU
static bool zone_is_lowmem(struct zone *zone)
{
	unsigned long total_free, free_cma;
	unsigned long min_watermark, low_watermark;

	total_free = zone_page_state(zone, NR_FREE_PAGES);
	free_cma = zone_page_state(zone, NR_FREE_CMA_PAGES);
	low_watermark = low_wmark_pages(zone);
	min_watermark = min_wmark_pages(zone);
	return (total_free < low_watermark) || (total_free - free_cma < min_watermark);
}

static void print_lowmemory_info(void)
{
	struct zone *zone = NULL;

	for_each_populated_zone(zone) {
#ifdef CONFIG_ZONE_MEDIA
		if (IS_MEIDA_ZONE_IDX(zone_idx(zone)))
			continue;
#endif
		if (zone_is_lowmem(zone)) {
			show_mem(SHOW_MEM_FILTER_NODES, NULL);
#if defined(CONFIG_TOPN_MEMINFO_DUMP) || defined(CONFIG_HUAWEI_MEMINFO_DUMP)
			topn_process_mem_show();
#endif
			return;
		}
	}
}
#endif

static void htbase_post_process(void)
{
	struct rb_node *n = NULL;
	u32 hungevent = 0;

	if (frozen_used) {
		pr_err("%s", frozen_buf);
		memset(frozen_buf, 0, sizeof(frozen_buf));
		frozen_used = 0;
		frozed_head = false;
	}

	if (dump_and_upload == HUNG_TASK_UPLOAD_ONCE) {
		hungevent |= HUNGTASK_EVENT_WHITELIST;
		dump_and_upload++;
	}
	if (dump_and_upload > 0) {
		time_since_upload++;
		if (time_since_upload > (whitelist_panic_cnt - whitelist_dump_cnt)) {
			dump_and_upload = 0;
			time_since_upload = 0;
		}
	}
	if (hung_task_must_panic) {
		htbase_show_state_filter(TASK_UNINTERRUPTIBLE);
		hung_task_must_panic = 0;
		pr_err("Task %s:%d blocked for %ds is causing panic\n",
			   upload.name, upload.pid,
			   whitelist_panic_cnt * HEARTBEAT_TIME);
		htmmap_post_process_check(upload.pid);
#if defined(CONFIG_HW_EAS_SCHED) || defined(CONFIG_HISI_EAS_SCHED)
		print_cpu_rq_info();
#endif
#ifndef CONFIG_CHIP_HONGHU
		print_lowmemory_info();
#endif
		do_panic();
	}
	htext_post_process_ignorelist(cur_heartbeat);
	htuser_post_process_userlist();
	shrink_list_tasks();
	for (n = rb_first(&list_tasks); n != NULL; n = rb_next(n)) {
		struct task_item *item = rb_entry(n, struct task_item, node);

		item->isdone_wa = true;
#ifdef CONFIG_DFX_HUNGTASK_EXT
		item->isdone_jank = true;
#endif
	}
	if (hungevent) {
#ifdef CONFIG_LOCK_DETECTOR
		lock_detector_start();
#endif
		htbase_report_zrhung(hungevent);
	}
}

void htbase_check_tasks(u64 timeout)
{
	int max_count = PID_MAX_LIMIT;
	int batch_count = HUNG_TASK_BATCHING;
	struct task_struct *g = NULL;
	struct task_struct *t = NULL;

	if (!hungtask_enable)
		return;
	if (test_taint(TAINT_DIE) || did_panic) {
		pr_err("already in doing panic\n");
		return;
	}

	htbase_pre_process();
	rcu_read_lock();
	for_each_process_thread(g, t) {
		if (!max_count--)
			goto unlock;
		if (!--batch_count) {
			batch_count = HUNG_TASK_BATCHING;
			if (!rcu_lock_break(g, t))
				goto unlock;
		}
		if ((t->state == TASK_UNINTERRUPTIBLE) ||
		    (t->state == TASK_KILLABLE))
			htbase_check_one_task(t);
	}
unlock:
	rcu_read_unlock();
	htbase_post_process();
}

static ssize_t htbase_enable_show(struct kobject *kobj,
				  struct kobj_attribute *attr,
				  char *buf)
{
	int ret;

	if (hungtask_enable)
		ret = snprintf_s(buf, ENABLE_SHOW_LEN, ENABLE_SHOW_LEN - 1,
				 "on\n");
	else
		ret = snprintf_s(buf, ENABLE_SHOW_LEN, ENABLE_SHOW_LEN - 1,
				 "off\n");
	return ret <= 0 ? 0 : ret;
}

static ssize_t htbase_enable_store(struct kobject *kobj,
				   struct kobj_attribute *attr,
				   const char *buf, size_t count)
{
	char tmp[6]; /* only store "on" "off" "kick" and enter */
	size_t len;
	char *p = NULL;

	if (!buf)
		return -EINVAL;
	if ((count < 2) || (count > (sizeof(tmp) - 1))) {
		pr_err("wrong value while writing hungtask enable\n");
		return -EINVAL;
	}

	p = memchr(buf, '\n', count);
	len = p ? (size_t)(p - buf) : count;
	memset(tmp, 0, sizeof(tmp));
	if (strncpy_s(tmp, sizeof(tmp), buf, len) != EOK)
		pr_err("strncpy from buf to tmp failed\n");

	if (!strncmp(tmp, "on", strlen(tmp))) {
		hungtask_enable = HT_ENABLE;
		pr_info("set hungtask_enable to enable\n");
	} else if (!strncmp(tmp, "off", strlen(tmp))) {
		hungtask_enable = HT_DISABLE;
		pr_info("set hungtask_enable to disable\n");
	} else {
		pr_err("only accept on or off\n");
	}
	return (ssize_t) count;
}

static ssize_t htbase_monitorlist_show(struct kobject *kobj,
				       struct kobj_attribute *attr,
				       char *buf)
{
	int i;
	char *start = buf;
	char all_buf[WHITELIST_STORE_LEN - 20]; /* exclude extra header len 20 */
	u64 len = 0;
	int ret;
	int left;

	memset(all_buf, 0, sizeof(all_buf));
	for (i = 0; i < WHITELIST_LEN; i++) {
		if (whitetmplist[i].pid > 0) {
			left = sizeof(all_buf) - len;
			ret = snprintf_s(all_buf + len, left,
					 (left >= 1) ? left - 1 : 0,
					 "%s-%d,", whitetmplist[i].name,
					 whitetmplist[i].pid);
			if (ret > 0)
				len += ret;
			if (len >= sizeof(all_buf)) {
				len = sizeof(all_buf) - 1;
				break;
			}
		}
	}
	if (len > 0)
		all_buf[len] = 0;
	if (whitelist_type == WHITE_LIST)
		ret = snprintf_s(buf, WHITELIST_STORE_LEN,
				 WHITELIST_STORE_LEN - 1,
				 "whitelist:[%s]\n", all_buf);
	else if (whitelist_type == BLACK_LIST)
		ret = snprintf_s(buf, WHITELIST_STORE_LEN,
				 WHITELIST_STORE_LEN - 1,
				 "blacklist:[%s]\n", all_buf);
	else
		ret = snprintf_s(buf, WHITELIST_STORE_LEN,
				 WHITELIST_STORE_LEN - 1, "\n");
	if (ret > 0)
		buf += ret;

	return buf - start;
}

static void htbase_monitorlist_update(char **cur)
{
	int index = 0;
	char *token = NULL;

	hashlist_clear(whitelist, WHITELIST_LEN);
	memset(whitetmplist, 0, sizeof(whitetmplist));
	/* generate the new whitelist */
	for (;;) {
		token = strsep(cur, ",");
		if (token && strlen(token)) {
			if (strncpy_s(whitetmplist[index].name,
				      TASK_COMM_LEN + 1,
				      token, TASK_COMM_LEN) != EOK)
				pr_err("failed to copy whitelist name\n");
			if (strlen(whitetmplist[index].name) > 0)
				whitelist_empty = false;
			index++;
			if (index >= WHITELIST_LEN)
				break;
		}
		if (!(*cur))
			break;
	}
}

static ssize_t htbase_monitorlist_store(struct kobject *kobj,
					struct kobj_attribute *attr,
					const char *buf, size_t n)
{
	size_t len;
	char *p = NULL;
	char all_buf[WHITELIST_STORE_LEN];
	char *cur = all_buf;

	if ((n < 2) || (n > (sizeof(all_buf) - 1))) {
		pr_err("whitelist input string illegal\n");
		return -EINVAL;
	}
	if (!buf)
		return -EINVAL;
	/*
	 * input format:
	 * write /sys/kernel/hungtask/monitorlist "whitelist,
	 * system_server,surfaceflinger"
	 */
	p = memchr(buf, '\n', n);
	len = p ? (size_t)(p - buf) : n; /* exclude the '\n' */

	memset(all_buf, 0, sizeof(all_buf));
	len = len > WHITELIST_STORE_LEN ? WHITELIST_STORE_LEN : len;
	if (strncpy_s(all_buf, sizeof(all_buf), buf, len) != EOK)
		pr_err("failed to copy all_buf\n");
	p = strsep(&cur, ",");
	if (!cur) {
		pr_err("input string is not correct\n");
		return -EINVAL;
	}
	if (!strncmp(p, "whitelist", n)) {
		whitelist_type = WHITE_LIST;
	} else {
		if (!strncmp(p, "blacklist", n))
			pr_err("blacklist is not support\n");
		else
			pr_err("wrong list type is set\n");
		return -EINVAL;
	}
	if (!strlen(cur)) {
		pr_err("at least one process should be set\n");
		return -EINVAL;
	}
	pr_err("whitelist is %s\n", cur);

	htbase_monitorlist_update(&cur);
	/* check again in case user input "whitelist,,,,,," */
	if (whitelist_empty) {
		pr_err("at least one process need to be set\n");
		return -EINVAL;
	}
	return (ssize_t)n;
}

/* used for sysctl at /proc/sys/kernel/hung_task_timeout_secs */
void htbase_set_timeout_secs(u64 new_val)
{
	if ((new_val > CONFIG_DEFAULT_HUNG_TASK_TIMEOUT) ||
	    (new_val % HEARTBEAT_TIME)) {
		pr_err("wrong hungtask timeout value\n");
		return;
	}
	hungtask_timeout_secs = new_val;

	whitelist_panic_cnt = (int)(hungtask_timeout_secs / HEARTBEAT_TIME);
	if (whitelist_panic_cnt > THIRTY_SECONDS)
		whitelist_dump_cnt = whitelist_panic_cnt / HT_DUMP_IN_PANIC_LOOSE;
	else
		whitelist_dump_cnt = whitelist_panic_cnt / HT_DUMP_IN_PANIC_STRICT;
	htext_set_app_dump_count(whitelist_dump_cnt);
}

void htbase_set_panic(int new_val)
{
	did_panic = new_val;
}

static struct kobj_attribute timeout_attribute = {
	.attr = {
		 .name = "enable",
		 .mode = 0640,
	},
	.show = htbase_enable_show,
	.store = htbase_enable_store,
};

static struct kobj_attribute monitorlist_attr = {
	.attr = {
		 .name = "monitorlist",
		 .mode = 0640,
	},
	.show = htbase_monitorlist_show,
	.store = htbase_monitorlist_store,
};

#ifdef CONFIG_DFX_HUNGTASK_USER
static struct kobj_attribute userlist_attr = {
	.attr = {
		 .name = "userlist",
		 .mode = 0640,
	},
	.show = htuser_list_show,
	.store = htuser_list_store,
};
#endif

static struct attribute *attrs[] = {
	&timeout_attribute.attr,
	&monitorlist_attr.attr,
#ifdef CONFIG_DFX_HUNGTASK_USER
	&userlist_attr.attr,
#endif
	NULL
};

static struct attribute_group hungtask_attr_group = {
	.attrs = attrs,
};

static struct kobject *hungtask_kobj;
int htbase_create_sysfs(void)
{
	int i;
	int ret;

	/* sleep 1000ms and wait /sys/kernel ready */
	while (!kernel_kobj)
		msleep(1000);

	hungtask_kobj = kobject_create_and_add("hungtask", kernel_kobj);
	if (!hungtask_kobj)
		return -ENOMEM;
	ret = sysfs_create_group(hungtask_kobj, &hungtask_attr_group);
	if (ret)
		kobject_put(hungtask_kobj);

	for (i = 0; i < WHITELIST_LEN; i++)
		INIT_HLIST_HEAD(&whitelist[i]);
	memset(whitetmplist, 0, sizeof(whitetmplist));

	INIT_WORK(&send_work, send_work_handler);
	htext_init();
	if (htmmap_sem_debug_init())
		return -ENOMEM;

	return ret;
}
