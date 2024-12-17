/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2015-2020. All rights reserved.
 * Description: coulometer hardware driver headfile
 *
 * This software is licensed under the terms of the GNU General Public
 * License, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <securec.h>
#include <platform_include/basicplatform/linux/power/platform/coul/coul_private_interface.h>
#include <platform_include/basicplatform/linux/spmi_platform.h>
#include <platform_include/basicplatform/linux/of_platform_spmi.h>
#include "../../coul_algo/coul_nv.h"
#include "hi6xxx_coul.h"

struct coul_device_info *g_coul_hardware_dev;
static u64 g_last_eco_in;
static u64 g_last_eco_out;
static int g_saved_abs_cc_mah;
static int g_r_coul_uohm = R_COUL_UOHM;

#if defined(CONFIG_COUL_PMIC6X21V700) || \
	defined(CONFIG_COUL_PMIC6X21V600) || \
	defined(CONFIG_COUL_PMIC6X55V200)
#else
static int g_soh_adc_a = DEFAULT_SOH_ADC_A;
static int g_soh_adc_b;
#endif

static void coul_cc_write_pro(u8 lock)
{
#if defined(CONFIG_COUL_PMIC6X21V700) || \
	defined(CONFIG_COUL_PMIC6X21V600) || \
	defined(CONFIG_COUL_PMIC6X55V200)
#else
	u8 val = COUL_WRITE_LOCK;

	if (lock == UNLOCK)
		val = COUL_WRITE_UNLOCK;

	coul_reg_write(COUL_HARDWARE_DEBUG_WRITE_PRO, val);
#endif
}

/*
 * set coul nv save success flag
 * Input: nv_flag: success 1, fail 0
 * Remark: the flag is read by fastboot
 */
void coul_set_nv_save_flag(int batt, int nv_flag)
{
	unsigned char val;

	val = coul_reg_read(COUL_HARDWARE_NV_SAVE_SUCCESS);
	if (nv_flag == NV_SAVE_SUCCESS)
		coul_reg_write(
			COUL_HARDWARE_NV_SAVE_SUCCESS, (val | NV_SAVE_BITMASK));
	else
		coul_reg_write(
			COUL_HARDWARE_NV_SAVE_SUCCESS, (val & (~NV_SAVE_BITMASK)));
}

/*
 * Description: get coul nv read success flag
 * Return:  success:1 fail:0
 * Remark:  the flag is written by fastboot
 */
int coul_get_nv_read_flag(int batt)
{
	unsigned char val;

	val = coul_reg_read(COUL_HARDWARE_NV_READ_SUCCESS);
	if (val & NV_READ_BITMASK)
		return NV_READ_SUCCESS;
	return NV_READ_FAIL;
}

/*
 * Return: 1:saved ocv, 0:not saved ocv
 * Remark:  the flag is written by fastboot
 */
int coul_get_use_saved_ocv_flag(int batt)
{
	unsigned char val;

	val = coul_reg_read(COUL_HARDWARE_OCV_CHOOSE);
	if (val & USE_SAVED_OCV_FLAG)
		return 1;
	return 0;
}

/*
 * Return: 1:not calculate delta rc,  0:calculate delta rc
 * Remark: the flag is written by fastboot
 */
int coul_get_delta_rc_ignore_flag(int batt)
{
	unsigned char use_delta_rc_flag;

	use_delta_rc_flag = coul_reg_read(COUL_HARDWARE_DELTA_RC_SCENE);
	if (use_delta_rc_flag & DELTA_RC_SCENE_BITMASK) {
		use_delta_rc_flag &=
			~DELTA_RC_SCENE_BITMASK; /* clear the flag after read */
		coul_reg_write(COUL_HARDWARE_DELTA_RC_SCENE, use_delta_rc_flag);
		return 1;
	}
	return 0;
}

/* Return: 0:not moved, 1: moved */
int coul_is_battery_moved(int batt)
{
	unsigned char val;

	val = coul_reg_read(COUL_HARDWARE_BATTERY_MOVE_ADDR);
	if (val == BATTERY_MOVE_MAGIC_NUM) {
		coul_hardware_inf("Battey not moved\n");
		return 0;
	}
	coul_hardware_inf("Battey moved\n");
	coul_reg_write(COUL_HARDWARE_BATTERY_MOVE_ADDR, BATTERY_MOVE_MAGIC_NUM);
	return 1;
}

/*
 * Description: set battery move magic num
 * Input: move flag 1:plug out 0:plug in
 */
void coul_set_battery_move_magic(int batt, int move_flag)
{
	unsigned char val;

	if (move_flag)
		val = BATTERY_PLUGOUT_SHUTDOWN_MAGIC_NUM;
	else
		val = BATTERY_MOVE_MAGIC_NUM;
	coul_reg_write(COUL_HARDWARE_BATTERY_MOVE_ADDR, val);
}

static int coul_get_fifo_depth(void)
{
	return FIFO_DEPTH;
}

/*
 * Description: get coulomb total(in and out) time
 * Return:  sum of total time
 */
static unsigned int coul_get_coul_time(int batt)
{
	unsigned int cl_in_time = 0;
	unsigned int cl_out_time = 0;

	udelay(110); /* 110: delay time, us */
	coul_regs_read(COUL_HARDWARE_CHG_TIMER_BASE, &cl_in_time, 4); /* 4: regs_num */
	coul_regs_read(COUL_HARDWARE_LOAD_TIMER_BASE, &cl_out_time, 4); /* 4: regs_num */

	return (cl_in_time + cl_out_time);
}

/* Description: clear coulomb total(in and out) time */
static void coul_hardware_clear_coul_time(int batt)
{
	unsigned int cl_time = 0;

	udelay(110); /* 110: delay time, us */
	coul_regs_write(COUL_HARDWARE_CHG_TIMER_BASE, &cl_time, 4); /* 4: regs_num */
	coul_regs_write(COUL_HARDWARE_LOAD_TIMER_BASE, &cl_time, 4); /* 4: regs_num */
}

static void coul_cali_params_get(int *v_offset_a, int *v_offset_b,
	int *c_offset_a, int *c_offset_b)
{
	struct coul_device_info *di = g_coul_hardware_dev;
	struct coul_cali_params *params = NULL;

	if (di == NULL)
		return;

	params = &di->cali_params;

	/* The parameter may be is null */
	if(v_offset_a)
		*v_offset_a = params->v_offset_a;

	if(v_offset_b)
		*v_offset_b = params->v_offset_b;

	if(c_offset_a)
		*c_offset_a = params->c_offset_a;

	if(c_offset_b)
		*c_offset_b = params->c_offset_b;
}

/*
 * Description:  convert register value to uv
 * Input: reg_val:voltage reg value
 * Return: value of register in uV
 */
static int coul_convert_ocv_regval2uv(int batt, short reg)
{
	s64 temp;
	unsigned short reg_val;
	int v_offset_a = DEFAULT_V_OFF_A;
	int v_offset_b = DEFAULT_V_OFF_B;

	coul_cali_params_get(&v_offset_a, &v_offset_b, NULL, NULL);

	reg_val = (unsigned short)reg;
	if (reg_val & INVALID_TO_UPDATE_FCC)
		reg_val &= (~INVALID_TO_UPDATE_FCC);

#if defined(CONFIG_COUL_PMIC6X21V600) || defined(CONFIG_COUL_PMIC6X55V200)
	/*
	 * 1 bit = 225.1196 uv = 4.8/21322 V = 4.8 /21322 * 1000 * 1000 uV
	 *    = 24 * 1000 * 100 / 10661 uV
	 * 1000000: gain to ensure calculation accuracy
	 */
	temp = (s64)((s64)(reg_val) * (s64)(24 * 1000 * 100));

	temp = div_s64(temp, 10661);

	temp = (s64)v_offset_a * temp;
	temp = div_s64(temp, 1000000);
	temp += v_offset_b;

#else
	/*
	 * code(15bit) * 1.3 * 5 * a / 2^15 + b
	 *  = code * 13 *a / 2^16 + b (a = 1000000, b = 0)
	 *  1bit = 198.364 uV (High 16bit)
	 */
	temp = (s64)reg_val * 13;
	temp = temp * (s64)(v_offset_a);
	temp = div_s64(temp, 65536);
	temp += v_offset_b;
#endif
	return (int)temp;
}

/* Description: convert uv value to Bit for register */
static unsigned short coul_convert_ocv_uv2regval(int uv_val)
{
	unsigned short ret;
	s64 temp;
	int v_offset_a = DEFAULT_V_OFF_A;
	int v_offset_b = DEFAULT_V_OFF_B;

	coul_cali_params_get(&v_offset_a, &v_offset_b, NULL, NULL);

#if defined(CONFIG_COUL_PMIC6X21V600) || defined(CONFIG_COUL_PMIC6X55V200)
	/*
	 *  1 bit = 225.1196 uv = 4.8/21322 V
	 *      = 4.8/21322 * 1000 * 1000 uV
	 *      =10661 /24 * 1000 * 100 uV
	 * 1000000: gain to ensure calculation accuracy
	 */
	temp = (s64)(uv_val - v_offset_b);
	temp = temp * 1000000;
	if (v_offset_a != 0) {
		temp = div_s64(temp, v_offset_a);
	} else {
		coul_hardware_err("v_offset_a = 0, division by default value %d\n",
			DEFAULT_V_OFF_A);
		temp = div_s64(temp, DEFAULT_V_OFF_A);
	}

	temp = (s64)(temp * (s64)10661);
	temp = div_s64(temp, 2400000);

	ret = (short)temp;
#else
	/*
	 * uv_val = code(15bit) * 1.3 * 5 *a / 2^15 + b
	 *     = code * 13 *a / 2^16 + b
	 */
	temp = (s64)(uv_val - v_offset_b) * 65536;
	temp = div_s64(temp, 13);
	if (v_offset_a != 0) {
		temp = div_s64(temp, v_offset_a);
	} else {
		coul_hardware_err("v_offset_a = 0, division by default value %d\n",
			DEFAULT_V_OFF_A);
		temp = div_s64(temp, DEFAULT_V_OFF_A);
	}
	ret = (unsigned short)temp;
#endif
	return ret;
}

