/*
 * vibrator_event.c
 *
 * vibrator_event driver
 *
 * Copyright (c) 2023 Huawei Technologies Co., Ltd.
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include "vibrator_event.h"

static ATOMIC_NOTIFIER_HEAD(vibrator_notifier_list);
static struct delayed_work vibrator_work;
static unsigned long vibrator_duration;

int vibrator_register_notifier(struct notifier_block *nb)
{
	return atomic_notifier_chain_register(&vibrator_notifier_list, nb);
}
EXPORT_SYMBOL_GPL(vibrator_register_notifier);

int vibrator_unregister_notifier(struct notifier_block *nb)
{
	return atomic_notifier_chain_unregister(&vibrator_notifier_list, nb);
}
EXPORT_SYMBOL_GPL(vibrator_unregister_notifier);

int vibrator_call_notifiers(unsigned long val, void *v)
{
	return atomic_notifier_call_chain(&vibrator_notifier_list, val, v);
}
EXPORT_SYMBOL_GPL(vibrator_call_notifiers);

void vibrator_duration_distinguish(unsigned long pressed)
{
	vibrator_duration = pressed;
	pr_info("%s duration = %lu\n", __func__, vibrator_duration);
	queue_delayed_work(system_power_efficient_wq, &vibrator_work, 0);
}
EXPORT_SYMBOL_GPL(vibrator_duration_distinguish);

static void vibrator_notify_delayed_work(struct work_struct *work)
{
	(void)work;
	vibrator_call_notifiers(vibrator_duration, NULL);
}

int vibrator_nb_init(void)
{
	pr_info("%s:init\n", __func__);
	INIT_DELAYED_WORK(&vibrator_work, vibrator_notify_delayed_work);
	return 0;
}
EXPORT_SYMBOL_GPL(vibrator_nb_init);

MODULE_LICENSE("GPL");