/*
 * hungtask_user.c
 *
 * Hungtask Module Used For Detect User Process
 *
 * Copyright (c) 2018-2021 Huawei Technologies Co., Ltd.
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

#define pr_fmt(fmt)	"hungtask_userlist " fmt

#include <linux/cred.h>
#if !defined(KERNEL_VERSION) || !defined(LINUX_VERSION_CODE)
#include <linux/sched/debug.h>
#else
#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
#include <linux/sched/debug.h>
#endif
#endif
#include <linux/profile.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/signal.h>
#include <linux/version.h>
#ifdef CONFIG_HW_EAS_SCHED
#include <linux/hw/eas_hw.h>
#endif
#include <platform/linux/zrhung.h>

#include "securec.h"
#include "hungtask_base.h"

#define CMD_MIN_LEN 3
#define CMD_MAX_LEN 20
#define PID_INIT 1
#define PID_INVALID (-1)
#define SIG_TO_INIT 40
#define SIG_INT_VALUE 1234
#define USERLIST_NUM 10
#define STATUS_USERLIST_OFF 0
#define STATUS_USERLIST_ON  1
#ifdef CONFIG_HLTHERM_RUNTEST
#define MAX_USER_TIMEOUT 300
#else
#define MAX_USER_TIMEOUT 120
#endif
#define MAX_SHOW_LEN 512
#define MAX_BLOCK_MSG_LEN 50

struct user_item {
	pid_t pid;
	pid_t tgid;
	int cur_cnt;
	int panic_cnt;
};

static struct user_item userlist[USERLIST_NUM];
static int userlist_count;
static DEFINE_SPINLOCK(userlist_lock);
static bool is_registered;
static bool need_panic;
static bool need_dump;
static bool is_init;
static bool is_kernel_stuck;
static int block_time;
static int block_pid;
static char block_msg[MAX_SHOW_LEN];

static void send_signal_to_user(int pid)
{
	int ret;
	struct task_struct *t = NULL;
#if !defined(KERNEL_VERSION) || !defined(LINUX_VERSION_CODE)
	struct siginfo info = {0};
#else
#if (KERNEL_VERSION(5, 4, 0) > LINUX_VERSION_CODE)
	struct siginfo info = {0};
#else
	kernel_siginfo_t info;

	clear_siginfo(&info);
#endif
#endif

	info.si_signo = SIG_TO_INIT;
	info.si_code = SI_QUEUE;
	info.si_int = SIG_INT_VALUE;
	rcu_read_lock();
	t = find_task_by_vpid(pid);
	if (t)
		get_task_struct(t);
	rcu_read_unlock();
	if (!t) {
		pr_err("no pid is %d\n", pid);
		return;
	}

	ret = send_sig_info(SIG_TO_INIT, &info, t);
	put_task_struct(t);
	if (ret < 0)
		pr_err("send signal to %d error\n", pid);
	else
		pr_err("send signal to %d success\n", pid);
}

static void htuser_show_task(int pid)
{
	struct task_struct *p = NULL;

	rcu_read_lock();
	p = pid_task(find_vpid(pid), PIDTYPE_PID);
	if (p)
		get_task_struct(p);
	rcu_read_unlock();
	if (!p) {
		pr_err("can not find pid %d\n", pid);
		return;
	}

	if (p->flags & PF_FROZEN) {
		put_task_struct(p);
		pr_info("process %d is frozen\n", pid);
		return;
	}

	pr_err("InitProcessWatchdog_KernelStack start\n");
	sched_show_task(p);
	pr_err("InitProcessWatchdog_KernelStack end\n");

	if ((p->state == TASK_UNINTERRUPTIBLE || p->state == TASK_KILLABLE) && (pid == PID_INIT))
		is_kernel_stuck = true;
	put_task_struct(p);
}

static void htuser_list_insert(int pid, int tgid, int count)
{
	int i;

	spin_lock(&userlist_lock);
	if (userlist_count >= USERLIST_NUM) {
		pr_err("list is full\n");
		goto out;
	}
	for (i = 0; i < userlist_count; i++) {
		if (userlist[i].pid == pid) {
			pr_err("pid is already exist\n");
			goto out;
		}
	}
	userlist[userlist_count].pid = pid;
	userlist[userlist_count].tgid = tgid;
	userlist[userlist_count].cur_cnt = 0;
	userlist[userlist_count].panic_cnt = count;
	userlist_count++;
out:
	spin_unlock(&userlist_lock);
}

static int htuser_list_remove(int pid, int tgid)
{
	int i;
	int j = 0;
	int ret;

	spin_lock(&userlist_lock);
	for (i = 0; i < userlist_count; i++) {
		if (userlist[i].pid == pid || userlist[i].tgid == tgid)
			continue;
		if (i != j)
			userlist[j] = userlist[i];
		j++;
	}
	ret = (j < userlist_count) ? 0 : -1;
	for (i = j; i < userlist_count; i++)
		memset(&userlist[i], 0, sizeof(userlist[i]));
	userlist_count = j;
	spin_unlock(&userlist_lock);
	return ret;
}

static void htuser_list_update(void)
{
	int i;

	need_panic = false;
	need_dump = false;
	is_init = false;
	is_kernel_stuck = false;
	spin_lock(&userlist_lock);
	for (i = 0; i < userlist_count; i++) {
		userlist[i].cur_cnt++;
		if ((userlist[i].cur_cnt >= userlist[i].panic_cnt) ||
		    (userlist[i].cur_cnt == userlist[i].panic_cnt / 2)) {
			htuser_show_task(userlist[i].pid);
			pr_err("process %d not scheduled for %ds\n",
			       userlist[i].pid,
			       userlist[i].cur_cnt * HEARTBEAT_TIME);
			if (userlist[i].pid == PID_INIT)
				is_init = true;
		}
		if (!(userlist[i].cur_cnt % userlist[i].panic_cnt)) {
			need_dump = true;
			block_time = userlist[i].cur_cnt * HEARTBEAT_TIME;
			block_pid = userlist[i].pid;
#ifndef CONFIG_HLTHERM_RUNTEST
			if (userlist[i].pid != PID_INIT)
#endif
				need_panic = true;
		}
	}
	spin_unlock(&userlist_lock);
}

static void htuser_list_kick(int pid)
{
	int i;

	spin_lock(&userlist_lock);
	for (i = 0; i < userlist_count; i++) {
		if (userlist[i].pid == pid) {
			userlist[i].cur_cnt = 0;
			spin_unlock(&userlist_lock);
			return;
		}
	}
	spin_unlock(&userlist_lock);
}

void htuser_post_process_userlist(void)
{
	htuser_list_update();
	if (is_init && !is_kernel_stuck && need_dump) {
		send_signal_to_user(PID_INIT);
		pr_err("init process blocked for long time\n");
		if (snprintf_s(block_msg, MAX_BLOCK_MSG_LEN,
			       MAX_BLOCK_MSG_LEN - 1,
			       "Init process blocked for %d",
			       block_time) <= 0)
			pr_err("failed to snprintf block_msg\n");
		zrhung_send_event(ZRHUNG_WP_INIT,
				  "R=InitProcessWatchdog_UserStack",
				  block_msg);
	}
	if (need_dump) {
		pr_err("print all cpu stack and D state stack\n");
		hungtask_show_state_filter(TASK_UNINTERRUPTIBLE);
#if defined(CONFIG_HW_EAS_SCHED) || defined(CONFIG_HISI_EAS_SCHED)
		print_cpu_rq_info();
#endif
	}
	if (need_panic)
		panic("UserList Process %d blocked for %ds causing panic",
		      block_pid,
		      block_time);
}

static int htuser_process_notifier(struct notifier_block *self,
				   unsigned long cmd, void *v)
{
	struct task_struct *task = v;

	if (task == NULL)
		return NOTIFY_OK;

	if (task->tgid == task->pid) {
		if (!htuser_list_remove(PID_INVALID, task->tgid))
			pr_err("remove success due to process %d die\n",
			       task->tgid);
	}

	return NOTIFY_OK;
}

static struct notifier_block htuser_process_notify = {
	.notifier_call = htuser_process_notifier,
};

ssize_t htuser_list_show(struct kobject *kobj,
			 struct kobj_attribute *attr, char *buf)
{
	int i;
	char tmp[MAX_SHOW_LEN] = {0};
	int len = 0;
	int ret;
	int left;

	left = MAX_SHOW_LEN - len;
	ret = snprintf_s(tmp + len, left, left >= 1 ? left - 1 : 0,
			 "   Pid  Tgid  Current(sec)  Expired(sec)\n");
	if (ret > 0)
		len += ret;
	spin_lock(&userlist_lock);
	for (i = 0; i < userlist_count; i++) {
		left = MAX_SHOW_LEN - len;
		ret = snprintf_s(tmp + len, left, left >= 1 ? left - 1 : 0,
				 "%5d  %5d   %5d      %5d\n",
				 userlist[i].pid, userlist[i].tgid,
				 userlist[i].cur_cnt * HEARTBEAT_TIME,
				 userlist[i].panic_cnt * HEARTBEAT_TIME);
		if (ret > 0)
			len += ret;
	}
	spin_unlock(&userlist_lock);
	pr_info("%s\n", tmp);
	if (strncpy_s(buf, len + 1, tmp, len) != EOK) {
		pr_err("%s, failed to copy userlist", __func__);
		return 0;
	}

	return len;
}

static int htuser_list_store_on(char *tmp, size_t len, int pid, int tgid)
{
	unsigned long sec = 0;

	if (kstrtoul(tmp + 3, 10, &sec)) {
		pr_err("invalid timeout value\n");
		return -EINVAL;
	}

#ifdef CONFIG_HLTHERM_RUNTEST
	if (pid == PID_INIT)
		sec = sec * 10;
#endif
	if ((sec > MAX_USER_TIMEOUT) || sec < HEARTBEAT_TIME) {
		pr_err("invalid timeout value, should be in %d-%d\n",
		       HEARTBEAT_TIME, MAX_USER_TIMEOUT);
		return -EINVAL;
	}
	if (sec % HEARTBEAT_TIME) {
		pr_err("invalid timeout value, should be devided by %d\n",
		       HEARTBEAT_TIME);
		return -EINVAL;
	}
	pr_info("process %d set to enable, timeout=%d\n", pid, sec);
	htuser_list_insert(pid, tgid, sec / HEARTBEAT_TIME);
	if (!is_registered) {
		profile_event_register(PROFILE_TASK_EXIT,
				       &htuser_process_notify);
		is_registered = true;
	}

	return 0;
}

ssize_t htuser_list_store(struct kobject *kobj,
			  struct kobj_attribute *attr,
			  const char *buf, size_t count)
{
	char tmp[CMD_MAX_LEN];
	size_t len;
	char *p = NULL;
	int pid = current->pid;
	int tgid = current->tgid;
	int uid = current->cred->euid.val;

	if (uid >= 10000)
		pr_err("non-system process %d(uid=%d) can not be added to hungtask userlist\n",
		       pid, uid);
	if ((count < CMD_MIN_LEN) || (count >= CMD_MAX_LEN)) {
		pr_err("string too long or too short\n");
		return -EINVAL;
	}
	if (!buf)
		return -EINVAL;

	memset(tmp, 0, sizeof(tmp));
	p = memchr(buf, '\n', count);
	len = p ? p - buf : count;
	if (strncpy_s(tmp, CMD_MAX_LEN, buf, len) != EOK)
		pr_err("%s, failed to copy userlist\n", __func__);
	if (strncmp(tmp, "on,", CMD_MIN_LEN) == 0) {
		if (htuser_list_store_on(tmp, len, pid, tgid))
			return -EINVAL;
	} else if (unlikely(strncmp(tmp, "off", CMD_MIN_LEN) == 0)) {
		pr_info("process %d set to disable\n", pid);
		if (!htuser_list_remove(pid, PID_INVALID))
			pr_err("remove success due to process %d call off\n",
			       pid);
	} else if (likely(strncmp(tmp, "kick", CMD_MIN_LEN) == 0)) {
		pr_info("process %d kicked\n", pid);
		htuser_list_kick(pid);
	} else {
		pr_err("only accept 'on,xx/off/kick'\n");
	}

	return (ssize_t) count;
}
