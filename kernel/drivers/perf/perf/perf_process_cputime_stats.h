#ifndef __PERF_CPU_USAGE_STATS_H__
#define __PERF_CPU_USAGE_STATS_H__

#include <linux/sched.h>

long perf_ioctl_process_cputime(unsigned int cmd, void __user *argp);

#endif

