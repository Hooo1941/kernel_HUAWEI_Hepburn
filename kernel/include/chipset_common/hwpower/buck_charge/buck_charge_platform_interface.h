/*
 * buck_charge_platform_interface.h
 *
 * Copyright (C) 2023-2023 Huawei Technologies Co., Ltd.
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

#ifndef _BUCK_CHARGE_PLATFORM_INTERFACE_
#define _BUCK_CHARGE_PLATFORM_INTERFACE_

#define BUCK_CHARGE_DCP_INPUT_CURRENT                  2000
#define BUCK_CHARGE_DCP_IBAT_CURRENT                   3000
#define BUCK_CHARGE_DEFAULT_IIN_THL                    2300
#define BUCK_CHARGE_CHGDIEID_NV_LEN                    9
#define BUCK_CHARGE_CHGDIEID_LEN                       8
#define BUCK_CHARGE_CLEAR_CHARGER_CVCAL                1
#ifdef CONFIG_HLTHERM_RUNTEST
#define BUCK_CHARGE_HLTHERM_CURRENT                    2000
#endif

enum buck_charge_core_type {
	BUCK_CHARGE_CORE_VTERM_RT = 0,
	BUCK_CHARGE_CORE_IIN_MAX,
	BUCK_CHARGE_CORE_ICHG_MAX,
	BUCK_CHARGE_CORE_ITERM,
	BUCK_CHARGE_CORE_VTERM,
	BUCK_CHARGE_CORE_END,
};

#if (defined(CONFIG_HUAWEI_CHARGER_AP) || defined(CONFIG_HUAWEI_CHARGER))
unsigned int buck_charge_get_charge_core_data(int info_name);
int buck_charge_get_vsys(void);
int buck_charge_get_hiz_mode(int hiz_mode);
int buck_charge_get_mains_online(void);
int buck_charge_get_plugusb(char *buf);
void buck_charge_set_sdp_iin_limit(unsigned int val);
void buck_charge_set_ichg_thermal(unsigned int val, unsigned int ichg_thl);
void buck_charge_set_ichg_runningtest(unsigned int ichg_rt);
void buck_charge_set_vterm_rt(unsigned int vterm_rt);
void buck_charge_set_batfet_disable(long val, unsigned int batfet_disable);
void buck_charge_set_inputcurrent(unsigned int inputcurrent);
void buck_charge_set_charger_cvcal(long val, unsigned int *charger_cvcal_value);
void buck_charge_set_charger_cvcal_clear(long val, unsigned int *charger_cvcal_clear);
int buck_charge_set_charge_en_flag(unsigned int val, unsigned int *charge_limit);
int buck_charge_set_charge_en(unsigned int val, unsigned int charge_enable);
int buck_charge_set_iin_limit(unsigned int val, unsigned int *inputcurrent);
void buck_charge_set_iin_thermal(unsigned int val, unsigned int iin_max, unsigned int *iin_thl_idx);
int buck_charge_set_ichg_limit(unsigned int val);
int buck_charge_set_iin_limit_array(unsigned int val);
int buck_charge_set_vterm_dec(unsigned int val);
void buck_charge_set_iin_runningtest(unsigned int val, unsigned int iin_max, unsigned int *iin_rt);
void buck_charge_set_iin_rt_current(unsigned int val, int *iin_rt_curr);
int buck_charge_set_iin_thermal_limit(unsigned int value);
void buck_charge_set_plugusb(const char *buf);
void buck_charge_set_adaptor_voltage(unsigned int val);
void buck_charge_set_chrg_config(unsigned int val, int *chr_config);
void buck_charge_set_factory_diag(unsigned int val);
void buck_charge_set_vr_charge_type(void);
void buck_charge_set_charge_core_iin_usb(unsigned int val);
int buck_charge_init_charge_core_data(void);
#else
static inline unsigned int buck_charge_get_charge_core_data(int info_name)
{
	return 0;
}

static inline int buck_charge_get_vsys(void)
{
	return 0;
}

static inline int buck_charge_get_hiz_mode(int hiz_mode)
{
	return 0;
}

static inline int buck_charge_get_mains_online(void)
{
	return 0;
}

static inline int buck_charge_get_plugusb(char *buf)
{
	return 0;
}

static inline void buck_charge_set_sdp_iin_limit(unsigned int val)
{
}

static inline void buck_charge_set_ichg_thermal(unsigned int val, unsigned int ichg_thl)
{
}

static inline void buck_charge_set_ichg_runningtest(unsigned int ichg_rt)
{
}

static inline void buck_charge_set_vterm_rt(unsigned int vterm_rt)
{
}

static inline void buck_charge_set_batfet_disable(long val, unsigned int batfet_disable)
{
}

static inline void buck_charge_set_inputcurrent(unsigned int inputcurrent)
{
}

static inline void buck_charge_set_charger_cvcal(long val, unsigned int *charger_cvcal_value)
{
}

static inline void buck_charge_set_charger_cvcal_clear(long val, unsigned int *charger_cvcal_clear)
{
}

static inline int buck_charge_set_charge_en_flag(unsigned int val, unsigned int *charge_limit)
{
	return 0;
}

static inline int buck_charge_set_charge_en(unsigned int val, unsigned int charge_enable)
{
	return 0;
}

static inline int buck_charge_set_iin_limit(unsigned int val, unsigned int *inputcurrent)
{
	return 0;
}

static inline void buck_charge_set_iin_thermal(unsigned int val, unsigned int iin_max,
	unsigned int *iin_thl_idx)
{
}

static inline int buck_charge_set_ichg_limit(unsigned int val)
{
	return 0;
}

static inline int buck_charge_set_iin_limit_array(unsigned int val)
{
	return 0;
}

static inline int buck_charge_set_vterm_dec(unsigned int val)
{
	return 0;
}

static inline void buck_charge_set_iin_runningtest(unsigned int val, unsigned int iin_max, unsigned int *iin_rt)
{
}

static inline void buck_charge_set_iin_rt_current(unsigned int val, int *iin_rt_curr)
{
}

static inline int buck_charge_set_iin_thermal_limit(unsigned int value)
{
	return 0;
}

static inline void buck_charge_set_plugusb(const char *buf)
{
}

static inline void buck_charge_set_adaptor_voltage(unsigned int val)
{
}

static inline void buck_charge_set_chrg_config(unsigned int val, int *chr_config)
{
}

static inline void buck_charge_set_factory_diag(unsigned int val)
{
}

static inline void buck_charge_set_vr_charge_type(void)
{
}

static inline void buck_charge_set_charge_core_iin_usb(unsigned int val)
{
}

static inline int buck_charge_init_charge_core_data(void)
{
	return 0;
}
#endif /* CONFIG_HUAWEI_CHARGER_AP || CONFIG_HUAWEI_CHARGER */

#endif /* _BUCK_CHARGE_PLATFORM_INTERFACE_ */
