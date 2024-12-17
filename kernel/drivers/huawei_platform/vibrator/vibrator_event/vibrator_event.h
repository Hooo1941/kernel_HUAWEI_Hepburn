/*
 * vibrator_event.h
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

#ifndef _VIBRATOR_EVENT_H
#define _VIBRATOR_EVENT_H
#include <linux/notifier.h>

int vibrator_register_notifier(struct notifier_block *nb);
int vibrator_unregister_notifier(struct notifier_block *nb);
int vibrator_call_notifiers(unsigned long val, void *v);
void vibrator_duration_distinguish(unsigned long pressed);
int vibrator_nb_init(void);

#endif