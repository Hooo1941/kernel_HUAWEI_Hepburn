/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: provide gpio hook register interfaces.
 * Create: 2022-03-15
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/idr.h>
#include <platform_include/basicplatform/linux/hck/gpio/hck_gpio.h>
#include "gpiolib.h"

void hck_gpio_find_base(struct list_head *gpio_devices, int ngpio, int *base)
{
	struct gpio_device *gdev = NULL;
	*base = 0;

	list_for_each_entry_reverse(gdev, gpio_devices, list) {
		if (gdev->base + (int)gdev->ngpio <= *base) {
			break;
		} else {
			/* nope, check the space right before the chip */
			*base = gdev->base + ngpio;
			if (*base > ARCH_NR_GPIOS) {
				*base = -ENOSPC;
				return;
			}
		}
	}

	if (gpio_is_valid(*base))
		pr_debug("%s: found new base at %d\n", __func__, *base);
	else
		pr_err("%s: cannot find free range\n", __func__);

	return;
}
void hck_get_gpio_id(struct ida *gpio_ida, int base, int ngpio, int *id)
{
	if(base < 0)
		*id = ida_alloc(gpio_ida, GFP_KERNEL);
	else
		*id = ida_simple_get(gpio_ida, base / ngpio, base / ngpio + 1, GFP_KERNEL);
}
static int hook_gpio_init(void)
{
	REGISTER_HCK_VH(gpio_find_base, hck_gpio_find_base);
	REGISTER_HCK_VH(get_gpio_id, hck_get_gpio_id);
	return 0;
}
early_initcall(hook_gpio_init);