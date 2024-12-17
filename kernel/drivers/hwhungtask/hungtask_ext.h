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

#ifndef _DFX_HUNGTASK_EXT_H
#define _DFX_HUNGTASK_EXT_H

#include <linux/types.h>

#ifdef CONFIG_DFX_HUNGTASK_EXT
void htext_do_dump_jank(struct task_struct *task, int flag, int d_state_time);
void htext_refresh_task_app(struct task_item *taskitem,
			    struct task_struct *task);
bool htext_check_conditions_app(struct task_struct *task, u32 task_type);
void htext_deal_task_app(struct task_item *item, struct task_struct *task,
			 bool is_called, int *any_dumped_num, int upload);
void htext_check_parameters_app(void);
bool htext_find_ignorelist(int pid);
bool htext_check_one_task_ignore(struct task_struct *t, int cur_heartbeat);
void htext_post_process_ignorelist(int cur_heartbeat);
void htext_set_app_dump_count(int count);
void htext_init(void);
#else
static inline void htext_do_dump_jank(struct task_struct *task, int flag,
				      int d_state_time)
{
}

static inline void htext_refresh_task_app(struct task_item *taskitem,
					  struct task_struct *task)
{
}

static inline bool htext_check_conditions_app(struct task_struct *task,
					      u32 task_type)
{
	return true;
}

static inline void htext_deal_task_app(struct task_item *item,
				       struct task_struct *task,
				       bool is_called,
				       int *any_dumped_num,
				       int upload)
{
}

static inline void htext_check_parameters_app(void)
{
}

static inline bool htext_find_ignorelist(int pid)
{
	return false;
}

static inline bool htext_check_one_task_ignore(struct task_struct *t,
					       int cur_heartbeat)
{
	return false;
}

static inline void htext_post_process_ignorelist(int cur_heartbeat)
{
}

static inline void htext_set_app_dump_count(int count)
{
}

static inline void htext_init(void)
{
}
#endif

#endif
