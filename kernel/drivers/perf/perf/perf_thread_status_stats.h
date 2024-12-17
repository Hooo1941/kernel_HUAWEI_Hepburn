#ifndef __PERF_CPU_STATE_STATS_H__
#define __PERF_CPU_STATE_STATS_H__

#include <linux/sched.h>

long perf_ioctl_thread_status(unsigned int cmd, void __user *argp);

#endif

