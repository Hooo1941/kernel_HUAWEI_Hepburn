#include <linux/init.h>
#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#endif
#include <linux/miscdevice.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include "utils/perf_buffered_log_sender.h"
#include "perf_thread_status_stats.h"
#include "perf_process_cputime_stats.h"
#include "perf_system_info.h"
#include "perf_system_memory_info.h"
#include "perf_proc_memory_info.h"
#include "perf_lpm_limit_freq.h"
#include "perf_ioctl.h"

#define PERFD_NAME "hiview"

static long (*perf_ioctl_cb[])(unsigned int cmd, void __user *argp) = {
	perf_ioctl_thread_status,       /* IOC_THREAD_STATUS */
	perf_ioctl_process_cputime,     /* IOC_PROCESS_CPUTIME */
	perf_ioctl_system_info,         /* IOC_SYSTEM_INFO */
	perf_ioctl_system_memory_info,  /* IOC_SYSTEM_MEM_INFO */
	perf_ioctl_proc_memory_info,    /* IOC_PROC_MEM_INFO */
	perf_ioctl_lpm_limit_freq,      /* IOC_LPM_LIMIT_FREQ */
};

static long perf_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	const char *comm = NULL;

	comm = current->group_leader ? current->group_leader->comm : current->comm;
	if (!strstr(comm, PERFD_NAME))
		return -EINVAL;

	if ((_IOC_TYPE(cmd) >= ARRAY_SIZE(perf_ioctl_cb)) ||
		(perf_ioctl_cb[_IOC_TYPE(cmd)] == NULL)) {
		pr_err("PerfD %s: invalid perf ioctrl type %u\n",
			__func__, _IOC_TYPE(cmd));
		return -EINVAL;
	}

	return perf_ioctl_cb[_IOC_TYPE(cmd)](cmd, argp);
}

#ifdef CONFIG_COMPAT
static long perf_compat_ioctl(
		struct file *filp, unsigned int cmd, unsigned long arg)
{
	return perf_ioctl(filp, cmd, (unsigned long) compat_ptr(arg));
}
#endif

static int perf_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int perf_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static const struct file_operations perf_device_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = perf_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = perf_compat_ioctl,
#endif
	.open = perf_open,
	.release = perf_release,
};

static struct miscdevice perf_device = {
	.name = "perf",
	.fops = &perf_device_fops,
	.minor = MISC_DYNAMIC_MINOR,
};

static int __init perf_init(void)
{
	int ret;

	perf_sock_lock_init();
	ret = misc_register(&perf_device);
	if (ret) {
		pr_err("PerfD %s: failed to register perf device\n", __func__);
		return ret;
	}

	return 0;
}

static void __exit perf_exit(void)
{
	perf_buffered_log_release();
}

late_initcall_sync(perf_init);
module_exit(perf_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Huawei Device Usage Peformance Driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