/* Description: convert register value to current(uA) */
static int coul_convert_ocv_regval2ua(int batt, short reg_val)
{
	int ret;
	s64 temp;
	int c_offset_a = DEFAULT_C_OFF_A;
	int c_offset_b = DEFAULT_C_OFF_B;

	coul_cali_params_get(NULL, NULL, &c_offset_a, &c_offset_b);

#if defined(CONFIG_COUL_PMIC6X21V600) || defined(CONFIG_COUL_PMIC6X55V200)
	/*
	 * 10 mohm resistance: 1 bit = 5 /10661 A = 5 * 1000 * 1000 / 10661 uA
	 * 20 mohm resistance: 1 bit = 10 mohm / 2
	 * 30 mohm resistance: 1 bit = 10 mohm / 3
	 * high bit = 1 is in, 0 is out
	 * 1000000: gain to ensure calculation accuracy
	 */
	ret = reg_val;
	temp = (s64)(ret) * (s64)(1000 * 1000 * 5);
	temp = div_s64(temp, 10661);

	temp = 10 * temp * 1000 / (s64)g_r_coul_uohm;
	temp = (s64)c_offset_a * temp;
	ret = div_s64(temp, 1000000);
	ret += c_offset_b;

#else
	/*
	 * 6x21V700:Current
	 *   = code * / 2^(n-1) * 1.3 * (1000/10) * (1000/Rsense)
	 *   = code *130000 / Rsense / 2^(n-1)
	 * 6x21V800:Current
	 *   = code * / 2^(n-1) * 0.65 * (1000/10) * (1000/Rsense)
	 *   = code *130000 / Rsense / 2^(n-1)
	 *   if Rsense in mohm, Current in mA
	 *   if Rsense in uohm, Current in uA
	 *   high bit = 1 is in, 0 is out
	 * 1000000: gain to ensure calculation accuracy
	 */
	temp = (s64)reg_val * (s64)130000;
	temp = div_s64(temp * 1000, g_r_coul_uohm);
	temp = temp * 1000; /* mA to uA */
	temp = div_s64(temp, 32768);

	temp = (s64)c_offset_a * temp;
	ret = div_s64(temp, 1000000);
	ret += c_offset_b;

#ifndef CONFIG_COUL_PMIC6X21V700
	ret = ret / 2;
#endif
#endif

	coul_hardware_dbg(
		"[%s] reg_val 0x%lx, ret 0x%lx\n", __func__, reg_val, ret);

	return ret;
}

/*
 * Description:  convert register value to uv
 * Input: reg_val:voltage reg value
 * Return: value of register in uV
 */
static int coul_hardware_convert_regval2uv(int batt, unsigned int reg_val)
{
	s64 temp;
	int v_offset_a = DEFAULT_V_OFF_A;
	int v_offset_b = DEFAULT_V_OFF_B;

#if defined(CONFIG_COUL_PMIC6X21V600) || defined(CONFIG_COUL_PMIC6X55V200)
	coul_cali_params_get(&v_offset_a, &v_offset_b, NULL, NULL);

	if (reg_val & INVALID_TO_UPDATE_FCC)
		reg_val &= (~INVALID_TO_UPDATE_FCC);

	/*
	 * 1 bit = 225.1196 uv = 4.8/21322 V = 4.8/21322 * 1000 * 1000 uV
	 *    = 24 * 1000 * 100/ 10661 uV
	 * 1000000: gain to ensure calculation accuracy
	 */
	temp = (s64)((s64)(reg_val) * (s64)(24 * 1000 * 100));

	temp = div_s64(temp, 10661);

	temp = (s64)v_offset_a * temp;
	temp = div_s64(temp, 1000000);
	temp += v_offset_b;

	coul_hardware_dbg(
		"[%s] reg_val 0x%lx, temp %d\n", __func__, reg_val, temp);

	return (int)temp;

#else
	int val;
	int ret;

	coul_cali_params_get(&v_offset_a, &v_offset_b, NULL, NULL);
	/*
	 * uv_val = code(23bit) * 1.3 * 5 *a / 2^23 + b
	 *     = code * 13 *a / 2^24 + b (a = 1000000, b = 0)
	 * 1bit = 0.77486 uV (Total 24bit)
	 */
	if (reg_val & COUL_BIT_MASK) {
		reg_val |= COUL_NEG_EXPAND;
		val = ((~reg_val) + 1) & (~COUL_BIT_MASK);
	} else {
		val = (int)reg_val;
	}

	temp = (s64)val * 13;
	temp = temp * (s64)(v_offset_a);
	temp = div_s64(temp, 16777216);
	temp += v_offset_b;

	ret = (int)temp;

	coul_hardware_dbg(
		"[%s] reg_val 0x%lx, temp %d\n", __func__, reg_val, ret);

	if (reg_val & COUL_BIT_MASK)
		return -ret;
	return ret;
#endif
}

#if defined(CONFIG_COUL_PMIC6X21V600) || defined(CONFIG_COUL_PMIC6X55V200)
#else

/*
 *  Description: convert register value to uv
 *  Input: reg_val:voltage reg value
 *  Output: NULL
 *  Return: value of register
 *  Remark: vol = code(23bit) * 1.3 / 2^23
 *         temp = (vol - 358.68mv )/ 1.3427  (vol = 1.3427 * temp + 358.68)
 *         For 6x21v700/6x21v800
 */
static int coul_hardware_convert_regval2temp(unsigned int reg_val)
{
	s64 temp;
	int val;
	struct coul_device_info *di = g_coul_hardware_dev;

	if (di == NULL) {
		coul_hardware_err("[%s]di is null\n", __func__);
		return -ENODEV;
	}

	if (reg_val & COUL_BIT_MASK) {
		reg_val |= COUL_NEG_EXPAND;
		val = ((~reg_val) + 1) & (~COUL_BIT_MASK);
		val = -val;
	} else {
		val = (int)reg_val;
	}

	/* reg2uv */
	temp = (s64)val * 1300000;
	temp = div_s64(temp, 8388608);

	if (di->chip_version != COUL_PMIC6X21V700)
		temp += 650000;

	/* uv2temp */
	temp = (temp - 358680) * 1000;
	temp = div_s64(temp, 1342700);
	coul_hardware_dbg(
		"[%s] reg_val 0x%lx, temp %d\n", __func__, reg_val, temp);

	return (int)temp;
}
#endif

/*
 * Description:  convert uv value to Bit for register
 * Remark: mv_val = code(23bit) * 1.3 * 5 *a / 2^23 + b  = code * 13 *a /2^24 + b
 */
static unsigned int coul_convert_uv2regval(int batt, int uv_val)
{
	unsigned int ret;
	s64 temp;
	int v_offset_a = DEFAULT_V_OFF_A;
	int v_offset_b = DEFAULT_V_OFF_B;

	coul_cali_params_get(&v_offset_a, &v_offset_b, NULL, NULL);

#if defined(CONFIG_COUL_PMIC6X21V600) || defined(CONFIG_COUL_PMIC6X55V200)
	/*
	 * 1 bit = 225.1196 uv = 4.8/21322 V = 4.8/21322 * 1000 * 1000 uV
	 *    = 24 * 1000 * 100/ 10661 uV
	 * 1000000: gain to ensure calculation accuracy
	 */
	temp = (s64)(uv_val - v_offset_b);
	temp = temp * 1000000;
	if (v_offset_a != 0) {
		temp = div_s64(temp, v_offset_a);
	} else {
		coul_hardware_err("v_offset_a = 0, division by default value %d\n",
			DEFAULT_V_OFF_A);
		temp = div_s64(temp, DEFAULT_V_OFF_A);
	}

	temp = (s64)(temp * (s64)10661);
	temp = div_s64(temp, 2400000);

	ret = (unsigned int)temp;
#else
	/*
	 * uv_val = code(23bit) * 1.3 * 5 *a / 2^23 + b
	 *     = code * 13 *a / 2^24 + b (a = 1000000, b = 0)
	 * 1bit = 0.77486 uV (Total 24bit)
	 */
	temp = (s64)(uv_val - v_offset_b) * 16777216;
	temp = div_s64(temp, 13);
	if (v_offset_a != 0) {
		temp = div_s64(temp, v_offset_a);
	} else {
		coul_hardware_err("v_offset_a = 0, division by default value %d\n",
			DEFAULT_V_OFF_A);
		temp = div_s64(temp, DEFAULT_V_OFF_A);
	}

	ret = (unsigned int)temp;
#endif
	coul_hardware_dbg("[%s] uv_val %d, ret 0x%lx\n", __func__, uv_val, ret);

	return ret;
}

/*
 * Description: convert register value to current(uA)
 * Remark:
 *  if Rsense in mohm, Current in mA
 *  if Rsense in uohm, Current in uA
 *  high bit = 1 is in, 0 is out
 */
static int coul_hardware_convert_regval2ua(int batt, unsigned int reg_val)
{
	int ret;
	s64 temp;
	int c_offset_a = DEFAULT_C_OFF_A;
	int c_offset_b = DEFAULT_C_OFF_B;

#if defined(CONFIG_COUL_PMIC6X21V600) || defined(CONFIG_COUL_PMIC6X55V200)
	coul_cali_params_get(NULL, NULL, &c_offset_a, &c_offset_b);
	/*
	 * Current = code * 1000 * 1000 * 5  * 10 /Rsense / 10661
	 * 1000000: gain to ensure calculation accuracy
	 */
	ret = (short)reg_val;
	temp = (s64)(ret) * (s64)(1000 * 1000 * 5);
	temp = div_s64(temp, 10661);

	temp = 10 * temp * 1000 / (s64)g_r_coul_uohm;
	temp = (s64)c_offset_a * temp;
	ret = div_s64(temp, 1000000);
	ret += c_offset_b;

	coul_hardware_dbg("[%s] h%d reg_val 0x%lx, ret %d\n", __func__, __LINE__,
			reg_val, ret);
	return ret;
#else
	int val;
	/*
	 *  6x21V700:Current = code * / 2^(n-1) * 1.3 * (1000/10) * (1000/Rsense)
	 *                 = code *130000 / Rsense / 2^(n-1)
	 *  6x21V800:Current = code * / 2^(n-1) * 0.65 * (1000/10) * (1000/Rsense)
	 *                 = code *130000 / Rsense / 2^(n-1)
	 * 1000000: gain to ensure calculation accuracy
	 */
	coul_cali_params_get(NULL, NULL, &c_offset_a, &c_offset_b);
	if (reg_val & COUL_BIT_MASK) {
		reg_val |= COUL_NEG_EXPAND;
		val = ((~reg_val) + 1) & (~COUL_BIT_MASK);
	} else {
		val = (int)reg_val;
	}

	temp = (s64)val * 130000;
	temp = div_s64(temp * 1000, g_r_coul_uohm);
	temp = temp * 1000;
	temp = div_s64(temp, 8388608);

	temp = (s64)c_offset_a * temp;
	ret = div_s64(temp, 1000000);
	ret += c_offset_b;

#ifndef CONFIG_COUL_PMIC6X21V700
	ret = ret / 2;
#endif

	coul_hardware_dbg("[%s] h%d reg_val 0x%lx, ret %d\n", __func__, __LINE__,
		reg_val, ret);

	if (reg_val & COUL_BIT_MASK)
		return -ret;

	return ret;
#endif
}

/*
 * Description: convert register value to uah
 * Input: reg_val:cc reg val
 * Return: uah value of reg
 */
