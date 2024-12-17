/*
 * hungtask_ext.c
 *
 * Hung Task Extention Module Used For Detect App Frozen
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

#define pr_fmt(fmt)	"hungtask_ext " fmt

#include <linux/kallsyms.h>
#include <linux/ptrace.h>
#include <platform/linux/zrhung.h>

#include "securec.h"
#include "hungtask_base.h"

#define IGNLIST_LEN 101
#define JANK_REPORT_TRESHOLD 1
#define DEFAULT_APP_DUMP_CNT MAX_LOOP_NUM
#define DEFAULT_JANK_DUMP_CNT JANK_REPORT_TRESHOLD
#define DEFAULT_OTHER_LOG_CNT MAX_LOOP_NUM
#define IGN_STATE_INIT 1
#define IGN_STATE_FIRST 2
#define IGN_STATE_DONE 3

static struct hlist_head ignlist[IGNLIST_LEN];
static struct hlist_head ignlisttmp[IGNLIST_LEN];
static int janklist_dump_cnt = DEFAULT_JANK_DUMP_CNT;
static char report_jank_text[REPORT_MSGLENGTH];
static int applist_dump_cnt = DEFAULT_APP_DUMP_CNT;
static int other_log_cnt = DEFAULT_OTHER_LOG_CNT;
static int ign_state = IGN_STATE_INIT;
static struct work_struct send_work;

static void send_work_handler(struct work_struct *data)
{
	zrhung_send_event(ZRHUNG_WP_HTSK_WARNING, NULL, report_jank_text);
}

void htext_set_app_dump_count(int count)
{
	applist_dump_cnt = count;
}

static void jank_print_task_wchan(struct task_struct *task)
{
	u64 wchan;
	char symname[KSYM_NAME_LEN] = {0};

	wchan = get_wchan(task);
	if (lookup_symbol_name(wchan, symname) < 0) {
		if (!ptrace_may_access(task, PTRACE_MODE_READ_FSCREDS))
			return;
		pr_err("Task %s pid:%d tgid: %d wchan:[<%08lx>]\n",
		       task->comm, task->pid, task->tgid, wchan);
	} else {
		pr_err("Task %s pid:%d tgid: %d wchan:[<%08lx>]-%s\n",
		       task->comm, task->pid, task->tgid, wchan, symname);
	}
	if (snprintf_s(report_jank_text, sizeof(report_jank_text),
		       sizeof(report_jank_text) - 1,
		       "janklist Blocked task %s pid:%d tgid:%d tgname:%s wchan %s\n",
		       task->comm, task->pid, task->tgid,
		       task->group_leader->comm,
		       symname) <= 0)
		pr_err("failed to snprintf report_jank_text\n");
	schedule_work(&send_work);
}

void htext_do_dump_jank(struct task_struct *task, u32 flag, int d_state_time)
{
	if (flag & FLAG_DUMP_JANK)
		do_dump_task(task);
}

void htext_refresh_task_app(struct task_item *taskitem,
			    struct task_struct *task)
{
	if (taskitem->task_type & TASK_TYPE_APP) {
		taskitem->isdone_wa = false;
		taskitem->dump_wa++;
		taskitem->panic_wa++;
	}
	if (taskitem->task_type & TASK_TYPE_JANK) {
		taskitem->isdone_jank = false;
		taskitem->dump_jank++;
	} else if (!(taskitem->task_type & (TASK_TYPE_WHITE | TASK_TYPE_APP))) {
		taskitem->dump_wa++;
		taskitem->isdone_wa = false;
	}
}

bool htext_check_conditions_app(struct task_struct *task,
				u32 task_type)
{
	bool no_check = true;

	if (task_type & TASK_TYPE_APP && applist_dump_cnt)
		no_check = false;
	if (task_type & TASK_TYPE_JANK && janklist_dump_cnt)
		no_check = false;
	else if (!(task_type & (TASK_TYPE_WHITE | TASK_TYPE_APP)) &&
		 (ign_state == IGN_STATE_DONE))
		no_check = false;

	return no_check;
}

#ifndef CONFIG_FINAL_RELEASE
static void show_unconcerned_block_task(struct task_item *item, struct task_struct *task)
{
	if (unlikely(item->task_type & TASK_TYPE_FROZEN))
#ifdef CONFIG_DFX_OHOS
		pr_err("hungtask: Unconcerned task %s:%d stat is frozen by %d, procs is frozen by %d for %ds\n",
		   item->name, item->pid, task->frozen_stat_by, task->frozen_procs_by, item->d_state_time * HEARTBEAT_TIME);
#else
		pr_err("hungtask: Unconcerned task %s:%d is frozen by %d for %ds\n",
		   item->name, item->pid, task->frozen_by, item->d_state_time * HEARTBEAT_TIME);
#endif
	else
		pr_err("hungtask: Unconcerned task %s:%d is blocked for %ds\n",
		   item->name, item->pid, item->d_state_time * HEARTBEAT_TIME);
}
#endif

void htext_deal_task_app(struct task_item *item, struct task_struct *task,
			 bool is_called, int *any_dumped_num, int upload)
{
	if (item->task_type & TASK_TYPE_APP)
		*any_dumped_num = dump_task_wa(item, applist_dump_cnt, task,
					       FLAG_DUMP_APP);
	if (item->task_type & TASK_TYPE_JANK && janklist_dump_cnt &&
	    (item->dump_jank > janklist_dump_cnt) && !item->isreport_jank &&
	    !upload) {
		item->isreport_jank = true;
		jank_print_task_wchan(task);
		do_show_task(task, FLAG_DUMP_JANK, item->d_state_time);
	} else if (!(item->task_type & (TASK_TYPE_WHITE | TASK_TYPE_APP)) &&
		   item->dump_wa > other_log_cnt &&
		   item->d_state_time < HUNG_ONE_HOUR) {
#ifndef CONFIG_FINAL_RELEASE
		show_unconcerned_block_task(item, task);
#endif
		item->dump_wa = 1;
		(*any_dumped_num)++;
	}
}

void htext_check_parameters_app(void)
{
	if ((applist_dump_cnt < 0) || (applist_dump_cnt > MAX_LOOP_NUM))
		applist_dump_cnt = DEFAULT_APP_DUMP_CNT;
	if ((janklist_dump_cnt < 0) || (janklist_dump_cnt > MAX_LOOP_NUM))
		janklist_dump_cnt = DEFAULT_JANK_DUMP_CNT;
}

bool htext_find_ignorelist(int pid)
{
	return hashlist_find(ignlist, IGNLIST_LEN, pid);
}

bool htext_check_one_task_ignore(struct task_struct *t, int cur)
{
	if (ign_state == IGN_STATE_DONE) {
		return hashlist_find(ignlist, IGNLIST_LEN, t->pid);
	} else if ((ign_state == IGN_STATE_INIT) && (cur > ONE_MINUTE)) {
		hashlist_insert(ignlisttmp, IGNLIST_LEN, t->pid);
	} else if ((ign_state == IGN_STATE_FIRST) &&
		   (cur <= ONE_AND_HALF_MINUTE)) {
		if (hashlist_find(ignlisttmp, IGNLIST_LEN, t->pid)) {
			if (strcmp(t->comm, "recovery") && strcmp(t->comm, "update_binary"))
				hashlist_insert(ignlist, IGNLIST_LEN, t->pid);
		}
	}
	return false;
}

void htext_post_process_ignorelist(int cur)
{
	if ((ign_state == IGN_STATE_INIT) && (cur > ONE_MINUTE))
		ign_state = IGN_STATE_FIRST;
	else if ((ign_state == IGN_STATE_FIRST) &&
		 (cur > ONE_AND_HALF_MINUTE))
		ign_state = IGN_STATE_DONE;
}

void htext_init(void)
{
	int i;

	INIT_WORK(&send_work, send_work_handler);
	for (i = 0; i < IGNLIST_LEN; i++) {
		INIT_HLIST_HEAD(&ignlist[i]);
		INIT_HLIST_HEAD(&ignlisttmp[i]);
	}
}
