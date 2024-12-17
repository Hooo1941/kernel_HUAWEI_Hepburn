/*
 * vibrator_state.h
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

#ifndef __VIBRATOR_STATE_H__
#define __VIBRATOR_STATE_H__

#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/device.h>
#include <linux/pm_wakeup.h>
#include "../audio_misc.h"

#define TIME_TO_STOP_VIBRATION_ROUTE 3
// time to make sure cpu is active before vibration route is closed
#define CPU_ACTIVE_TIME (TIME_TO_STOP_VIBRATION_ROUTE + 1)

enum {
	VIBRATOR_OFF = 0,
	VIBRATOR_ON = 1,
	VIBRATOR_MAX_STATE = 2,
};

enum {
	PHONE_UNFOLD = 0,
	PHONE_FOLD = 1,
	PHONE_HALL_MAX_STATE = 2,
};

struct vibrator_state_priv {
	uint32_t vibrator_state;
	uint32_t phone_hall_state;
	struct mutex vibrator_state_lock;
	struct mutex hall_state_lock;
	struct wakeup_source *wake_lock;
	struct delayed_work stop_vibrator_dw;
	struct notifier_block vibrator_state_notifier;
	struct notifier_block hall_state_notifier;
};

#ifdef CONFIG_AUDIO_VIBRATOR_STATE
int vibrator_state_probe(struct device *dev, struct device_node *np);
#else
static inline int vibrator_state_probe(struct device *dev, struct device_node *np)
{
	return 0;
}
#endif /* CONFIG_AUDIO_VIBRATOR_STATE */
#endif  /* __VIBRATOR_STATE_H__ */