static s64 coul_hardware_convert_regval2uah(int batt, u64 reg_val)
{
	s64 ret;
	s64 temp;
	int c_offset_a = DEFAULT_C_OFF_A;

#if defined(CONFIG_COUL_PMIC6X21V600)
	coul_cali_params_get(NULL, NULL, &c_offset_a, NULL);

	/*
	* temp * 10 * BIT_FOR_UAH_DCXO_586  / r_coul_mohm  * 10E-7   (uAh)
	* 1000000: gain to ensure calculation accuracy
	*/
	temp = (s64)reg_val;
	temp = 10 * temp * 1000 / (s64)g_r_coul_uohm;
	temp = temp * BIT_FOR_UAH_DCXO_586;
	temp = (s64)div_s64(temp, 10000000);

	temp = (s64)c_offset_a * temp;
	ret = div_s64(temp, 1000000);

#elif defined(CONFIG_COUL_PMIC6X55V200)
	u8 coul_clk_mode;

	/*
	 * temp * BIT_FOR_UAH_32K  / r_coul_mohm  * 10E-7   (uAh)
	 * temp * BIT_FOR_UAH_DCXO_586  / r_coul_mohm  * 10E-7   (uAh)
	 * 1000000: gain to ensure calculation accuracy
	 */
	coul_cali_params_get(NULL, NULL, &c_offset_a, NULL);
	temp = reg_val;
	temp = 10 * temp * 1000 / (s64)g_r_coul_uohm;
	coul_clk_mode = coul_reg_read(COUL_CLK_MODE_ADDR);
	if ((coul_clk_mode & XO32K_MODE_MSK) == COUL_32K_CLK_MODE)
		temp = temp * BIT_FOR_UAH_32K;
	else
		temp = temp * BIT_FOR_UAH_DCXO_586;
	temp = (s64)div_s64(temp, 10000000);

	temp = (s64)c_offset_a * temp;
	ret = div_s64(temp, 1000000);

#else
	/*
	 * 6x21V700: temp * 10763  / r_coul_mohm  * 10E-10   (mAh)
	 *         temp * 10763  / r_coul_mohm  * 10E-7   (uAh)
	 * 6x21V800: temp * 5381.5  / r_coul_mohm  * 10E-10   (mAh)
	 * 1000000: gain to ensure calculation accuracy
	 */
	coul_cali_params_get(NULL, NULL, &c_offset_a, NULL);
	temp = (s64)reg_val;
	temp = temp * 10763 * 10 / ((s64)g_r_coul_uohm / 100);
	temp = (s64)div_s64(temp, 10000000);

	temp = (s64)c_offset_a * temp;
	ret = div_s64(temp, 1000000);

#ifndef CONFIG_COUL_PMIC6X21V700
	ret = ret / 2;
#endif
#endif
	coul_hardware_dbg("[%s] reg_val 0x%lx, ret %d\n", __func__, reg_val, ret);

	return ret;
}
#if defined(CONFIG_COUL_PMIC6X21V600)
static u64 coul_convert_uah2regval(unsigned int uah)
{
	u64 ret;
	u64 temp;
	int c_offset_a = DEFAULT_C_OFF_A;

	/*
	 * 1bit = 1bit current * 0.11 c = 5/10661 * 11/100 c
	 *   = 5/10661 * 11/100 * 1000/3600 mAh
	 *   = 11 / (10661*2*36) mAh = 11 * 1000/ (10661 *2 *36) uAh
	 *   = 11 * 125/ (10661* 9) uAh
	 * 1000000: gain to ensure calculation accuracy
	 */
	coul_cali_params_get(NULL, NULL, &c_offset_a, NULL);
	temp = uah;
	temp = temp * 1000000;

	/* temp * 10 * BIT_FOR_UAH_DCXO_586  / r_coul_mohm  * 10E-7   (uAh) */
	if (c_offset_a != 0) {
		temp = div_s64(temp, c_offset_a);
	} else {
		coul_hardware_err("c_offset_a = 0, division by default value %d\n",
			DEFAULT_C_OFF_A);
		temp = div_s64(temp, DEFAULT_C_OFF_A);
	}

	temp = temp * 10000000;
	temp = div_s64(temp, BIT_FOR_UAH_DCXO_586);
	ret = (int)(temp * (s64)g_r_coul_uohm / 1000 / 10);

	coul_hardware_dbg("[%s] uah %d, ret 0x%lx\n", __func__, uah, ret);

	return ret;
}
#elif defined(CONFIG_COUL_PMIC6X55V200)
static u64 coul_convert_uah2regval(unsigned int uah)
{
	u64 ret;
	u64 temp;
	int c_offset_a = DEFAULT_C_OFF_A;
	u8 coul_clk_mode;

	/*
	 * temp * BIT_FOR_UAH_32K  / r_coul_mohm  * 10E-7   (uAh)
	 * temp * BIT_FOR_UAH_DCXO_586  / r_coul_mohm  * 10E-7   (uAh)
	 * 1000000: gain to ensure calculation accuracy
	 */
	coul_cali_params_get(NULL, NULL, &c_offset_a, NULL);
	temp = uah;
	temp = temp * 1000000;
	if (c_offset_a != 0) {
		temp = div_s64(temp, c_offset_a);
	} else {
		coul_hardware_err("c_offset_a = 0, division by default value %d\n",
			DEFAULT_C_OFF_A);
		temp = div_s64(temp, DEFAULT_C_OFF_A);
	}

	temp = temp * 10000000;
	coul_clk_mode = coul_reg_read(COUL_CLK_MODE_ADDR);
	if ((coul_clk_mode & XO32K_MODE_MSK) == COUL_32K_CLK_MODE)
		temp = div_s64(temp, BIT_FOR_UAH_32K);
	else
		temp = div_s64(temp, BIT_FOR_UAH_DCXO_586);
	ret = (int)(temp * g_r_coul_uohm / 1000 / 10);

	coul_hardware_dbg("[%s] uah %d, ret 0x%lx\n", __func__, uah, ret);

	return ret;
}
#else
/* Description: convert uah value to reg */
static u64 coul_convert_uah2regval(unsigned int uah)
{
	u64 ret;
	u64 temp;
	int c_offset_a = DEFAULT_C_OFF_A;
	/*
	 * 6x21V700: mAh = temp * 10763  / r_coul_mohm  * 10E-10   (mAh)
	 *     uAh = temp * 10763  / r_coul_mohm  * 10E-7   (uAh)
	 *     code = uAh * 10^7 * r_coul_mohm / 10763
	 * 6x21V800: mAh = temp * 5381.5  / r_coul_mohm  * 10E-10   (mAh)
	 * 1000000: gain to ensure calculation accuracy
	 */
	coul_cali_params_get(NULL, NULL, &c_offset_a, NULL);
#ifndef CONFIG_COUL_PMIC6X21V700
	uah = uah * 2;
#endif

	temp = uah;
	temp = temp * 1000000;
	if (c_offset_a != 0) {
		temp = (u64)div_s64(temp, c_offset_a);
	} else {
		coul_hardware_err("c_offset_a = 0, division by default value %d\n",
			DEFAULT_C_OFF_A);
		temp = (u64)div_s64(temp, DEFAULT_C_OFF_A);
	}

	temp = temp * 10000000;
	temp = temp / 1000 * (unsigned int)g_r_coul_uohm;
	ret = (u64)div_s64(temp, 10763);

	coul_hardware_dbg("[%s] uah %d, ret 0x%lx\n", __func__, uah, ret);

	return ret;
}
#endif
/*
 * Description:  value of out_uah - in_uah recorded by  coulomb
 * Return: value of uah through coulomb
 * Remark: adjusted by offset integrated on time
 */
static int coul_calculate_cc_uah(int batt)
{
	u64 cc_in = 0;
	u64 cc_out = 0;
	unsigned int cl_in_time = 0;
	unsigned int cl_out_time = 0;
	s64 cc_uah_in;
	s64 cc_uah_out;
	int cc_uah;
	int c_offset_b = DEFAULT_C_OFF_B;

	coul_cali_params_get(NULL, NULL, NULL, &c_offset_b);

	coul_regs_read(COUL_HARDWARE_CL_IN_BASE, &cc_in, CC_REG_NUM);
	coul_regs_read(COUL_HARDWARE_CL_OUT_BASE, &cc_out, CC_REG_NUM);

	cc_uah_out = coul_hardware_convert_regval2uah(0, cc_out);
	cc_uah_in = coul_hardware_convert_regval2uah(0, cc_in);

	coul_regs_read(COUL_HARDWARE_CHG_TIMER_BASE, &cl_in_time, 4); /* 4: regs_num */
	coul_regs_read(COUL_HARDWARE_LOAD_TIMER_BASE, &cl_out_time, 4); /* 4: regs_num */
	/* uah = uas/3700 = ua*s/3700 */
	cc_uah_in -= div_s64((s64)cl_in_time * c_offset_b, SECONDS_PER_HOUR);
	cc_uah_out += div_s64((s64)cl_out_time * c_offset_b, SECONDS_PER_HOUR);
	cc_uah = (int)(cc_uah_out - cc_uah_in);

	coul_hardware_inf(
		"cc_out=0x%llx,cc_in=0x%llx,cout_time=%d,cin_time=%d,cc_adj=%duah\n",
		cc_out, cc_in, cl_out_time, cl_in_time, cc_uah);
	return cc_uah;
}

static void coul_save_cc_uah(int batt, int cc_uah)
{
	u64 reg;

	coul_cc_write_pro(UNLOCK);
	if (cc_uah > 0) {
		reg = coul_convert_uah2regval(cc_uah);
		udelay(110); /* 110: delay time, us */
		coul_regs_write(COUL_HARDWARE_CL_OUT_BASE, &reg, CC_REG_NUM);
		reg = 0;
		coul_regs_write(COUL_HARDWARE_CL_IN_BASE, &reg, CC_REG_NUM);
	} else {
		reg = coul_convert_uah2regval(-cc_uah);
		udelay(110); /* 110: delay time, us */
		coul_regs_write(COUL_HARDWARE_CL_IN_BASE, &reg, CC_REG_NUM);
		reg = 0;
		coul_regs_write(COUL_HARDWARE_CL_OUT_BASE, &reg, CC_REG_NUM);
	}
	coul_cc_write_pro(LOCK);
}

/*
 * Description: coul save ocv
 * Input: ocv:ocv vol uv
 * invalid_fcc_up_flag: no update fcc 1, update 0
 */
void coul_save_ocv(int batt, int ocv, int invalid_fcc_up_flag)
{
	unsigned short ocvreg = coul_convert_ocv_uv2regval(ocv);

	if (invalid_fcc_up_flag)
		ocvreg |= INVALID_TO_UPDATE_FCC;
	coul_regs_write(COUL_HARDWARE_SAVE_OCV_ADDR, &ocvreg, 2); /* 2: regs_num */
	coul_hardware_inf("save ocv, ocv=%d,reg=%d", ocv, ocvreg);
}

void coul_clear_ocv(int batt)
{
	unsigned short ocvreg = 0;

	coul_regs_write(COUL_HARDWARE_SAVE_OCV_ADDR, &ocvreg, 2); /* 2: regs_num */
}

/*
 * Description: get saved ocv
 * Return: OCV(reg value)
 */
