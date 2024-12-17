/*
 * direct_charge_pmode.h
 *
 * direct charge pmode driver
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
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

#ifndef _DIRECT_CHARGE_PMODE_H_
#define _DIRECT_CHARGE_PMODE_H_

enum dc_dis_flag {
	DC_EN = 0,
	DC_DIS_BY_CHRG_DONE = BIT(0),
	DC_DIS_BY_PRIORITY = BIT(1),
	DC_DIS_BY_IBAT_TH_LOW = BIT(2),
	DC_DIS_BY_SYSFS_DISABLE = BIT(3),
};

enum pmode_check_type {
	DEFAULT_CHECK,
	QUICK_CHECK,
	NORMAL_CHECK,
};

#ifdef CONFIG_DIRECT_CHARGER
void dc_pmode_clear_data(void);
void dc_pmode_clear_data_in_exit(void);
void dc_pmode_enable_mode(unsigned int flag, unsigned int mode);
void dc_pmode_disable_mode(unsigned int flag, unsigned int mode);
int dc_pmode_init_local_mode(void);
int dc_pmode_init_adp_mode(unsigned int *mode);
void dc_pmode_update_mode_power(void);
int dc_pmode_get_max_cur_adp(void);
int dc_pmode_get_max_product_power(void);
bool dc_pmode_is_cable_limit_power(void);
unsigned int dc_pmode_get_optimal_mode(int type, int mode, bool cur_flag);
int dc_pmode_get_optimal_mode_max_pwr(int type);
int dc_pmode_get_optimal_mode_max_cur(int type);
#else
static inline void dc_pmode_clear_data(void)
{
}

static inline void dc_pmode_clear_data_in_exit(void)
{
}

static inline void dc_pmode_enable_mode(unsigned int flag, unsigned int mode)
{
}

static inline void dc_pmode_disable_mode(unsigned int flag, unsigned int mode)
{
}

static inline int dc_pmode_init_local_mode(void)
{
	return -EINVAL;
}

static inline int dc_pmode_init_adp_mode(unsigned int *mode)
{
	return -EINVAL;
}

static inline int dc_pmode_get_max_cur_adp(void)
{
	return 0;
}

static inline int dc_pmode_get_max_product_power(void)
{
	return 0;
}

static inline bool dc_pmode_is_cable_limit_power(void)
{
	return false;
}

static inline void dc_pmode_update_mode_power(void)
{
}

static inline unsigned int dc_pmode_get_optimal_mode(int type, int mode, bool cur_flag)
{
	return UNDEFINED_MODE;
}

static inline int dc_pmode_get_optimal_mode_max_pwr(int type)
{
	return 0;
}

static inline int dc_pmode_get_optimal_mode_max_cur(int type);
{
	return 0;
}
#endif /* CONFIG_DIRECT_CHARGER */

#endif /* _DIRECT_CHARGE_PMODE_H_ */
