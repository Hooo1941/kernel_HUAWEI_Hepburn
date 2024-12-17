/*
 * audio_misc.h
 *
 * audio_misc driver
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

#ifndef __AUDIO_MISC_H__
#define __AUDIO_MISC_H__

#include <linux/mutex.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/list.h>

#define MAX_NAME_LEN 128

struct audio_misc_device {
	struct device *dev;
	struct list_head mod_head;
	struct mutex list_lock;
};

struct audio_misc_module_ops {
	void (*remove)(void *priv);
	long (*ioctl)(unsigned int command, unsigned long arg);
};

struct audio_misc_module {
	const char *name;
	struct list_head node;
	void *priv;
	char io_type;
	const struct audio_misc_module_ops *ops;
};

struct audio_misc_module_table {
	const char name[MAX_NAME_LEN];
	int (*probe)(struct device *dev, struct device_node *np);
};

int audio_misc_module_constructor(const char *name, void *priv, char io_type,
	const struct audio_misc_module_ops *ops);
void audio_misc_module_destructor(const char *name);
void audio_misc_report_uevent(const char *mod_name, const char *action);

#endif  /* __AUDIO_MISC_H__ */