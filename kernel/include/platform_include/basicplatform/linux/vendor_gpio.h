/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 * Description: provide gpio access function interfaces.
 * Create: 2021-01-01
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
#ifndef __VENDOR_GPIO_H__
#define __VENDOR_GPIO_H__

#include <linux/spinlock_types.h>

raw_spinlock_t *get_gpio_lock_by_id(int id);

#endif
