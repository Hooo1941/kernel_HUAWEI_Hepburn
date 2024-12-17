// SPDX-License-Identifier: GPL-2.0
/*
 * ffrt_rtg.c
 *
 * ffrt rtg ioctl
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
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
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/compat.h>
#include <linux/kernel.h>
#include <linux/hisi_rtg.h>
#include <../kernel/sched/sched_rtg.h>
#include <platform_include/cee/linux/sched/ffrt/ffrt_rtg.h>
#define CREATE_TRACE_POINTS
#include <trace/events/ffrt.h>
#define FFRT_RTG_IPC_MAGIC 0XBC

#define ffrt_rtg_err(fmt, ...) pr_err("FFRT_RTG "fmt"\n", ##__VA_ARGS__)
#define ffrt_rtg_info(fmt, ...) pr_info("FFRT_RTG "fmt"\n", ##__VA_ARGS__)
#define ffrt_rtg_debug(fmt, ...) pr_debug("FFRT_RTG "fmt"\n", ##__VA_ARGS__)

#define ffrt_rtg_assert_cmd(msg_cmd, cmd) \
	if (msg_cmd != cmd) { \
		ffrt_rtg_err("Invalid RTG cmd (%d, %d)", msg_cmd, cmd); \
		return RTG_INVALID_CMD; \
	}

enum ffrt_rtg_errno {
	RTG_SUCC,
	RTG_INVALID_ARG,
	RTG_INVALID_MAGIC,
	RTG_INVALID_CMD,
	RTG_FAILED,
	RTG_ERRNO_MAX,
};

enum ffrt_rtg_cmdid {
	RTG_CMD_CREATE_RTG,
	RTG_CMD_RELEASE_RTG,
	RTG_CMD_ADD_RTG_THREAD,
	RTG_CMD_DEL_RTG_THREAD,
	RTG_CMD_SET_GROUP_UTIL,
	RTG_CMD_SET_GROUP_FREQ,
	RTG_CMD_GET_THREAD_LOAD,
	RTG_CMD_GET_GROUP_LOAD,
	RTG_CMD_SET_GROUP_WINDOW_SIZE,
	RTG_CMD_SET_GROUP_WINDOW_ROLLOVER,
	RTG_CMD_SET_PREFERRED_CLUSTER,
	RTG_CMD_SET_INVALID_INTERVAL,
	RTG_CMD_ID_MAX,
};

struct ffrt_rtg_msg {
	u32 cmd;
	s32 tgid;
	s64 data;
	u32 in_size;
	u32 out_size;
	void __user *in;
	void __user *out;
};

struct ffrt_rtg_load_info {
	u64 load;
	u64 runtime;
};

struct ffrt_rtg_id_map {
	DECLARE_BITMAP(id_map, FFRT_RTG_NUM);
	rwlock_t lock;
};

static struct ffrt_rtg_id_map g_ffrt_map = {
	.id_map = {0},
	.lock = __RW_LOCK_UNLOCKED(g_ffrt_map.lock)
};

typedef int(*ffrt_rtg_callback)(struct ffrt_rtg_msg *);

static ffrt_rtg_callback g_ffrt_rtg_callback[RTG_CMD_ID_MAX];

int proc_ffrt_open(struct inode *inode, struct file *filp)
{
	return RTG_SUCC;
}

long proc_ffrt_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	void __user *uarg = (void __user *)arg;
	struct ffrt_rtg_msg msg;

	if (_IOC_TYPE(cmd) != FFRT_RTG_IPC_MAGIC) {
		ffrt_rtg_err("Invalid Magic %d", _IOC_TYPE(cmd));
		return -RTG_INVALID_MAGIC;
	}

	if (uarg == NULL || copy_from_user(&msg, uarg, sizeof(msg))) {
		ffrt_rtg_err("Invalid Arg");
		return -RTG_INVALID_ARG;
	}

	if (msg.cmd >= RTG_CMD_ID_MAX) {
		ffrt_rtg_err("Invalid Cmd %d", msg.cmd);
		return -RTG_INVALID_CMD;
	}

	return g_ffrt_rtg_callback[msg.cmd](&msg);
}

#ifdef CONFIG_COMPAT
long proc_ffrt_compat_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	return proc_ffrt_ioctl(file, cmd, (unsigned long)(compat_ptr((compat_uptr_t)arg)));
}
#endif

bool is_ffrt_rtg(int id)
{
	return (id >= FFRT_RTG_ID) && (id < FFRT_RTG_ID + FFRT_RTG_NUM);
}

static int ffrt_set_group_id(pid_t pid, unsigned int group_id)
{
	struct task_struct *p = NULL;
	int ret;

	rcu_read_lock();
	p = find_task_by_vpid(pid);
	if (p == NULL) {
		rcu_read_unlock();
		ffrt_rtg_err("pid is invalid");
		return -RTG_FAILED;
	}

	/* Prevent p going away */
	get_task_struct(p);
	rcu_read_unlock();

	ret = _sched_set_group_id(p, group_id);
	put_task_struct(p);

	return ret;
}

