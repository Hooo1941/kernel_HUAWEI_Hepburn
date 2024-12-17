/* SPDX-License-Identifier: GPL-2.0 */
/*
 * ffc_control.h
 *
 * head file for ffc control driver
 *
 * Copyright (c) 2020-2023 Huawei Technologies Co., Ltd.
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

#ifndef _FFC_CONTROL_H_
#define _FFC_CONTROL_H_

struct ffc_ctrl_charge_info {
	bool ffc_charge_flag;
	bool dc_mode;
	int iterm;
};

#ifdef CONFIG_HUAWEI_FFC_CONTROL
void ffc_ctrl_set_default_info(int vterm, int iterm);
int ffc_ctrl_get_incr_vterm(void);
void ffc_ctrl_notify_ffc_info(void);
#else
static inline void ffc_ctrl_set_default_info(int vterm, int iterm)
{
}

static inline int ffc_ctrl_get_incr_vterm(void)
{
	return 0;
}

static inline void ffc_ctrl_notify_ffc_info(void)
{
}
#endif /* CONFIG_HUAWEI_FFC_CONTROL */

#endif /* _FFC_CONTROL_H_ */
