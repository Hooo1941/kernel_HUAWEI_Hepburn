#include "perf_process_cputime_stats.h"

#include <linux/cpufreq_times.h>
#include <linux/jiffies.h>
#include <linux/ioctl.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/net.h>
#include <linux/profile.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/sched/stat.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
#include <linux/sched.h>
#include <linux/sched/cputime.h>
#include <linux/sched/signal.h>
#endif
#ifdef CONFIG_SMT_MODE_GOV
#include <platform_include/cee/linux/time_in_state.h>
#endif
#include "perf_ioctl.h"
#include "utils/perf_buffered_log_sender.h"
#include "securec.h"
#include "utils/perf_utils.h"

#define SET_MSG_MAX_NUM 20

struct proc_cputime_set_msg {
	int count;
	int data[SET_MSG_MAX_NUM];
};

/*
 * The uploaded data structure is:
 *	int pid;
 *	int prio;
 *	unsigned int cpu_usage_time;
 *	unsigned long cpu_load_time;
 *	unsigned int cpuset_len;
 *	char *cpuset;
 *	unsigned long long cpu_time[];
 * Because we do not know the length of the uploaded cpuset string,
 * no cpu_time variable is set in the structure perf_process_cputime_info,
 * but directly follows the cpuset string.
 */
struct perf_process_cputime_info {
	int pid;
	int prio;
	int oon_score_adj;
	unsigned int cpuset_len;
	unsigned long long cpu_usage_time;
	unsigned long long cpu_load_time;
	char cpuset[0];
};

#define INIT_PID 1
#define KTHREADD_PID 2
#define DMIPS_NUM 16

#ifdef CONFIG_SMT_MODE_GOV
#define SMT_CPU_EXPANSION_MULTIPLIER 2
#endif

#define IOC_GET_PROCESS_CPUTIME _IO(IOC_PROCESS_CPUTIME, 1)
#define IOC_SET_CPU_DIMPS _IOW(IOC_PROCESS_CPUTIME, 1, struct proc_cputime_set_msg)

static int dmips_value_buffer[DMIPS_NUM];

static int perf_get_per_core_max_freq(unsigned int *max_freq, unsigned int cpu)
{
	struct cpufreq_policy *cpu_policy;

	cpu_policy = cpufreq_cpu_get(cpu);
	if (!cpu_policy)
		return -EINVAL;

	*max_freq = cpu_policy->max;

	return 0;
}

static unsigned long long get_proc_mt_max_system_cpuload(
	int dmips_value_buffer[], unsigned int dmips_num)
{
	unsigned long long proc_mt_cpuload_total = 0;
#ifdef CONFIG_SMT_MODE_GOV
	unsigned int cpu = 0;
	if (dmips_value_buffer == NULL) {
		pr_err("PerfD %s: get mt cpuload para err, dmips_buffer: %p\n",
			__func__, dmips_value_buffer);
		return proc_mt_cpuload_total;
	}

	for_each_possible_cpu(cpu) {
		if (cpu > dmips_num)
			break;

		unsigned int max_freq = 0;
		int ret = perf_get_per_core_max_freq(&max_freq, cpu);
		if (ret != 0) {
			pr_err("PerfD %s: get core max freq failed, ret %d\n",
				__func__, ret);
			continue;
		}

		proc_mt_cpuload_total +=
			(unsigned long long)max_freq * get_mt_time_by_cpu(cpu) *
			dmips_value_buffer[cpu];
	}
	proc_mt_cpuload_total /= SMT_CPU_EXPANSION_MULTIPLIER;
#endif
	return proc_mt_cpuload_total;
}

static unsigned long long get_process_load_cputime(struct task_struct *task)
{
	unsigned long long proc_cputime_total;

	if (task->tgid == INIT_PID || task->tgid == KTHREADD_PID)
		return 0;
	if (task->real_parent && task->real_parent->tgid == KTHREADD_PID &&
		!strstr(task->comm, "swapd"))
		return 0;
	proc_cputime_total = get_proc_cpu_load(task, dmips_value_buffer,
		DMIPS_NUM);

	return proc_cputime_total;
}

static unsigned long long get_process_usage_cputime(struct task_struct *task)
{
	unsigned long long utime, stime, sum_time;
	unsigned long long cputime;

	thread_group_cputime_adjusted(task, &utime, &stime);
	sum_time = utime + stime + task->signal->cutime + task->signal->cstime;
	cputime = (unsigned int)(sum_time / NS_TO_MS);

	return cputime;
}

static int perf_get_per_proc_cpuset(struct task_struct *task,
	struct perf_process_cputime_info *info, int length)
{
	int ret;
	char cpuset[PATH_MAX] = {0};
	int len_temp;

	ret = perf_get_cpuset(task->pid, task, cpuset);
	if (ret != 0) {
		pr_err("PerfD %s: get cpuset failed, ret %d\n",
			__func__, ret);
		return -EINVAL;
	}

	len_temp = strlen(cpuset) + 1;
	if (len_temp + length >= MAX_PERF_LOG_LENGTH) {
		pr_err("PerfD %s: get cpuset memory overflow, strlen %d\n",
			__func__, len_temp);
		return 0;
	}

	ret = strcpy_s(info->cpuset, len_temp, cpuset);
	if (ret != 0) {
		pr_err("PerfD %s: cpuset strcpy_s failed, pid %d, ret %d\n",
			__func__, info->pid, ret);
		return -EINVAL;
	}
	info->cpuset_len = len_temp;