static int sched_create_group(void)
{
	int map_id;
	int tgid = 0;

	write_lock(&g_ffrt_map.lock);
	map_id = find_next_zero_bit(g_ffrt_map.id_map, FFRT_RTG_NUM, 0);
	if (map_id >= FFRT_RTG_NUM) {
		write_unlock(&g_ffrt_map.lock);
		ffrt_rtg_err("All FFRT Group is full");
		return -RTG_FAILED;
	}

	set_bit(map_id, g_ffrt_map.id_map);
	tgid = map_id + FFRT_RTG_ID;
	write_unlock(&g_ffrt_map.lock);

	ffrt_rtg_info("Create RTG group successfully, group id is %d", tgid);
	return tgid;
}

static void sched_release_group(int id)
{
	int map_id = id - FFRT_RTG_ID;
	if (!is_ffrt_rtg(id)) {
		ffrt_rtg_err("RTG group id:%d not found", id);
		return;
	}

	read_lock(&g_ffrt_map.lock);
	if (!test_bit(map_id, g_ffrt_map.id_map)) {
		read_unlock(&g_ffrt_map.lock);
		ffrt_rtg_err("RTG group id:%d doesn't exist", id);
		return;
	}
	read_unlock(&g_ffrt_map.lock);

	write_lock(&g_ffrt_map.lock);
	clear_bit(map_id, g_ffrt_map.id_map);
	write_unlock(&g_ffrt_map.lock);
	ffrt_rtg_info("Release RTG group successfully, group id is %d", id);
}

static int ffrt_rtg_create(struct ffrt_rtg_msg *msg)
{
	int tgid;
	int ret;
	struct rtg_load_mode mode = {
		.freq_enabled = 1,
		.util_enabled = 1
	};

	ffrt_rtg_assert_cmd(msg->cmd, RTG_CMD_CREATE_RTG);

	if (msg->out == NULL || msg->out_size < sizeof(tgid)) {
		ffrt_rtg_err("Create RTG assert param 0x%x, %d failed", msg->out, msg->out_size);
		return -RTG_INVALID_ARG;
	}

	tgid = sched_create_group();
	if (tgid < 0) {
		ffrt_rtg_err("Create RTG failed %d", tgid);
		return -RTG_FAILED;
	}

	mode.grp_id = (unsigned int)tgid;
	ret = sched_set_group_load_mode(&mode);
	if (ret < 0) {
		ffrt_rtg_err("Create RTG set group load mode failed %d, %d", tgid, ret);
		return -RTG_FAILED;
	}

	if (copy_to_user(msg->out, &tgid, sizeof(tgid))) {
		ffrt_rtg_err("Create RTG pass tgid %d failed", tgid);
		return -RTG_FAILED;
	}

	return RTG_SUCC;
}

static int ffrt_rtg_release(struct ffrt_rtg_msg *msg)
{
	ffrt_rtg_assert_cmd(msg->cmd, RTG_CMD_RELEASE_RTG);

	sched_release_group(msg->tgid);
	return RTG_SUCC;
}

