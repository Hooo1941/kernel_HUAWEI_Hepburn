/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 * Description: xpmic driver
 */

#ifndef __XPMIC_REGISTERS_H__
#define __XPMIC_REGISTERS_H__

enum {
	XPMIC_GPIO_PULL_LOW = 0,
	XPMIC_GPIO_PULL_HIGH,
	XPMIC_GPIO_MAX_OPS,
};

struct xpmic_info {
	struct regulator_desc *rdesc;
	struct regulator_dev *rdev;
	struct regulator_dev *real_rdev;
	struct device_node *of_node;
	struct regulator *reg;
	struct device *dev;
};

int xpmic_gpio_ops(int gpio, int ops);

#endif
/* __XPMIC_REGISTERS_H__ */
