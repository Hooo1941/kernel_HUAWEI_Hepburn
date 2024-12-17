
#include <linux/sched.h>
#include <linux/pid.h>
#include <linux/mm_types.h>
#include <linux/sched/mm.h>
#include <linux/mm_types_task.h>
#include <linux/ioctl.h>

#include "securec.h"
#include "utils/perf_buffered_log_sender.h"
#include "perf_ioctl.h"
#include "utils/perf_utils.h"
#include "perf_proc_memory_info.h"

struct proc_meminfo_set_msg {
	int proc_mem_threshold;
	int mem_available;
};

struct perf_proc_memory_info {
	int pid;
	int vmpeak;
	int vmsize;
	int vmlck;
	int vmpin;
	int vmhwm;
	int vmrss;
	int rssanon;
	int rssfile;
	int rssshmem;
	int vmdata;
	int vmstk;
	int vmexe;
	int vmlib;
	int vmpte;
	int vmswap;
};

#define IOC_GET_PROC_MEM_INFO _IO(IOC_PROC_MEM_INFO, 1)
#define IOC_SET_MEM_THRESHOLD _IOW(IOC_PROC_MEM_INFO, 1, struct proc_meminfo_set_msg)

/* proc memory filtering threshold for per proc information statistics */
static int g_proc_mem_threshold;

/* available memory filtering threshold for total information statistics */
static int g_mem_available;

static bool perf_proc_meminfo_is_enable()
{
	long available;

	available = si_mem_available();
	/* mem_available < filtering threshold, record proc mem info */
	if (((available) << (PAGE_SHIFT - 10)) < g_mem_available)
		return true;

	return false;
}

static int perf_per_proc_meminfo_get_data(
	struct perf_proc_memory_info *proc_mem_info, int pid)
{
	struct task_struct *task;
	struct mm_struct *mm;
	unsigned long text, lib, swap, anon, file, shmem;
	unsigned long hiwater_vm, total_vm, hiwater_rss, total_rss;

	rcu_read_lock();
	task = perf_get_task_by_pid(pid);
	if (task == NULL) {
		rcu_read_unlock();
		pr_err("PerfD %s: get task by pid failed, pid %d\n", __func__, pid);
		return -EINVAL;
	}

	mm = get_task_mm(task);
	/* some tasks struct mm_struct* is NULL, skip these tasks */
	if (mm == NULL) {
		rcu_read_unlock();
		return -EINVAL;
	}

	anon = get_mm_counter(mm, MM_ANONPAGES);
	file = get_mm_counter(mm, MM_FILEPAGES);
	shmem = get_mm_counter(mm, MM_SHMEMPAGES);

	hiwater_vm = total_vm = mm->total_vm;
	if (hiwater_vm < mm->hiwater_vm)
		hiwater_vm = mm->hiwater_vm;
	hiwater_rss = total_rss = anon + file + shmem;
	if (hiwater_rss < mm->hiwater_rss)
		hiwater_rss = mm->hiwater_rss;

	/* split executable areas between text and lib */
	text = PAGE_ALIGN(mm->end_code) - (mm->start_code & PAGE_MASK);
	text = min(text, mm->exec_vm << PAGE_SHIFT);
	lib = (mm->exec_vm << PAGE_SHIFT) - text;
	swap = get_mm_counter(mm, MM_SWAPENTS);

	proc_mem_info->pid = pid;
	proc_mem_info->vmpeak = (hiwater_vm) << (PAGE_SHIFT - 10);
	proc_mem_info->vmsize = (total_vm) << (PAGE_SHIFT - 10);
	proc_mem_info->vmlck = (mm->locked_vm) << (PAGE_SHIFT - 10);
	proc_mem_info->vmpin = (atomic64_read(&mm->pinned_vm)) <<
		(PAGE_SHIFT - 10);
	proc_mem_info->vmhwm = (hiwater_rss) << (PAGE_SHIFT - 10);
	proc_mem_info->vmrss = (total_rss) << (PAGE_SHIFT - 10);
	proc_mem_info->rssanon = (anon) << (PAGE_SHIFT - 10);
	proc_mem_info->rssfile = (file) << (PAGE_SHIFT - 10);
	proc_mem_info->rssshmem = (shmem) << (PAGE_SHIFT - 10);
	proc_mem_info->vmdata = (mm->data_vm) << (PAGE_SHIFT - 10);
	proc_mem_info->vmstk = (mm->stack_vm) << (PAGE_SHIFT - 10);
	proc_mem_info->vmexe = text >> 10;
	proc_mem_info->vmlib = lib >> 10;
	proc_mem_info->vmpte = mm_pgtables_bytes(mm) >> 10;
	proc_mem_info->vmswap = (swap) << (PAGE_SHIFT - 10);