static int ffrt_rtg_add_thread(struct ffrt_rtg_msg *msg)
{
	pid_t tid = (int)(msg->data);
	int ret;

	ffrt_rtg_assert_cmd(msg->cmd, RTG_CMD_ADD_RTG_THREAD);

	ret = ffrt_set_group_id(tid, (unsigned int)(msg->tgid));
	if (ret < 0) {
		ffrt_rtg_err("ADD RTG Thread failed %d, %d, %d", msg->tgid, tid, ret);
		return -RTG_FAILED;
	}

	return RTG_SUCC;
}

static int ffrt_rtg_del_thread(struct ffrt_rtg_msg *msg)
{
	pid_t tid = (int)(msg->data);
	int ret;

	ffrt_rtg_assert_cmd(msg->cmd, RTG_CMD_DEL_RTG_THREAD);

	/* when tgid == 0 means to remove task from rtg */
	ret = ffrt_set_group_id(tid, 0);
	if (ret < 0) {
		ffrt_rtg_err("DEL RTG Thread failed %d, %d, %d", msg->tgid, tid, ret);
		return -RTG_FAILED;
	}

	return RTG_SUCC;
}

static int ffrt_rtg_set_group_util(struct ffrt_rtg_msg *msg)
{
	u64 util = (u64)(msg->data);
	int ret;

	ffrt_rtg_assert_cmd(msg->cmd, RTG_CMD_SET_GROUP_UTIL);

	/* set flag = 1 means update util now */
	ret = sched_set_group_normalized_util((unsigned int)(msg->tgid), util, 1);
	trace_perf(ffrt_rtg_update_tgid_util, (u64)(msg->tgid), util);
	if (ret < 0) {
		ffrt_rtg_err("Set RTG Util failed %d, %d, %d", msg->tgid, util, ret);
		return -RTG_FAILED;
	}

	return RTG_SUCC;
}

static int ffrt_rtg_set_group_freq(struct ffrt_rtg_msg *msg)
{
	u32 freq = (u32)(msg->data);
	int ret;

	ffrt_rtg_assert_cmd(msg->cmd, RTG_CMD_SET_GROUP_FREQ);

	ret = sched_set_group_freq((unsigned int)(msg->tgid), freq);
	if (ret < 0) {
		ffrt_rtg_err("Set RTG Freq failed %d, %d, %d", msg->tgid, freq, ret);
		return -RTG_FAILED;
	}

	return RTG_SUCC;
}

static int sched_get_thread_load(pid_t pid, struct ffrt_rtg_load_info *info)
{
	struct task_struct *p = NULL;

	rcu_read_lock();
	p = find_task_by_vpid(pid);
	if (p == NULL) {
		rcu_read_unlock();
		return -RTG_FAILED;
	}
	rcu_read_unlock();

	info->load = p->ravg.curr_window_load;
	info->runtime = p->ravg.curr_window_exec;

	return RTG_SUCC;
}

static int ffrt_rtg_get_thread_load(struct ffrt_rtg_msg *msg)
{
	pid_t tid = (int)(msg->data);
	struct ffrt_rtg_load_info info;
	int ret;

	ffrt_rtg_assert_cmd(msg->cmd, RTG_CMD_GET_THREAD_LOAD);

	if (msg->out == NULL || msg->out_size < sizeof(info)) {
		ffrt_rtg_err("Get Thread Load assert param 0x%x, %d failed", msg->out, msg->out_size);
		return -RTG_INVALID_ARG;
	}

	ret = sched_get_thread_load(tid, &info);
	if (ret < 0) {
		ffrt_rtg_err("Get Thread Load failed %d, %d", tid, ret);
		return -RTG_FAILED;
	}

	if (copy_to_user(msg->out, &info, sizeof(info))) {
		ffrt_rtg_err("Get Thread Load pass %d, %llu, %llu failed", tid, info.load, info.runtime);
		return -RTG_FAILED;
	}

	return RTG_SUCC;
}

static int sched_get_group_load(int tgid, struct ffrt_rtg_load_info *info)
{
	struct related_thread_group *grp = lookup_related_thread_group((unsigned int)tgid);
	if (grp == NULL)
		return -RTG_FAILED;

	if (list_empty(&grp->tasks)) {
		info->load = 0;
		info->runtime = 0;
	} else {
		info->load = grp->time.curr_window_load;
		info->runtime = grp->time.curr_window_exec;
	}

	return RTG_SUCC;
}

