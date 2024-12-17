/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 * Description: split shrinker_list into hot list and cold list
 * Author: Gong Chen <gongchen4@huawei.com>
 * Create: 2021-05-28
 */
#ifndef _SPLIT_SHRINKER_H
#define _SPLIT_SHRINKER_H
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/rwsem.h>
#include <linux/shrinker.h>

#ifdef CONFIG_SPLIT_SHRINKER_DEBUG
#include <linux/sysctl.h>
#endif

void register_split_shrinker(struct shrinker *shrinker);
void unregister_split_shrinker(struct shrinker *shrinker);

void set_fs_shrinker_type(struct shrinker *s, const char *fs_name, int flags);
bool should_delay_fs_shrink(const char *fs_name, const char *dev_name, const char *mnt_dir);
int get_delay_shrinker_prio(void);
int get_shrinker_number_limit(void);
int get_shrinker_percent_limit(void);

static inline void set_delay_fs_shrinker(struct shrinker *s, unsigned long flags)
{
	if (flags & SB_DELAY_SHRINK)
		s->flags |= SHRINKER_DELAY;
}

#ifdef CONFIG_SPLIT_SHRINKER_DEBUG
int split_shrinker_sysctl_handler(struct ctl_table *table, int write,
	void *buffer, size_t *length, loff_t *ppos);
int register_skip_shrinker_sysctl_handler(struct ctl_table *table, int write,
	void *buffer, size_t *length, loff_t *ppos);

bool is_split_shrinker_enable(void);
bool is_debug_skip_shrinker_enable(void);
#else
static inline bool is_split_shrinker_enable(void)
{
	return true;
}
static inline bool is_debug_skip_shrinker_enable(void)
{
	return false;
}
#endif
#endif
