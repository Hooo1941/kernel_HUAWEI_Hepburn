/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: fbe_hibernate source file
 * Create : 2024/04/24
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/delay.h>
#include <linux/suspend.h>
#include <linux/notifier.h>
#include <securec.h>
#include <platform_include/see/fbe_ctrl.h>

#define FBEX_MAX_UFS_SLOT 32

static u32 hibernate_slot = -1;

u32 hibernate_get_slot(u32 *slot)
{
	if (!slot) {
		pr_err("%s, slot is null\n", __func__);
		return -1;
	}
	*slot = hibernate_slot;
	return (hibernate_slot >=  FBEX_MAX_UFS_SLOT) ? -1 : 0;
}

EXPORT_SYMBOL(hibernate_get_slot);

static int fbe_hibernate_restore_k(void)
{
	u32 ret;

	pr_info("PM: %s entering.\n", __func__);
	ret = fbex_restore_key_s4(&hibernate_slot);
	if (ret) {
		pr_err("PM: %s, fbe restore key error, ret 0x%x, hibernate_slot %d\n", __func__, ret, hibernate_slot);
		return -1;
	}
	pr_info("PM: %s success, %d\n", __func__, hibernate_slot);
	return 0;
}

static int fbe_hibernate_add_k(void)
{
	u32 ret;

	pr_info("PM: %s entering.\n", __func__);

	ret = fbex_add_key_s4(&hibernate_slot);
	if (ret) {
		pr_err("PM: %s, fbe add key error, ret 0x%x, hibernate_slot %d\n", __func__, ret, hibernate_slot);
		return -1;
	}
	pr_info("PM: %s success, %d\n", __func__, hibernate_slot);
	return 0;
}

static int fbe_hibernate_delete_k(void)
{
	u32 ret;

	pr_info("PM: %s entering.\n", __func__);
	ret = fbex_delete_key_s4();
	if (ret != 0) {
		pr_err("PM: %s, fbe delete key error, ret 0x%x\n", __func__, ret);
		return -1;
	}
	pr_info("PM: %s success\n", __func__);
	return 0;
}

static int fbe_hibernate_notify(struct notifier_block *nb,
				 unsigned long mode,
				 void *_unused)
{
	int ret = 0;

	switch (mode) {
	case PM_HIBERNATION_PREPARE:
		ret = fbe_hibernate_add_k();
		break;
	case PM_POST_HIBERNATION:
	case PM_POST_RESTORE:
		ret = fbe_hibernate_delete_k();
		break;
	case PM_RESTORE_PREPARE:
		ret = fbe_hibernate_restore_k();
		break;
	default:
		break;
	}

	if (ret != 0) {
		pr_err("PM: %s event %lx error\n", __func__, mode);
		return NOTIFY_BAD;
	}

	return NOTIFY_OK;
}

static struct notifier_block fbe_hibernate_nb = {
	.notifier_call = fbe_hibernate_notify,
};

static int __init fbe_hibernate_init(void)
{
	int ret;

	ret = register_pm_notifier(&fbe_hibernate_nb);
	if (ret != 0)
		pr_err("Can't register fbe_hibernate notifier ret %d\n", ret);

	return 0;
}

late_initcall(fbe_hibernate_init);
