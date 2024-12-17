// SPDX-License-Identifier: GPL-2.0
/*
 * direct_charge_control.h
 *
 * control module for direct charge
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

#ifndef _DIRECT_CHARGE_CONTROL_H_
#define _DIRECT_CHARGE_CONTROL_H_

#ifdef CONFIG_DIRECT_CHARGER
void dc_ctrl_init(struct device *dev, int mode);
void dc_ctrl_remove(struct device *dev, int mode);
void dc_preparation_before_switch_to_singlepath(int working_mode, int ratio, int vdelt);
void dc_preparation_before_stop(void);
void dc_charge_regulation(void);
void dc_ctrl_fill_eh_buf_and_set_err_flag_true(int type, const char *str);
void dc_ctrl_revise_stage(int *cur_stage);
int dc_ctrl_start_charging(int mode);
time64_t dc_ctrl_get_start_time(void);
bool dc_ctrl_is_ibat_abnormal(int mode, int ibat_abnormal_th, int *cnt);
#else
static inline void dc_ctrl_init(struct device *dev, int mode)
{
}

static inline void dc_ctrl_remove(struct device *dev, int mode)
{
}

static inline void dc_preparation_before_switch_to_singlepath(int working_mode, int ratio, int vdelt)
{
}

static inline void dc_preparation_before_stop(void)
{
}

static inline void dc_charge_regulation(void)
{
}

static inline void dc_ctrl_fill_eh_buf_and_set_err_flag_true(int type, const char *str)
{
}

static inline void dc_ctrl_revise_stage(int *cur_stage)
{
}

static inline int dc_ctrl_start_charging(void)
{
	return -EPERM;
}

static inline time64_t dc_ctrl_get_start_time(void)
{
	return 0;
}

static inline bool dc_ctrl_is_ibat_abnormal(int mode, int ibat_abnormal_th, int *cnt)
{
	return false;
}
#endif /* CONFIG_DIRECT_CHARGER */

#endif /* _DIRECT_CHARGE_CONTROL_H_ */
