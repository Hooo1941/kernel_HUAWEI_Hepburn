/* SPDX-License-Identifier: GPL-2.0 */
/*
 * direct_charge_cable.h
 *
 * cable detect for direct charge module
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#ifndef _DIRECT_CHARGE_CABLE_H_
#define _DIRECT_CHARGE_CABLE_H_

#include <linux/errno.h>

enum dc_cable_type {
	DC_UNKNOWN_CABLE,
	DC_NONSTD_CABLE,
	DC_STD_CABLE,
};

enum dc_ui_cable_type {
	DC_UI_UNKNOWN_CABLE,
	DC_UI_NONSTD_ATC_CABLE,
	DC_UI_STD_CABLE,
	DC_UI_NONSTD_CTC_CABLE,
};

/* define cable operator for direct charge */

#define CABLE_DETECT_OK              1
#define CABLE_DETECT_NOK             0

struct dc_cable_ops {
	int (*detect)(void);
	int (*cable_auth_det)(void);
};

enum dc_cable_mode_info {
	DC_CABLE_DETECT_OK,
	DC_CABLE_TYPE,
	DC_IS_CTC_CABLE,
	DC_CTC_CABLE_TYPE,
	DC_UI_CABLE_TYPE,
};

enum dc_cable_inherent_info {
	CABLE_FULL_ATH_RES_THLD,
	CABLE_FULL_PATH_RES,
	MAX_CURR_FOR_NONSTD_CABLE,
	SEC_RES_CHECK_EN,
	SEC_RES_CHECK_OK,
};

#ifdef CONFIG_DIRECT_CHARGER
int dc_cable_ops_register(struct dc_cable_ops *ops);
bool dc_is_support_cable_detect(void);
int dc_cable_detect(void);
unsigned int dc_get_cable_inherent_info(unsigned int type);
unsigned int  dc_get_cable_mode_info(int mode, int type);
void dc_clear_cable_inherent_info_in_exit(void);
void dc_clear_cable_inherent_info(void);
void dc_clear_cable_mode_info(int mode);
int dc_get_cable_max_current(int mode);
void dc_update_cable_max_current(int *cable_max_cur);
void dc_init_cable_curr(int working_mode);
int dc_get_cable_curr(void);
int dc_update_cable_type(void);
void dc_update_cable_detect_ok(void);
void dc_update_ui_cable_type(void);
void dc_update_cable_resistance_thld(int mode);
int dc_calculate_path_resistance(int working_mode, int *rpath);
void dc_calculate_second_path_resistance(int working_mode, int ibus);
int dc_resist_handler(int mode, int value);
void dc_send_cable_type_uevent(int mode);
void dc_cable_init(struct device *dev, int mode);
void dc_cable_remove(struct device *dev, int mode);
#else
static inline int dc_cable_ops_register(struct dc_cable_ops *ops)
{
	return -EINVAL;
}

static inline bool dc_is_support_cable_detect(void)
{
	return false;
}

static inline int dc_cable_detect(void)
{
	return -EINVAL;
}

static inline unsigned int dc_get_cable_inherent_info(unsigned int type)
{
	return 0;
}

static inline unsigned int dc_get_cable_mode_info(int mode, unsigned int type)
{
	return 0;
}

static inline void dc_clear_cable_inherent_info_in_exit(void)
{
}

static inline void dc_clear_cable_inherent_info(void)
{
}

static inline void dc_clear_cable_mode_info(int mode)
{
}

static inline int dc_get_cable_max_current(int mode)
{
	return -EPERM;
}

static inline void dc_update_cable_max_current(int *cable_max_cur)
{
}

static inline void dc_init_cable_curr(int working_mode)
{
}

static inline int dc_get_cable_curr(void)
{
	return 0;
}

static inline int dc_update_cable_type(void)
{
	return -EPERM;
}

static inline void dc_update_cable_detect_ok(void)
{
}

static inline void dc_update_ui_cable_type(void)
{
}

static inline void dc_update_cable_resistance_thld(int mode)
{
}

static inline int dc_resist_handler(int mode, int value)
{
	return -EPERM;
}

static inline int dc_calculate_path_resistance(int working_mode, int *rpath)
{
	return -EPERM;
}

static inline void dc_calculate_second_path_resistance(int working_mode, int ibus)
{
}

static inline void dc_send_cable_type_uevent(int mode)
{
}

static inline void dc_cable_init(struct device *dev, int mode)
{
}

static inline void dc_cable_remove(struct device *dev, int mode)
{
}
#endif /* CONFIG_DIRECT_CHARGER */

#endif /* _DIRECT_CHARGE_CABLE_H_ */
