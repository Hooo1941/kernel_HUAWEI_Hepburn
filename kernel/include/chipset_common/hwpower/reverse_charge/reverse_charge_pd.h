/*
 * reverse_charge_pd.h
 *
 * reverse charge pd driver
 *
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd.
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

#ifndef _REVERSE_CHARGE_PD_H_
#define _REVERSE_CHARGE_PD_H_

#include <linux/device.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>

struct rchg_pd_ic_ops {
	void *dev_data;
	const char *dev_name;
	int (*enable_reverse_charge)(bool enable, void *dev_data);
};

struct otg_cap {
	u32 otg_vol;
	u32 otg_cur;
};

#ifdef CONFIG_REVERSE_CHARGE_PD
int rchg_pd_ops_register(struct rchg_pd_ic_ops *ops);
void rchg_pd_otg_enable(bool enable);
int rchg_pd_9v_enable(bool enable);
#else
static inline int rchg_pd_ops_register(struct rchg_pd_ic_ops *ops)
{
	return -EPERM;
}

static inline void rchg_pd_otg_enable(bool enable)
{
}

static inline int rchg_pd_9v_enable(bool enable)
{
	return -EPERM;
}
#endif /* CONFIG_REVERSE_CHARGE_PD */

#endif /* _REVERSE_CHARGE_PD_H_ */
