/* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved. */

#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <securec.h>
#include <linux/io.h>
#if defined (CONFIG_PLATDRV_SUPPORT_FFA)
#include <linux/arm_ffa.h>
#include <platform_include/see/ffa/ffa_msg_id.h>
#include <platform_include/see/ffa/ffa_plat_drv.h>
#endif
#include <ddr_vote_statistic_struct.h>

static struct ddr_freq_stat g_ddr_freq_stat_data;
static int ddr_stat_show(struct seq_file *m, void *v)
{
	(void)v;
#if defined (CONFIG_PLATDRV_SUPPORT_FFA)
	int ret;
	unsigned int i;
	struct ffa_send_direct_data args = {
		.data0 = FID_LP_DDR_STAT_GET,
		.data1 = SHMEM_OUT,
		.data4 = sizeof(g_ddr_freq_stat_data),
	};

	ret = ffa_platdrv_send_msg_with_shmem(&args, (u8 *)&g_ddr_freq_stat_data, SHMEM_OUT);
	if (ret != 0) {
		pr_err("%s: ffa send fail\n", __func__);
		seq_printf(m, "stat data is invalid\n");
		return -EINVAL;
	}
	seq_printf(m, "freq id    time(ms)\n");
	for (i = 0; i <= g_ddr_freq_stat_data.freq_num; i++)
		seq_printf(m, "%d:    %d\n", i, g_ddr_freq_stat_data.stat_ms[i]);
#endif
	return 0;
}

static int ddr_stat_open(struct inode *inode, struct file *file)
{
	(void)inode;
	return single_open(file, ddr_stat_show, NULL);
}

static const struct proc_ops ddr_stat_proc_fops = {
	.proc_open = ddr_stat_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = seq_release_private,
};

static int __init ddr_stat_init(void)
{
	proc_create("ddr_stat", S_IRUGO, (struct proc_dir_entry *)NULL, &ddr_stat_proc_fops);
	return 0;
}

static void __exit ddr_stat_exit(void)
{
	remove_proc_entry("ddr_stat", NULL);
}

module_init(ddr_stat_init);
module_exit(ddr_stat_exit);
