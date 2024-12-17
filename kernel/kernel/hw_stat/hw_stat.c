/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: hw_stat's framework
 * Author: Qin Hongwei <qinhongwei7@huawei.com>
 * Create: 2023-02-17
 */
#include <linux/hw_stat.h>
#include <linux/export.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/blkdev.h>

#ifdef CONFIG_HUAWEI_QOS_BLKIO
#include <chipset_common/hwqos/hwqos_common.h>
#endif

#ifdef CONFIG_HW_STAT_PSI
#include "psi/hw_stat_psi.h"
#endif
#ifdef CONFIG_HW_STAT_BLK_INFO
#include "blk_info/hw_stat_blk_info.h"
#endif

#ifdef CONFIG_HW_STAT_PHYMEM
#include "phymem/hw_stat_phymem.h"
#endif

int current_is_high_prio(void)
{
#ifdef CONFIG_HUAWEI_QOS_BLKIO
	int qos;

	qos = get_task_qos(current);
	if (qos >= BLKIO_QOS_HIGH)
		return 1;
#endif
	return 0;
}
EXPORT_SYMBOL(current_is_high_prio);

static int __init hw_stat_init(void)
{
	struct proc_dir_entry *dir = NULL;

	dir = proc_mkdir_mode("hw_stat", S_IRUSR|S_IXUSR, NULL);
	if (IS_ERR_OR_NULL(dir))
		return -EINVAL;
#ifdef CONFIG_HW_STAT_FAULT
	hw_stat_init_faultstat(dir);
#endif

#ifdef CONFIG_HW_STAT_IOSTAT
	hw_stat_init_iostat(dir);
#endif
#ifdef CONFIG_HW_STAT_PHYMEM
	hw_stat_init_phymem(dir);
#endif

#ifdef CONFIG_HW_STAT_PSI
	hw_stat_psi_init(dir);
#endif

#ifdef CONFIG_HW_STAT_BLK_INFO
	hw_stat_init_blk_info(dir);
#endif

#ifdef CONFIG_HW_STAT_TRACING_MARK_WRITE
	hw_stat_init_trace_marker(dir);
#endif
	return 0;
}
late_initcall(hw_stat_init);