short coul_get_ocv(int batt)
{
	unsigned short ocvreg = 0;

	coul_regs_read(COUL_HARDWARE_SAVE_OCV_ADDR, &ocvreg, 2); /* 2: regs_num */
	return (short)ocvreg;
}

/*
 * Description: coul save ocv temp
 * Input: ocv_temp: temp*10
 */
void coul_save_ocv_temp(int batt, short ocv_temp)
{
	coul_regs_write(COUL_HARDWARE_SAVE_OCV_TEMP_ADDR, &ocv_temp, 2); /* 2: regs_num */
	coul_hardware_inf("save ocv temp, ocv_temp=%d\n", ocv_temp);
}

void coul_clear_ocv_temp(int batt)
{
	short ocv_temp = 0;

	coul_regs_write(COUL_HARDWARE_SAVE_OCV_TEMP_ADDR, &ocv_temp, 2); /* 2: regs_num */
}

/*
 * Description: get saved ocv temp
 * Return: OCV temp(temp*10)
 */
short coul_get_ocv_temp(int batt)
{
	short ocv_temp = 0;

	coul_regs_read(COUL_HARDWARE_SAVE_OCV_TEMP_ADDR, &ocv_temp, 2); /* 2: regs_num */
	return ocv_temp;
}

/*
 * Description: get fcc update flag
 * Return: no up:1 is up:1
 */
int coul_get_fcc_invalid_up_flag(int batt)
{
	unsigned short ocvreg;

	ocvreg = (unsigned short)coul_get_ocv(0);
	if (ocvreg & INVALID_TO_UPDATE_FCC)
		return 1;
	return 0;
}

int coul_hardware_get_battery_voltage_uv(int batt)
{
	int uv;
	unsigned int regval = 0;

	coul_regs_read(COUL_HARDWARE_V_OUT, &regval, REG_NUM);
	uv = coul_hardware_convert_regval2uv(0, regval);

	coul_hardware_dbg("[%s] 0x%x, uv %d\n", __func__, regval, uv);
	return uv;
}

int coul_get_battery_current_ua(int batt)
{
	int ua;
	unsigned int regval = 0;

	coul_regs_read(COUL_HARDWARE_CURRENT, &regval, REG_NUM);
	ua = coul_hardware_convert_regval2ua(0, regval);
	coul_hardware_dbg("[%s] 0x%x, ua %d\n", __func__, regval, ua);
	return ua;
}

/*
 * Description:   get battery vol in uv from fifo
 * Intput: fifo_order:fifo serial number 0-9
 */
int coul_get_battery_vol_uv_from_fifo(int batt, short fifo_order)
{
	int uv;
	unsigned int regval = 0;

	if (fifo_order > FIFO_DEPTH)
		fifo_order = 0;
	coul_regs_read((COUL_HARDWARE_VOL_FIFO_BASE + REG_NUM * fifo_order), &regval,
		REG_NUM);

	uv = coul_hardware_convert_regval2uv(0, regval);

	coul_hardware_dbg("[%s] 0x%x, uv\n", __func__, regval, uv);
	return uv;
}

/*
 * Description: get battery cur in ua from fifo
 * Intput: fifo_order:fifo serial number 0-9
 */
int coul_get_battery_cur_ua_from_fifo(int batt, short fifo_order)
{
	int ua;
	unsigned int regval = 0;

	if (fifo_order > FIFO_DEPTH)
		fifo_order = 0;
	coul_regs_read((COUL_HARDWARE_CUR_FIFO_BASE + REG_NUM * fifo_order), &regval,
		REG_NUM);

	ua = coul_hardware_convert_regval2ua(0, regval);

	coul_hardware_dbg("[%s] 0x%x, ua %d\n", __func__, regval, ua);
	return ua;
}

/* Return: current offset mod value */
short coul_get_offset_current_mod(void)
{
	short regval = 0;

#if defined(CONFIG_COUL_PMIC6X21V600) || defined(CONFIG_COUL_PMIC6X55V200)
	coul_regs_read(COUL_HARDWARE_OFFSET_CUR_MODIFY_BASE, &regval, REG_NUM);
#endif
	return regval;
}

/* Return: vol offset mod value */
short coul_get_offset_vol_mod(void)
{
	short regval = 0;

#if defined(CONFIG_COUL_PMIC6X21V600)
	coul_regs_read(COUL_HARDWARE_OFFSET_VOL_MODIFY_BASE, &regval, REG_NUM);
#endif
	return regval;
}

void coul_set_offset_vol_mod(void)
{
#if defined(CONFIG_COUL_PMIC6X21V600)
	short regval = 0;

	coul_regs_write(COUL_HARDWARE_OFFSET_VOL_MODIFY_BASE, &regval, REG_NUM);
#endif
}

static void get_vc_by_cur(struct vcdata *vc, int cur)
{
	if (cur > vc->max_c)
		vc->max_c = cur;
	else if (cur < vc->min_c)
		vc->min_c = cur;
}
/*
 * Description:  get coul fifo average vol/current value(uv/ua)
 * Output: struct vcdata:avg , max and min cur, vol
 */
static void coul_get_fifo_avg_data(int batt, struct vcdata *vc)
{
	short i;
	int abnormal_value_cnt = 0;
	static unsigned int vol_fifo[FIFO_DEPTH];
	static unsigned int cur_fifo[FIFO_DEPTH];
	int cur, vols, curs;

	if (vc == NULL) {
		coul_hardware_inf("NULL point in [%s]\n", __func__);
		return;
	}
	for (i = 0; i < FIFO_DEPTH; i++) {
		coul_regs_read(COUL_HARDWARE_VOL_FIFO_BASE + i * REG_NUM,
			&vol_fifo[i], REG_NUM);
		coul_regs_read(COUL_HARDWARE_CUR_FIFO_BASE + i * REG_NUM,
			&cur_fifo[i], REG_NUM);
	}

	/* COUL_FIFO_VOL_DEFAULT:    vol_reset_value */
	if (vol_fifo[0] != COUL_FIFO_VOL_DEFAULT) {
		vols = coul_hardware_convert_regval2uv(0, vol_fifo[0]) / PERMILLAGE;
		cur = coul_hardware_convert_regval2ua(0, cur_fifo[0]) / PERMILLAGE;
	} else {
		vols = 0;
		cur = 0;
	}

	curs = cur;
	vc->max_c = cur;
	vc->min_c = cur;

	for (i = 1; i < FIFO_DEPTH; i++) {
		if (vol_fifo[i] != COUL_FIFO_VOL_DEFAULT) {
			vols += coul_hardware_convert_regval2uv(0, vol_fifo[i]) / PERMILLAGE;
			cur = coul_hardware_convert_regval2ua(0, cur_fifo[i]) / PERMILLAGE;
			curs += cur;
			get_vc_by_cur(vc, cur);
		} else {
			abnormal_value_cnt++;
		}
	}
	vc->avg_v = vols / (FIFO_DEPTH - abnormal_value_cnt);
	vc->avg_c = curs / (FIFO_DEPTH - abnormal_value_cnt);

	coul_hardware_inf("avg_v = %d, avg_c = %d, max_c = %d, min_c = %d\n",
		vc->avg_v, vc->avg_c, vc->max_c, vc->min_c);
}

static int coul_get_abs_cc(int batt)
{
	return g_saved_abs_cc_mah;
}

/* Description: get v_offset a value */
static int coul_get_ate_a(void)
{
	unsigned short regval;
	unsigned char a_low;
	unsigned char a_high;

	a_low = coul_reg_read(COUL_HARDWARE_VOL_OFFSET_A_ADDR_0);
	a_high = coul_reg_read(COUL_HARDWARE_VOL_OFFSET_A_ADDR_1);

#if defined(CONFIG_COUL_PMIC6X21V600) || defined(CONFIG_COUL_PMIC6X55V200)
	/* 6: Right shift six bits, 2: Left shift two bits */
	regval = (((a_low >> 6) & VOL_OFFSET_A_LOW_VALID_MASK) |
		((a_high << 2) & VOL_OFFSET_A_HIGH_VALID_MASK)) &
		VOL_OFFSET_A_VALID_MASK;
#else
	/* 1: Right shift ont bit */
	regval = ((a_low & VOL_OFFSET_A_LOW_VALID_MASK) |
		((a_high << 1) & VOL_OFFSET_A_HIGH_VALID_MASK)) &
		VOL_OFFSET_A_VALID_MASK;
#endif
	return (VOL_OFFSET_A_BASE + regval * VOL_OFFSET_A_STEP);
}

/* get v_offset b value */
static int coul_get_ate_b(void)
{
	unsigned char regval;

	regval = coul_reg_read(COUL_HARDWARE_VOL_OFFSET_B_ADDR);
	/*
	 * 6x21v700v800 :bit[1-7]
	 * 6x21v600 :bit[0-5]
	 */
	regval &= VOL_OFFSET_B_VALID_MASK;

#if defined(CONFIG_COUL_PMIC6X21V600) || defined(CONFIG_COUL_PMIC6X55V200)
	return (VOL_OFFSET_B_BASE + regval * VOL_OFFSET_B_STEP);
#else
	regval = (regval >> 1);
	return ((VOL_OFFSET_B_BASE + regval * VOL_OFFSET_B_STEP) / PERMILLAGE); /* uv */
#endif
}

static void coul_hardware_clear_cc_register(int batt)
{
	u64 ccregval = 0;

	g_saved_abs_cc_mah += (coul_calculate_cc_uah(0) / PERMILLAGE);
	udelay(110); /* 110: delay time, us */
	coul_cc_write_pro(UNLOCK);
	coul_regs_write(COUL_HARDWARE_CL_IN_BASE, &ccregval, CC_REG_NUM);
	coul_regs_write(COUL_HARDWARE_CL_OUT_BASE, &ccregval, CC_REG_NUM);
	coul_cc_write_pro(LOCK);
}

/*
 * Description: set low int vol val
 * Input: vol_value:low int vol val(mV)
 */
static void coul_set_low_vol_val(int vol_mv)
{
	unsigned int regval;

	regval = coul_convert_uv2regval(0, vol_mv * PERMILLAGE);

	coul_hardware_dbg("[%s] mv %d, reg 0x%x\n", __func__, vol_mv, regval);
	udelay(110); /* 110: delay time, us */
	coul_regs_write(COUL_HARDWARE_VOL_INT_BASE, &regval, REG_NUM);
}

/* Return: 0:success -1:fail */
static int coul_check_version(struct coul_device_info *di)
{
	int tryloop = 0;
	u8 ver[6] = {0}; /* 6:  num of  coul version name */

	do {
		coul_regs_read(COUL_HARDWARE_VERSION_ADDR, &ver, 6); /* 6: regs_num */
		coul_hardware_inf("do a dummy read, version is 0x%x\n", ver[0]);
		usleep_range(500, 510); /* 500: min delay 500us, 510: max delay 510us */
		/* 5: try times of get coul version */
		if ((tryloop++) > 5) {
			coul_hardware_err("version is not correct!\n");
			return -1;
		}
	} while ((ver[0] & 0xff) != COUL_HARDWARE);

	/* 12: left shift 12 bits, 8: left shift 8 bits, 4: left shift 4 bits */
	di->chip_proj = ((ver[0] & 0x0f) << 12) | ((ver[1] & 0x0f) << 8) |
		((ver[2] & 0x0f) << 4) | (ver[3] & 0x0f);
	di->chip_version = (ver[4] << 8) | ver[5];

	return 0;
}

