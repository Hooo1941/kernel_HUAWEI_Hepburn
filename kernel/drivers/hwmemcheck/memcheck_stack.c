/*
 * memcheck_stack.c
 *
 * save and read stack information
 *
 * Copyright (c) 2021-2022 Huawei Technologies Co., Ltd.
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

#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <linux/thread_info.h>
#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
#include <linux/sched/signal.h>
#else
#include <linux/sched.h>
#endif
#include "memcheck_stack.h"
#include "memcheck_ioctl.h"

/* for stack information save and read */
#define STACK_WAIT_TIME_SEC	5
#define STACK_NUM		2
#define ADDR_NUM		2

static DEFINE_MUTEX(stack_mutex);
static void *stack_buf[STACK_NUM];
static u64 stack_len[STACK_NUM];
static wait_queue_head_t stack_ready;
static bool is_waiting[STACK_NUM];
static u64 addr_array[][ADDR_NUM] = {
	/* MEMCMD_NONE */
	{ 0, 0 },
	/* MEMCMD_ENABLE */
	{ ADDR_JAVA_ENABLE, ADDR_NATIVE_ENABLE },
	/* MEMCMD_DISABLE */
	{ ADDR_JAVA_DISABLE, ADDR_NATIVE_DISABLE },
	/* MEMCMD_SAVE_LOG */
	{ ADDR_JAVA_SAVE, ADDR_NATIVE_SAVE },
	/* MEMCMD_CLEAR_LOG */
	{ ADDR_JAVA_CLEAR, ADDR_NATIVE_CLEAR },
};

int memcheck_stack_read(void *buf, struct stack_info *info)
{
	int ret = -EFAULT;
	size_t len;
	size_t java_len = 0;
	size_t total_len = 0;
	int idx;

	mutex_lock(&stack_mutex);
	for (idx = 0; idx < STACK_NUM; idx++) {
		if (!stack_buf[idx])
			continue;
		len = min(stack_len[idx], info->size - total_len);
		if (copy_to_user
		    (buf + sizeof(*info) + java_len, stack_buf[idx], len)) {
			memcheck_err("copy_to_user failed\n");
			goto unlock;
		}
		if (info->type & MTYPE_JAVA)
			java_len = len;
		memcheck_info("read idx=%d,len=%llu\n", idx, len);
		total_len += len;
		if (total_len >= info->size)
			break;
	}
	if (total_len != info->size) {
		info->size = total_len;
		if (copy_to_user(buf, info, sizeof(*info))) {
			memcheck_err("copy_to_user failed\n");
			goto unlock;
		}
	}

	ret = 0;

unlock:
	mutex_unlock(&stack_mutex);

	return ret;
}

int memcheck_stack_clear(void)
{
	int idx;

	mutex_lock(&stack_mutex);
	for (idx = 0; idx < STACK_NUM; idx++) {
		if (stack_buf[idx]) {
			vfree(stack_buf[idx]);
			stack_buf[idx] = NULL;
			stack_len[idx] = 0;
		}
	}
	mutex_unlock(&stack_mutex);

	return 0;
}

int memcheck_stack_write(const void *buf, const struct stack_info *info)
{
	char *tmp = NULL;
	int idx;

	tmp = vzalloc(info->size + 1);
	if (!tmp)
		return -EFAULT;
	if (copy_from_user(tmp, buf + sizeof(*info), info->size)) {
		vfree(tmp);
		memcheck_err("copy_from_user failed\n");
		return -EFAULT;
	}
	tmp[info->size] = 0;

	idx = (info->type & MTYPE_JAVA) ? IDX_JAVA : IDX_NATIVE;
	mutex_lock(&stack_mutex);
	if (stack_buf[idx])
		vfree(stack_buf[idx]);
	stack_buf[idx] = tmp;
	stack_len[idx] = info->size;
	mutex_unlock(&stack_mutex);

	if (is_waiting[idx])
		wake_up_interruptible(&stack_ready);

	return 0;
}

static int memcheck_check_wait_result(int left, bool is_java, bool is_native)
{
	if (!left) {
		if (is_java && (!is_native))
			memcheck_err("wait for java stack timeout\n");
		else if ((!is_java) && is_native)
			memcheck_err("wait for native stack timeout\n");
		else if (is_java && is_native)
			memcheck_err("wait for java and native timeout\n");
		return -ETIMEDOUT;
	} else if (left < 0) {
		if (is_java && (!is_native))
			memcheck_err("wait for java stack return %d\n", left);
		else if ((!is_java) && is_native)
			memcheck_err("wait for native stack return %d\n",
				     left);
		else if (is_java && is_native)
			memcheck_err("wait for java and native return %d\n",
				     left);
		return -EFAULT;
	}

	return 0;
}

int memcheck_wait_stack_ready(u16 type)
{
	int left;
	bool is_java = (type & MTYPE_JAVA) ? true : false;
	bool is_native = (type & MTYPE_NATIVE) ? true : false;
	int index;
	int ret = 0;

	index = is_java ? IDX_JAVA : IDX_NATIVE;

	mutex_lock(&stack_mutex);
	is_waiting[index] = true;
	init_waitqueue_head(&stack_ready);
	mutex_unlock(&stack_mutex);
	left = wait_event_interruptible_timeout(stack_ready,
					     stack_buf[index],
					     STACK_WAIT_TIME_SEC * HZ);
	mutex_lock(&stack_mutex);
	if (stack_buf[index])
		memcheck_info("get %s stack successfully\n",
			      is_java ? "java" : "native");
	else
		ret = memcheck_check_wait_result(left, is_java, is_native);
	is_waiting[index] = false;
	mutex_unlock(&stack_mutex);

	return ret;
}

