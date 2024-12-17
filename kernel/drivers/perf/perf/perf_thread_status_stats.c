#include "perf_thread_status_stats.h"

#include <linux/ioctl.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/net.h>
#include <linux/profile.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/sched/stat.h>
#include <linux/cpufreq_times.h>
#include <linux/sched.h>
#include <../../../kernel/sched/sched.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
#include <linux/sched.h>
#include <linux/sched/cputime.h>
#include <linux/sched/signal.h>
#endif

#include "perf_ioctl.h"
#include "utils/perf_buffered_log_sender.h"
#include "utils/perf_utils.h"
#include "securec.h"

#define IOC_GET_THREAD_STATUS _IOW(IOC_THREAD_STATUS, 1, int)

struct perf_thread_status_info {
	int pid;
	int prio;
	unsigned long long running_count;
	unsigned long long runnable_count;
	unsigned long long block_count;
	unsigned long long io_block_count;
	unsigned long long sleep_count;
	char cpuset[PATH_MAX];
	unsigned long long cpu_time[0];
};

static void perf_thread_status_get_proc_time(struct task_struct *task,
	struct perf_thread_status_info *info)
{
	/* Memory is limited. Stub first. */
	/* running */
	info->running_count = 0;
	/* runnable */
	info->runnable_count = 0;
	/* uninterrupt non-io */
	info->block_count = 0;
	/* uninterrupt */
	info->io_block_count = 0;
	/* Sleeping */
	info->sleep_count = 0;
}

static int perf_thread_status_get_info(struct perf_buffered_log_entry *entry,
	int key_task_id, int cpu_core_num)
{
	struct task_struct *task;
	int ret;
	struct perf_thread_status_info *info;

	info = (struct perf_thread_status_info *)entry->data;

	rcu_read_lock();
	task = perf_get_task_by_pid(key_task_id);
	if (task == NULL) {
		rcu_read_unlock();
		pr_err("PerfD %s: get task by pid failed, pid: %d\n", __func__, key_task_id);
		return -EFAULT;
	}

	info->pid = task->pid;
	info->prio = task->prio;
	perf_thread_status_get_proc_time(task, info);
	perf_get_cpu_time(task, info->cpu_time);

	ret = perf_get_cpuset(key_task_id, task, info->cpuset);
	if (ret != 0) {
		rcu_read_unlock();
		pr_err("PerfD %s: perf thread status get cpuset failed, ret %d\n",
			__func__, ret);
		return ret;
	}

	rcu_read_unlock();

	return 0;
}

static int perf_thread_status_fetch(void __user *argp)
{
	int ret = -EINVAL;
	unsigned int entry_size;
	int key_task_id;
	int cpu_core_num;
	struct perf_buffered_log_entry *entry = NULL;

	/* accept key pid from hiview */
	if (copy_from_user(&key_task_id, (void*)argp, sizeof(int))) {
		pr_err("PerfD %s: failed to copy_from_user\n", __func__);
		return -EFAULT;
	}

	cpu_core_num = perf_get_cpu_num();

	entry_size = sizeof(struct perf_buffered_log_entry) +
		sizeof(struct perf_thread_status_info) +
		cpu_core_num * sizeof(unsigned long long);
	entry = perf_create_log_entry(entry_size, IOC_THREAD_STATUS, 1);
	if (entry == NULL) {
		pr_err("PerfD %s: failed to create log entry\n", __func__);
		goto error;
	}

	if (perf_thread_status_get_info(entry, key_task_id, cpu_core_num)) {
		pr_err("PerfD %s: failed to get thread status\n", __func__);
		goto error;
	}

	ret = send_perf_buffered_log_entry(entry);
	if (ret < 0) {
		pr_err("PerfD %s: failed to send log entry, ret %d\n", __func__, ret);
		goto error;
	}

error:
	free_perf_buffered_log_entry(entry);
	return ret;
}

long perf_ioctl_thread_status(unsigned int cmd, void __user *argp)
{
	int ret = 0;

	switch (cmd) {
	case  IOC_GET_THREAD_STATUS:
		ret = perf_thread_status_fetch(argp);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}