/* Return: 0:success -1:fail */
static int coul_check_debug(void)
{
#if defined(CONFIG_COUL_PMIC6X21V600)
	u8 val;
	short ocvreg = 0;

	val = coul_reg_read(COUL_HARDWARE_DEBUG_REG2);
	if (val)
		coul_hardware_err("debug reg2 is 0x%x\n", val);
	val = coul_reg_read(COUL_HARDWARE_DEBUG_REG);
	if (val) {
		coul_hardware_err("debug reg is not 0x0\n");
		coul_reg_write(COUL_HARDWARE_DEBUG_WRITE_PRO, COUL_WRITE_UNLOCK);
		coul_reg_write(COUL_HARDWARE_DEBUG_REG, 0x0);
		coul_reg_write(COUL_HARDWARE_DEBUG_WRITE_PRO, COUL_WRITE_LOCK);
		usleep_range(500, 510); /* 500: min delay 500us, 510: max delay 510us */
		coul_regs_write(COUL_HARDWARE_SAVE_OCV_ADDR, &ocvreg, 2); /* 2: regs_num */
		usleep_range(500, 510); /* 500: min delay 500us, 510: max delay 510us */
		return -1;
	}
#endif
	return 0;
}

/* Description: coul calibration */
static void coul_hardware_cali_adc(int batt)
{
	unsigned char reg_val;

	reg_val = coul_reg_read(COUL_HARDWARE_STATE_REG);
	if (reg_val == COUL_CALI_ING) {
		coul_hardware_inf("cali ing, don't do it again!\n");
		return;
	}

	coul_hardware_inf("calibrate!\n");
	reg_val = coul_reg_read(COUL_HARDWARE_CTRL_REG);
	/* Mode */
	reg_val = reg_val | COUL_CALI_ENABLE;
	udelay(110); /* 110: delay time, us */
	coul_reg_write(COUL_HARDWARE_CTRL_REG, reg_val);
}

/*
 * Description: clear coul interrupt
 * Remark: clear low vol/capacity INT before coul self_adjust when start up
 */
static void coul_clear_irq(void)
{
	char val = COUL_CLEAR_IRQ;
	coul_reg_write(COUL_HARDWARE_IRQ_REG, val);
}

static void coul_enable_irq(int batt)
{
	unsigned char irq_enable_flag =
		~((unsigned char)(COUL_VBAT_INT_MASK | COUL_CL_IN_MASK |
		COUL_CL_OUT_MASK));

	/* disable cl_int interrupt, disable i out/in */
	coul_reg_write(COUL_HARDWARE_IRQ_MASK_REG, irq_enable_flag);
	coul_hardware_inf("Enable coul irq!\n");
}

static void coul_disable_irq(int batt)
{
	unsigned char irq_disable_flag = COUL_INT_MASK_ALL;

	coul_reg_write(COUL_HARDWARE_IRQ_MASK_REG, irq_disable_flag);
	coul_hardware_inf("Mask coul irq!\n");
}

#if defined(CONFIG_COUL_PMIC6X21V600) || defined(CONFIG_COUL_PMIC6X55V200)
#else

/*
 * Description:  convert register value to current(uA)
 * Input:  reg_val:current reg value
 * Return: value of register in uA
 * Remark:
 *  6x21V700: Code = mA * 2^23 / 1.3 * 10 * Rsense / 1000 / 1000
 *  6x21V800&6x55V300: Code = mA * 2^23 / 0.65 * 10 * Rsense / 1000 / 1000
 *  high bit = 1 is in, 0 is out,  Rsense in mohm
 *  only for 6x21V700 & V800 & V300
 * 1000000: gain to ensure calculation accuracy
 */
static unsigned int coul_convert_ua2regval(int ua)
{
	unsigned int ret;
	s64 temp;
	int val = ua;
	int c_offset_a = DEFAULT_C_OFF_A;

	coul_cali_params_get(NULL, NULL, &c_offset_a, NULL);

#ifndef CONFIG_COUL_PMIC6X21V700
	val = val * 2;
#endif

	temp = (s64)val * 1000000;
	if (c_offset_a != 0) {
		temp = div_s64(temp, c_offset_a);
	} else {
		coul_hardware_err("c_offset_a = 0, division by default value %d\n",
			DEFAULT_C_OFF_A);
		temp = div_s64(temp, DEFAULT_C_OFF_A);
	}

	temp = temp * 8388608;
	temp = temp * 100 / 1000 * (s64)g_r_coul_uohm;
	temp = div_s64(temp, 13);
	temp = div_s64(temp, 1000000000);

	ret = (unsigned int)temp;

	coul_hardware_dbg("[%s] ua 0x%lx, ret %d\n", __func__, ua, ret);

	return ret;
}
#endif

/*
 * Description: set i in gate
 * Input: ma, should < 0
 */
static void coul_set_i_in_event_gate(int batt, int ma)
{
#if defined(CONFIG_COUL_PMIC6X21V600) || defined(CONFIG_COUL_PMIC6X55V200)
#else
	unsigned int reg_val;

	if (ma > 0)
		ma = -ma;
	reg_val = coul_convert_ua2regval(ma * PERMILLAGE);
	udelay(110); /* 110: delay time, us */
	coul_regs_write(COUL_HARDWARE_I_IN_GATE, &reg_val, REG_NUM);
#endif
}

/*
 * Description: set i out gate
 * Input: ma, should > 0
 */
static void coul_set_i_out_event_gate(int batt, int ma)
{
#if defined(CONFIG_COUL_PMIC6X21V600) || defined(CONFIG_COUL_PMIC6X55V200)
#else
	unsigned int reg_val;

	if (ma < 0)
		ma = -ma;
	reg_val = coul_convert_ua2regval(ma * PERMILLAGE);
	udelay(110); /* 110: delay time, us */
	coul_regs_write(COUL_HARDWARE_I_OUT_GATE, &reg_val, REG_NUM);
#endif
}

static void coul_eco_delay_enable(u8 enable)
{
#if defined(CONFIG_COUL_PMIC6X21V700) || \
	defined(CONFIG_COUL_PMIC6X21V600) || \
	defined(CONFIG_COUL_PMIC6X55V200)
#else
	u8 val;

	val = coul_reg_read(COUL_HARDWARE_ECO_CONFIG_ADDR);
	val = (val & ~(ECO_DELAY_EN_MASK)) | (enable << ECO_DELAY_EN_SHIFT);
	coul_reg_write(COUL_HARDWARE_ECO_CONFIG_ADDR, val);
#endif
}

static void coul_wait_comp_enable(u8 enable)
{
#if defined(CONFIG_COUL_PMIC6X21V700) || \
	defined(CONFIG_COUL_PMIC6X21V600) || \
	defined(CONFIG_COUL_PMIC6X55V200)
#else
	u8 val;

	val = coul_reg_read(COUL_HARDWARE_WAIT_COMP_ADDR);
	val = (val & ~(WAIT_COMP_EN_MASK)) | (enable << WAIT_COMP_EN_SHIFT);
	coul_reg_write(COUL_HARDWARE_WAIT_COMP_ADDR, val);
#endif
}

/* Description: config init */
static void coul_chip_init(void)
{
	coul_clear_irq();
	coul_disable_irq(0);

	coul_set_i_in_event_gate(0, DEFAULT_I_GATE_VALUE);
	coul_set_i_out_event_gate(0, DEFAULT_I_GATE_VALUE);

	/* unmask coul eco */
	udelay(110); /* 110: delay time, us */
#if defined(CONFIG_COUL_PMIC6X55V500) || defined(CONFIG_COUL_PMIC6X21V900)
	coul_reg_write(COUL_HARDWARE_COUL_ECO_MASK, 0);
#else
	coul_reg_write(COUL_HARDWARE_COUL_ECO_MASK_0, 0);
	coul_reg_write(COUL_HARDWARE_COUL_ECO_MASK_1, 0);
#endif
	/* config coul Mode */
	udelay(110); /* 110: delay time, us */
	coul_reg_write(COUL_HARDWARE_CTRL_REG, DEFAULT_COUL_CTRL_VAL);

#if defined(CONFIG_COUL_PMIC6X21V600) || defined(CONFIG_COUL_PMIC6X55V200)
	/* open coul cali auto */
	udelay(110); /* 110: delay time, us */
	coul_reg_write(COUL_HARDWARE_CLJ_CTRL, CALI_CLJ_DEFAULT_VALUE);
#endif
	coul_wait_comp_enable(PMU_ENABLE);
}

/* Description: show key register info for bug */
void coul_show_key_reg(void)
{
	unsigned char reg0 = 0;
	unsigned int reg1 = 0;
	unsigned int reg2 = 0;
	unsigned int reg3 = 0;
	unsigned char reg4 = 0;
	unsigned int reg5 = 0;

	udelay(50); /* 50: delay time, us */
	coul_regs_read(COUL_HARDWARE_STATE_REG, &reg0, 1); /* 1: reg_num */
	coul_regs_read(COUL_HARDWARE_VOL_FIFO_BASE, &reg1, REG_NUM);
	coul_regs_read(COUL_HARDWARE_VOL_FIFO_BASE + REG_NUM, &reg2, REG_NUM);
	coul_regs_read(COUL_HARDWARE_OFFSET_VOLTAGE, &reg3, REG_NUM);
	coul_regs_read(COUL_HARDWARE_CTRL_REG, &reg4, 1); /* 1: reg_num */
	coul_regs_read(COUL_HARDWARE_V_OUT, &reg5, REG_NUM);

	coul_hardware_inf("\n"
		"0x4033(state)= 0x%x, 0x4059-0x405b(vol fifo0) = 0x%x, "
		"0x405c-0x405e(vol fifo1) = 0x%x\n"
		"0x403d-0x403f(vol offset) = 0x%x, 0x4003(ctrl)= 0x%x\n"
		"0x4037-0x4039(cur vol)= 0x%x\n",
		reg0, reg1, reg2, reg3, reg4, reg5);
}

/*
 * Description: interrupt_notifier_work - send a notifier event to battery monitor.
 * Remark: capacity INT : low level and shutdown level.
 */
