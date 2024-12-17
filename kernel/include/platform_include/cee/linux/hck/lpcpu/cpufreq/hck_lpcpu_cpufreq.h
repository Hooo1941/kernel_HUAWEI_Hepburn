#ifndef __HCK_LPCPU_CPUFREQ__
#define __HCK_LPCPU_CPUFREQ__

#include <linux/types.h>
#include <platform_include/basicplatform/linux/hck_vendor_hooks.h>
/*
 * Following tracepoints are not exported in tracefs and provide a
 * mechanism for vendor modules to hook and extend functionality
 */

struct kobject;
struct kobj_type;
DECLARE_HCK_VH(lpcpu_cpufreq_policy_kobject_init,
	       TP_PROTO(struct kobject *kobj, struct kobj_type *ktype, struct kobject *parent,
	       		const char *fmt, unsigned int cpu, int *ret, int *flag),
	       TP_ARGS(kobj, ktype, parent, fmt, cpu, ret, flag));

#endif /* __HCK_LPCPU_CPUFREQ__ */
