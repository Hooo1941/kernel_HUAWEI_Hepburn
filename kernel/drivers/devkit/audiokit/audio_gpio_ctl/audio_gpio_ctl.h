/*
 * audio_gpio_ctl.h
 *
 * audio_gpio_ctl driver
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

#ifndef __AUDIO_GPIO_CTL_H__
#define __AUDIO_GPIO_CTL_H__

#include <linux/mutex.h>

#define AUDIO_GPIO_GROUP_NAME_MAX 10
#define AUDIO_GPIO_MODE_NUM_MAX   2
#define AUDIO_GPIO_MODE_MAX       10
#define AUDIO_GPIO_NAME_MAX       15

#define AUDIO_GPIO_DEFAULT_MODE   0
#define AUDIO_GPIO_ACTIVE_MODE    1

struct audio_gpio_info {
	char group_name[AUDIO_GPIO_GROUP_NAME_MAX];
	char gpio_mode[AUDIO_GPIO_MODE_MAX];
};

#define AUDIO_GPIO_CTRL  _IOW('A', 0x01, struct audio_gpio_info)

struct audio_gpio {
	u8 state;
	int gpio;
};

struct audio_gpio_group {
	const char *group_name;
	int group_id;
	u8 group_mode[AUDIO_GPIO_MODE_NUM_MAX];
	int group_register;
	int cur_mode;
	struct audio_gpio *gpios;
	int gpio_num;
};

struct audio_gpio_ctrl_priv {
	struct mutex do_ioctl_lock;
	struct device *dev;
	struct audio_gpio_group *audio_groups;
	int group_num;
};

#endif  /* __AUDIO_GPIO_CTL_H__ */