	return info->cpuset_len;
}

/*
 * return val:
 *	> 0  : valid data len
 *	== 0 : Data is obtained successfully, but the memory usage exceeds
 *	       the threshold. Do not save the current data.
 *	< 0  : Failed to obtain data. Exit data processing.
 */
static int perf_get_per_proc_cpu_stat(struct task_struct *task,
	void *data, int cpu_num, int length)
{
	struct perf_process_cputime_info *info;
	unsigned long long *cpu_time;
	int len;
	int len_temp;

	info = (struct perf_process_cputime_info *)data;
	info->pid = task->pid;
	info->prio = task->prio;
	if (task->signal != NULL)
		info->oon_score_adj = task->signal->oom_score_adj;
	info->cpu_load_time = get_process_load_cputime(task);
	info->cpu_usage_time = get_process_usage_cputime(task);
	len = sizeof(struct perf_process_cputime_info);

	len_temp = perf_get_per_proc_cpuset(task, info, length);
	if (len_temp <= 0) {
		pr_err("PerfD %s: get per proc cpuset failed, ret %d\n",
			__func__, len_temp);
		return len_temp;
	}
	len += len_temp;

	len_temp = cpu_num * sizeof(unsigned long long);
	if (len_temp + length >= MAX_PERF_LOG_LENGTH) {
		pr_err("PerfD %s: get cputime memory overflow, strlen %d\n",
			__func__, len_temp);
		return 0;
	}
	cpu_time = (unsigned long long *)(data + len);
	(void)memset_s(cpu_time, cpu_num * sizeof(unsigned long long), 0, cpu_num * sizeof(unsigned long long));
	perf_get_cpu_time(task, cpu_time);
	len += len_temp;

	return len;
}

static int get_perf_process_cputime_stats(
	struct perf_buffered_log_entry *entry, int cpu_num)
{
	int max_count;
	struct task_struct *task;
	int ret_len;
	unsigned char *data = entry->data;

	max_count = entry->count;
	entry->count = 0;
	entry->length = sizeof(struct perf_buffered_log_entry);
	*((unsigned long long *)data) =
		get_proc_mt_max_system_cpuload(dmips_value_buffer, DMIPS_NUM);
	data += sizeof(unsigned long long);

	rcu_read_lock();
	task = &init_task;
	for_each_process(task) {
		if (entry->count >= max_count)
			break;
		if (task->pid != task->tgid)
			continue;

		ret_len = perf_get_per_proc_cpu_stat(task, data, cpu_num,
			entry->length);
		if (ret_len < 0) {
			rcu_read_unlock();
			pr_err("PerfD %s: get info failed, ret %d\n",
				__func__, ret_len);
			return ret_len;
		}
		if (ret_len == 0) {
			rcu_read_unlock();
			pr_info("PerfD %s: get info exit, memory overflow\n",
				__func__);
			return ret_len;
		}
		if (((struct perf_process_cputime_info *)data)->cpu_load_time == 0)
			continue;
		data += ret_len;
		entry->count++;
		entry->length += ret_len;
	}
	rcu_read_unlock();

	return 0;
}

static int perf_process_cputime_fetch(void __user *argp)
{
	int ret;
	unsigned int entry_size;
	struct perf_buffered_log_entry *entry = NULL;
	int cpu_num;

	cpu_num = perf_get_cpu_num();

	/* We don't know the length of the cpuset, so prepare the max memory */
	entry_size = MAX_PERF_LOG_LENGTH;
	entry = perf_create_log_entry(entry_size, IOC_PROCESS_CPUTIME,
		MAX_COUNT_PROC_NUM);

	if (entry == NULL) {
		pr_err("PerfD %s: failed to create log entry\n", __func__);
		goto error;
	}

	if (get_perf_process_cputime_stats(entry, cpu_num)) {
		pr_err("PerfD %s: failed to get process cputime\n", __func__);
		goto error;
	}

	ret = send_perf_buffered_log_entry(entry);
	if (ret < 0) {
		pr_err("PerfD %s: failed to send log entry\n", __func__);
		goto error;
	}

	free_perf_buffered_log_entry(entry);

	return 0;

error:
	free_perf_buffered_log_entry(entry);
	return -EINVAL;
}

static int perf_init_cpu_load_dimps(void __user *argp)
{
	int ret;
	int i;
	struct proc_cputime_set_msg message;
	ret = memset_s(dmips_value_buffer, sizeof(dmips_value_buffer), 0,
		sizeof(dmips_value_buffer));
	if (ret != EOK) {
		pr_err("PerfD %s: memset_s failed\n", __func__);
		return ret;
	}

	if (copy_from_user(&message, (void*)argp,
		sizeof(struct proc_cputime_set_msg))) {
		pr_err("PerfD %s: copy_from_user failed\n", __func__);
		return -EFAULT;
	}

	if ((message.count < 0) || (message.count > DMIPS_NUM)) {
		pr_err("PerfD %s: invalid message count\n", __func__);
		return -EINVAL;
	}

	for (i = 0 ; i < message.count ; i++)
		dmips_value_buffer[i] = message.data[i];

	return 0;
}

long perf_ioctl_process_cputime(unsigned int cmd, void __user *argp)
{
	int ret = 0;

	switch (cmd) {
	case  IOC_GET_PROCESS_CPUTIME:
		ret = perf_process_cputime_fetch(argp);
		break;
	case IOC_SET_CPU_DIMPS:
		ret = perf_init_cpu_load_dimps(argp);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

