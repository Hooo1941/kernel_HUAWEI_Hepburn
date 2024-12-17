
#ifndef __PERF_PROC_MEMORY_INFO_H__
#define __PERF_PROC_MEMORY_INFO_H__

#include <linux/sched.h>

long perf_ioctl_proc_memory_info(unsigned int cmd, void __user *argp);

#endif /* __PERF_PROC_MEMORY_INFO_H__ */