static bool process_disappear(u64 t, const struct track_cmd *cmd)
{
	if (cmd->cmd == MEMCMD_ENABLE)
		return false;
	if (cmd->timestamp != nsec_to_clock_t(t))
		return true;

	return false;
}

#ifdef CONFIG_DFX_LIBLINUX
int memcheck_do_command(const struct track_cmd *cmd)
{
	int ret = 0;
	u64 addr = 0;
	bool is_java = (cmd->type & MTYPE_JAVA) ? true : false;
	bool is_native = (cmd->type & MTYPE_NATIVE) ? true : false;
	struct kernel_siginfo info;

	clear_siginfo(&info);

	if (is_java == is_native) {
		memcheck_err("invalid type=%d\n", cmd->type);
		return -EFAULT;
	}

	info.si_signo = SIGNO_MEMCHECK;
	info.si_code = SI_TKILL;

#ifdef CONFIG_DFX_OHOS
	if (cmd->cmd != MEMCMD_JS_HEAP_DUMP) {
		addr |= addr_array[cmd->cmd][IDX_NATIVE];
		info.si_addr = (void *)addr;
		ret = liblinux_send_memcheck_signal(cmd->id, SIGNO_MEMCHECK, &info, cmd->timestamp);
		memcheck_info("send native SIGNO_MEMCHECK, addr is: %d, ret is: %d\n", addr, ret);
	}
	if (is_java && (cmd->cmd == MEMCMD_JS_HEAP_DUMP)) {
		info.si_signo = MUSL_SIGNAL_JSHEAP_PRIV;
		ret = liblinux_send_memcheck_signal(cmd->id, MUSL_SIGNAL_JSHEAP_PRIV, &info, cmd->timestamp);
		memcheck_info("send js MUSL_SIGNAL_JSHEAP_PRIV, ret is: %d\n", ret);
	}
#else
	if (is_java)
		addr = addr_array[cmd->cmd][IDX_JAVA];
	if (is_native)
		addr |= addr_array[cmd->cmd][IDX_NATIVE];
	info.si_addr = (void *)addr;
	if (is_java || is_native)
		ret = liblinux_send_memcheck_signal(cmd->id, SIGNO_MEMCHECK, &info, cmd->timestamp);
#endif

	if ((!ret) && (cmd->cmd == MEMCMD_SAVE_LOG))
		memcheck_wait_stack_ready(cmd->type);
	else if ((!ret) && (cmd->cmd == MEMCMD_CLEAR_LOG))
		memcheck_stack_clear();

	return ret;
}
#else
int memcheck_do_command(const struct track_cmd *cmd)
{
	int ret = 0;
	struct task_struct *p = NULL;
	u64 addr = 0;
	bool is_java = (cmd->type & MTYPE_JAVA) ? true : false;
	bool is_native = (cmd->type & MTYPE_NATIVE) ? true : false;
#if (KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE)
	kernel_siginfo_t info;

	clear_siginfo(&info);
#else
	struct siginfo info;

	memset(&info, 0, sizeof(info));
#endif

	if (is_java == is_native) {
		memcheck_err("invalid type=%d\n", cmd->type);
		return -EFAULT;
	}
	info.si_signo = SIGNO_MEMCHECK;
	info.si_errno = 0;
	info.si_code = SI_TKILL;
	info.si_pid = task_tgid_vnr(current);
	info.si_uid = from_kuid_munged(current_user_ns(), current_uid());

	rcu_read_lock();
	p = find_task_by_vpid(cmd->id);
	if (p)
		get_task_struct(p);
	rcu_read_unlock();

	if (p && (task_tgid_vnr(p) == cmd->id)) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
		if (process_disappear(p->real_start_time, cmd)) {
#else
		if (process_disappear(p->start_boottime, cmd)) {
#endif
			memcheck_err("pid %d disappear\n", cmd->id);
			ret = MEMCHECK_PID_INVALID;
			goto err_pid_disappear;
		}

#ifdef CONFIG_DFX_OHOS
		if (cmd->cmd != MEMCMD_JS_HEAP_DUMP) {
			addr |= addr_array[cmd->cmd][IDX_NATIVE];
			info.si_addr = (void *)addr;
			ret = do_send_sig_info(SIGNO_MEMCHECK, &info, p, false);
			memcheck_info("send native SIGNO_MEMCHECK, addr is: %d, ret is: %d\n", addr, ret);
		}
		if (is_java && (cmd->cmd == MEMCMD_JS_HEAP_DUMP)) {
			info.si_signo = MUSL_SIGNAL_JSHEAP_PRIV;
			ret = do_send_sig_info(MUSL_SIGNAL_JSHEAP_PRIV, &info, p, false);
			memcheck_info("send js MUSL_SIGNAL_JSHEAP_PRIV, ret is: %d\n", ret);
		}
#else
		if (is_java)
			addr = addr_array[cmd->cmd][IDX_JAVA];
		if (is_native)
			addr |= addr_array[cmd->cmd][IDX_NATIVE];
		info.si_addr = (void *)addr;
		if (is_java || is_native)
			ret = do_send_sig_info(SIGNO_MEMCHECK, &info, p, false);
#endif
	}

err_pid_disappear:
	if (p)
		put_task_struct(p);
	if ((!ret) && (cmd->cmd == MEMCMD_SAVE_LOG))
		memcheck_wait_stack_ready(cmd->type);
	else if ((!ret) && (cmd->cmd == MEMCMD_CLEAR_LOG))
		memcheck_stack_clear();

	return ret;
}
#endif