static void coul_interrupt_notifier_work(struct work_struct *work)
{
	struct coul_device_info *di = container_of(
		work, struct coul_device_info, irq_work.work);
	unsigned char intstat;

	intstat = di->irq_mask;
	di->irq_mask = 0;

	coul_show_key_reg();

	if (intstat & COUL_I_OUT_MASK) {
		coul_hardware_inf("IRQ: COUL_I_OUT_INT\n");
		coul_core_notify_fault(0, COUL_FAULT_I_OUT, NULL);
	}
	if (intstat & COUL_I_IN_MASK) {
		coul_hardware_inf("IRQ: COUL_I_IN_INT\n");
		coul_core_notify_fault(0, COUL_FAULT_I_IN, NULL);
	}
	if (intstat & COUL_VBAT_INT_MASK) {
		coul_hardware_inf("IRQ: COUL_CCOUT_LOW_VOL_INT\n");
		coul_core_notify_fault(0, COUL_FAULT_LOW_VOL, NULL);
	}
	if (intstat & COUL_CL_INT_MASK) {
		coul_hardware_inf("IRQ: COUL_CCOUT_BIG_INT\n");
		coul_core_notify_fault(0, COUL_FAULT_CL_INT, NULL);
	}
	if (intstat & COUL_CL_IN_MASK) {
		coul_hardware_inf("IRQ: COUL_CCIN_CNT_INT\n");
		coul_core_notify_fault(0, COUL_FAULT_CL_IN, NULL);
	}
	if (intstat & COUL_CL_OUT_MASK) {
		coul_hardware_inf("IRQ: COUL_CCOUT_CNT_INT\n");
		coul_core_notify_fault(0, COUL_FAULT_CL_OUT, NULL);
	}
}

/*
 * Description: coul irq handler
 * Input: int irq ---- irq number
 *     void *_di ---- coul device
 * Return: IRQ_NONE: irq not exist,  IRQ_HANDLED: be handled
 */
static irqreturn_t coul_irq_cb(int irq, void *_di)
{
	struct coul_device_info *di = _di;
	unsigned char val;

	coul_hardware_inf("coul_irq_cb irq=%d\n", irq);

	val = coul_reg_read(COUL_HARDWARE_IRQ_REG);
	coul_hardware_inf("COUL_HARDWARE_IRQ_ADDR reg=%d\n", val);

	coul_reg_write(COUL_HARDWARE_IRQ_REG, val);

	di->irq_mask |= val;

	queue_delayed_work(
		system_power_efficient_wq, &di->irq_work, msecs_to_jiffies(0));

	return IRQ_HANDLED;
}

/*
 * Description: calculate capacity leak from existing ECO MODE to calc soc first time
 * Remark: ECO uah register keep the same value after exist from ECO
 */
static int coul_calculate_eco_leak_uah(int batt)
{
	int rst_uah;
	int eco_uah;
	int cur_uah;
	s64 eco_in_uah;
	s64 eco_out_uah;
	s64 present_in_uah;
	s64 present_out_uah;
	u64 in_val = 0;
	u64 out_val = 0;

	coul_regs_read(COUL_HARDWARE_ECO_OUT_CLIN_REG_BASE, &in_val, CC_REG_NUM);
	coul_regs_read(COUL_HARDWARE_ECO_OUT_CLOUT_REG_BASE, &out_val, CC_REG_NUM);
	/* if: first time to calc soc after exiting from ECO Mode */
	if ((g_last_eco_in != in_val) || (g_last_eco_out != out_val)) {
		eco_out_uah = coul_hardware_convert_regval2uah(0, out_val);
		eco_in_uah = coul_hardware_convert_regval2uah(0, in_val);
		eco_uah = (int)(eco_out_uah - eco_in_uah);

		/* current cc */
		coul_regs_read(COUL_HARDWARE_CL_OUT_BASE, &out_val, CC_REG_NUM);
		coul_regs_read(COUL_HARDWARE_CL_IN_BASE, &in_val, CC_REG_NUM);
		present_in_uah = coul_hardware_convert_regval2uah(0, in_val);
		present_out_uah = coul_hardware_convert_regval2uah(0, out_val);
		cur_uah = (int)(present_out_uah - present_in_uah);

		/* leak cc from exisingt eco mode to first calc soc */
		rst_uah = cur_uah - eco_uah;

		coul_hardware_err(
			"eco_in=%d,eco_out=%d,cc_in=%d,cc_out=%d,leakcc=%d\n",
			eco_in_uah, eco_out_uah, present_in_uah,
			present_out_uah, rst_uah);
	} else {
		rst_uah = 0;
		coul_hardware_inf(
			"Not the FIRST calc soc out eco, leak cc=0\n");
	}
	return rst_uah;
}

/* Description: clear coul vol/current fifo value */
static void coul_clear_fifo(void)
{
	unsigned char reg_value;

	reg_value = coul_reg_read(COUL_HARDWARE_FIFO_CLEAR);
	coul_reg_write(COUL_HARDWARE_FIFO_CLEAR, (reg_value | COUL_FIFO_CLEAR));
}

/* Description: clear coul vol/current eco fifo value */
static void coul_clear_enable_eco_fifo(void)
{
#if defined(CONFIG_COUL_PMIC6X21V700) || \
	defined(CONFIG_COUL_PMIC6X21V600) || \
	defined(CONFIG_COUL_PMIC6X55V200)
#else
	unsigned char reg_value;

	reg_value = coul_reg_read(COUL_HARDWARE_ECO_FIFO_CLEAR);
	coul_reg_write(COUL_HARDWARE_ECO_FIFO_CLEAR,
		(reg_value | ECO_FIFO_CLEAR | ECO_FIFO_EN));
#endif
}

/* Remark: coul eco follow pmu eco */
static void coul_enter_eco(int batt)
{
	unsigned char reg_val;
	u64 eco_in_reg = 0;
	u64 eco_out_reg = 0;

	coul_regs_read(COUL_HARDWARE_ECO_OUT_CLIN_REG_BASE, &eco_in_reg, CC_REG_NUM);
	coul_regs_read(
		COUL_HARDWARE_ECO_OUT_CLOUT_REG_BASE, &eco_out_reg, CC_REG_NUM);

	g_last_eco_in = eco_in_reg;
	g_last_eco_out = eco_out_reg;

#if defined(CONFIG_COUL_PMIC6X55V200)
	reg_val = coul_reg_read(COUL_HARDWARE_STATE_REG);
	if (reg_val == COUL_CALI_ING) {
		coul_hardware_inf("cali ing, don't do it again!\n");
		reg_val = ECO_COUL_CTRL_VAL;
	} else {
		coul_hardware_inf("calibrate!\n");
		reg_val = (ECO_COUL_CTRL_VAL | COUL_CALI_ENABLE);
	}
#else
	reg_val = ECO_COUL_CTRL_VAL;

#endif

	udelay(110); /* 110: delay time, us */
	coul_reg_write(COUL_HARDWARE_CTRL_REG, reg_val);
	coul_clear_fifo();

	coul_clear_enable_eco_fifo();

	coul_eco_delay_enable(PMU_ENABLE);
}

/* Remark: coul eco follow pmu eco */
static void coul_exit_eco(int batt)
{
	coul_clear_fifo();
	udelay(110); /* 110: delay time, us */
	coul_reg_write(COUL_HARDWARE_CTRL_REG, DEFAULT_COUL_CTRL_VAL);

	coul_eco_delay_enable(PMU_DISABLE);
}

static void coul_save_last_soc(short soc)
{
	unsigned short val;

	if (soc < 0)
		soc = 0;
	val = (unsigned short)soc;
	coul_reg_write(COUL_HARDWARE_SAVE_LAST_SOC,
		SAVE_LAST_SOC_FLAG | (val & SAVE_LAST_SOC));
}

static void coul_hardware_get_last_soc(short *soc)
{
	unsigned short val;

	val = coul_reg_read(COUL_HARDWARE_SAVE_LAST_SOC);
	*soc = (short)(val & SAVE_LAST_SOC);
}

static void coul_clear_last_soc_flag(void)
{
	coul_reg_write(COUL_HARDWARE_SAVE_LAST_SOC, 0);
	coul_hardware_err("%s clear last soc flag !\n", __func__);
}

static void coul_get_last_soc_flag(bool *valid)
{
	bool val;

	val = SAVE_LAST_SOC_FLAG & coul_reg_read(COUL_HARDWARE_SAVE_LAST_SOC);
	*valid = val;
}

static void coul_cancel_auto_cali(void)
{
	u8 val;

	val = coul_reg_read(COUL_HARDWARE_CLJ_CTRL);
	val = val & (~MASK_CALI_AUTO_OFF);
	udelay(110); /* 110: delay time, us */
	coul_reg_write(COUL_HARDWARE_CLJ_CTRL, val);
}

static void coul_save_ocv_level(int batt, u8 level)
{
	u8 val;

	val = coul_reg_read(COUL_HARDWARE_OCV_LEVEL_ADDR);
	val &= ~SAVE_OCV_LEVEL;
	val |= ((level << OCV_LEVEL_SHIFT) & SAVE_OCV_LEVEL);
	coul_reg_write(COUL_HARDWARE_OCV_LEVEL_ADDR, val);
}

static void coul_get_ocv_level(int batt, u8 *level)
{
	u8 val;

	val = coul_reg_read(COUL_HARDWARE_OCV_LEVEL_ADDR);
	val &= SAVE_OCV_LEVEL;
	*level = val >> OCV_LEVEL_SHIFT;
}

static int coul_get_drained_battery_flag(int batt)
{
	u8 val = 0;

#if defined(CONFIG_COUL_PMIC6X55V200)
#else
	val = coul_reg_read(DRAINED_BATTERY_FLAG_ADDR);
	coul_hardware_err("%s get reg value %d!!!\n", __func__, val);
	val &= DRAINED_BATTERY_FLAG_BIT;
#endif
	return val;
}

static void coul_clear_drained_battery_flag(int batt)
{
#if defined(CONFIG_COUL_PMIC6X55V200)
#else
	u8 val;

	val = coul_reg_read(DRAINED_BATTERY_FLAG_ADDR);
	coul_reg_write(
		DRAINED_BATTERY_FLAG_ADDR, val & (~DRAINED_BATTERY_FLAG_BIT));
	val = coul_reg_read(DRAINED_BATTERY_FLAG_ADDR);
	coul_hardware_err("%s after clear reg value %d!!!\n", __func__, val);
#endif
}

static void coul_set_bootocv_sample(int batt, u8 set_val)
{
#if defined(CONFIG_COUL_PMIC6X55V200)
#else
	u8 val;

	val = coul_reg_read(BOOT_OCV_ADDR);
	if (set_val)
		val |= EN_BOOT_OCV_SAMPLE;
	else
		val &= (~EN_BOOT_OCV_SAMPLE);
	coul_reg_write(BOOT_OCV_ADDR, val);
	val = coul_reg_read(BOOT_OCV_ADDR);
	coul_hardware_err("%s set_bootocv:%d!!!\n", __func__, val);
#endif
}

