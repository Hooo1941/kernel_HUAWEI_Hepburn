/* SPDX-License-Identifier: GPL-2.0 */
/*
 * power_gpio_misc.h
 *
 * gpio misc interface for power module
 *
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
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

#ifndef _POWER_GPIO_MISC_H_
#define _POWER_GPIO_MISC_H_

#ifdef CONFIG_HUAWEI_POWER_GPIO_MISC
int power_gpio_misc_is_trim_charger(void);
#else
static inline int power_gpio_misc_is_trim_charger(void)
{
	return 0;
}
#endif /* CONFIG_HUAWEI_POWER_GPIO_MISC */

#endif /* _POWER_GPIO_MISC_H_ */
