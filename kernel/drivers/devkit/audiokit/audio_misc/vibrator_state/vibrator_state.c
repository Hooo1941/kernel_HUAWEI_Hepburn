/*
 * vibrator_state.c
 *
 * vibrator_state driver
 *
 * Copyright (c) 2022-2023 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "vibrator_state.h"
#include "vibrator_event.h"
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/ioctl.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/delay.h>
#include <linux/compat.h>
#include <linux/of.h>
#include <linux/notifier.h>
#include <securec.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/sensor/ext_hall_event.h>

#define HWLOG_TAG vibrator_state
#define MOD_NAME "vibrator_state"

static unsigned int is_support_hall_notifier = 0;

HWLOG_REGIST();

static void stop_vibrator_work(struct work_struct *work)
{
	struct vibrator_state_priv *priv = container_of(work, struct vibrator_state_priv, stop_vibrator_dw.work);

	audio_misc_report_uevent(MOD_NAME, "VIBRATOR_OFF");
	mutex_lock(&priv->vibrator_state_lock);
	priv->vibrator_state = VIBRATOR_OFF;
	mutex_unlock(&priv->vibrator_state_lock);
}
static int hall_state_notifier_call(struct notifier_block *nb,
	unsigned long hall_state, void *data)
{
	struct vibrator_state_priv *priv = container_of(nb, struct vibrator_state_priv, hall_state_notifier);

	mutex_lock(&priv->hall_state_lock);
	// bit 1  1 is PHONE_FOLD  0 is PHONE_UNFOLD
	hall_state = (hall_state >> EXT_HALL_BIT_HINGLE) & PHONE_FOLD;
	if (hall_state) {
		audio_misc_report_uevent(MOD_NAME, "PHONE_FOLD");
		priv->phone_hall_state = PHONE_FOLD;
	} else {
		audio_misc_report_uevent(MOD_NAME, "PHONE_UNFOLD");
		priv->phone_hall_state = PHONE_UNFOLD;
	}
	mutex_unlock(&priv->hall_state_lock);
	hwlog_info("%s: phone_hall_state = %d\n", __func__, priv->phone_hall_state);
	return NOTIFY_DONE;
}


static int vibrator_state_notifier_call(struct notifier_block *nb,
	unsigned long duration, void *data)
{
	struct vibrator_state_priv *priv = container_of(nb, struct vibrator_state_priv, vibrator_state_notifier);

	if (duration > 0) { // vibration start
		cancel_delayed_work_sync(&priv->stop_vibrator_dw);
		audio_misc_report_uevent(MOD_NAME, "VIBRATOR_ON");
		mutex_lock(&priv->vibrator_state_lock);
		priv->vibrator_state = VIBRATOR_ON;
		mutex_unlock(&priv->vibrator_state_lock);
	} else { // vibration end
		mutex_lock(&priv->vibrator_state_lock);
		if (priv->vibrator_state == VIBRATOR_OFF) {
			mutex_unlock(&priv->vibrator_state_lock);
			hwlog_info("%s: duplicated off notifications received\n", __func__);
			return NOTIFY_DONE;
		}
		mutex_unlock(&priv->vibrator_state_lock);
		__pm_wakeup_event(priv->wake_lock, jiffies_to_msecs(CPU_ACTIVE_TIME * HZ));
		schedule_delayed_work(&priv->stop_vibrator_dw, TIME_TO_STOP_VIBRATION_ROUTE * HZ);
	}
	return NOTIFY_DONE;
}

static void vibrator_state_remove(void *priv)
{
	struct vibrator_state_priv *vs_priv = (struct vibrator_state_priv *)priv;

	if (is_support_hall_notifier)
		(void)ext_hall_unregister_notifier(&vs_priv->hall_state_notifier);
	(void)vibrator_unregister_notifier(&vs_priv->vibrator_state_notifier);
	wakeup_source_unregister(vs_priv->wake_lock);
	kfree(vs_priv);
	vs_priv = NULL;
}

static const struct audio_misc_module_ops vibrator_state_ops = {
	.remove = vibrator_state_remove,
};

int hall_state_init(struct device_node *np, struct vibrator_state_priv *priv)
{
	int ret = 0;
	unsigned int tmp = 0;

	struct device_node *np_child = of_get_child_by_name(np, "vibrator_state");
	if (!of_property_read_u32(np_child, "is_support_hall_notifier", &tmp)) {
		is_support_hall_notifier = tmp;
		hwlog_info("is_support_hall_notifier = %u.\n", is_support_hall_notifier);
	}
	if (is_support_hall_notifier) {
		priv->hall_state_notifier.notifier_call = hall_state_notifier_call;
		ret = ext_hall_register_notifier(&priv->hall_state_notifier);
		if (ret) {
			hwlog_err("%s: register hall_state failed\n", __func__);
			return ret;
		}
	}
	return ret;
}

int vibrator_state_probe(struct device *dev, struct device_node *np)
{
	int ret;
	struct vibrator_state_priv *priv = NULL;

	hwlog_info("%s: in\n", __func__);
	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;
	mutex_init(&priv->vibrator_state_lock);
	priv->wake_lock = wakeup_source_register(dev, "vibrator_state_wake_lock");
	if (!priv->wake_lock) {
		hwlog_err("%s: init wake_lock failed\n", __func__);
		ret = -EINVAL;
		goto wake_lock_err;
	}
	INIT_DELAYED_WORK(&priv->stop_vibrator_dw, stop_vibrator_work);
	priv->vibrator_state_notifier.notifier_call = vibrator_state_notifier_call;
	ret = vibrator_register_notifier(&priv->vibrator_state_notifier);
	if (ret) {
		hwlog_err("%s: register vibrator failed\n", __func__);
		goto vibrator_err;
	}

	ret = hall_state_init(np, priv);
	if (ret) {
		hwlog_err("%s: register hall_state failed\n", __func__);
		goto hall_err;
	}

	ret = audio_misc_module_constructor(MOD_NAME, priv, -1, &vibrator_state_ops);
	if (ret) {
		ret = -ENOMEM;
		goto mod_err;
	}
	return ret;

mod_err:
	if (is_support_hall_notifier)
		(void)ext_hall_unregister_notifier(&priv->hall_state_notifier);
hall_err:
	(void)vibrator_unregister_notifier(&priv->vibrator_state_notifier);
vibrator_err:
	wakeup_source_unregister(priv->wake_lock);
wake_lock_err:
	kfree(priv);
	priv = NULL;
	return ret;
}