	rcu_read_unlock();
	return 0;
}

static int perf_proc_meminfo_get_data(struct perf_buffered_log_entry *entry)
{
	int max_count;
	struct task_struct *task;
	struct perf_proc_memory_info info;
	int ret;
	unsigned char *data = entry->data;

	max_count = entry->count;
	entry->count = 0;

	rcu_read_lock();
	task = &init_task;
	for_each_process(task) {
		if (entry->count >= max_count)
			break;
		if (task->pid != task->tgid)
			continue;
		/*
		 * If the PID is less than 500, the process is considered as
		 * a kernel process and its information is not collected.
		 */
		if (task->pid < 500)
			continue;
		info.pid = task->pid;
		ret = perf_per_proc_meminfo_get_data(&info, task->pid);
		if (ret != 0)
			continue;
		if (info.vmrss < g_proc_mem_threshold)
			continue;
		ret = memcpy_s(data, sizeof(struct perf_proc_memory_info), &info,
			sizeof(struct perf_proc_memory_info));
		if (ret != EOK) {
			pr_err("PerfD %s: proc meminfo memcpy_s failed, pid %d, ret %d\n",
				__func__, task->pid, ret);
			rcu_read_unlock();
			return ret;
		}
		data += sizeof(struct perf_proc_memory_info);
		entry->count++;
	}
	rcu_read_unlock();
	entry->length = cal_log_entry_len(sizeof(struct perf_buffered_log_entry),
		sizeof(struct perf_proc_memory_info), entry->count);

	return 0;
}

static long perf_proc_memory_info_fetch(void __user *argp)
{
	int ret = -EINVAL;
	unsigned int entry_size;
	struct perf_buffered_log_entry *entry = NULL;

	if (!perf_proc_meminfo_is_enable())
		return 0;

	entry_size = cal_log_entry_len(sizeof(struct perf_buffered_log_entry),
		sizeof(struct perf_proc_memory_info), MAX_COUNT_PROC_NUM);
	entry = perf_create_log_entry(entry_size, IOC_PROC_MEM_INFO,
		MAX_COUNT_PROC_NUM);
	if (entry == NULL) {
		pr_err("PerfD %s: failed to create log entry\n", __func__);
		goto error;
	}

	if (perf_proc_meminfo_get_data(entry)) {
		pr_err("PerfD %s: failed to get proc meminfo\n", __func__);
		goto error;
	}

	ret = send_perf_buffered_log_entry(entry);
	if (ret < 0) {
		pr_err("PerfD %s: failed to send log entry ret %d\n", __func__, ret);
		goto error;
	}

error:
	free_perf_buffered_log_entry(entry);
	return ret;
}

static long perf_set_mem_filter_threshold(void __user *argp)
{
	struct proc_meminfo_set_msg mem_threshold;

	/* accept mem filter threshold from hiview */
	if (copy_from_user(&mem_threshold, (void*)argp, sizeof(struct proc_meminfo_set_msg))) {
		pr_err("PerfD %s: failed to copy_from_user\n", __func__);
		return -EFAULT;
	}

	g_proc_mem_threshold = mem_threshold.proc_mem_threshold;
	g_mem_available = mem_threshold.mem_available;

	return 0;
}

long perf_ioctl_proc_memory_info(unsigned int cmd, void __user *argp)
{
	int ret = 0;

	switch (cmd) {
	case  IOC_GET_PROC_MEM_INFO:
		ret = perf_proc_memory_info_fetch(argp);
		break;
	case IOC_SET_MEM_THRESHOLD:
		ret = perf_set_mem_filter_threshold(argp);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}