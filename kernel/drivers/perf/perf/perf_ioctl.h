#ifndef __PERF_IOCTL_H__
#define __PERF_IOCTL_H__

#include <linux/ioctl.h>

/* empirical value, usually no more than 1000 */
#define MAX_COUNT_PROC_NUM 1100

enum perf_ioctl_type_t {
	IOC_THREAD_STATUS = 0,
	IOC_PROCESS_CPUTIME,
	IOC_SYSTEM_INFO,
	IOC_SYSTEM_MEM_INFO,
	IOC_PROC_MEM_INFO,
	IOC_LPM_LIMIT_FREQ,
};

#endif // __PERF_IOCTL_H__