#ifdef CONFIG_COUL_PMIC6X21V700
static int coul_get_normal_mode_chip_temp(void)
{
	unsigned char val;
	unsigned int temp_reg = 0;
	int retry;
	int ret;

	/* read mstat is normal */
	retry = 30; /* 30: retry times, From cali mode to normal mode takes up to 2.75s  */
	do {
		val = coul_reg_read(COUL_HARDWARE_STATE_REG);

		retry--;
		mdelay(100); /* 100: delay time, ms */

		if (retry == 0) {
			coul_hardware_err("%s  val is 0x%x, wait coul working fail!\n", __func__, val);
			ret = INVALID_TEMP;
			goto reset_auto_cali;
		}
	} while ((val & COUL_MSTATE_MASK) != COUL_WORKING);

	coul_reg_write(COUL_HARDWARE_TEMP_CTRL, TEMP_EN);

	/* read temp_rdy = 1 */
	retry = 10; /* 10, retry times, Collecting temperature takes 200ms  */
	do {
		val = coul_reg_read(COUL_HARDWARE_TEMP_CTRL);

		retry--;
		mdelay(25); /* 25: delay time, ms */

		if (retry == 0) {
			coul_hardware_err("%s  val is 0x%x, wait temp ready fail !\n", __func__, val);
			ret = INVALID_TEMP;
			goto reset_vol_sel;
		}
	} while (!(val & TEMP_RDY));

	/* read temp data */
	coul_regs_read(COUL_HARDWARE_TEMP_DATA, &temp_reg, 3); /* 3: regs_num */
	ret = coul_hardware_convert_regval2temp(temp_reg);

reset_vol_sel:
	/* set temp vol channel = 0 */
	coul_reg_write(COUL_HARDWARE_TEMP_CTRL, ~TEMP_EN);

	/* 20: retry times, Collecting voltage takes 750ms */
	retry = 20;
	do {
		val = coul_reg_read(COUL_HARDWARE_TEMP_CTRL);

		retry--;
		mdelay(50); /* 50: delay time, ms */

		if (retry == 0) {
			coul_hardware_err("%s val is 0x%x, wait vout fail!\n", __func__, val);
			ret = INVALID_TEMP;
			goto reset_auto_cali;
		}
	} while (!(val & VOUT_RDY));

	/* auto cali on  */
reset_auto_cali:
	return ret;
}

/*
 * Description: Get the coul chip temperature at different times
 * Input: type: start up ocv chip temp; eco out chip temp; normal mode, chip temp
 * Return: ret: chip temp or INVALID_TEMP
 * Remark: When the type is equal to Normal,
 * you need to add wakelock, to avoid the PMU into ECO mode
 */
static int coul_hardware_get_chip_temp(void)
{
	return coul_get_normal_mode_chip_temp();
}
#endif

#if defined(CONFIG_COUL_PMIC6X21V700) || \
	defined(CONFIG_COUL_PMIC6X21V600) || \
	defined(CONFIG_COUL_PMIC6X55V200)
#else

/*
 * Description:  wrrite reg by mask
 * Input: reg addr, mask bit, bit value
 */
static void reg_write_mask(u16 addr, u8 mask, u8 value)
{
	u8 data;

	data = coul_reg_read(addr);
	data = data & ~mask;
	data = data | value;
	coul_reg_write(addr, data);
}

/* NTC Table */
static int g_t_v_table[][2] = {
	{ -273, 4095 }, { -40, 3764 }, { -36, 3689 }, { -32, 3602 },
	{ -28, 3500 }, { -24, 3387 }, { -20, 3261 }, { -16, 3122 },
	{ -12, 2973 }, { -8, 2814 }, { -4, 2650 }, { 0, 2480 }, { 4, 2308 },
	{ 8, 2136 }, { 12, 1967 }, { 16, 1803 }, { 20, 1646 }, { 24, 1497 },
	{ 28, 1360 }, { 32, 1230 }, { 36, 1111 }, { 40, 1001 }, { 44, 903 },
	{ 48, 812 }, { 52, 729 }, { 56, 655 }, { 60, 590 }, { 64, 531 },
	{ 74, 406 }, { 84, 313 }, { 125, 110 }, { 0, 0 },
};
#define T_V_ARRAY_LENGTH        31 /* NTC Table length */

static int adc_to_temp_permille(int temp_volt)
{
	int i;
	int temprature = 0;

	if (temp_volt >= g_t_v_table[0][1])
		return g_t_v_table[0][0];
	if (temp_volt <= g_t_v_table[T_V_ARRAY_LENGTH - 1][1])
		return g_t_v_table[T_V_ARRAY_LENGTH - 1][0];
	for (i = 0; i < T_V_ARRAY_LENGTH; i++) {
		if (temp_volt == g_t_v_table[i][1])
			return g_t_v_table[i][0];
		if (temp_volt > g_t_v_table[i][1])
			break;
	}
	if (i == 0)
		return g_t_v_table[0][0];
	if ((g_t_v_table[i][1] - g_t_v_table[i - 1][1]) != 0)
		temprature = g_t_v_table[i - 1][0] +
			(long)(temp_volt - g_t_v_table[i - 1][1]) *
			(g_t_v_table[i][0] - g_t_v_table[i - 1][0]) /
			(g_t_v_table[i][1] - g_t_v_table[i - 1][1]);
	return temprature;
}

/* Description: Get the temp of battery by soh */
static int coul_get_tbat(int batt)
{
	u16 tbat_code = 0;
	int tbat;

	/* get adc data */
	coul_regs_read(COUL_HARDWARE_SOH_TBAT_DATA_BASE, &tbat_code, 2); /* 2: regs_num */
	tbat = adc_to_temp_permille(tbat_code);
	coul_hardware_inf("%s code %d, tbat %d\n", __func__, tbat_code, tbat);
	return tbat;
}

/*
 * Description: Get the temp of chip die by soh adc
 * Remark:  tdie =(tdie_code * 180000 / 4096 - 35868) * 100 / 13427
 */
static int coul_get_tdie(void)
{
	u16 tdie_code = 0;
	int tdie;

	/* get adc data */
	coul_regs_read(COUL_HARDWARE_SOH_TDIE_DATA_BASE, &tdie_code, 2); /* 2: regs_num */

	tdie_code = (unsigned int)g_soh_adc_a * tdie_code / PERMILLAGE + (unsigned int)g_soh_adc_b;
	tdie = (int)(((s32)tdie_code * 180000 / 4096 - 35868) * 100 / 13427);
	coul_hardware_dbg("%s code %d, tdie %d\n", __func__, tdie_code, tdie);

	return tdie;
}

/*
 * Description: get soh adc code
 * Input: chanel 15 and 16 for calibration
 * Return: adc code
 */
static int coul_soh_adc_get(u8 chanel)
{
	int retry = ADC_RETRY;
	u8 reg_value;
	u16 code = 0;

	coul_reg_write(SOH_EN_ADDR, SOH_DIS);
	/* slect adc chenel and enable adc */
	reg_write_mask(SOH_ADC_CTRL_ADDR, ADC_CHANEL_MASK, chanel);
	reg_write_mask(SOH_ADC_CTRL_ADDR, ADC_EN_MASK, ADC_EN);

	udelay(SOH_ADC_WAIT_US);
	coul_reg_write(SOH_ADC_START_ADDR, SOH_ADC_START);

	do {
		if (retry == 0) {
			coul_hardware_err("[%s] retry fail!\n", __func__);
			goto reset_adc;
		}
		udelay(SOH_ADC_WAIT_US); /* chip requires 12.5us */
		reg_value = coul_reg_read(SOH_ADC_STATUS_ADDR);
		retry--;
	} while (!(reg_value & SOH_ADC_READY));

	coul_regs_read(SOH_ADC_CALI_DATA0_ADDR, &code, 2); /* 2: regs_num */

	reg_write_mask(SOH_ADC_CTRL_ADDR, ADC_EN_MASK, ADC_DIS);

	/* close chip adc channel */
reset_adc:
	coul_reg_write(SOH_ADC_CTRL_ADDR, SOH_ADC_EN_DEFAULT);

	coul_reg_write(SOH_EN_ADDR, SOH_EN);
	return (int)code;
}
#endif

static void coul_soh_adc_cali(void)
{
#if defined(CONFIG_COUL_PMIC6X21V700) || \
	defined(CONFIG_COUL_PMIC6X21V600) || \
	defined(CONFIG_COUL_PMIC6X55V200)
#else
	int a, b;
	int y1, y2;

	y1 = coul_soh_adc_get(ADC_CALI_PATH1);
	y2 = coul_soh_adc_get(ADC_CALI_PATH2);
	if (y2 - y1 == 0) {
		coul_hardware_inf("[%s] fail Y1 %d, Y2 %d\n", __func__, y1, y2);
		return;
	}

	/* This is the ADC calibration formula */
	a = 1792 * 1000 / (y2 - y1);
	b = (2047 * y1 - 255 * y2) / (y1 - y2);

	coul_hardware_inf("[%s] Y1 %d, Y2 %d, a %d, b %d\n",
			__func__, y1, y2, a, b);

	if (a > ADC_CALI_A_MAX || a < ADC_CALI_A_MIN ||
		b < ADC_CALI_B_MIN || b > ADC_CALI_B_MAX) {
		coul_hardware_err("[%s] adc cali fail\n", __func__);
	} else {
		g_soh_adc_a = a;
		g_soh_adc_b = b;
	}

#endif
}

#ifdef CONFIG_SYSFS

static long g_reg_addr;
ssize_t coul_set_reg_sel(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int status = (int)count;

	g_reg_addr = 0;
	if (kstrtol(buf, 0, &g_reg_addr) < 0)
		return -EINVAL;
	return status;
}

ssize_t coul_set_reg_value(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	long val = 0;
	size_t status = count;

	if (kstrtol(buf, 0, &val) < 0)
		return -EINVAL;
#ifdef CONFIG_DFX_DEBUG_FS
	coul_reg_write(g_reg_addr, (char)val);
#endif
	return status;
}

ssize_t coul_show_reg_info(
	struct device *dev, struct device_attribute *attr, char *buf)
{
	u8 val = 0;

#ifdef CONFIG_DFX_DEBUG_FS
	val = coul_reg_read(g_reg_addr);
#endif
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "reg[0x%x]=0x%x\n",
		(u32)g_reg_addr, val);
}

static void coul_set_cali_param(int batt, struct coul_cali_params *params)
{
	struct coul_device_info *di = g_coul_hardware_dev;

	if (di == NULL) {
		coul_hardware_err("[%s]di is null\n", __func__);
		return;
	}

	(void)memcpy_s(&di->cali_params,
		sizeof(struct coul_cali_params), params, sizeof(*params));

	coul_hardware_inf("%s batt %d, v_offset_a %d, v_offset_b %d, c_offset_a %d, c_offset_b %d\n",
		__func__, batt, params->v_offset_a, params->v_offset_b,
		params->c_offset_a, params->c_offset_b);
	coul_hardware_inf("%s batt %d, v_offset_series_a %d, v_offset_series_b %d, "
		"c_offset_series_a %d, c_offset_series_b %d\n",
		__func__, batt, params->v_offset_series_a, params->v_offset_series_b,
		params->c_offset_series_a, params->c_offset_series_b);
}

