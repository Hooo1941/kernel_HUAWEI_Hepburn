/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: function declaration for tee livepatch
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

#ifndef LIVEPATCH_CMD_H
#define LIVEPATCH_CMD_H

#include <linux/device.h>

#ifdef CONFIG_LIVEPATCH_ENABLE
int livepatch_init(const struct device *dev);
void livepatch_down_read_sem(void);
void livepatch_up_read_sem(void);
void free_livepatch(void);
#endif

#endif
