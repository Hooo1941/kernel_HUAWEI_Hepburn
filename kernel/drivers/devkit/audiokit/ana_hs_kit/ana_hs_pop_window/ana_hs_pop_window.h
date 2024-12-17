/*
 * ana_hs_pop_window.h
 *
 * ana_hs_pop_window driver
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
 */

#ifndef __ANA_HS_POP_WINDOW_H__
#define __ANA_HS_POP_WINDOW_H__

#include "../ana_hs.h"
#include <linux/notifier.h>

struct ana_hs_pop_window_priv {
	struct device *dev;
	struct notifier_block event_nb;
};

#endif  /* __ANA_HS_POP_WINDOW_H__ */
