#ifndef __PERF_UTILS_H__
#define __PERF_UTILS_H__

#include <linux/sched.h>

struct perf_cpu_time_info {
	unsigned long long user;
	unsigned long long nice;
	unsigned long long system;
	unsigned long long idle;
	unsigned long long iowait;
	unsigned long long irq;
	unsigned long long softirq;
};

struct task_struct *perf_get_task_by_pid(int upid);
int perf_get_cpu_num(void);
int perf_get_cpuset(int upid, struct task_struct *tsk, char *buf);
void perf_get_cpu_time(struct task_struct *p, unsigned long long *cpu_time);
void perf_sysinfo_get_per_core_cpu_time(
	struct perf_cpu_time_info *per_cpu_time, int cpu);

#endif /* __PERF_UTILS_H__ */