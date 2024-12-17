/*
 * recordboottime.c
 *
 * record boottime into filesystem when booting kernel
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/stat.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>   /* For copy_to_user */
#include <linux/vmalloc.h>   /* For vmalloc */
#include <linux/module.h>
#include <linux/syscalls.h>
#include <asm/early_ioremap.h>
#include <platform_include/basicplatform/linux/util.h> /* For mach_call_usermoduleshell */
#include <platform_include/basicplatform/linux/pr_log.h>
#include <platform_include/basicplatform/linux/rdr_pub.h>
#include <mntn_public_interface.h>
#include <securec.h>
#include <boot_time_keypoint.h>
#include "blackbox/rdr_inner.h"

#define PR_LOG_TAG RECORD_BOOTTIME_TAG
#include "blackbox/rdr_print.h"

#define BOOTTIME_DUMP_LOG_ADDR SUB_RESERVED_BOOT_TIME_KEYPOINT_PYHMEM_BASE
#define BOOTTIME_DUMP_LOG_SIZE SUB_RESERVED_BOOT_TIME_KEYPOINT_PYHMEM_SIZE

enum MAP_BOOT_ADDR_TYPE {
	MAP_NONE,
	MAP_EARLY_IOREMAP_WC,
	MAP_PHYS_TO_VIRT,
	MAP_IOREMAP_WC,
};

static char *g_boottime_buff = NULL;
static int g_boottime_addr_map = MAP_NONE;
static int g_boottime_addr_map_early = 0;
static DEFINE_MUTEX(g_boot_time_record_mutex);

void set_boottime_addr_early_map_end(void)
{
	g_boottime_addr_map_early = 1;
}

/* read dump file content */
static ssize_t boottime_dump_file_read(struct file *file,
					char __user *userbuf, size_t bytes,
					loff_t *off)
{
	ssize_t copy;

	if (off == NULL)
		return 0;

	if (*off > (loff_t)BOOTTIME_DUMP_LOG_SIZE) {
		BB_PRINT_ERR("%s: read offset error\n", __func__);
		return 0;
	}

	if (*off == (loff_t)BOOTTIME_DUMP_LOG_SIZE) {
		/* end of file */
		return 0;
	}

	if (userbuf == NULL) {
		/* end of file */
		return 0;
	}

	copy = (ssize_t)(uintptr_t) min(bytes, (size_t) ((loff_t)BOOTTIME_DUMP_LOG_SIZE - *off));

	if (copy_to_user(userbuf, g_boottime_buff + *off, copy)) {
		BB_PRINT_ERR("%s: copy to user error\n", __func__);
		copy = -EFAULT;
		goto copy_err;
	}

	*off += copy;

copy_err:
	return copy;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
static const struct proc_ops boottime_dump_file_fops = {
	.proc_read = boottime_dump_file_read,
#else
static const struct file_operations boottime_dump_file_fops = {
	.read = boottime_dump_file_read,
#endif
};

static char *boottime_addr_map(void)
{
	if (!g_boottime_addr_map_early) {
		if (g_boottime_addr_map == MAP_EARLY_IOREMAP_WC) {
			return g_boottime_buff;
		} else {
			g_boottime_addr_map = MAP_EARLY_IOREMAP_WC;
			return early_ioremap(BOOTTIME_DUMP_LOG_ADDR, BOOTTIME_DUMP_LOG_SIZE);
		}
	}

	if (g_boottime_addr_map == MAP_EARLY_IOREMAP_WC ) {
		g_boottime_addr_map = MAP_NONE;
		early_iounmap(g_boottime_buff, BOOTTIME_DUMP_LOG_SIZE);
	}

	if (g_boottime_addr_map == MAP_PHYS_TO_VIRT || g_boottime_addr_map == MAP_IOREMAP_WC)
		return g_boottime_buff;

	if (pfn_valid(__phys_to_pfn(BOOTTIME_DUMP_LOG_ADDR))) {
		g_boottime_addr_map = MAP_PHYS_TO_VIRT;
		return phys_to_virt(BOOTTIME_DUMP_LOG_ADDR);
	}

	g_boottime_addr_map = MAP_IOREMAP_WC;
	return ioremap_wc(BOOTTIME_DUMP_LOG_ADDR, BOOTTIME_DUMP_LOG_SIZE);
}
/*
 * Description :  boottime_log init
 *                if there is log, launch work queue, and create /proc/rdr/boottime_log
 * Input:         NA
 * Output:        NA
 * Return:        OK:success
 */
static int __init boottime_dump_init(void)
{
	mutex_lock(&g_boot_time_record_mutex);
	g_boottime_buff = boottime_addr_map();
	mutex_unlock(&g_boot_time_record_mutex);
	if (g_boottime_buff == NULL) {
		BB_PRINT_ERR(
		       "%s: fail to get the virtual address of boottime\n", __func__);
		return -1;
	}

	if (!check_mntn_switch(MNTN_GOBAL_RESETLOG))
		return 0;

	if (g_boottime_buff != NULL)
		dfx_create_log_proc_entry("boottime_log", S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP,
					     &boottime_dump_file_fops, NULL);

	return 0;
}

module_init(boottime_dump_init);

/*
 * Description :  boottime exit
 *                destroy the workqueue
 * Input:         NA
 * Output:        NA
 * Return:        NA
 */
static void __exit boottime_dump_exit(void)
{
	dfx_remove_log_proc_entry("boottime_log");

	if (g_boottime_buff == NULL)
		return;

	if (g_boottime_addr_map == MAP_EARLY_IOREMAP_WC)
		early_iounmap(g_boottime_buff, BOOTTIME_DUMP_LOG_SIZE);

	if (g_boottime_addr_map == MAP_IOREMAP_WC)
		iounmap(g_boottime_buff);
}

module_exit(boottime_dump_exit);

static void boottime_save_one(struct boot_time_keypoint *info)
{
	struct boot_time_keypoint *boot_time_info = NULL;

	g_boottime_buff = boottime_addr_map();
	if (g_boottime_buff == NULL) {
		BB_PRINT_ERR("%s: g_boottime_buff = NULL\n", __func__);
		return;
	}
	boot_time_info = (struct boot_time_keypoint *)((unsigned long)(BOOT_TIME_KEYPOINT_ADDR(g_boottime_buff, info->stage)));
	boot_time_info->stage = info->stage;
	boot_time_info->start_time = info->start_time ? info->start_time : boot_time_info->start_time;
	boot_time_info->end_time = info->end_time ? info->end_time : boot_time_info->end_time;
	boot_time_info->value = info->value ? info->value : boot_time_info->value;
}

static unsigned short boot_time_map_stage(unsigned short s_stage)
{
	switch(s_stage) {
	case MAP_KERNEL_STAGE:
		return BOOTTIME_KERNEL_STAGE;
	case MAP_ANDROID_STAGE:
		return BOOTTIME_ANDROID_STAGE;
	default:
		BB_PRINT_ERR(
		       "%s: error no map s_stage[%d]\n", __func__, (int)s_stage);
		return BOOTTIME_MAX_STAGE;
	}
}

void set_boot_time_keypoint(unsigned short stage, int step, unsigned short time, unsigned short value)
{
	struct boot_time_keypoint info = {0};

	info.stage = boot_time_map_stage(stage);
	if (step == START_BOOT_TIME)
		info.start_time = time;
	else
		info.end_time = time;
	info.value = value;
	mutex_lock(&g_boot_time_record_mutex);
	boottime_save_one(&info);
	mutex_unlock(&g_boot_time_record_mutex);
}
