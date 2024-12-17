/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2001-2023. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Description: extend slub_debug info with the slab usage of each process
 * Author: Lin Mu <linmu4@huawei.com> Li Xilun <lixilun1@huawei.com>
 * Create: 2023-03-23
 */

static bool do_slab_usage_show(struct seq_file *seq, void *v)
{
	struct location *l = NULL;
	struct loc_track *t = seq->private;
	unsigned int idx = *(unsigned int *)v;

	if (strcmp(seq->file->f_path.dentry->d_name.name, "usage_traces") != 0)
		return false;

	if (idx < t->count) {
		l = &t->loc[idx];
		seq_printf(seq, "%15ld %*s pid=%d tgid=%d\n", l->count, TASK_COMM_LEN,
				l->comm, l->pid, l->tgid);
	}

	if (!idx && !t->count)
		seq_puts(seq, "No data\n");

	return true;
}

/* orig_size is added by memory wasting of kmalloc feature */
#ifdef SLAB_KMALLOC
static int add_usage_location(struct loc_track *t, struct kmem_cache *s,
		const struct track *track, unsigned int orig_size)
#else
static int add_usage_location(struct loc_track *t, struct kmem_cache *s,
		const struct track *track)
#endif
{
	long start, end, pos;
	struct location *l = NULL;
	pid_t pid;
	int cmp;

	start = -1;
	end = t->count;

	while (1) {
		pos = start + (end - start + 1) / 2;

		if (pos == end)
			break;

		pid = t->loc[pos].pid;
		cmp = strcmp(track->comm, t->loc[pos].comm);
		if (track->pid == pid && cmp == 0) {
			l = &t->loc[pos];
			l->count++;
			return 1;
		}

		if (track->pid < pid || (track->pid == pid && cmp < 0))
			end = pos;
		else
			start = pos;
	}

	if (t->count >= t->max && !alloc_loc_track(t, 2 * t->max, GFP_ATOMIC))
		return 0;

	l = t->loc + pos;
	if (pos < t->count && memmove_s(l + 1, (t->max - pos - 1) *
			sizeof(struct location), l, (t->count - pos) *
			sizeof(struct location)) != EOK)
		return 0;
	t->count++;
	l->count = 1;
	l->pid = track->pid;
	l->tgid = track->tgid;
	if (memcpy_s(l->comm, TASK_COMM_LEN, track->comm, TASK_COMM_LEN) != EOK)
		return 0;

	return 1;
}

static inline void do_loc_prepare(const struct file *f, struct loc_track *t,
		enum track_item *item)
{
	if (strcmp(f->f_path.dentry->d_name.name, "usage_traces") == 0) {
		t->get_loc_map = add_usage_location;
		*item = TRACK_ALLOC;
	} else {
		t->get_loc_map = add_location;
	}
}
