/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: implement the platform interface for boot fail
 */
#include "adapter_nvt.h"

#include <linux/printk.h>
#include <linux/kdebug.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/stacktrace.h>
#include <linux/sched/clock.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>

#include <securec.h>
#include <hwbootfail/core/boot_detector.h>
#include <hwbootfail/chipsets/common/bootfail_common.h>
#include <hwbootfail/chipsets/common/bootfail_chipsets.h>
#include <hwbootfail/chipsets/common/adapter_common.h>

#define NS_PER_SEC                 1000000000
#define BOOT_SUCC_TIME             90 /* unit: second */
#define SHORT_TIMER_TIMEOUT_VALUE  (1000 * 60 * 10)
#define LONG_TIMER_TIMEOUT_VALUE   (1000 * 60 * 30)
#define ACTION_TIMER_TIMEOUT_VALUE (1000 * 60 * 30)
#define UFS_TYPE_OFFSET            0x10
#define UFS_FAULT_SIMULATION_SIZE  0x20

static bool g_is_poweroff_charge;
static uintptr_t g_panic_tag_addr = (uintptr_t)NULL;

static int __init early_parse_power_off_charge_cmdline(char *p)
{
	if (p != NULL) {
		g_is_poweroff_charge = (strncmp(p, "charger",
			strlen("charger")) == 0) ? true : false;
	}

	return BF_OK;
}
early_param("androidboot.mode", early_parse_power_off_charge_cmdline);

char *get_bfi_dev_path(void)
{
	return BFI_DEV_PATH;
}

unsigned int get_long_timer_timeout_value(void)
{
	return LONG_TIMER_TIMEOUT_VALUE;
}

unsigned int get_short_timer_timeout_value(void)
{
	return SHORT_TIMER_TIMEOUT_VALUE;
}

unsigned int get_action_timer_timeout_value(void)
{
	return ACTION_TIMER_TIMEOUT_VALUE;
}

unsigned int get_bootup_time(void)
{
	unsigned long long ts;
	ts = sched_clock();
	return (unsigned int)(ts / (NS_PER_SEC));
}

#ifdef CONFIG_BOOT_DETECTOR_DBG
long simulate_storge_rdonly(unsigned long arg)
{
	return BF_OK;
}
#else
long simulate_storge_rdonly(unsigned long arg)
{
	return -EFAULT;
}
#endif

int __init bootfail_init(void)
{
	enum bootfail_errorcode err_code;
	struct common_init_params init_param;
	struct adapter adp;

	(void)memset_s(&init_param, sizeof(init_param), 0, sizeof(init_param));
	if (bootfail_common_init(&init_param) != 0) {
		print_err("int bootfail common failed!\n");
		return BF_NOT_INIT_SUCC;
	}

	(void)memset_s(&adp, sizeof(adp), 0, sizeof(adp));
	if (nvt_adapter_init(&adp) != 0) {
		print_err("int adapter failed!\n");
		return BF_NOT_INIT_SUCC;
	}

	err_code = boot_detector_init(&adp);
	if (err_code != BF_OK) {
		print_err("init boot detector failed [ret: %d]\n", err_code);
		return BF_NOT_INIT_SUCC;
	}

	(void)kthread_run(bootfail_rw_thread, NULL, "bootfail_rw");

	/* set kernel stage */
	err_code = set_boot_stage(KERNEL_STAGE);
	print_err("set KERNEL_STAGE [%d]\n", err_code);

	return BF_OK;
}

module_init(bootfail_init);
MODULE_LICENSE("GPL");