static int ffrt_rtg_get_group_load(struct ffrt_rtg_msg *msg)
{
	struct ffrt_rtg_load_info info;
	int ret;

	ffrt_rtg_assert_cmd(msg->cmd, RTG_CMD_GET_GROUP_LOAD);

	if (msg->out == NULL || msg->out_size < sizeof(info)) {
		ffrt_rtg_err("Get Load assert param 0x%x, %d failed", msg->out, msg->out_size);
		return -RTG_INVALID_ARG;
	}

	ret = sched_get_group_load(msg->tgid, &info);
	if (ret < 0) {
		ffrt_rtg_err("Get Load failed %d, %d", msg->tgid, ret);
		return -RTG_FAILED;
	}
	trace_perf(ffrt_rtg_update_group_load, (u64)(msg->tgid), info.load, info.runtime);
	if (copy_to_user(msg->out, &info, sizeof(info))) {
		ffrt_rtg_err("Get Load pass %d, %llu, %llu failed", msg->tgid, info.load, info.runtime);
		return -RTG_FAILED;
	}

	return RTG_SUCC;
}

static int ffrt_rtg_set_group_window_size(struct ffrt_rtg_msg *msg)
{
	int ret;

	ffrt_rtg_assert_cmd(msg->cmd, RTG_CMD_SET_GROUP_WINDOW_SIZE);

	ret = sched_set_group_window_size(msg->tgid, NSEC_PER_SEC / msg->data);
	if (ret < 0) {
		ffrt_rtg_err("Set Group Window Size failed %d, %llu, %d", msg->tgid, msg->data, ret);
		return -RTG_FAILED;
	}

	return RTG_SUCC;
}

static int ffrt_rtg_set_group_window_rollover(struct ffrt_rtg_msg *msg)
{
	int ret;

	ffrt_rtg_assert_cmd(msg->cmd, RTG_CMD_SET_GROUP_WINDOW_ROLLOVER);

	ret = sched_set_group_window_rollover((unsigned int)(msg->tgid));
	if (ret < 0) {
		ffrt_rtg_err("Set Group Window Rollover failed %d, %d", msg->tgid, ret);
		return -RTG_FAILED;
	}

	return RTG_SUCC;
}

static int ffrt_rtg_set_preferred_cluster(struct ffrt_rtg_msg *msg)
{
	int cluster_id = (int)(msg->data);
	int ret;

	ffrt_rtg_assert_cmd(msg->cmd, RTG_CMD_SET_PREFERRED_CLUSTER);

	ret = sched_set_preferred_cluster((unsigned int)(msg->tgid), cluster_id);
	if (ret < 0) {
		ffrt_rtg_err("Set Preferred Cluster failed %d, %d", msg->tgid, ret);
		return -RTG_FAILED;
	}

	return RTG_SUCC;
}

static int ffrt_rtg_set_invalid_interval(struct ffrt_rtg_msg *msg)
{
	int ret;

	ffrt_rtg_assert_cmd(msg->cmd, RTG_CMD_SET_INVALID_INTERVAL);

	ret = sched_set_group_util_invalid_interval((unsigned int)(msg->tgid), (unsigned int)(msg->data));
	if (ret < 0) {
		ffrt_rtg_err("Set Invalid Interval failed %d, %d", msg->tgid, ret);
		return -RTG_FAILED;
	}

	return RTG_SUCC;
}

static ffrt_rtg_callback g_ffrt_rtg_callback[RTG_CMD_ID_MAX] = {
	ffrt_rtg_create,
	ffrt_rtg_release,
	ffrt_rtg_add_thread,
	ffrt_rtg_del_thread,
	ffrt_rtg_set_group_util,
	ffrt_rtg_set_group_freq,
	ffrt_rtg_get_thread_load,
	ffrt_rtg_get_group_load,
	ffrt_rtg_set_group_window_size,
	ffrt_rtg_set_group_window_rollover,
	ffrt_rtg_set_preferred_cluster,
	ffrt_rtg_set_invalid_interval,
};