static void coul_set_default_cali_param(struct coul_device_info *di)
{
	di->cali_params.v_offset_a = DEFAULT_V_OFF_A;
	di->cali_params.v_offset_b = DEFAULT_V_OFF_B;
	di->cali_params.c_offset_a = DEFAULT_C_OFF_A;
	di->cali_params.c_offset_b = DEFAULT_C_OFF_B;
}

static DEVICE_ATTR(sel_reg, 0640, NULL, coul_set_reg_sel);
static DEVICE_ATTR(
	set_reg, 0640, coul_show_reg_info, coul_set_reg_value);

static struct attribute *coul_attributes[] = {
	&dev_attr_sel_reg.attr, &dev_attr_set_reg.attr, NULL,
};

static const struct attribute_group coul_attr_group = {
	.attrs = coul_attributes,
};
#endif

/* check whether coul is ok 0: success  1: fail */
static int coul_ic_check(void)
{
	u8 val;
	struct coul_device_info *di = g_coul_hardware_dev;

	if(!di) {
		coul_hardware_err("%s di is null.\n", __func__);
		return 1;
	}

	val = coul_reg_read(0);
	if (val == VERSION_VAL) {
		return 0;
	} else {
		coul_hardware_err("%s fail, 0x%x\n", __func__, val);
		return 1;
	}
}

struct coul_device_ops hi6xxx_coul_ops = {
	.calculate_cc_uah = coul_calculate_cc_uah,
	.save_cc_uah = coul_save_cc_uah,
	.convert_ocv_regval2ua = coul_convert_ocv_regval2ua,
	.convert_ocv_regval2uv = coul_convert_ocv_regval2uv,
	.is_battery_moved = coul_is_battery_moved,
	.set_battery_moved_magic_num = coul_set_battery_move_magic,
	.get_fifo_depth = coul_get_fifo_depth,
	.get_fifo_avg_data = coul_get_fifo_avg_data,
	.get_delta_rc_ignore_flag = coul_get_delta_rc_ignore_flag,
	.get_nv_read_flag = coul_get_nv_read_flag,
	.set_nv_save_flag = coul_set_nv_save_flag,
	.get_use_saved_ocv_flag = coul_get_use_saved_ocv_flag,
	.get_fcc_invalid_up_flag = coul_get_fcc_invalid_up_flag,
	.save_ocv = coul_save_ocv,
	.get_ocv = coul_get_ocv,
	.clear_ocv = coul_clear_ocv,
	.save_ocv_temp = coul_save_ocv_temp,
	.get_ocv_temp = coul_get_ocv_temp,
	.clear_ocv_temp = coul_clear_ocv_temp,
	.set_low_low_int_val = coul_set_low_vol_val,
	.get_abs_cc = coul_get_abs_cc,
	.get_coul_time = coul_get_coul_time,
	.clear_coul_time = coul_hardware_clear_coul_time,
	.clear_cc_register = coul_hardware_clear_cc_register,
	.cali_adc = coul_hardware_cali_adc,
	.get_battery_voltage_uv = coul_hardware_get_battery_voltage_uv,
	.get_battery_current_ua = coul_get_battery_current_ua,
	.get_battery_vol_uv_from_fifo = coul_get_battery_vol_uv_from_fifo,
	.get_battery_cur_ua_from_fifo = coul_get_battery_cur_ua_from_fifo,
	.get_offset_current_mod = coul_get_offset_current_mod,
	.get_offset_vol_mod = coul_get_offset_vol_mod,
	.set_offset_vol_mod = coul_set_offset_vol_mod,
	.get_ate_a = coul_get_ate_a,
	.get_ate_b = coul_get_ate_b,
	.irq_enable = coul_enable_irq,
	.irq_disable = coul_disable_irq,
	.show_key_reg = coul_show_key_reg,
	.enter_eco = coul_enter_eco,
	.exit_eco = coul_exit_eco,
	.calculate_eco_leak_uah = coul_calculate_eco_leak_uah,
	.get_last_soc = coul_hardware_get_last_soc,
	.save_last_soc = coul_save_last_soc,
	.get_last_soc_flag = coul_get_last_soc_flag,
	.clear_last_soc_flag = coul_clear_last_soc_flag,
	.cali_auto_off = coul_cancel_auto_cali,
	.save_ocv_level = coul_save_ocv_level,
	.get_ocv_level = coul_get_ocv_level,
	.get_drained_battery_flag = coul_get_drained_battery_flag,
	.clear_drained_battery_flag = coul_clear_drained_battery_flag,
	.set_bootocv_sample = coul_set_bootocv_sample,
#if defined(CONFIG_COUL_PMIC6X21V600) || defined(CONFIG_COUL_PMIC6X55V200)
#else
	.set_i_in_event_gate = coul_set_i_in_event_gate,
	.set_i_out_event_gate = coul_set_i_out_event_gate,
	.convert_regval2uv = coul_hardware_convert_regval2uv,
	.convert_regval2ua = coul_hardware_convert_regval2ua,
	.convert_regval2temp = coul_hardware_convert_regval2temp,
	.convert_uv2regval = coul_convert_uv2regval,
	.convert_regval2uah = coul_hardware_convert_regval2uah,
#endif
#if defined(CONFIG_COUL_PMIC6X21V700) || \
	defined(CONFIG_COUL_PMIC6X21V600) || \
	defined(CONFIG_COUL_PMIC6X55V200)
#else
	.get_bat_temp = coul_get_tbat,
#endif

#if defined(CONFIG_COUL_PMIC6X21V600) || defined(CONFIG_COUL_PMIC6X55V200)
#elif defined(CONFIG_COUL_PMIC6X21V700)
	.get_chip_temp = coul_hardware_get_chip_temp,
#else
	.get_chip_temp = coul_get_tdie,
#endif
	.set_dev_cali_param = coul_set_cali_param,
	.coul_dev_check = coul_ic_check,
};

static int coul_hardware_create_sysfs(struct coul_device_info *di)
{
	int retval;

	retval = sysfs_create_group(&di->dev->kobj, &coul_attr_group);
	if (retval) {
		coul_hardware_err(
			"%s failed to create sysfs group!!!\n", __func__);
		return -1;
	}
	return 0;
}

static void get_bci_dts_info(struct coul_device_info *di)
{
	int ret;
	struct device_node *np = NULL;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,bci_battery");
	if (np == NULL) {
		di->is_board_type = BAT_BOARD_SFT;
		return;
	}
	ret = of_property_read_u32(np, "battery_board_type", &di->is_board_type);
	if (ret) {
		di->is_board_type = BAT_BOARD_SFT;
		coul_core_err("dts:get board type fail\n");
	}
	coul_hardware_inf("%s is_board_type %d\n", __func__, di->is_board_type);
}

static int coul_hardware_algo_register(struct device_node *root)
{
	struct device_node *child;
	int ret = 0;

	for_each_child_of_node(root, child) {
		ret = coul_core_register(&hi6xxx_coul_ops, child);
		if (ret) {
			coul_hardware_err("%s failed, ret %d\n", __func__, ret);
			return ret;
		}
	}

	return ret;
}

static int coul_hardware_probe(struct spmi_device *pdev)
{
	struct coul_device_info *di = NULL;
	struct device_node *np = NULL;
	int retval;

	di = (struct coul_device_info *)devm_kzalloc(
		     &pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di) {
		coul_hardware_err("%s failed to alloc di struct\n", __func__);
		return -1;
	}
	di->dev = &pdev->dev;
	np = di->dev->of_node;
	if (np == NULL) {
		coul_hardware_err("%s np is null!\n", __func__);
		return -1;
	}
	di->irq = spmi_get_irq_byname(pdev, NULL, "coulirq");
	if (di->irq < 0) {
		coul_hardware_err("failed to get irq resource!\n");
		return -1;
	}

	spmi_set_devicedata(pdev, di);

	if (coul_check_version(di)) {
		retval = -EINVAL;
		goto failed_0;
	}

	if (coul_check_debug()) {
		retval = -EINVAL;
		goto failed_0;
	}

	coul_set_default_cali_param(di);

	/* config coul ctrl and irq */
	coul_chip_init();

	/* Init interrupt notifier work */
	INIT_DELAYED_WORK(&di->irq_work, coul_interrupt_notifier_work);

	get_bci_dts_info(di);
	if (di->is_board_type == BAT_BOARD_ASIC) {
		retval = request_irq(di->irq, coul_irq_cb, IRQF_NO_SUSPEND,
			"hi6xxx_coul_irq", di);
		if (retval) {
			coul_hardware_err("Failed to request coul irq\n");
			goto failed_1;
		}
	}

	/* set shutdown vol level */
	coul_set_low_vol_val(DEFAULT_BATTERY_VOL_0_PERCENT);
	coul_enable_irq(0);

	retval = of_property_read_u32(
		of_find_compatible_node(NULL, NULL, "hisilicon,coul_core"),
		"r_coul_uohm", (u32 *)&g_r_coul_uohm);
	if (retval) {
		g_r_coul_uohm = R_COUL_UOHM;
		coul_hardware_err(
			"get r_coul_uohm fail, use default value 10000 uohm!\n");
	}

	retval = coul_hardware_create_sysfs(di);
	if (retval) {
		coul_hardware_err(
			"%s failed to create sysfs group!!!\n", __func__);
		goto failed_3;
	}

	g_coul_hardware_dev = di;
	coul_hardware_inf("coul hardware probe ok, chip %x, version is v%x !\n",
		di->chip_proj, di->chip_version);

	coul_soh_adc_cali();

	return coul_hardware_algo_register(np);

failed_3:
	sysfs_remove_group(&di->dev->kobj, &coul_attr_group);
	coul_disable_irq(0);
	free_irq(di->irq, di);
failed_1:
	cancel_delayed_work(&di->irq_work);
failed_0:
	spmi_set_devicedata(pdev, NULL);
	coul_hardware_err("coul hardware probe failed!\n");
	return retval;
}

static void coul_hardware_remove(struct spmi_device *pdev)
{
	struct coul_device_info *di = spmi_get_devicedata(pdev);

	if (di == NULL) {
		coul_hardware_err("[%s]di is null\n", __func__);
		return;
	}
	devm_kfree(&pdev->dev, di);
}

static const struct of_device_id coul_match_table[] = {
	{
		.compatible = "hisilicon,hi6xxx_coul",
	},
	{
	},
};

static const struct spmi_device_id coul_spmi_id[] = {
	{
		"hi6xxx_coul",
		0
	},
	{
	}
};

static struct spmi_driver coul_driver = {
	.probe = coul_hardware_probe,
	.remove = coul_hardware_remove,
	.driver = {
		.name = "hi6xxx_coul",
		.owner = THIS_MODULE,
		.of_match_table = coul_match_table,
	},
	.id_table = coul_spmi_id,
};

int __init coul_hardware_init(void)
{
	return spmi_driver_register(&coul_driver);
}

void __exit coul_hardware_exit(void)
{
	spmi_driver_unregister(&coul_driver);
}

fs_initcall(coul_hardware_init);
module_exit(coul_hardware_exit);

MODULE_DESCRIPTION("coul hardware driver");
MODULE_LICENSE("GPL");
