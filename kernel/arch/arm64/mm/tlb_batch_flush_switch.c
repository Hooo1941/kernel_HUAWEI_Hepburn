#include <linux/sysctl.h>
#include <linux/types.h>

static int tlb_batch_flush_enable = 1;

bool arch_tlb_batch_flush_enable(void)
{
	return !!tlb_batch_flush_enable;
}

static struct ctl_table ctl_table[] = {
	{
		.procname       = "tlb_batch_flush_optimization",
		.data           = &tlb_batch_flush_enable,
		.maxlen         = sizeof(tlb_batch_flush_enable),
		.mode           = 0644,
		.proc_handler   = proc_dointvec_minmax,
		.extra1         = SYSCTL_ZERO,
		.extra2         = SYSCTL_ONE,
	},
	{}
};

static struct ctl_table optimization_dir_table[] = {
	{
		.procname = "vm",
		.maxlen = 0,
		.mode = 0555,
		.child = ctl_table,
	},
	{}
};

static int __init camera_info_init(void)
{
	if (register_sysctl_table(optimization_dir_table)) {
		pr_err("register optimization sysctl failed\n");
		return -ENOMEM;
	}

	return 0;
}
fs_initcall(camera_info_init);
