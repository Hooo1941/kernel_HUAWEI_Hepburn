/* SPDX-License-Identifier: GPL-2.0 */
/*
 * buck_charge.h
 *
 * buck charge module
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#ifndef _BUCK_CHARGE_H_
#define _BUCK_CHARGE_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/workqueue.h>
#include <chipset_common/hwpower/buck_charge/buck_charge_jeita.h>

#define BUCK_CHARGE_WORK_TIMEOUT            10000

struct buck_charge_dev {
	struct device *dev;
	u32 jeita_support;
	u32 force_term_support;
	u32 check_full_count;
	u32 vterm;
	u32 iterm;
	u32 ibus_limit_after_chg_done;
	bool charging_on;
	struct delayed_work buck_charge_work;
	struct work_struct stop_charge_work;
	struct notifier_block event_nb;
	struct notifier_block chg_event_nb;
	struct bc_jeita_para jeita_table[BC_JEITA_PARA_LEVEL];
	struct bc_jeita_result jeita_result;
};

#endif /* _BUCK_CHARGE_H_ */
