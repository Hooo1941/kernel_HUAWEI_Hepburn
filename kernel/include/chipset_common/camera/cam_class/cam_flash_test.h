/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * Description: camera flash test interface
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

#ifndef _CAM_FLASH_TEST_H_
#define _CAM_FLASH_TEST_H_

#include <linux/device.h>

struct flashlight_test_operations {
	struct device *dev;
	void (*cam_torch_turn_on)(struct device *dev);
	void (*cam_torch_turn_off)(struct device *dev);
};

enum flashlight_pos {
	FLASH_LIGHT_BACK,
	FLASH_LIGHT_FRONT,
	MAX_FLASH_LIGHT_POS, /* warn!!! you must add new type before this item */
};

enum flash_ctrl_type {
	PMIC_CTRL_TYPE,
	I2C_CTRL_TYPE,
	GPIO_CTRL_TYPE,
	MAX_CTRL_TYPE, /* warn!!! you must add new type before this item */
};

int cam_flashlight_classdev_register(struct flashlight_test_operations *ops,
	enum flashlight_pos pos, enum flash_ctrl_type ctrl_ype);

int flashlight_test_init(void);
#endif /* _CAM_FLASH_TEST_H_ */