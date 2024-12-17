/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Device driver for schargerV700 buck
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "scharger_v700_buck.h"
#include "scharger_v700_include.h"

#ifndef abs
#define abs(a) ((a) > 0 ? (a) : -(a))  /* Absolute value */
#endif

static int charger_present_flag = 0;

struct opt_regs buck_common_v700_option_regs[] = {
	/* reg, mask, shift, val, before,after */
	reg_cfg(0x372, 0xff, 0, 0x6a, 0, 0),
	reg_cfg(0x378, 0xff, 0, 0x45, 0, 0),
	reg_cfg(0x925, 0xff, 0, 0x5f, 0, 0),
	reg_cfg(0x928, 0xff, 0, 0x12, 0, 0),
	reg_cfg(0x92d, 0xff, 0, 0x0d, 0, 0),
	reg_cfg(0x929, 0xff, 0, 0x28, 0, 0),
	reg_cfg(0x92a, 0xff, 0, 0xc8, 0, 0),
	reg_cfg(0x926, 0xff, 0, 0x0d, 0, 0),
	reg_cfg(0x92b, 0xff, 0, 0x38, 0, 0),
	reg_cfg(0x92f, 0xff, 0, 0x09, 0, 0),
};

struct opt_regs buck_common_v710_option_regs[] = {
	/* reg, mask, shift, val, before,after */
	reg_cfg(0x372, 0xff, 0, 0x6a, 0, 0),
	reg_cfg(0x378, 0xff, 0, 0x45, 0, 0),
	reg_cfg(0x925, 0xff, 0, 0x57, 0, 0),
	reg_cfg(0x928, 0xff, 0, 0x12, 0, 0),
	reg_cfg(0x92d, 0xff, 0, 0x0d, 0, 0),
	reg_cfg(0x929, 0xff, 0, 0x28, 0, 0),
	reg_cfg(0x926, 0xff, 0, 0x0d, 0, 0),
	reg_cfg(0x92b, 0xff, 0, 0x38, 0, 0),
	reg_cfg(0x92f, 0xff, 0, 0x0a, 0, 0),
	reg_cfg(0x923, 0xff, 0, 0x49, 0, 0),
};

struct opt_regs otg_option_regs[] = {
	/* reg, mask, shift, val, before,after */
	reg_cfg(0x37b, 0xff, 0, 0xc0, 0, 0),
	reg_cfg(0x37f, 0xff, 0, 0x2b, 0, 0),
	reg_cfg(0x37c, 0xff, 0, 0x34, 0, 0),
	reg_cfg(0x92d, 0xff, 0, 0x0d, 0, 0),
	reg_cfg(0x380, 0xff, 0, 0x20, 0, 0),
	reg_cfg(0x381, 0xff, 0, 0x59, 0, 0),
	reg_cfg(0x371, 0xff, 0, 0x05, 0, 0),
	reg_cfg(0x935, 0x06, 0, 0x04, 0, 0),
	reg_cfg(0x9A8, 0xff, 0, 0x0,  0, 0), // otg en delay: 32ms
};

struct opt_regs buck_v710_5v_extra_opt_regs[] = {
	/* reg, mask, shift, val, before,after */
	reg_cfg(0x92a, 0xff, 0, 0x48, 0, 0),
};

struct opt_regs buck_v710_9v_extra_opt_regs[] = {
	/* reg, mask, shift, val, before,after */
	reg_cfg(0x92a, 0xff, 0, 0x4b, 0, 0),
};

struct opt_regs buck_v710_12v_extra_opt_regs[] = {
	/* reg, mask, shift, val, before,after */
	reg_cfg(0x92a, 0xff, 0, 0x48, 0, 0),
};

enum usb_mode_state get_buck_charge_mode(void)
{
	if (g_buck == NULL)
		return -1;
	scharger_err("%s usb_mode = %d!\n", __func__, g_buck->usb_mode);
	return g_buck->usb_mode;
}

static void opt_regs_set(struct opt_regs *opt, unsigned int len)
{
	unsigned int i;
	struct scharger_buck *buck = g_buck;

	if (buck == NULL) {
		scharger_err("%s scharger_buck is NULL!\n", __func__);
		return;
	}

	for (i = 0; i < len; i++) {
		(void)mdelay(opt[i].before);
		(void)regmap_update_bits(buck->regmap, opt[i].reg, opt[i].mask,
					opt[i].val);
		(void)mdelay(opt[i].after);
	}
}

static struct power_log_ops scharger_log_ops = {
	.dev_name = "schargerv700",
	.dump_log_head = scharger_get_register_head,
	.dump_log_content = scharger_dump_register,
};

/*
 * chg_limit_v700[CHG_LMT_NUM_V700]: the current limit when setting charger
 *                                   value, unit: mA
 * These arrays are set to replace the tedious "else if" statements,instead,
 * a concise "for" statement is used in this version, the items transferred from
 * the original Macro definition like #define CHG_ILIMIT_1100           1100 in
 * the arrays are the values to be given to the variables in the "for" loop
 */
const int chg_limit_v700[CHG_LMT_NUM_V700] = { 85, 130, 200, 300, 400, 475, 600,
					       700, 800, 825, 1000, 1100, 1200,
					       1300, 1400, 1500, 1600, 1700,
					       1800, 1900, 2000, 2100, 2200,
					       2300, 2400, 2500, 2600, 2700,
					       2800, 2900, 3000, 3100 };


int set_batfet_single_ctrl(int enable, int bat_index)
{
	int ret = 0;
	u32 en = !!enable;

	/* BAT_L/BAT_H independent control */
	if (enable) {
		ret += regmap_update_bits(g_buck->regmap, SC_DBFET_SEL_REG,
			SC_DBFET_SEL_MSK, bat_index << SC_DBFET_SEL_SHIFT);
		ret += regmap_update_bits(g_buck->regmap, SC_DBFET_SEL_EN_REG,
			SC_DBFET_SEL_EN_MSK, DBFET_SINGLE_CTRL_MODE << SC_DBFET_SEL_EN_SHIFT);
	} else {
		ret += regmap_update_bits(g_buck->regmap, SC_DBFET_SEL_EN_REG,
					SC_DBFET_SEL_EN_MSK, DBFET_INDEP_CTRL_MODE << SC_DBFET_SEL_EN_SHIFT);
	}

	if (ret) {
		scharger_err("%s: fail\n", __func__);
		return -1;
	}
	return 0;
}

static int get_batfet_single_ctrl_status(void)
{
	int val = 0;

	(void)regmap_read(g_buck->regmap, SC_DBFET_SEL_EN_REG, &val);
	return val;
}

int scharger_psw_enable(int enable)
{
	int ret = 0;
	struct scharger_buck *buck = g_buck;

	if (buck == NULL) {
		scharger_err("%s scharger_buck is NULL!\n", __func__);
		return -ENOMEM;
	}
	scharger_inf("%s: set ovp2 %d\n", __func__, enable);

	if (buck->scharger_version == CHIP_ID_V700) {
		if (enable == 0) {
			ret += regmap_update_bits(buck->regmap, PSW_EN_REG, PSW_EN_MSK,
					PSW_DISABLE << PSW_EN_SHIFT);
			ret += regmap_update_bits(buck->regmap, PSW_MODE_REG, PSW_MODE_MSK,
					PSW_REVERSE_MODE << PSW_MODE_SHIFT);
		} else {
			ret += regmap_update_bits(buck->regmap, PSW_MODE_REG, PSW_MODE_MSK,
					PSW_REVERSE_MODE << PSW_MODE_SHIFT);
			ret += regmap_update_bits(buck->regmap, PSW_EN_REG, PSW_EN_MSK,
					PSW_ENABLE << PSW_EN_SHIFT);
		}
	} else {
		if (enable == 0) {
			ret += regmap_update_bits(buck->regmap, PSW_OVP_CFG_REG_1, OVP2_EN_MSK,
					PSW_DISABLE << OVP2_EN_SHIFT);
		} else {
			ret += regmap_update_bits(buck->regmap, PSW_OVP_CFG_REG_1, OVP2_EN_MSK,
					PSW_ENABLE << OVP2_EN_SHIFT);
		}
	}

	if (ret)
		scharger_err("%s: scharger psw %d fail\n", __func__, enable);

	return ret;
}

/* v700 does not support */
int scharger_ovp3_enable(int enable)
{
	int ret = 0;
	struct scharger_buck *buck = g_buck;

	if (buck == NULL) {
		scharger_err("%s scharger_buck is NULL!\n", __func__);
		return -ENOMEM;
	}

	scharger_inf("%s: set ovp3 %d\n", __func__, enable);

	if (enable == 0) {
		ret = regmap_update_bits(buck->regmap, PSW_OVP_CFG_REG_1, OVP3_EN_MSK,
				OVP3_DISABLE << OVP3_EN_SHIFT);
	} else {
		ret = regmap_update_bits(buck->regmap, PSW_OVP_CFG_REG_1, OVP3_EN_MSK,
				OVP3_ENABLE << OVP3_EN_SHIFT);
	}

	if (ret)
		scharger_err("%s: scharger psw %d fail\n", __func__, enable);

	return ret;
}

static int scharger_ovp_enable(struct scharger_buck *buck, int enable)
{
	int ret = 0;

	scharger_inf("%s: set ovp1 %d\n", __func__, enable);

	if (buck->ovpsw_status == POWER_SW_OFF) {
		ret += regmap_update_bits(buck->regmap, OVP_EN_REG, OVP_EN_MSK,
				 OVP_DISABLE << OVP_EN_SHIFT);
		ret += regmap_update_bits(buck->regmap, OVP_MODE_REG, OVP_MODE_MSK,
				 OVP_REVERSE_MODE << OVP_MODE_SHIFT);
	} else {
		ret += regmap_update_bits(buck->regmap, OVP_MODE_REG, OVP_MODE_MSK,
				 OVP_POSITIVE_MODE << OVP_MODE_SHIFT);
		ret += regmap_update_bits(buck->regmap, OVP_EN_REG, OVP_EN_MSK,
				 OVP_ENABLE << OVP_EN_SHIFT);
	}

	if (ret)
		scharger_err("%s: scharger ovp %d fail\n", __func__, enable);

	return ret;
}

static int get_input_current_set(void)
{
	return iin_set;
}

static int scharger_get_terminal_voltage(void)
{
	unsigned int data_1 = 0;
	unsigned int data_2 = 0;
	int cv_1 = 0;
	int cv_2 = 0;

	int ret;

	ret = regmap_read(g_buck->regmap, CHG_FAST_VCHG_REG, &data_1);
	if (ret < 0) {
		scharger_err("%s err\n", __func__);
		return FALSE;
	}
	data_1 &= CHG_FAST_VCHG_MSK;
	data_1 >>= CHG_FAST_VCHG_SHIFT;

	/* transfer from code value to actual value ,step 8mV */
	cv_1 = (int)((data_1 * CHG_FAST_VCHG_STEP_8000UV +
		     CHG_FAST_VCHG_BASE_UV) / 1000);
	if (g_buck->scharger_version == CHIP_ID_V700) {
		return cv_1;
	} else {
		ret = regmap_read(g_buck->regmap, REG_CHG_TOP_CFG_REG_17, &data_2);
		if (ret < 0) {
			scharger_err("%s err\n", __func__);
			return FALSE;
		}
		data_2 &= BATL_MINUTE_ADJUST_CV_MSK;
		data_2 >>= BATL_MINUTE_ADJUST_CV_SHIFT;
		cv_2 = (int)data_2;
		return cv_1 + cv_2;
	}
}

/*
 * Description: check whether VINDPM or IINDPML
 * return value: TRUE means VINDPM or IINDPM
 *               FALSE means NoT DPM
 */
static int scharger_check_input_dpm_state(void)
{
	unsigned int reg = 0;
	int ret;
	struct scharger_buck *buck = g_buck;

	if (buck == NULL) {
		scharger_err("%s scharger_buck is NULL!\n", __func__);
		return -ENOMEM;
	}

	ret = regmap_read(buck->regmap, CHG_TOP_RO_REG24_STATE, &reg);
	if (ret < 0) {
		scharger_err("%s err\n", __func__);
		return FALSE;
	}

	if ((reg & CHG_IN_DPM_STATE) == CHG_IN_DPM_STATE) {
		scharger_inf("CHG_STATUS0_REG:%x in vdpm state\n", reg);
		return TRUE;
	}

	return FALSE;
}

/* return value: TRUE means acl, FALSE means NoT acl */
static int check_input_acl_state(void)
{
	unsigned int reg = 0;
	int ret = 0;
	struct scharger_buck *buck = g_buck;

	scharger_dbg("%s\n", __func__);

	if (buck == NULL) {
		scharger_err("%s scharger_buck is NULL!\n", __func__);
		return -ENOMEM;
	}

	ret += regmap_update_bits(buck->regmap, CHG_ACL_RPT_EN_REG,
				 CHG_ACL_PRT_EN_MASK, true << CHG_ACL_RPT_EN_SHIFT);
	ret += regmap_read(buck->regmap, CHG_TOP_RO_REG24_STATE, &reg);

	ret += regmap_update_bits(buck->regmap, CHG_ACL_RPT_EN_REG,
				 CHG_ACL_PRT_EN_MASK, true << CHG_ACL_RPT_EN_SHIFT);
	if (ret) {
		scharger_err("%s err\n", __func__);
		return FALSE;
	}

	if ((reg & CHG_IN_ACL_STATE) == CHG_IN_ACL_STATE) {
		scharger_inf("CHG_STATUS0_REG:%x in acl state\n", reg);
		return TRUE;
	}

	return FALSE;
}

/* return value: TRUE means therm, FALSE means NoT therm */
static int check_therm_state(void)
{
	unsigned int reg = 0;
	int ret;
	struct scharger_buck *buck = g_buck;

	if (buck == NULL) {
		scharger_err("%s scharger_buck is NULL!\n", __func__);
		return -ENOMEM;
	}

	ret = regmap_read(buck->regmap, CHG_TOP_RO_REG26_STATE, &reg);
	if (ret < 0) {
		scharger_err("%s err\n", __func__);
		return FALSE;
	}

	if ((reg & CHG_IN_THERM_STATE) == CHG_IN_THERM_STATE)
		return TRUE;
	return FALSE;
}

static void set_boot_weaksource_flag(void)
{
	unsigned int reg_val;

	reg_val = pmic_read_reg(WEAKSOURCE_FLAG_REG);
	reg_val |= WAEKSOURCE_FLAG;
	(void)pmic_write_reg(WEAKSOURCE_FLAG_REG, reg_val);
}

/*
 * limit buck current to 470ma according to rboost count
 * Return: 0: do nothing; 1:limit buck current 470ma
 */
static int scharger_rboost_buck_limit(void)
{
	struct scharger_buck *buck = g_buck;

	scharger_dbg("%s\n", __func__);

	if (buck == NULL) {
		scharger_err("%s scharger_buck is NULL!\n", __func__);
		return -ENOMEM;
	}

	if (buck->reverbst_cnt > ILIMIT_RBOOST_CNT) {
		set_boot_weaksource_flag();
		scharger_inf("%s:rboost cnt:%d\n", __func__, buck->reverbst_cnt);
		return 1;
	}
	buck->reverbst_cnt = 0;
	return 0;
}

/*
 * Parameters:   enable:terminal enable or not
 *               0&1:dbc control. 2:original charger procedure
 * return value:  0-success or others-fail
 */
static int force_set_term_enable(int enable)
{
	scharger_dbg("%s enable:%d\n", __func__, enable);

	if ((enable == 0) || (enable == 1)) {
		force_set_term_flag = CHG_STAT_ENABLE;
	} else {
		force_set_term_flag = CHG_STAT_DISABLE;
		return 0;
	}
	return regmap_update_bits(g_buck->regmap, CHG_EN_TERM_REG,
				 CHG_EN_TERM_MSK, (u8)enable << CHG_EN_TERM_SHIFT);
}

static int set_batfet_h_enable(int enable)
{
	int ret = 0;
	u32 en = !!enable;

	if (batfet_h_chg_err_flag == 1) {
		ret = regmap_update_bits(g_buck->regmap, BATFET_H_CTRL_CFG_REG, BATFET_H_CTRL_CFG_MSK
					 , BATFET_H_DISABLE << BATFET_H_CTRL_CFG_SHIFT);
		scharger_err("%s batfet_h en err, batfet_h_chg_err_flag:%u\n", __func__, batfet_h_chg_err_flag);
		return ret;
	}
	ret = regmap_update_bits(g_buck->regmap, BATFET_H_CTRL_CFG_REG, BATFET_H_CTRL_CFG_MSK
					 , en << BATFET_H_CTRL_CFG_SHIFT);

	return ret;
}

static int set_batfet_ctrl(u32 status)
{
	int ret = 0;

	ret += regmap_update_bits(g_buck->regmap, BATFET_L_CTRL_CFG_REG, BATFET_L_CTRL_CFG_MSK
					 , !!status << BATFET_L_CTRL_CFG_SHIFT);
	ret += set_batfet_h_enable(status);

	return ret;
}

static int scharger_set_batfet_disable(int disable)
{
	scharger_inf("%s disable %d\n", __func__, disable);

	return set_batfet_ctrl(!disable);
}

static int batfet_init(void)
{
	struct scharger_buck *buck = g_buck;
	int ret = 0;

	if (buck == NULL) {
		scharger_err("%s scharger_buck is NULL!\n", __func__);
		return -ENOMEM;
	}

	/* BAT_L/BAT_H independent control */
	ret += regmap_update_bits(buck->regmap, SC_DBFET_SEL_REG,
				 SC_DBFET_SEL_MSK, DBFET_BAT_H_OFF << SC_DBFET_SEL_SHIFT);
	ret += regmap_update_bits(buck->regmap, SC_DBFET_SEL_EN_REG,
				 SC_DBFET_SEL_EN_MSK, DBFET_INDEP_CTRL_MODE << SC_DBFET_SEL_EN_SHIFT);
	if (ret) {
		scharger_err("%s: batfet_init fail\n", __func__);
		return -1;
	}
	return 0;
}

static int scharger_set_otg_current(int value)
{
	u8 reg;
	unsigned int temp_current_ma;

	scharger_dbg("%s %smA\n", __func__, value);

	temp_current_ma = (unsigned int)value;

	if (temp_current_ma < BOOST_LIM_MIN || temp_current_ma > BOOST_LIM_MAX)
		scharger_inf("set otg current %dmA is out of range!\n", value);
	if (temp_current_ma < BOOST_LIM_500)
		reg = 0; /* 0: marking 0 ~ BOOST_LIM_500 exclude 0 and 500 */
	else if (temp_current_ma >= BOOST_LIM_500 && temp_current_ma <
		 BOOST_LIM_1000)
		reg = 0; /* 0: marking [BOOST_LIM_500, BOOST_LIM_1000) */
	else if (temp_current_ma >= BOOST_LIM_1000 && temp_current_ma <
		 BOOST_LIM_1500)
		reg = 1; /* 1: marking [BOOST_LIM_1000, BOOST_LIM_1500) */
	else if (temp_current_ma >= BOOST_LIM_1500 && temp_current_ma <
		 BOOST_LIM_2000)
		reg = 2; /* 2: marking [BOOST_LIM_1500, BOOST_LIM_2000) */
	else if (temp_current_ma >= BOOST_LIM_2000 && temp_current_ma <
		 BOOST_LIM_2700)
		reg = 3; /* 3: 2000mA */
	else
		reg = 3;

	scharger_inf("otg current reg is set 0x%x\n", reg);
	return regmap_update_bits(g_buck->regmap, CHG_OTG_REG0, CHG_OTG_LIM_MSK,
				 reg << CHG_OTG_LIM_SHIFT);
}

/*
 * return value: charger state:
 *               1:Charge Termination Done
 *               0:Not Charging and buck is closed;Pre-charge;Fast-charg;
 *              -1:error
 */
static int scharger_get_charger_state(void)
{
	int ret;
	unsigned int data = 0;
	u8 data_h, data_l = 0;
	int state = 0;
	struct scharger_buck *buck = g_buck;

	scharger_dbg("%s +\n", __func__);

	if (buck == NULL) {
		scharger_err("%s scharger_buck is NULL!\n", __func__);
		return -ENOMEM;
	}

	ret = regmap_read(buck->regmap, CHG_STATUS_REG, &data);
	if (ret) {
		scharger_err("[%s]:read charge status reg fail\n", __func__);
		return -1;
	}

	data_h = data;
	data_l = data;

	data_h &= CHG_STAT_H_MASK;
	data_h = data_h >> CHG_STAT_H_SHIFT;
	data_l &= CHG_STAT_L_MASK;
	data_l = data_l >> CHG_STAT_L_SHIFT;
	if (buck->bat_sel == DUAL_BATTERY) {
		if (data_h == CHG_TERM_DONE && data_l == CHG_TERM_DONE)
			state = 1;
	} else {
		if (data_l == CHG_TERM_DONE)
			state = 1;
	}
	return state;
}

static int scharger_get_charge_state(unsigned int *state)
{
	unsigned int reg0 = 0;
	unsigned int reg1 = 0;
	unsigned int reg2 = 0;
	int ret = 0;
	struct scharger_buck *buck = g_buck;

	if (buck == NULL || !state) {
		scharger_err("%s scharger_buck is NULL!\n", __func__);
		return -ENOMEM;
	}

	*state = 0;
	ret += regmap_read(buck->regmap, CHG_BUCK_STATUS_REG, &reg0);
	ret += regmap_read(buck->regmap, CHG_STATUS_REG, &reg1);
	ret += regmap_read(buck->regmap, WATCHDOG_STATUS_REG, &reg2);
	if (ret) {
		scharger_err("[%s]read charge status reg fail\n", __func__);
		return -1;
	}

	if ((reg0 & CHG_BUCK_OK) != CHG_BUCK_OK)
		*state |= CHAGRE_STATE_NOT_PG;
	if (buck->bat_sel == DUAL_BATTERY) {
		if (((reg1 & CHG_STAT_CHARGE_DONE_H) == CHG_STAT_CHARGE_DONE_H) &&
					((reg1 & CHG_STAT_CHARGE_DONE_L) == CHG_STAT_CHARGE_DONE_L))
			*state |= CHAGRE_STATE_CHRG_DONE;
	} else {
		if ((reg1 & CHG_STAT_CHARGE_DONE_L) == CHG_STAT_CHARGE_DONE_L)
			*state |= CHAGRE_STATE_CHRG_DONE;
	}
	if ((reg2 & WATCHDOG_OK) != WATCHDOG_OK)
		*state |= CHAGRE_STATE_WDT_FAULT;

	scharger_inf("%s >>> reg0:0x%x, reg1 0x%x, reg2 0x%x, state 0x%x\n",
		     __func__, reg0, reg1, reg2, *state);
	return 0;
}

static int set_vbus_ovp(int vbus)
{
	int ret;
	u8 ov_vol;

	if (vbus < VBUS_VSET_9V)
		ov_vol = 0; /* 0: set ovp to 6.5V */
	else if (vbus < VBUS_VSET_12V)
		ov_vol = 1; /* 1: set ovp to 10.5V */
	else
		ov_vol = 2; /* 2: set ovp to default value 13.5V */

	ret = regmap_update_bits(g_buck->regmap, REG_DET_TOP_CFG_REG_5, CHG_BUCK_OVP_SET_MSK,
		ov_vol << CHG_BUCK_OVP_SET_SHIFT);
	return ret;
}

static bool get_charge_enable(void)
{
	unsigned int charge_state = 0;

	(void)regmap_read(g_buck->regmap, CHG_OTG_MODE_CFG_REG, &charge_state);

	if (charge_state == DA_CHG_EN)
		return TRUE;
	else
		return FALSE;
}

static int set_vbus_uvp_ovp(int vbus)
{
	int ret = 0;
	const u8 uv_vol = 0; /* 0: representing 3.8V */
	struct scharger_buck *buck = g_buck;

	if (buck == NULL) {
		scharger_err("%s scharger_buck is NULL!\n", __func__);
		return -ENOMEM;
	}

	ret += set_vbus_ovp(vbus);
	ret += regmap_update_bits(buck->regmap, CHG_UVP_VOLTAGE_REG,
				 CHG_BUCK_UVP_VOLTAGE_MSK, uv_vol << CHG_BUCK_UVP_VOLTAGE_SHIFT);
	if (ret) {
		scharger_err("%s:uvp&ovp voltage set failed, ret %d\n", __func__, ret);
		return -1;
	}
	return 0;
}

int set_vusb_ovp(int ov_vol)
{
	int ret = 0;
	unsigned int reg_val;

	if (ov_vol < USB_OVP_11V)
		reg_val = 0; /* set vusb_ovp 7.5V */
	else if (ov_vol < USB_OVP_14V)
		reg_val = 1; /* set vusb_ovp 11V */
	else if (ov_vol < USB_OVP_15V)
		reg_val = 2; /* set vusb_ovp 14V */
	else if (ov_vol < USB_OVP_18V)
		reg_val = 3; /* set vusb_ovp 15V */
	else if (ov_vol < USB_OVP_22V)
		reg_val = 4; /* set vusb_ovp 18V */
	else if (ov_vol < USB_OVP_23V)
		reg_val = 5; /* set vusb_ovp 22V */
	else if (ov_vol < USB_OVP_24V)
		reg_val = 6; /* set vusb_ovp 23V */
	else
		reg_val = 7; /* set vusb_ovp 24V */

	scharger_inf("%s usb ovp reg_val:%u\n", __func__, reg_val);
	ret = regmap_update_bits(g_buck->regmap, REG_USB_OVP_CFG_REG_2, CHG_USB_OVP_SET_MSK,
		reg_val << CHG_USB_OVP_SET_SHIFT);
	if (ret)
		scharger_err("%s set vusb ovp fail\n", __func__);

	return ret;
}

/* Parameters: vbus_set voltage: 5V/9V/12V */
static int scharger_set_vbus_vset(u32 value)
{
	u8 data;
	u32 charger_flag = 0;
	struct scharger_buck *buck = g_buck;

	if (buck == NULL) {
		scharger_err("%s scharger_buck is NULL!\n", __func__);
		return -ENOMEM;
	}

	scharger_dbg("%s:%u v\n", __func__, value);

	/* check charge state, if open, close charge */
	if (get_charge_enable() == TRUE) {
		scharger_set_charge_enable(CHG_DISABLE);
		charger_flag = 1;
	}
	if (value < VBUS_VSET_9V) {
		data = 0; /* 0: set vbus to 5V */
	} else if (value < VBUS_VSET_12V) {
		data = 1; /* 1: set vbus to 9V */
	} else {
		data = 2; /* 2: set vbus to 12V */
	}
	buck->buck_vbus_set = (int)value;

	/* resume charge state */
	if (charger_flag == 1)
		scharger_set_charge_enable(CHG_ENABLE);

	set_vbus_uvp_ovp(value);
	set_vusb_ovp(buck->param_dts.vusb_ovp);

	return regmap_update_bits(buck->regmap, CHG_VBUS_VSET_REG,
				 VBUS_VSET_MSK, data << VBUS_VSET_SHIFT);
}

static int scharger_stop_charge_config(void)
{
	int ret;
	struct scharger_buck *buck = g_buck;

	scharger_dbg("%s\n", __func__);

	if (buck == NULL) {
		scharger_err("%s scharger_buck is NULL!\n", __func__);
		return -ENOMEM;
	}

	ret = scharger_set_vbus_vset(VBUS_VSET_5V);

	is_weaksource = WEAKSOURCE_FALSE;
	buck->reverbst_cnt = 0;

	return ret;
}

int scharger_set_charge_current(int charge_current)
{
	u8 i_chg_limit;
	/* Chip limit */
	int min_curr = CHG_FAST_ICHG_100MA;
	int max_curr = CHG_FAST_ICHG_2500MA;
	int ret = 0;
	int vbatt_mv_l, vbatt_mv_h;
	struct scharger_buck *buck = g_buck;

	scharger_dbg("%s %d\n", __func__, charge_current);
	if (buck == NULL) {
		scharger_err("%s scharger_buck is NULL!\n", __func__);
		return -ENOMEM;
	}

	if (buck->bat_sel == DUAL_BATTERY &&
		dc_get_direct_charge_mode() != SC &&
		!get_batfet_single_ctrl_status()) {
		vbatt_mv_l = coul_merge_drv_battery_voltage_mv(BATT_L);
		vbatt_mv_h = coul_merge_drv_battery_voltage_mv(BATT_H);
		if (abs(vbatt_mv_l - vbatt_mv_h) < CHG_VBAT_TH)
			charge_current /= 2;
	}

	if (dc_get_direct_charge_mode() == SC) {
		max_curr = CHG_FAST_ICHG_3000MA;
		scharger_dbg("%s now is sc,ibat max %d\n", __func__, CHG_FAST_ICHG_3000MA);
	}

	if (charge_current > max_curr)
		charge_current = max_curr;
	else if (charge_current < min_curr)
		charge_current = min_curr;

	i_chg_limit = (u8)((charge_current / CHG_FAST_ICHG_STEP_100) - 1);

	dev_dbg(buck->dev, "%s: %dmA, reg is set 0x%x\n", __func__, charge_current,
		      i_chg_limit);
	ret += regmap_update_bits(buck->regmap, CHG_FAST_CURRENT_H_REG,
				 CHG_FAST_ICHG_H_MSK, i_chg_limit << CHG_FAST_ICHG_H_SHIFT);
	ret += regmap_update_bits(buck->regmap, CHG_FAST_CURRENT_L_REG,
				 CHG_FAST_ICHG_L_MSK, i_chg_limit << CHG_FAST_ICHG_L_SHIFT);

	return ret;
}

static int scharger_get_charge_current(void)
{
	int ret;
	unsigned int reg = 0;
	int charge_current;
	struct scharger_buck *buck = g_buck;

	scharger_dbg("%s +\n", __func__);

	if (buck == NULL) {
		scharger_err("%s scharger_buck is NULL!\n", __func__);
		return -ENOMEM;
	}

	ret = regmap_read(buck->regmap, CHG_FAST_CURRENT_L_REG, &reg);
	if (ret) {
		scharger_err("read mask fail\n");
		return CHG_FAST_ICHG_00MA;
	}

	charge_current = (reg + 1) * CHG_FAST_ICHG_STEP_100;

	return charge_current;
}

/* Parameters: vbus_vol:5V/9V/12V */
static void buck_opt_param(int vbus_vol, struct scharger_buck *buck)
{
	if (buck == NULL) {
		scharger_err("%s scharger_buck is NULL!\n", __func__);
		return;
	}
	if (buck->scharger_version == CHIP_ID_V700) {
		opt_regs_set(buck_common_v700_option_regs,
			    ARRAY_SIZE(buck_common_v700_option_regs));
	} else {
		opt_regs_set(buck_common_v710_option_regs,
			    ARRAY_SIZE(buck_common_v710_option_regs));
		switch (vbus_vol) {
		case VBUS_VSET_5V:
			opt_regs_set(buck_v710_5v_extra_opt_regs,
					ARRAY_SIZE(buck_v710_5v_extra_opt_regs));
			break;
		case VBUS_VSET_9V:
			opt_regs_set(buck_v710_9v_extra_opt_regs,
					ARRAY_SIZE(buck_v710_9v_extra_opt_regs));
			break;
		case VBUS_VSET_12V:
			opt_regs_set(buck_v710_12v_extra_opt_regs,
						ARRAY_SIZE(buck_v710_12v_extra_opt_regs));
			break;
		default:
			break;
		}
	}
}

int scharger_set_input_current(int cin_limit)
{
	u8 i;
	int max;
	u8 i_in_limit = 0;
	struct scharger_buck *buck = g_buck;

	scharger_dbg("%s cin_limit %d\n", __func__, cin_limit);

	if (buck == NULL) {
		scharger_err("%s scharger_buck is NULL!\n", __func__);
		return -ENOMEM;
	}

	if (buck->buck_vbus_set < VBUS_VSET_9V)
		max = CHG_ILIMIT_2500;
	else if (buck->buck_vbus_set < VBUS_VSET_12V)
		max = CHG_ILIMIT_1400;
	else
		max = CHG_ILIMIT_1200;

	if (dc_get_direct_charge_mode() == SC) {
		max = CHG_ILIMIT_1600;
		scharger_dbg("%s now is sc ,ibus max %d\n", __func__, CHG_ILIMIT_1600);
	}

	buck->input_current = cin_limit;

	if (cin_limit > max) {
		scharger_err("%s cin_limit %d, max %d, vbus set is %d\n",
			      __func__, cin_limit, max, buck->buck_vbus_set);
		cin_limit = max;
	}

	for (i = 0; i < CHG_LMT_NUM_V700; i++) {
		if (cin_limit >= chg_limit_v700[i])
			i_in_limit = i;
		else
			break;
	}
	scharger_dbg("%s:cin_limit %d ma, reg is set 0x%x, buck_vbus_set %d\n", __func__,
		      cin_limit, i_in_limit, buck->buck_vbus_set);

	iin_set = cin_limit;
	(void)regmap_update_bits(buck->regmap, CHG_INPUT_SOURCE_REG,
				CHG_ILIMIT_MSK, i_in_limit << CHG_ILIMIT_SHIFT);
	return 0;
}

static void reverbst_delay_work(struct work_struct *work)
{
	struct scharger_buck *buck = g_buck;

	if (buck == NULL)
		return;

	(void)regmap_update_bits(buck->regmap, CHG_ANTI_REVERBST_REG,
				CHG_ANTI_REVERBST_EN_MSK,
				CHG_ANTI_REVERBST_EN << CHG_ANTI_REVERBST_EN_SHIFT);
}

static void set_anti_reverbst_reset(void)
{
	struct scharger_buck *buck = g_buck;

	if (buck == NULL)
		return;

	(void)regmap_update_bits(buck->regmap, CHG_ANTI_REVERBST_REG,
				CHG_ANTI_REVERBST_EN_MSK,
				CHG_ANTI_REVERBST_DIS << CHG_ANTI_REVERBST_EN_SHIFT);
	queue_delayed_work(system_power_efficient_wq, &buck->reverbst_work,
			   msecs_to_jiffies(REVERBST_DELAY_ON));
}

int scharger_set_buck_mode_enable(u32 enable)
{
	struct scharger_buck *buck = g_buck;

	if (buck == NULL) {
		scharger_err("%s scharger_buck is NULL!\n", __func__);
		return -ENOMEM;
	}

	if (enable)
		set_anti_reverbst_reset();

	if (enable) {
		buck->batt_ovp_cnt_30s = 0;
		buck->chg_mode = BUCK;
	} else if (buck->chg_mode == BUCK) {
		buck->chg_mode = NONE;
	}

	(void)regmap_update_bits(buck->regmap, CHG_HIZ_CTRL_REG,
				 CHG_HIZ_ENABLE_MSK, !enable << CHG_HIZ_ENABLE_SHIFT);

	return 0;
}

/* set the charger hiz close watchdog */
static int scharger_set_charger_hiz(int enable)
{
	scharger_inf("%s enable %d\n", __func__, enable);

	return scharger_set_buck_mode_enable(!enable);
}

int scharger_set_charge_enable(int enable)
{
	struct scharger_buck *buck = g_buck;

	scharger_inf("%s %d\n", __func__, enable);

	if (buck == NULL)
		return -ENOMEM;
	/* invalidate charge enable on udp board */
	if ((buck->is_board_type == BAT_BOARD_UDP) && (enable == CHG_ENABLE))
		return 0;

	if (charge_err_flag == 1) {
		scharger_inf("%s charge_err_flag:%u, disable charge\n", __func__, charge_err_flag);
		return regmap_update_bits(buck->regmap, CHG_OTG_MODE_CFG_REG,
					CHG_OTG_MODE_CFG_MSK, DA_CHG_DIS << CHG_OTG_MODE_CFG_SHIFT);
	}

	if (enable)
		return regmap_update_bits(buck->regmap, CHG_OTG_MODE_CFG_REG,
					CHG_OTG_MODE_CFG_MSK, DA_CHG_EN << CHG_OTG_MODE_CFG_SHIFT);
	else
		return regmap_update_bits(buck->regmap, CHG_OTG_MODE_CFG_REG,
					CHG_OTG_MODE_CFG_MSK, DA_CHG_DIS << CHG_OTG_MODE_CFG_SHIFT);
}

static int scharger_otg_switch_mode(int enable)
{
	struct scharger_buck *buck = g_buck;

	if (buck == NULL) {
		scharger_err("%s scharger_buck is NULL!\n", __func__);
		return -ENOMEM;
	}
	scharger_inf("%s enable %d\n", __func__, enable);

	(void)regmap_update_bits(buck->regmap, CHG_OTG_SWITCH_CFG_REG,
				 CHG_OTG_SWITCH_MASK, !!enable << CHG_OTG_SWITCH_SHIFT);

	/* OTG switch mode disable RENG */
	if (buck->scharger_version == CHIP_ID_V700) {
		return 0;
	} else {
		(void)regmap_update_bits(buck->regmap, CHG_PSEL_TOP_CFG_REG_2_REG,
				 OTG_REGN_FORCE_CLOSE_EN_MSK, !!enable << OTG_REGN_FORCE_CLOSE_EN_SHIFT);
		(void)regmap_update_bits(buck->regmap, CHG_BUCK_OTG_CFG_REG_34_REG,
				 OTG_REGN_EN_MSK, !!enable << OTG_REGN_EN_SHIFT);
	}

	return 0;
}

static int scharger_set_otg_enable(int enable)
{
	struct scharger_buck *buck = g_buck;
	static int last_state = 0;

	if (buck == NULL) {
		scharger_err("%s scharger_buck is NULL!\n", __func__);
		return -ENOMEM;
	}

	mutex_lock(&buck->otg_lock);
	scharger_inf("%s %d\n", __func__, enable);

	if (enable) {
		opt_regs_set(otg_option_regs, ARRAY_SIZE(otg_option_regs));
		(void)regmap_update_bits(buck->regmap, CHG_OTG_MODE_CFG_REG,
				 CHG_OTG_MODE_CFG_MSK, DA_CHG_OTG_EN << CHG_OTG_MODE_CFG_SHIFT);

		mdelay(35);
		if (hisi_usb_typec_sourcing_vbus())
			(void)regmap_update_bits(buck->regmap, OVP_MODE_REG, OVP_MODE_MSK, OVP_REVERSE_MODE);
	} else {
		scharger_otg_switch_mode(0);
		(void)regmap_update_bits(buck->regmap, CHG_OTG_MODE_CFG_REG,
				 CHG_OTG_MODE_CFG_MSK, DA_CHG_OTG_DIS << CHG_OTG_MODE_CFG_SHIFT);

		mdelay(50); /* 50: for chip set requirement */
		(void)regmap_update_bits(buck->regmap, OVP_MODE_REG, OVP_MODE_MSK, OVP_POSITIVE_MODE);

		/* Set optimization parameters to buck mode */
		if (last_state != 0)
			buck_opt_param(VBUS_VSET_5V, buck);
	}
	last_state = enable;

	mutex_unlock(&buck->otg_lock);
	return 0;
}

static int scharger_soft_vbatt_ovp_protect(void)
{
	struct scharger_buck *buck = g_buck;
	int vbatt_mv_l, vbatt_mv_h, vbatt_max_l, vbatt_max_h;
	int is_vbat_ovp = 0;

	scharger_dbg("%s\n", __func__);

	if (buck == NULL)
		return -ENOMEM;
	vbatt_mv_l = coul_merge_drv_battery_voltage_mv(BATT_L);
	vbatt_mv_h = coul_merge_drv_battery_voltage_mv(BATT_H);
	vbatt_max_l = coul_merge_drv_battery_vbat_max(BATT_L);
	vbatt_max_h = coul_merge_drv_battery_vbat_max(BATT_H);

	if (buck->bat_sel == DUAL_BATTERY)
		is_vbat_ovp = ((vbatt_mv_l >= min(CHG_VBATT_SOFT_OVP_MAX, chg_vbatt_cv_103(vbatt_max_l)))
			|| (vbatt_mv_h >= min(CHG_VBATT_SOFT_OVP_MAX, chg_vbatt_cv_103(vbatt_max_h))));
	else
		is_vbat_ovp = (vbatt_mv_l >= min(CHG_VBATT_SOFT_OVP_MAX, chg_vbatt_cv_103(vbatt_max_l)));

	if (is_vbat_ovp) {
		buck->batt_ovp_cnt_30s++;
		if (buck->batt_ovp_cnt_30s == CHG_VBATT_SOFT_OVP_CNT) {
			scharger_set_charger_hiz(TRUE);
			scharger_err("%s:vbat_l:%d,vbat_h:%d,cv_mv:%d,ovp_cnt:%u,shutdown buck\n",
				     __func__, vbatt_mv_l, vbatt_mv_h, buck->term_vol_mv,
				     buck->batt_ovp_cnt_30s);
			buck->batt_ovp_cnt_30s = 0;
		}
	} else {
		buck->batt_ovp_cnt_30s = 0;
	}
	return 0;
}

/*
 * set the terminal current in charging process
 * (min value is 400ma for scharger ic bug)
 */
int scharger_set_terminal_current(int term_current)
{
	u8 i_term;
	struct scharger_buck *buck = g_buck;

	if (buck == NULL) {
		scharger_err("%s scharger_buck is NULL!\n", __func__);
		return -ENOMEM;
	}

	if (term_current < CHG_TERM_ICHG_200MA)
		i_term = 0; /* 0: <200mA */
	else if (term_current >= CHG_TERM_ICHG_200MA &&
		term_current < CHG_TERM_ICHG_300MA)
		i_term = 1; /* 1: 200mA */
	else if (term_current >= CHG_TERM_ICHG_300MA &&
		term_current < CHG_TERM_ICHG_400MA)
		i_term = 2; /* 2: 300mA */
	else
		i_term = 3; /* 3: default 400mA */

	/* for chip bug */
	i_term = 3; /* 3: default 400mA */

	dev_dbg(buck->dev, "term_current: %d, term current reg is set 0x%x\n",
		      term_current, i_term);
	return regmap_update_bits(buck->regmap, CHG_TERM_ICHG_REG,
				 CHG_TERM_ICHG_MSK, i_term << CHG_TERM_ICHG_SHIFT);
}

static int __set_terminal_voltage(int charge_voltage)
{
	int ret;
	u8 data;
	u32 data_minute_adjust;
	u8 minute_adjust_cv;
	struct scharger_buck *buck = g_buck;

	if (buck == NULL) {
		scharger_err("%s scharger_buck is NULL!\n", __func__);
		return -ENOMEM;
	}

	scharger_inf("%s set terminal voltage:%d\n", __func__, charge_voltage);

	if (charge_voltage < CHG_FAST_VCHG_MIN)
		charge_voltage = CHG_FAST_VCHG_MIN;
	else if (charge_voltage > CHG_FAST_VCHG_MAX)
		charge_voltage = CHG_FAST_VCHG_MAX;

	buck->term_vol_mv = charge_voltage;
	/* transfer from code value to actual value */
	data = (u8)((charge_voltage - CHG_FAST_VCHG_MIN) * 1000 / CHG_FAST_VCHG_STEP_8000UV);

	ret = regmap_update_bits(buck->regmap, CHG_FAST_VCHG_REG,
				 CHG_FAST_VCHG_MSK, data << CHG_FAST_VCHG_SHIFT);

	if (buck->scharger_version == CHIP_ID_V700) {
	} else {
		data_minute_adjust = ((u32)charge_voltage - CHG_FAST_VCHG_MIN) % (CHG_FAST_VCHG_STEP_8000UV / 1000);
		minute_adjust_cv = MINUTE_ADJUST_CV_MAX - data_minute_adjust;

		scharger_inf("%s data:%d, data_minute_adjust:%d\n", __func__, data, data_minute_adjust);

		/* minor adjustment cv */
		ret += regmap_update_bits(buck->regmap, REG_CHG_TOP_CFG_REG_18,
					BATH_MINUTE_ADJUST_CV_SIGN_MSK, 0 << BATH_MINUTE_ADJUST_CV_SIGN_SHIFT);
		ret += regmap_update_bits(buck->regmap, REG_CHG_TOP_CFG_REG_18,
					BATH_MINUTE_ADJUST_CV_MSK, minute_adjust_cv << BATH_MINUTE_ADJUST_CV_SHIFT);

		ret += regmap_update_bits(buck->regmap, REG_CHG_TOP_CFG_REG_17,
					BATL_MINUTE_ADJUST_CV_SIGN_MSK, 1 << BATL_MINUTE_ADJUST_CV_SIGN_SHIFT);
		ret += regmap_update_bits(buck->regmap, REG_CHG_TOP_CFG_REG_17,
					BATL_MINUTE_ADJUST_CV_MSK, data_minute_adjust << BATL_MINUTE_ADJUST_CV_SHIFT);
	}
	return ret;
}

int scharger_set_terminal_voltage(int charge_voltage)
{
	int ret = 0;

	scharger_inf("%s charge_voltage:%d\n", __func__, charge_voltage);

	if (g_cv_info->cv_ori_val != charge_voltage) {
		g_cv_info->cv_trim_flag = 0;
		scharger_err("%s The vterm:%d has changed, reset vterm:%d\n", __func__,
				 g_cv_info->cv_ori_val, charge_voltage);
	}
	if (g_cv_info->cv_trim_flag == 1) {
		charge_voltage = g_cv_info->cv_new_set_val;
		scharger_err("%s The vterm:%d has already been trimmed, use the trimmed value:%d\n",
				 __func__, g_cv_info->cv_ori_val, g_cv_info->cv_new_set_val);
	}

	ret = __set_terminal_voltage(charge_voltage);

	return ret;
}

static void vbat_lv_handle(void)
{
	int ret;
	unsigned int val_h = 0;
	unsigned int val_l = 0;

	if (g_buck->bat_sel == DUAL_BATTERY) {
		ret = regmap_read(g_buck->regmap, VBAT_LV_H_REG, &val_h);
		if (ret)
			scharger_inf("%s:regmap_raw_read fail ret=%d\n", __func__, ret);
		scharger_inf("%s:VBAT_LV_H_REG is 0x%x\n", __func__, val_h);
		(void)regmap_update_bits(g_buck->regmap, VBAT_LV_H_REG,
				VBAT_LV_H_MASK, VBAT_LV_SET1 << VBAT_LV_H_SHIFT);
	}

	ret = regmap_read(g_buck->regmap, VBAT_LV_L_REG, &val_l);
	if (ret)
		scharger_inf("%s:regmap_raw_read fail ret=%d\n", __func__, ret);
	scharger_inf("%s:VBAT_LV_L_REG is 0x%x\n", __func__, val_l);
	(void)regmap_update_bits(g_buck->regmap, VBAT_LV_L_REG,
				VBAT_LV_L_MASK, VBAT_LV_SET1 << VBAT_LV_L_SHIFT);
}

/* ibat resisitance val for ADC */
static int ibat_res_sel(int r_uohm)
{
	int ret;
	u8 val = 0;

	if (r_uohm == R_UOHM_1000)
		val = 0;
	else if (r_uohm == R_UOHM_2000)
		val = 1;
	else
		scharger_err("%s: %d uohm, not expected\n",
			     __func__, r_uohm);

	ret = regmap_update_bits(g_buck->regmap, IBAT_RES_SEL_REG,
				 IBAT_RES_SEL_MASK, val << IBAT_RES_SEL_SHIFT);

	return ret;
}

static int force_disbale_psw(void)
{
	int ret = 0;

	ret += scharger_psw_enable(0);

	/* disable psw plugin, set psw ovp 14v */
	ret += regmap_update_bits(g_buck->regmap, PSW_OVP_CFG_REG_2, PSW_PLUGIN_EN_MSK,
			 PSW_PLUGIN_DIS << PSW_PLUGIN_EN_SHIFT);
	ret += regmap_update_bits(g_buck->regmap, PSW_OVP_CFG_REG_2, PSW_OVP_SET_MSK,
			 PSW_OVP_14V << PSW_OVP_SET_SHIFT);

	return ret;
}

int config_opt_param(int vbus_vol)
{
	int ret;
	struct scharger_buck *buck = g_buck;

	if (buck == NULL) {
		scharger_err("%s scharger_buck is NULL!\n", __func__);
		return -ENOMEM;
	}

	buck_opt_param(vbus_vol, buck);
	ret = scharger_set_vbus_vset(vbus_vol);
	if (ret != 0)
		scharger_err("%s set_vbus_vset fail\n", __func__);
	return 0;
}

int dpm_init(void)
{
	int ret = 0;
	struct scharger_buck *buck = g_buck;

	if (buck == NULL) {
		scharger_err("%s scharger_buck is NULL!\n", __func__);
		return -ENOMEM;
	}
	/* set dpm mode auto */
	ret += regmap_update_bits(buck->regmap, CHG_DPM_MODE_REG,
				 CHG_DPM_MODE_MSK, CHG_DPM_MODE_AUTO << CHG_DPM_MODE_SHIFT);

	/* set dpm voltage sel 90% vbus */
	ret += regmap_update_bits(buck->regmap, CHG_DPM_SEL_REG,
				 CHG_DPM_SEL_MSK, CHG_DPM_SEL_DEFAULT << CHG_DPM_SEL_SHIFT);

	return !!ret;
}

static int dummy_fun_1(void)
{
	return 0;
}

static int dummy_fun_2(int val)
{
	return 0;
}

static int set_recharge_vol(u8 rechg)
{
	return regmap_update_bits(g_buck->regmap, CHG_RECHG_REG,
				 CHG_RECHG_MSK, rechg << CHG_RECHG_SHIFT);
}

int set_fast_safe_timer(u32 safe_timer)
{
	return regmap_update_bits(g_buck->regmap, CHG_FASTCHG_TIMER_REG,
				 CHG_FASTCHG_TIMER_MSK, (u8)safe_timer << CHG_FASTCHG_TIMER_SHIFT);
}

static int set_precharge_current(int precharge_current, int bat)
{
	u8 prechg_limit;

	if (precharge_current < CHG_PRG_ICHG_MIN)
		precharge_current = CHG_PRG_ICHG_MIN;
	else if (precharge_current > CHG_PRG_ICHG_MAX)
		precharge_current = CHG_PRG_ICHG_MAX;

	prechg_limit =
		(u8)((precharge_current - CHG_PRG_ICHG_MIN) / CHG_PRG_ICHG_STEP);

	if (bat == BATTERY_H) {
		return regmap_update_bits(g_buck->regmap, CHG_PRE_ICHG_REG,
				 CHG_PRE_ICHG_H_MSK, prechg_limit << CHG_PRE_ICHG_H_SHIFT);
	} else if (bat == BATTERY_L) {
		return regmap_update_bits(g_buck->regmap, CHG_PRE_ICHG_REG,
				 CHG_PRE_ICHG_L_MSK, prechg_limit << CHG_PRE_ICHG_L_SHIFT);
	} else {
		scharger_err("%s bat is invalid!\n", __func__);
		return -1;
	}
}

static int set_precharge_voltage(u32 pre_vchg)
{
	u8 vprechg;

	if (pre_vchg <= CHG_PRG_VCHG_2800)
		vprechg = 0; /* 0: set precharge vol <=2800mV */
	else if (pre_vchg > CHG_PRG_VCHG_2800 && pre_vchg <= CHG_PRG_VCHG_3000)
		vprechg = 1; /* 1: set precharge vol <=3000mV  and >2800mV */
	else if (pre_vchg > CHG_PRG_VCHG_3000 && pre_vchg <= CHG_PRG_VCHG_3100)
		vprechg = 2; /* 2: set precharge vol <=3100mV  and >3000mV */
	else if (pre_vchg > CHG_PRG_VCHG_3100 && pre_vchg <= CHG_PRG_VCHG_3200)
		vprechg = 3; /* 3: set precharge vol <=3200mV  and >3100mV */
	else
		vprechg = 0; /* default 2.8V */
	return regmap_update_bits(g_buck->regmap, CHG_PRE_ICHG_REG,
				 CHG_PRE_VCHG_MSK, vprechg << CHG_PRE_VCHG_SHIFT);
}

struct ir_comp_tbl bath_ir_com_tbl[] = {
	{0,   CHG_IR_COMP_REG, CHG_IR_COMP_H_MSK, CHG_IR_COMP_H_SHIFT, 0},
	{10,  CHG_IR_COMP_REG, CHG_IR_COMP_H_MSK, CHG_IR_COMP_H_SHIFT, 1},
	{20,  CHG_IR_COMP_REG, CHG_IR_COMP_H_MSK, CHG_IR_COMP_H_SHIFT, 2},
	{30,  CHG_IR_COMP_REG, CHG_IR_COMP_H_MSK, CHG_IR_COMP_H_SHIFT, 3},
	{40,  CHG_IR_COMP_REG, CHG_IR_COMP_H_MSK, CHG_IR_COMP_H_SHIFT, 4},
	{50,  CHG_BATH_IR_COMP_REG2, CHG_IR_COMP_MSK2, CHG_IR_COMP_SHIFT2, 2},
	{60,  CHG_IR_COMP_REG, CHG_IR_COMP_H_MSK, CHG_IR_COMP_H_SHIFT, 5},
	{70,  CHG_BATH_IR_COMP_REG2, CHG_IR_COMP_MSK2, CHG_IR_COMP_SHIFT2, 1},
	{80,  CHG_IR_COMP_REG, CHG_IR_COMP_H_MSK, CHG_IR_COMP_H_SHIFT, 6},
	{90,  CHG_BATH_IR_COMP_REG2, CHG_IR_COMP_MSK2, CHG_IR_COMP_SHIFT2, 0},
	{100, CHG_IR_COMP_REG, CHG_IR_COMP_H_MSK, CHG_IR_COMP_H_SHIFT, 7},
};

struct ir_comp_tbl batl_ir_com_tbl[] = {
	{0,   CHG_IR_COMP_REG, CHG_IR_COMP_L_MSK, CHG_IR_COMP_L_SHIFT, 0},
	{10,  CHG_IR_COMP_REG, CHG_IR_COMP_L_MSK, CHG_IR_COMP_L_SHIFT, 1},
	{20,  CHG_IR_COMP_REG, CHG_IR_COMP_L_MSK, CHG_IR_COMP_L_SHIFT, 2},
	{30,  CHG_IR_COMP_REG, CHG_IR_COMP_L_MSK, CHG_IR_COMP_L_SHIFT, 3},
	{40,  CHG_IR_COMP_REG, CHG_IR_COMP_L_MSK, CHG_IR_COMP_L_SHIFT, 4},
	{50,  CHG_BATL_IR_COMP_REG2, CHG_IR_COMP_MSK2, CHG_IR_COMP_SHIFT2, 1},
	{60,  CHG_IR_COMP_REG, CHG_IR_COMP_L_MSK, CHG_IR_COMP_L_SHIFT, 5},
	{70,  CHG_BATL_IR_COMP_REG2, CHG_IR_COMP_MSK2, CHG_IR_COMP_SHIFT2, 2},
	{80,  CHG_IR_COMP_REG, CHG_IR_COMP_L_MSK, CHG_IR_COMP_L_SHIFT, 6},
	{90,  CHG_BATL_IR_COMP_REG2, CHG_IR_COMP_MSK2, CHG_IR_COMP_SHIFT2, 3},
	{100, CHG_IR_COMP_REG, CHG_IR_COMP_L_MSK, CHG_IR_COMP_L_SHIFT, 7},
};

static int bat_ir_comp_index(int val)
{
	int i;

	if (val < 10)
		i = 0;
	else if (val >= 10 && val < 20)
		i = 1;
	else if (val >= 20 && val < 30)
		i = 2;
	else if (val >= 30 && val < 40)
		i = 3;
	else if (val >= 40 && val < 50)
		i = 4;
	else if (val >= 50 && val < 60)
		i = 5;
	else if (val >= 60 && val < 70)
		i = 6;
	else if (val >= 70 && val < 80)
		i = 7;
	else if (val >= 80 && val < 90)
		i = 8;
	else if (val >= 90 && val < 100)
		i = 9;
	else
		i = 10;

	return i;
}

static int set_bat_comp_v700(int value, int bat)
{
	u8 reg;

	scharger_inf("%s batt %d, value %d\n", __func__, bat, value);

	if (value < CHG_IR_COMP_10MOHM)
		reg = 0; /* 0: set bat comp 0mohm */
	else if (value >= CHG_IR_COMP_10MOHM && value < CHG_IR_COMP_20MOHM)
		reg = 1; /* 1: set bat comp 10mohm */
	else if (value >= CHG_IR_COMP_20MOHM && value < CHG_IR_COMP_30MOHM)
		reg = 2; /* 2: set bat comp 20mohm */
	else if (value >= CHG_IR_COMP_30MOHM && value < CHG_IR_COMP_40MOHM)
		reg = 3; /* 3: set bat comp 30mohm */
	else if (value >= CHG_IR_COMP_40MOHM && value < CHG_IR_COMP_60MOHM)
		reg = 4; /* 4: set bat comp 40mohm */
	else if (value >= CHG_IR_COMP_60MOHM && value < CHG_IR_COMP_80MOHM)
		reg = 5; /* 5: set bat comp 60mohm */
	else if (value >= CHG_IR_COMP_80MOHM && value < CHG_IR_COMP_100MOHM)
		reg = 6; /* 6: set bat comp 80mohm */
	else
		reg = 7; /* 7: set bat comp 100mohm */

	if (bat == BATTERY_H) {
		return regmap_update_bits(g_buck->regmap, CHG_IR_COMP_REG, CHG_IR_COMP_H_MSK, reg << CHG_IR_COMP_H_SHIFT);
	} else if (bat == BATTERY_L) {
		return regmap_update_bits(g_buck->regmap, CHG_IR_COMP_REG, CHG_IR_COMP_L_MSK, reg << CHG_IR_COMP_L_SHIFT);
	} else {
		scharger_err("%s bat is invalid!\n", __func__);
		return -1;
	}
}

static int _set_bat_comp(int value, int batt)
{
	int index;

	index = bat_ir_comp_index(value);
	scharger_err("%s batt %d, value %d, index %d\n", __func__, batt, value, index);

	if(batt == BATTERY_H) {
		/* clear */
		regmap_update_bits(g_buck->regmap, CHG_IR_COMP_REG, CHG_IR_COMP_H_MSK, 0 << CHG_IR_COMP_H_SHIFT);
		regmap_update_bits(g_buck->regmap, CHG_BATH_IR_COMP_REG2, CHG_IR_COMP_MSK2, 0x3 << CHG_IR_COMP_SHIFT2);

		regmap_update_bits(g_buck->regmap,
			bath_ir_com_tbl[index].reg, bath_ir_com_tbl[index].mask,
			bath_ir_com_tbl[index].reg_val <<  bath_ir_com_tbl[index].shift);
	} else { /* BATTERY_L */
		/* clear */
		regmap_update_bits(g_buck->regmap, CHG_IR_COMP_REG, CHG_IR_COMP_L_MSK, 0 << CHG_IR_COMP_L_SHIFT);
		regmap_update_bits(g_buck->regmap, CHG_BATL_IR_COMP_REG2, CHG_IR_COMP_MSK2, 0 << CHG_IR_COMP_SHIFT2);

		regmap_update_bits(g_buck->regmap,
			batl_ir_com_tbl[index].reg, batl_ir_com_tbl[index].mask,
			batl_ir_com_tbl[index].reg_val <<  batl_ir_com_tbl[index].shift);
	}

	return 0;
}

static int set_bat_comp(int value, int bat)
{
	struct scharger_buck *buck = g_buck;

	if (buck == NULL) {
		scharger_err("%s scharger_buck NULL!\n", __func__);
		return -ENOMEM;
	}

	if (buck->scharger_version == CHIP_ID_V700)
		set_bat_comp_v700(value, bat);
	else
		_set_bat_comp(value, bat);

	return 0;
}

/*
 * Parameters: value:vclamp mv, schargerv100 can't set vclamp due to lx bug
 * return value:  0-success or others-fail
 */
static int set_vclamp(int value, int bat)
{
	u8 reg;

	if (value < CHG_IR_VCLAMP_MIN)
		value = CHG_IR_VCLAMP_MIN;
	else if (value > CHG_IR_VCLAMP_MAX)
		value = CHG_IR_VCLAMP_MAX;

	reg = (u8)(value / CHG_IR_VCLAMP_STEP);

	if (bat == BATTERY_H) {
		return regmap_update_bits(g_buck->regmap, CHG_IR_VCLAMP_H_REG, CHG_IR_VCLAMP_H_MSK, reg << CHG_IR_VCLAMP_H_SHIFT);
	} else if (bat == BATTERY_L) {
		return regmap_update_bits(g_buck->regmap, CHG_IR_VCLAMP_L_REG, CHG_IR_VCLAMP_L_MSK, reg << CHG_IR_VCLAMP_L_SHIFT);
	} else {
		scharger_err("%s bat is invalid!\n", __func__);
		return -1;
	}
}

static int set_vdrop_ovp(int drop_vol)
{
	int ret = 0;
	unsigned int reg_val;

	if (drop_vol <= VUSB_DROP_OVP_200MV)
		reg_val = 0; /* set vdrop ovp 200mv */
	else if (drop_vol <= VUSB_DROP_OVP_300MV)
		reg_val = 1; /* set vdrop ovp 300mv */
	else
		reg_val = 0;

	scharger_inf("%s usb drop ovp reg_val:%u\n", __func__, reg_val);
	ret = regmap_update_bits(g_buck->regmap, PSW_OVP_CFG_REG_2, OVP2_DROPOVP_MODE_MSK,
		reg_val << OVP2_DROPOVP_MODE_SHIFT);
	if (ret)
		scharger_err("%s set vdrop ovp fail\n", __func__);

	return ret;
}

int check_ovp2_state(u8 *ovp2_state)
{
	unsigned int data = 0;
	int ret;
	struct scharger_buck *buck = g_buck;

	if (buck == NULL || ovp2_state == NULL) {
		scharger_err("%s scharger_buck or ovp2_state NULL!\n", __func__);
		return -1;
	}

	if (buck->scharger_version == CHIP_ID_V700) {
		ret = regmap_read(buck->regmap, PSW_EN_REG, &data);
		if (ret < 0) {
			scharger_err("%s err\n", __func__);
			return 0;
		}
		data &= PSW_EN_MSK;
		data >>= PSW_EN_SHIFT;
	} else {
		ret = regmap_read(buck->regmap, PSW_OVP_CFG_REG_1, &data);
		if (ret < 0) {
			scharger_err("%s err\n", __func__);
			return 0;
		}
		data &= OVP2_EN_MSK;
		data >>= OVP2_EN_SHIFT;
	}
	scharger_inf("%s ovp2_state:%u\n", __func__, data);
	*ovp2_state = data;

	return ret;
}

/* v700 does not support */
static int check_ovp3_state(u8 *ovp3_state)
{
	unsigned int data = 0;
	int ret;
	struct scharger_buck *buck = g_buck;

	if (buck == NULL || ovp3_state == NULL) {
		scharger_err("%s scharger_buck or ovp3_state NULL!\n", __func__);
		return -1;
	}

	ret = regmap_read(buck->regmap, PSW_OVP_CFG_REG_1, &data);
	if (ret < 0) {
		scharger_err("%s err\n", __func__);
		return 0;
	}
	data &= OVP3_EN_MSK;
	data >>= OVP3_EN_SHIFT;

	scharger_inf("%s ovp3_state:%u\n", __func__, data);
	*ovp3_state = data;

	return ret;
}

static int scharger_dsm_report_fault_info(const char *buff, int err_num)
{
#ifdef CONFIG_HUAWEI_DSM
	char dsm_buf[DSM_BUFF_SIZE_MAX] = {0};
	int ret = 0;

	if (buff == NULL)
		return -1;

	ret = sprintf_s(dsm_buf, DSM_BUFF_SIZE_MAX, "%s\n", buff);
	if (ret < 0)
		scharger_err("%s, sprintf_s error\n", __func__);

	return power_dsm_report_dmd(POWER_DSM_BATTERY, err_num, dsm_buf);
#else
	return 0;
#endif
}

#ifdef CONFIG_HUAWEI_DSM
static void ovp_dmd_delay_work(struct work_struct *work)
{
	char buff[DSM_BUFF_SIZE_MAX] = {0};
	int ret = 0;

	scharger_inf("%s ovp2 ovp3 state err\n", __func__);
	ret = sprintf_s(buff, (size_t)DSM_BUFF_SIZE_MAX,
			"[ovp2 ovp3 jump]ovp2 ovp3 state err");
	if (ret < 0)
		scharger_err("%s, sprintf_s error\n", __func__);

	scharger_dsm_report_fault_info(buff, POWER_DSM_CHARGE_SCHARGER_USB_PORT_VDROPMIN_WARNING);
}
#endif

#ifdef CONFIG_HUAWEI_DSM
static void ovp2_ovp3_jump_report(void)
{
	u8 ovp2_state, ovp3_state = 0;
	int ret = 0;
	struct scharger_buck *buck = g_buck;

	if (buck == NULL) {
		scharger_err("%s scharger_buck is NULL!\n", __func__);
		return;
	}

	ret = check_ovp2_state(&ovp2_state);
	ret += check_ovp3_state(&ovp3_state);
	if (ret) {
		scharger_err("%s: check ovp2 ovp3 state fail\n", __func__);
		return;
	}

	if (ovp2_ovp3_err_flag || ovp2_state || ovp3_state)
		(void)queue_delayed_work(system_power_efficient_wq, &buck->ovp_dmd_work,
			 msecs_to_jiffies(OVP_DMD_WORK_DELAY));
}
#endif

static int buck_init(struct charge_init_data *init_crit)
{
	int ret = 0;
	struct scharger_buck *buck = g_buck;

	scharger_inf("%s\n", __func__);

	if (buck == NULL || init_crit == NULL) {
		scharger_err("%s scharger_buck or charge_init_data is NULL!\n", __func__);
		return -ENOMEM;
	}

	switch (init_crit->vbus) {
	case ADAPTER_5V:
		ret += ibat_res_sel(buck->param_dts.r_coul_uohm);
		ret += config_opt_param(VBUS_VSET_5V);
		ret += dpm_init();
		ret += scharger_set_vbus_vset(VBUS_VSET_5V);
		break;
	case ADAPTER_9V:
		ret += config_opt_param(VBUS_VSET_5V);
		buck_opt_param(VBUS_VSET_9V, buck);
		ret += set_vbus_uvp_ovp(VBUS_VSET_9V);
		break;
	case ADAPTER_12V:
		ret += config_opt_param(VBUS_VSET_12V);
		ret += set_vbus_uvp_ovp(VBUS_VSET_12V);
		break;
	default:
		scharger_err("%s: init mode err\n", __func__);
		return -EINVAL;
	}
	if (buck->scharger_version == CHIP_ID_V700) {
		ret += force_disbale_psw();
	} else {
#ifdef CONFIG_HUAWEI_DSM
		ovp2_ovp3_jump_report();
#endif
		ret += scharger_psw_enable(0);
		ret += scharger_ovp3_enable(0);
	}
	ret += scharger_set_buck_mode_enable(CHG_ENABLE);
	ret += scharger_set_charge_enable(CHG_DISABLE);
	ret += set_recharge_vol(CHG_RECHG_150);
	ret += set_fast_safe_timer(CHG_FASTCHG_TIMER_20H);
	ret += scharger_set_term_enable(CHG_TERM_DIS);
	ret += scharger_set_input_current(CHG_ILIMIT_475);
	ret += scharger_set_charge_current(CHG_FAST_ICHG_500MA);
	ret += scharger_set_terminal_voltage(CHG_FAST_VCHG_4430);
	ret += scharger_set_terminal_current(CHG_TERM_ICHG_150MA);
	ret += scharger_set_watchdog_timer(WATCHDOG_TIMER_40_S);
	ret += set_precharge_current(CHG_PRG_ICHG_400MA, BATTERY_H);
	ret += set_precharge_current(CHG_PRG_ICHG_400MA, BATTERY_L);
	ret += set_precharge_voltage(CHG_PRG_VCHG_2800);
	ret += set_batfet_ctrl(CHG_BATFET_EN);
	/* IR compensation voatge clamp,IR compensation resistor setting */
	ret += set_bat_comp(buck->param_dts.bat_comp_h, BATTERY_H);
	ret += set_bat_comp(buck->param_dts.bat_comp_l, BATTERY_L);
	ret += set_vclamp(buck->param_dts.vclamp_h, BATTERY_H);
	ret += set_vclamp(buck->param_dts.vclamp_l, BATTERY_L);
	ret += scharger_set_otg_current(BOOST_LIM_1000);
	ret += scharger_set_otg_enable(CHARGE_OTG_DISABLE);
	ret += set_vdrop_ovp(buck->param_dts.vdrop_ovp);
	if (ret)
		return -1;

	return 0;
}

/* 0-success or others-fail */
static int scharger_device_check(void)
{
	int ret;
	u32 chip_id = 0;
	struct scharger_buck *buck = g_buck;

	scharger_dbg("%s\n", __func__);

	if (buck == NULL) {
		scharger_err("%s scharger_buck is NULL!\n", __func__);
		return -ENOMEM;
	}

	/* byte length of version 0 chip id is 4 */
	ret = regmap_bulk_read(buck->regmap, CHIP_VERSION_0, &chip_id, 4);
	if (ret) {
		scharger_err("[%s]:read chip_id fail\n", __func__);
		return CHARGE_IC_BAD;
	}

	scharger_dbg("%s, chip id is 0x%x\n", __func__, chip_id);
	return CHARGE_IC_GOOD;
}

unsigned int scharger_get_device_version(void)
{
	unsigned int scharger_version = 0;

	struct scharger_buck *buck = g_buck;

	if (buck == NULL) {
		scharger_err("%s scharger_buck is NULL!\n", __func__);
		return -ENOMEM;
	}

	/* byte length of version 0 is 2 */
	(void)regmap_bulk_read(buck->regmap, CHIP_VERSION_4,
				&scharger_version, 2);

	scharger_inf("%s, chip version is 0x%x\n", __func__, scharger_version);
	return scharger_version;
}

static int scharger_efuse_read_enable(u32 enable)
{
	int ret = 0;
	u32 en;

	en = !!enable;
	ret = regmap_update_bits(g_buck->regmap, EFUSE_TESTBUS_CFG_REG, EFUSE_TESTBUS_EN_MSK,
					en << EFUSE_TESTBUS_EN_SHIFT);
	if (ret) {
		scharger_err("%s enable efuse testbus fail\n", __func__);
		return -1;
	}

	return ret;
}

static int scharger_efuse_read(u8 addr, u8 *value)
{
	int ret = 0;
	unsigned int val = 0;

	scharger_efuse_read_enable(EFUSE_RD_ENABLE);
	ret = regmap_update_bits(g_buck->regmap, EFUSE_TESTBUS_CFG_REG, EFUSE_TESTBUS_SEL_MSK,
				addr << EFUSE_TESTBUS_SEL_SHIFT);
	ret += regmap_read(g_buck->regmap, EFUSE_TESTBUS_RDATA_REG, &val);
	if (ret) {
		scharger_err("%s read efuse testbus data fail\n", __func__);
		return -1;
	}
	*value = (u8)val;
	scharger_efuse_read_enable(EFUSE_RD_DISABLE);

	return ret;
}

static int scharger_efuse_write_enable(u32 enable)
{
	int ret = 0;
	u32 en;

	en = !!enable;
	ret = regmap_update_bits(g_buck->regmap, EFUSE_PDOB_SEL_REG, EFUSE_PDOB_SEL_MSK,
					en << EFUSE_PDOB_SEL_SHIFT);
	if (ret) {
		scharger_err("%s enable efuse pre mode fail\n", __func__);
		return -1;
	}

	return ret;
}

static int scharger_efuse_write(u8 addr, u8 value, u8 mask)
{
	int ret = 0;
	u8 val;

	ret = scharger_efuse_read(addr, &val);
	if (ret) {
		scharger_err("%s read efuse fail\n", __func__);
		return -1;
	}
	val &= ~mask;
	val |= value & mask;

	ret = scharger_efuse_write_enable(EFUSE_WR_ENABLE);
	ret += regmap_update_bits(g_buck->regmap, EFUSE_PDOB_PRE_ADDR_WE_REG, EFUSE_PDOB_PRE_ADDR_MSK,
					addr << EFUSE_PDOB_PRE_ADDR_SHIFT);
	ret += regmap_update_bits(g_buck->regmap, EFUSE_PDOB_PRE_WDATA_REG, EFUSE_PDOB_PRE_WDATA_MSK,
					val << EFUSE_PDOB_PRE_WDATA_SHIFT);
	ret += regmap_update_bits(g_buck->regmap, EFUSE_PDOB_PRE_ADDR_WE_REG, EFUSE_PDOB_PRE_WR_EN_MSK,
					EFUSE_WR_START << EFUSE_PDOB_PRE_WR_EN_SHIFT);
	ret += regmap_update_bits(g_buck->regmap, EFUSE_PDOB_PRE_ADDR_WE_REG, EFUSE_PDOB_PRE_WR_EN_MSK,
					EFUSE_WR_END << EFUSE_PDOB_PRE_WR_EN_SHIFT);
	ret += scharger_efuse_write_enable(EFUSE_WR_DISABLE);

	return ret;
}

static void scharger_mask_reset_n_disable(struct scharger_buck *buck)
{
	int ret = 0;

	ret = scharger_efuse_write(MASK_RESET_N_EFUSE_ADDR, DISABLE_MASK_RESET_N, DISABLE_MASK_RESET_N_MSK);
	if (ret)
		scharger_err("%s write efuse fail\n", __func__);
}

static int scharger_get_dieid(char *dieid, unsigned int len)
{
	int ret = 0;
	int i = 0;
	errno_t err;
	u8 val[SCHARGER_DIEID_LENGTH];
	u8 dieid_addr[SCHARGER_DIEID_LENGTH] = {
				SCHARGER_DIEID_BYTE0,
				SCHARGER_DIEID_BYTE1,
				SCHARGER_DIEID_BYTE2,
				SCHARGER_DIEID_BYTE3,
				SCHARGER_DIEID_BYTE4,
				SCHARGER_DIEID_BYTE5,
				SCHARGER_DIEID_BYTE6,
				SCHARGER_DIEID_BYTE7
	};
	if (dieid == NULL) {
		scharger_err("%s: dieid is null\n", __func__);
		return -1;
	}
	if (len < SCHARGER_DIEID_LENGTH) {
		scharger_err("%s: mem length is not enough!\n", __func__);
		return -1;
	}

	err = memset_s(dieid, len, 0, len);
	if (err != EOK)
		scharger_err("[%s]memset_s fail, err=%d\n", __func__, err);

	for (i = 0 ; i < SCHARGER_DIEID_LENGTH; i++)
		scharger_efuse_read(dieid_addr[i], &val[i]);

	ret = snprintf_s(dieid, len, len - 1,
				 "\r\nHi6526v710:0x%02x%02x%02x%02x%02x%02x%02x%02x\r\n",
				 val[7], val[6], val[5], val[4], val[3],
				 val[2], val[1], val[0]);
	if (ret < 0)
		scharger_err("%s, offset %d, snprintf_s error\n",
			     __func__, ret);

	return 0;
}

static int scharger_set_ship_mode_enable(bool enable)
{
	int ret;
	struct scharger_buck *buck = g_buck;

	if (buck == NULL) {
		scharger_err("%s scharger_buck is NULL!\n", __func__);
		return -EINVAL;
	}

	ret = regmap_update_bits(buck->regmap, REG_SHIP_MODE_ENB,
		 SHIP_MODE_ENB_MSK, !enable << SHIP_MODE_ENB_SHIFT);
	if (ret) {
		scharger_err("[%s]:set ship mode %d fail\n", __func__, enable);
		return -1;
	}

	return ret;
}

/* The chip will automatically exit the forced conduction mode when the USB is inserted  */
static void dual_batfet_forced_conduction(struct scharger_buck *buck, int enable)
{
	int ret = 0;
	int en;

	if (buck->scharger_version == CHIP_ID_V700)
		return;
	if (buck->bat_sel == SINGLE_BATTERY)
		return;

	en = !!enable;
	scharger_inf("%s enalbe:%d\n", __func__, en);
	if (en) {
		ret = regmap_update_bits(buck->regmap, SCHG_LOGIC_CFG_REG_1, BATFET_FORCE_CONDUCTION_MSK,
				BATFET_CONDUCTION_DISABLE << BATFET_FORCE_CONDUCTION_SHIFT);
		ret += regmap_update_bits(buck->regmap, SCHG_LOGIC_CFG_REG_1, BATFET_FORCE_CONDUCTION_MSK,
				BATFET_CONDUCTION_ENABLE << BATFET_FORCE_CONDUCTION_SHIFT);
	} else {
		ret = regmap_update_bits(buck->regmap, SCHG_LOGIC_CFG_REG_1, BATFET_FORCE_CONDUCTION_MSK,
				BATFET_CONDUCTION_DISABLE << BATFET_FORCE_CONDUCTION_SHIFT);
	}
	if (ret)
		scharger_err("%s set batfet fail \n", __func__);
}

static int scharger_boost_enable(bool enable)
{
	struct scharger_buck *buck = g_buck;

	if (buck == NULL) {
		scharger_err("%s scharger_buck is NULL!\n", __func__);
		return -ENOMEM;
	}

	scharger_inf("%s enable:%d\n", __func__, enable);
	return regmap_update_bits(buck->regmap, REG_LOWBAT_EN, LOWBAT_EN_MSK,
			 enable << LOWBAT_EN_SHIFT);
}

static int scharger_set_low_power_mode_enable(bool enable)
{
	int ret;
	struct scharger_buck *buck = g_buck;

	if (buck == NULL) {
		scharger_err("%s scharger_buck is NULL!\n", __func__);
		return -ENOMEM;
	}

	buck->boost_en_flag = enable;
	if (enable) {
		dual_batfet_forced_conduction(buck, BATFET_CONDUCTION_DISABLE);
		ret = scharger_boost_enable(BOOST_ENABLE);
	} else {
		ret = scharger_boost_enable(BOOST_DISABLE);
		mdelay(50);
		dual_batfet_forced_conduction(buck, BATFET_CONDUCTION_ENABLE);
	}

	if (ret) {
		scharger_err("[%s]:set emergency mode %d fail\n", __func__, enable);
		return -1;
	}

	return ret;
}

static int scharger_set_dpm_enable(bool enable)
{
	struct scharger_buck *buck = g_buck;
	unsigned int val = (unsigned int)enable;

	if (!buck) {
		scharger_err("%s scharger_buck is NULL!\n", __func__);
		return -ENOMEM;
	}
	return regmap_update_bits(buck->regmap, REG_DPM_ENB, DPM_EN_MASK,
		val << DPM_EN_SHIFT);
}

int scharger_set_term_enable(int enable)
{
	int vbatt_mv_l, vbatt_mv_h, term_mv, chg_state1, chg_state2, dpm, acl, therm;
	struct scharger_buck *buck = g_buck;

	scharger_inf("%s enable %d\n", __func__, enable);

	if (buck == NULL) {
		scharger_err("%s scharger_buck is NULL!\n", __func__);
		return -ENOMEM;
	}

	if (force_set_term_flag == CHG_STAT_ENABLE) {
		scharger_inf("Charger is in the production line test phase!\n");
		return 0;
	}

	if (enable) {
		dpm = scharger_check_input_dpm_state();
		acl = check_input_acl_state();
		therm = check_therm_state();
		term_mv = scharger_get_terminal_voltage();
		vbatt_mv_l = coul_merge_drv_battery_voltage_mv(BATT_L);
		scharger_dbg("%s vbatt_mv_l:%d\n", __func__, vbatt_mv_l);
		chg_state1 = (abs(term_mv - vbatt_mv_l) > 100) ? 1 : 0;

		if (buck->bat_sel == DUAL_BATTERY) {
			vbatt_mv_h = coul_merge_drv_battery_voltage_mv(BATT_H);
			chg_state1 = (chg_state1 || ((abs(term_mv - vbatt_mv_h) > 100) ? 1 : 0));
			scharger_dbg("%s vbatt_mv_h:%d\n", __func__, vbatt_mv_h);
		}
		chg_state2 = (dpm || acl || therm);
		scharger_dbg("%s enable:%d, dpm:%d,acl:%d,therm:%d,term_mv:%d,chg_state1:%d, chg_state2:%d\n",
						 __func__, enable, dpm, acl, therm, term_mv, chg_state1, chg_state2);
		/* ref vol 100 */
		if (chg_state1 || chg_state2) {
			scharger_inf("%s enable:%d, %d, but in dpm or acl or thermal state\n",
				     __func__, enable, chg_state2);
			enable = 0;
		} else {
			scharger_inf("%s enable:%d\n", __func__, enable);
		}
	}

	/* For charger ic bug, set bat IR to 0mohm when EOC. */
	if (enable) {
		scharger_inf("%s set IR 0mohm\n", __func__);
		set_bat_comp(CHG_IR_COMP_0MOHM, BATTERY_L);
		set_bat_comp(CHG_IR_COMP_0MOHM, BATTERY_H);
	}

	return regmap_update_bits(buck->regmap, CHG_EN_TERM_REG,
				 CHG_EN_TERM_MSK, (!!enable) << CHG_EN_TERM_SHIFT);
}

static int scharger_get_vbat(void)
{
	int ret;
	if (g_buck->bat_sel == DUAL_BATTERY)
		ret = scharger_get_vbat_h();
	else
		ret = scharger_get_vbat_l();

	return ret;
}

static struct charge_device_ops scharger_ops = {
	.chip_init = buck_init,
	.get_dieid = scharger_get_dieid,
	.dev_check = scharger_device_check,
	.set_input_current = scharger_set_input_current,
	.set_charge_current = scharger_set_charge_current,
	.get_charge_current = scharger_get_charge_current,
	.set_terminal_voltage = scharger_set_terminal_voltage,
	.get_terminal_voltage = scharger_get_terminal_voltage,
	.set_terminal_current = scharger_set_terminal_current,
	.set_term_enable = scharger_set_term_enable,
	.set_force_term_enable = force_set_term_enable,
	.set_charge_enable = scharger_set_charge_enable,
	.get_charge_state = scharger_get_charge_state,
	.get_charger_state = scharger_get_charger_state,
	.set_watchdog_timer = scharger_set_watchdog_timer,
	.reset_watchdog_timer = scharger_reset_watchdog_timer,
	.set_batfet_disable = scharger_set_batfet_disable,
	.set_charger_hiz = scharger_set_charger_hiz,
	.get_ibus = scharger_get_ibus,
	.get_vbus = scharger_adc_get_vbus,
	.get_vbat_sys = dummy_fun_1,
	.check_input_dpm_state = scharger_check_input_dpm_state,
	.check_input_vdpm_state = scharger_check_input_dpm_state,
	.check_input_idpm_state = check_input_acl_state,
	.set_dpm_voltage = dummy_fun_2,
	.set_vbus_vset = scharger_set_vbus_vset,
	.set_uvp_ovp = dummy_fun_1,
	.soft_vbatt_ovp_protect = scharger_soft_vbatt_ovp_protect,
	.rboost_buck_limit = scharger_rboost_buck_limit,
	.stop_charge_config = scharger_stop_charge_config,
	.set_otg_enable = scharger_set_otg_enable,
	.set_otg_current = scharger_set_otg_current,
	.set_otg_switch_mode_enable = scharger_otg_switch_mode,
	.get_iin_set = get_input_current_set,
	.get_vbat = scharger_get_vbat,
	.get_vusb = scharger_adc_get_vusb,
	.set_ship_mode_enable = scharger_set_ship_mode_enable,
	.set_low_power_mode_enable = scharger_set_low_power_mode_enable,
	.set_dpm_enable = scharger_set_dpm_enable,
};

struct charger_otg_device_ops scharger_otg_ops = {
	.dev_name = "schargerv700",
	.otg_set_charger_enable = scharger_set_charge_enable,
	.otg_set_enable = scharger_set_otg_enable,
	.otg_set_current = scharger_set_otg_current,
	.otg_set_watchdog_timer = scharger_set_watchdog_timer,
	.otg_reset_watchdog_timer = scharger_reset_watchdog_timer,
	.otg_set_switch_mode = scharger_otg_switch_mode,
};

static void buck_parse_ovpsw_dts(struct device_node *np, struct power_sw_attr *sw_attr)
{
	int i, len;
	const char *prop = "ovpgate_sw";
	const char *str = NULL;

	len = of_property_count_strings(np, prop);
	if (len <= 0) {
		sw_attr->num = -1;
		return;
	}

	for (i = 0; i < len; i++) {
		if (of_property_read_string_index(np, prop, i, &str))
			goto err;
		switch (i % POWER_SW_ATTR_TOTAL) {
		case POWER_SW_ATTR_LABEL:
			sw_attr->label = str;
			break;
		case POWER_SW_ATTR_INDEX:
			if (kstrtoint(str, 0, &sw_attr->num))
				goto err;
			break;
		case POWER_SW_ATTR_DFLT:
			if (kstrtoint(str, 0, &sw_attr->dflt))
				goto err;
			break;
		case POWER_SW_ATTR_EN:
			if (kstrtoint(str, 0, &sw_attr->en))
				goto err;
			break;
		default:
			goto err;
		}
	}
	return;

err:
	scharger_err("parse_ovpsw_dts: synx err\n");
	sw_attr->num = -1;
}

static void buck_parse_dts(struct device_node *np, struct scharger_buck *buck)
{
	struct device_node *coul_np = NULL;
	struct device_node *batt_node = NULL;
	int ret;

	if (np == NULL || buck == NULL) {
		scharger_err("%s input is NULL!\n", __func__);
		return;
	}

	buck_parse_ovpsw_dts(np, &buck->ovpsw_attr);
	ret = of_property_read_u32(np, "bat_comp_h", (u32 *)&(buck->param_dts.bat_comp_h));
	if (ret) {
		scharger_err("get bat_comp_h failed\n");
		return;
	}
	ret = of_property_read_u32(np, "bat_comp_l", (u32 *)&(buck->param_dts.bat_comp_l));
	if (ret) {
		scharger_err( "get bat_comp_l failed\n");
		return;
	}
	ret = of_property_read_u32(np, "vclamp_h", (u32 *)&(buck->param_dts.vclamp_h));
	if (ret) {
		scharger_err("get vclamp_h failed\n");
		return;
	}
	scharger_inf("prase_dts vclamp_h=%d\n", buck->param_dts.vclamp_h);
	ret = of_property_read_u32(np, "vclamp_l", (u32 *)&(buck->param_dts.vclamp_l));
	if (ret) {
		scharger_err("get vclamp_l failed\n");
		return;
	}
	scharger_inf("prase_dts vclamp_l=%d\n", buck->param_dts.vclamp_l);
	ret = of_property_read_u32(np, "vusb_ovp", (u32 *)&(buck->param_dts.vusb_ovp));
	if (ret) {
		scharger_err("get vusb_ovp failed\n");
		return;
	}
	scharger_inf("prase_dts vusb_ovp=%d\n", buck->param_dts.vusb_ovp);

	ret = of_property_read_u32(np, "vdrop_ovp", (u32 *)&(buck->param_dts.vdrop_ovp));
	if (ret) {
		scharger_err("get vusb_ovp failed\n");
		buck->param_dts.vdrop_ovp = DBFET_VUSB_DROP_OVP;
	}
	scharger_inf("prase_dts vdrop_ovp=%d\n", buck->param_dts.vdrop_ovp);

	batt_node = of_find_compatible_node(NULL, NULL, "hisilicon,bci_battery");
	if (batt_node != NULL) {
		if (of_property_read_u32(batt_node, "battery_board_type",
					 &buck->is_board_type)) {
			scharger_err("get battery_board_type fail!\n");
			buck->is_board_type = BAT_BOARD_ASIC;
		}
	} else {
		scharger_err("get hisi_bci_battery fail!\n");
		buck->is_board_type = BAT_BOARD_ASIC;
	}
	scharger_inf("%s is_board_type %d!\n", __func__, buck->is_board_type);

	coul_np = of_find_compatible_node(NULL, NULL, "hisilicon,coul_algo_core");
	if (!coul_np) {
		scharger_err("%s coul_core unfind\n", __func__);
		return;
	}
	ret = of_property_read_u32(coul_np, "r_coul_uohm",
				(u32 *)&(buck->param_dts.r_coul_uohm));
	if (ret) {
		buck->param_dts.r_coul_uohm = R_UOHM_DEFAULT;
		pr_info("get r_coul_uohm err,use default value 2000 uohm!\n");
	}
	pr_info("prase_dts r_coul_uohm=%d\n", buck->param_dts.r_coul_uohm);
}

static void buck_vbat_ovp_handle(void)
{
	int i, ret;
	u8 vbat_ovp_cnt = 0;
	unsigned int irq_st0 = 0;

	scharger_dbg("%s\n", __func__);
	for (i = 0; i < 5; i++) { /* status number is 5 */
		ret = regmap_read(g_buck->regmap, CHG_IRQ_STATUS_8, &irq_st0);
		if (ret || ((irq_st0 & CHG_VBATH_OVP) == CHG_VBATH_OVP) || ((irq_st0 & CHG_VBATL_OVP) == CHG_VBATL_OVP)) {
			vbat_ovp_cnt++;
			mdelay(2); /* 2: need 2mS for next read */
		} else {
			vbat_ovp_cnt = 0;
			break;
		}
	}
	if (vbat_ovp_cnt >= 5) { /* 5: total cnt of ovp */
		scharger_err("%s:CHARGE_FAULT_VBAT_OVP\n", __func__);
#ifdef CONFIG_HUAWEI_CHARGER_AP
		power_event_anc_notify(POWER_ANT_CHARGE_FAULT,
			POWER_NE_CHG_FAULT_VBAT_OVP, NULL);
#endif
	}
}

static int buck_get_irq_byname(struct device *dev, const char *name)
{
	if (!dev || !name)
		return -ENOMEM;
	if (dev->of_node->name != NULL)
		scharger_err("%s: name:%s\n", __func__, dev->of_node->name);
	else
		scharger_err("%s: of_node->name is NULL!\n", __func__);
	return of_irq_get_byname(dev->of_node, name);
}

static void scharger_direct_fault_report(u32 fault_type, struct nty_data *data)
{
	unsigned int notifier_type;
	enum chg_mode_state chg_mode = NONE;

	chg_mode = dc_get_direct_charge_mode();
	scharger_inf("%s chg_mode:%d\n", __func__, chg_mode);
	if (!(chg_mode == SC || chg_mode == LVC))
		return;

	if (chg_mode == SC)
		notifier_type = POWER_ANT_SC_FAULT;
	else
		notifier_type = POWER_ANT_LVC_FAULT;

	power_event_anc_notify(notifier_type, fault_type, data);
}

u8 scharger_get_charge_err_flag(void)
{
	u8 flag = 0;

	/* bit0:charge_err_flag, bit1:sc_chg_err_flag, bit2:lvc_chg_err_flag */
	flag |= charge_err_flag;
	flag |= sc_chg_err_flag << 1;
	flag |= lvc_chg_err_flag << 2;

	return flag;
}

static void direct_charge_fault_handle(u32 buck_irq_state)
{
	u32 fault_type = POWER_NE_DC_FAULT_NON;
	struct nty_data data;
	char buf[DSM_BUFF_SIZE_MAX];
	int ret = 0;
	int factory_mode = 0;

	if (!(buck_irq_state & (FAULT_BATSW1_OPEN | FAULT_BATSW2_OPEN |
					 FAULT_BATSW1_SCP | FAULT_BATSW2_SCP)))
		return;

	data.addr = 0x6c; /* i2c client addr 0x6c */
	data.event2 = !!(buck_irq_state & (FAULT_BATSW1_OPEN | FAULT_BATSW2_OPEN |
					 FAULT_BATSW1_SCP | FAULT_BATSW2_SCP));

	if(strstr(saved_command_line, "androidboot.swtype=factory")) {
		scharger_inf("%s factory mode\n", __func__);
		factory_mode = 1;
	}

	if (buck_irq_state & FAULT_BATSW1_SCP) {
		scharger_err("charge_fault:QSW1 short circuit happened!\n");
		if (factory_mode == 0) {
			fault_type = POWER_NE_DC_FAULT_IC;
			sc_chg_err_flag = 1;
		}
#ifdef CONFIG_HUAWEI_DSM
		ret = snprintf_s(buf, sizeof(buf), sizeof(buf) - 1, "[FAULT_BATSW1_SCP]QSW1 short circuit\n");
		if (ret < 0)
			scharger_err("%s, offset %d, snprintf_s error\n",
				     __func__, ret);
		scharger_dsm_report_fault_info(buf, POWER_DSM_CHARGE_SCHARGER_SERIAL_PARALLEL_SWITCH_QSW1_ERROR);
#endif
	}
	if (buck_irq_state & FAULT_BATSW2_SCP) {
		scharger_err("charge_fault:QSW2 short circuit happened!\n");
		if (factory_mode == 0) {
			fault_type = POWER_NE_DC_FAULT_IC;
			scharger_set_charge_enable(CHG_DISABLE);
#ifdef CONFIG_HUAWEI_DSM
			ret = snprintf_s(buf, sizeof(buf), sizeof(buf) - 1, "[FAULT_BATSW2_SCP]QSW2 short circuit\n");
			if (ret < 0)
				scharger_err("%s, offset %d, snprintf_s error\n",
					     __func__, ret);
			scharger_dsm_report_fault_info(buf, POWER_DSM_CHARGE_SCHARGER_SERIAL_PARALLEL_SWITCH_QSW2_ERROR);
#endif
		}
	}
	if (buck_irq_state & FAULT_BATSW1_OPEN) {
		scharger_err("charge_fault:QSW1 circuit breaking happened!\n");
		fault_type = POWER_NE_DC_FAULT_IC;
		set_batfet_h_enable(0);
		batfet_h_chg_err_flag = 1;
		sc_chg_err_flag = 1;
		lvc_chg_err_flag = 1;
#ifdef CONFIG_HUAWEI_DSM
		ret = snprintf_s(buf, sizeof(buf), sizeof(buf) - 1, "[FAULT_BATSW1_OPEN]QSW1 circuit breaking\n");
		if (ret < 0)
			scharger_err("%s, offset %d, snprintf_s error\n",
				     __func__, ret);
		scharger_dsm_report_fault_info(buf, POWER_DSM_CHARGE_SCHARGER_SERIAL_PARALLEL_SWITCH_QSW1_ERROR);
#endif
	}
	if (buck_irq_state & FAULT_BATSW2_OPEN) {
		scharger_err("charge_fault:QSW2 circuit breaking happened!\n");
		fault_type = POWER_NE_DC_FAULT_IC;
		sc_chg_err_flag = 1;
#ifdef CONFIG_HUAWEI_DSM
		ret = snprintf_s(buf, sizeof(buf), sizeof(buf) - 1, "[FAULT_BATSW2_OPEN]QSW2 circuit breaking\n");
		if (ret < 0)
			scharger_err("%s, offset %d, snprintf_s error\n",
				     __func__, ret);
		scharger_dsm_report_fault_info(buf, POWER_DSM_CHARGE_SCHARGER_SERIAL_PARALLEL_SWITCH_QSW2_ERROR);
#endif
	}

	if (factory_mode == 1 && (buck_irq_state & (FAULT_BATSW1_SCP | FAULT_BATSW2_SCP)))
		return;

	scharger_direct_fault_report(fault_type, &data);
	scharger_sc_enable(DC_DISABLE);
	scharger_lvc_enable(DC_DISABLE);
}

static void buck_fault_handle(u32 buck_irq_state)
{
	struct scharger_buck *buck = g_buck;

	if (buck == NULL) {
		scharger_err("%s scharger_buck is NULL!\n", __func__);
		return;
	}

	if (buck_irq_state & FAULT_OTG_SCP) {
		scharger_err("%s:charge_fault:CHARGE_FAULT_BOOST_SCP\n", __func__);
		power_event_anc_notify(POWER_ANT_CHARGE_FAULT,
			POWER_NE_CHG_FAULT_BOOST_OCP, NULL);
	}

	if (buck_irq_state & FAULT_REVERSBST) {
		scharger_err("%s:charge_fault:irq_reversbst, cnt:%d\n",
			     __func__, buck->reverbst_cnt);
		buck->reverbst_cnt++;
		if (buck->reverbst_cnt < REVERBST_RETRY) {
			set_anti_reverbst_reset();
		} else {
			set_boot_weaksource_flag();
			scharger_err("%s:charge_fault:CHARGE_FAULT_WEAKSOURCE\n", __func__);
			power_event_anc_notify(POWER_ANT_CHARGE_FAULT,
				POWER_NE_CHG_FAULT_WEAKSOURCE, NULL);
		}
	}
	if (buck_irq_state & FAULT_CHG_DONE) {
#ifdef CONFIG_SCHARGER_DYNAMIC_CV
		if (scharger_cv_is_force_trim() == 1) {
			scharger_set_term_enable(CHG_TERM_DIS);
			scharger_set_charge_enable(CHG_DISABLE);
			set_bat_comp(buck->param_dts.bat_comp_h, BATTERY_H);
			set_bat_comp(buck->param_dts.bat_comp_l, BATTERY_L);
			mdelay(5);
			scharger_set_charge_enable(CHG_ENABLE);
			scharger_inf("[%s]force trim CV\n", __func__);
		} else {
			power_event_anc_notify(POWER_ANT_CHARGE_FAULT,
				POWER_NE_CHG_FAULT_CHARGE_DONE, NULL);
			scharger_err("%s : CHARGE_FAULT_CHARGE_DONE\n", __func__);
		}
#else
		power_event_anc_notify(POWER_ANT_CHARGE_FAULT,
			POWER_NE_CHG_FAULT_CHARGE_DONE, NULL);
		scharger_err("%s : CHARGE_FAULT_CHARGE_DONE\n", __func__);
#endif
	}

	direct_charge_fault_handle(buck_irq_state);
}

static void qsw2_rcp_handle(u32 others_irq_state)
{
	static unsigned long rcrf_time = 0;
	unsigned long vbath_lv_time = 0;
	struct nty_data data;
	struct scharger_buck *buck = g_buck;
	char buf[DSM_BUFF_SIZE_MAX];
	int ret = 0;

	if (buck == NULL) {
		scharger_err("%s scharger_buck is NULL!\n", __func__);
		return;
	}

	data.addr = 0x6c; /* i2c client addr 0x6c */
	data.event1 = !!(others_irq_state & (OTHERS_RCPF_DET_IRQ | OTHERS_VBATH_LV_IRQ));

	if (others_irq_state & OTHERS_RCPF_DET_IRQ) {
		rcrf_time = jiffies;
		scharger_err("charge_fault:USB or VBATH short circuit happened!\n");
		set_qsw2_rcp_protect(QSW2_RCP_PROTECT_DISABLE);
		scharger_direct_fault_report(POWER_NE_DC_FAULT_IC, &data);
		scharger_sc_enable(DC_DISABLE);
	}

	if (others_irq_state & OTHERS_VBATH_LV_IRQ) {
		vbath_lv_time = jiffies;
		if (rcrf_time != 0 && time_before(vbath_lv_time, rcrf_time + msecs_to_jiffies(VBATH_LV_TIMEOUT))) {
			scharger_err("charge_fault:VBATH short circuit happened!\n");
			rcrf_time = 0;
			scharger_sc_enable(DC_DISABLE);
			scharger_lvc_enable(DC_DISABLE);
			scharger_ovp_enable(buck, 0);
			scharger_set_charge_enable(CHG_DISABLE);
			scharger_direct_fault_report(POWER_NE_DC_FAULT_IC, &data);
			charge_err_flag = 1;
#ifdef CONFIG_HUAWEI_DSM
			ret = snprintf_s(buf, sizeof(buf), sizeof(buf) - 1, "[OTHERS_VBATH_LV_IRQ]VBATH short circuit\n");
			if (ret < 0)
				scharger_err("%s, offset %d, snprintf_s error\n",
					    __func__, ret);
			scharger_dsm_report_fault_info(buf, POWER_DSM_CHARGE_SCHARGER_SERIAL_CHARGE_QSW2_RCP_ERROR);
#endif
		}
	}
}

static void vdropmin_check_delay_work(struct work_struct *work)
{
	int vusb = 0;
	int ret = 0;
	char buf[DSM_BUFF_SIZE_MAX] = {0};

	vusb = scharger_get_vusb();
	if (vusb > VUSB_6000MV || (vusb > VUSB_3000MV && vusb < VUSB_4500MV)) {
		scharger_err("charge_fault:vdropmin when vusb on\n");
#ifdef CONFIG_HUAWEI_DSM
		ret = snprintf_s(buf, sizeof(buf), sizeof(buf) - 1,
				 "[OTHERS_OVP2_DROPMIN_IRQ]vdropmin when vusb on, vusb:%d\n", vusb);
		if (ret < 0)
			scharger_err("%s, offset %d, snprintf_s error\n",
				     __func__, ret);
		scharger_dsm_report_fault_info(buf, POWER_DSM_CHARGE_SCHARGER_USB_PORT_VDROPMIN_WARNING);
#endif
	}
}

static void others_fault_handle(u32 others_irq_state)
{
	struct nty_data data;
	struct scharger_buck *buck = g_buck;

	data.addr = 0x6c; /* i2c client addr 0x6c */

	if ((others_irq_state & OTHERS_VBATL_OVP_MASK) || (others_irq_state & OTHERS_VBATH_OVP_MASK))
		buck_vbat_ovp_handle();

	if (others_irq_state & OTHERS_VBATL_LV_IRQ_MASK)
		vbat_lv_handle();

	if ((others_irq_state & OTHERS_RCPF_DET_IRQ) || (others_irq_state & OTHERS_VBATH_LV_IRQ))
		qsw2_rcp_handle(others_irq_state);

	if ((others_irq_state & OTHERS_TBAT1_OTP_IRQ) || (others_irq_state & OTHERS_TBAT2_OTP_IRQ)) {
		scharger_err("charge_fault:tbat1 or tbat2 OTP happened!\n");
		scharger_sc_enable(DC_DISABLE);
		scharger_lvc_enable(DC_DISABLE);
		scharger_direct_fault_report(POWER_NE_DC_FAULT_TSBAT_OTP, &data);
	}

	if (others_irq_state & OTHERS_TDIE_OTP_IRQ) {
		scharger_err("charge_fault:TDIE OTP happened!\n");
		scharger_sc_enable(DC_DISABLE);
		scharger_lvc_enable(DC_DISABLE);
		scharger_direct_fault_report(POWER_NE_DC_FAULT_TDIE_OTP, &data);
	}

	if (others_irq_state & OTHERS_WGT_TOT_IRQ) {
		scharger_err("charge_fault:WGT TOT happened!\n");
		scharger_sc_enable(DC_DISABLE);
		scharger_lvc_enable(DC_DISABLE);
		scharger_direct_fault_report(POWER_NE_DC_FAULT_IC, &data);
	}

	if (others_irq_state & OTHERS_OVP2_DROPMIN_IRQ) {
		if (dc_get_direct_charge_mode() == SC)
			scharger_sc_enable(DC_DISABLE);
		scharger_direct_fault_report(POWER_NE_DC_FAULT_V700_VDROPMIN, &data);
		queue_delayed_work(system_power_efficient_wq, &buck->vdropmin_check_work,
			 msecs_to_jiffies(VDROPMIN_CHECK_WORK_DELAY));
		scharger_err("charge_fault:ovp2 vdropmin happened!\n");
	}
}

static void ovp_psw_fault_handle(int ovp_psw_irq_state)
{
	char buf[DSM_BUFF_SIZE_MAX] = {0};
	int ret = 0;

	if (ovp_psw_irq_state & OVP_PSW_VUSB_OVP_IRQ) {
		scharger_err("charge_fault:vusb ovp happened!\n");
#ifdef CONFIG_HUAWEI_DSM
		ret = snprintf_s(buf, sizeof(buf), sizeof(buf) - 1, "[OVP_PSW_VUSB_OVP_IRQ]USB port OVP\n");
		if (ret < 0)
			scharger_err("%s, offset %d, snprintf_s error\n",
				     __func__, ret);
		scharger_dsm_report_fault_info(buf, POWER_DSM_CHARGE_SCHARGER_USB_PORT_VDROPMIN_WARNING);
#endif
	}
}

static void buck_irq_work(struct work_struct *work)
{
	unsigned int irq_state = 0;
	u32 buck_irq_state, others_irq_state;
	u16 ovp_psw_irq_state;
	int ret = 0;

	struct scharger_buck *buck =
		 container_of(work, struct scharger_buck, irq_work);

	scharger_dbg("%s +\n", __func__);
	if (buck == NULL) {
		scharger_inf("input ptr work or scharger_buck buck is NULL\n");
		return;
	}

	ret = regmap_read(buck->regmap, CHG_IRQ_ADDR, &irq_state);

	scharger_inf("%s :irq_state:0x%x\n", __func__, irq_state);
	if (irq_state & CHG_BUCK_IRQ) {
		/* BUCK IRQ:0x801~0x804 */
		(void)regmap_bulk_read(buck->regmap, CHG_BUCK_IRQ_ADDR,
						 &buck_irq_state, BUCK_IRQ_BULK_NUM);
		scharger_err(
			"%s:CHG_BUCK_IRQ,irq_state:0x%x,buck_irq_state:0x%8.8x\n",
			__func__, irq_state, buck_irq_state);
		buck_fault_handle(buck_irq_state);
		(void)regmap_bulk_write(buck->regmap, CHG_BUCK_IRQ_ADDR,
						 &buck_irq_state, BUCK_IRQ_BULK_NUM);
	}

	if (irq_state & CHG_OTHERS_IRQ) {
		/* OTHERS IRQ:0x808~0x80b */
		(void)regmap_bulk_read(buck->regmap, CHG_OTHERS_IRQ_ADDR,
						 &others_irq_state, OTHERS_IRQ_BULK_NUM);
		scharger_err(
			"%s:CHG_OTHERS_IRQ,irq_state:0x%x,others_irq_state:0x%8.8x\n",
			__func__, irq_state, others_irq_state);
		others_fault_handle(others_irq_state);
		(void)regmap_bulk_write(buck->regmap, CHG_OTHERS_IRQ_ADDR,
						 &others_irq_state, OTHERS_IRQ_BULK_NUM);
	}

	if (irq_state & CHG_OVP_PSW_IRQ) {
		/* OVP_PSW IRQ 0x805~0x806 */
		(void)regmap_bulk_read(buck->regmap, CHG_OVP_PSW_IRQ_ADDR,
						 &ovp_psw_irq_state, OVP_PSW_IRQ_BULK_NUM);
		scharger_err(
			"%s:CHG_OVP_PSW_IRQ,irq_state:0x%x,ovp_psw_irq_state:0x%4.4x\n",
			__func__, irq_state, ovp_psw_irq_state);
		ovp_psw_fault_handle(ovp_psw_irq_state);
		(void)regmap_bulk_write(buck->regmap, CHG_OVP_PSW_IRQ_ADDR,
						 &ovp_psw_irq_state, OVP_PSW_IRQ_BULK_NUM);
	}

	if (ret)
		scharger_inf("%s:i2c return error\n", __func__);
}

static irqreturn_t buck_interrupt(int irq, void *_buck)
{
	struct scharger_buck *buck = _buck;

	(void)queue_work(system_power_efficient_wq, &buck->irq_work);

	return IRQ_HANDLED;
}

static void buck_irq_prc(struct scharger_buck *buck)
{
	u32 buck_irq_state;
	(void)regmap_bulk_read(buck->regmap, CHG_BUCK_IRQ_ADDR,
				&buck_irq_state, BUCK_IRQ_BULK_NUM);
	scharger_dbg("%s buck_irq_state:%u\n", __func__, buck_irq_state);
	(void)regmap_bulk_write(buck->regmap, CHG_BUCK_IRQ_ADDR,
				&buck_irq_state, BUCK_IRQ_BULK_NUM);
}

static int buck_irq_init(struct scharger_buck *buck)
{
	int ret = 0;

	buck->buck_irq = buck_get_irq_byname(buck->dev, "buck_irq");
	if (buck->buck_irq < 0) {
		scharger_err("failed to get buck_irq %d\n", buck->buck_irq);
		return -ENOENT;
	}
	scharger_inf("buck_irq: %d\n", buck->buck_irq);
	buck_irq_prc(buck);
	ret = request_threaded_irq(buck->buck_irq, NULL, buck_interrupt, IRQF_ONESHOT,
				 "buck_irq", buck);
	if (ret) {
		free_irq(buck->buck_irq, buck);
		scharger_err("request buck irq failed\n");
		return ret;
	}

	return ret;
}

static void others_irq_prc(struct scharger_buck *buck)
{
	u32 others_irq_state;
	(void)regmap_bulk_read(buck->regmap, CHG_OTHERS_IRQ_ADDR,
				 &others_irq_state, OTHERS_IRQ_BULK_NUM);
	scharger_dbg("%s others_irq_state:0x%x\n", __func__, others_irq_state);
	(void)regmap_bulk_write(buck->regmap, CHG_OTHERS_IRQ_ADDR,
				 &others_irq_state, OTHERS_IRQ_BULK_NUM);
}

static int others_irq_init(struct scharger_buck *buck)
{
	int ret = 0;

	buck->others_irq = buck_get_irq_byname(buck->dev, "others_irq");
	if (buck->others_irq < 0) {
		scharger_err("failed to get others_irq %d\n", buck->others_irq);
		return -ENOENT;
	}
	scharger_inf("others_irq: %d\n", buck->others_irq);
	others_irq_prc(buck);
	ret = request_threaded_irq(buck->others_irq, NULL, buck_interrupt, IRQF_ONESHOT,
				 "others_irq", buck);
	if (ret) {
		free_irq(buck->others_irq, buck);
		scharger_err("request others irq failed\n");
		return ret;
	}

	return ret;
}

static void ovp_psw_irq_prc(struct scharger_buck *buck)
{
	u32 ovp_psw_irq_state;
	(void)regmap_bulk_read(buck->regmap, CHG_OVP_PSW_IRQ_ADDR,
				 &ovp_psw_irq_state, OVP_PSW_IRQ_BULK_NUM);
	scharger_inf("%s ovp_psw_irq_state:0x%x\n", __func__, ovp_psw_irq_state);
	(void)regmap_bulk_write(buck->regmap, CHG_OVP_PSW_IRQ_ADDR,
				 &ovp_psw_irq_state, OVP_PSW_IRQ_BULK_NUM);
}

static int ovp_psw_irq_init(struct scharger_buck *buck)
{
	int ret = 0;

	buck->ovp_psw_irq = buck_get_irq_byname(buck->dev, "ovp_psw_irq");
	if (buck->ovp_psw_irq < 0) {
		scharger_err("failed to get ovp_psw_irq %d\n", buck->ovp_psw_irq);
		return -ENOENT;
	}
	scharger_inf("ovp_psw_irq: %d\n", buck->ovp_psw_irq);
	ovp_psw_irq_prc(buck);
	ret = request_threaded_irq(buck->ovp_psw_irq, NULL, buck_interrupt, IRQF_ONESHOT,
				 "ovp_psw_irq", buck);
	if (ret) {
		free_irq(buck->ovp_psw_irq, buck);
		scharger_err("request ovp_psw irq failed\n");
		return ret;
	}

	return ret;
}

static int buck_set_ovpsw(struct power_sw_attr *attr, int status)
{
	struct scharger_buck *buck = NULL;

	if (attr == NULL || attr->dev == NULL) {
		scharger_err("set_ovpsw: attr/dev null\n");
		return -ENODEV;
	}

	buck = attr->dev;
	buck->ovpsw_status = status;
	return scharger_ovp_enable(buck, buck->ovpsw_status);
}

static int buck_get_ovpsw(struct power_sw_attr *attr)
{
	struct scharger_buck *buck = NULL;

	if (attr == NULL || attr->dev == NULL) {
		scharger_err("get_ovpsw: attr/dev null\n");
		return POWER_SW_OFF;
	}

	buck = attr->dev;
	return buck->ovpsw_status;
}

static void buck_free_ovpsw(struct power_sw_attr *attr)
{
	unused(attr);
}

static void buck_ovpsw_register(struct scharger_buck *buck)
{
	struct power_sw sw;

	if (buck->ovpsw_attr.num < 0)
		return;

	sw.attr.dev = (void *)buck;
	sw.attr.num = buck->ovpsw_attr.num;
	sw.attr.dflt = buck->ovpsw_attr.dflt;
	sw.attr.en = buck->ovpsw_attr.en;
	sw.attr.label = buck->ovpsw_attr.label;
	sw.set = buck_set_ovpsw;
	sw.get = buck_get_ovpsw;
	sw.free = buck_free_ovpsw;

	(void)power_sw_register(sw.attr.num, &sw);
}

static int get_chg_mode(void)
{
	return g_buck->chg_mode == BUCK;
}

static int get_chg_enable(void)
{
	return (int)get_charge_enable();
}

static void clear_cv_trim_flag(void)
{
	g_cv_info->cv_trim_flag = 0;
}

static void reset_cv_value(void)
{
	int ret;

	if (g_cv_info->cv_trim_flag == 1) {
		ret = __set_terminal_voltage(g_cv_info->cv_ori_val);
		if (ret)
			scharger_err("%s set vterm fail\n", __func__);
	}
	g_cv_info->cv_new_set_val = 0;
	g_cv_info->cv_trim_flag = 0;
	g_cv_info->cv_ori_val = 0;
}

static int cv_get_dieid(char *dieid, unsigned int len)
{
	int i;
	int err;
	char dieid_addr[SCHARGER_DIEID_LENGTH] = {
				SCHARGER_DIEID_BYTE0,
				SCHARGER_DIEID_BYTE1,
				SCHARGER_DIEID_BYTE2,
				SCHARGER_DIEID_BYTE3,
				SCHARGER_DIEID_BYTE4,
				SCHARGER_DIEID_BYTE5,
				SCHARGER_DIEID_BYTE6,
				SCHARGER_DIEID_BYTE7
	};

	if (dieid == NULL) {
		scharger_err("%s: dieid is null\n", __func__);
		return -1;
	}
	if (len < SCHARGER_DIEID_LENGTH) {
		scharger_err("%s: mem length is not enough!\n", __func__);
		return -1;
	}

	err = memset_s(dieid, len, 0, len);
	if (err != EOK)
		scharger_err("[%s]memset_s fail, err=%d\n", __func__, err);

	for (i = 0 ; i < SCHARGER_DIEID_LENGTH; i++)
		scharger_efuse_read(dieid_addr[i], &dieid[i]);

	return 0;
}

static int cv_set_vterm(int cv_ori, int cv_new_set)
{
	int cv_cur;
	int cv_set;
	int ret;

	if (cv_new_set - cv_ori > CV_TRIM_PROTECT) {
		g_cv_info->cv_trim_flag = 0;
		scharger_err("%s cv_new_set is too large,cv_new_set:%d, cv_ori:%d, stop cv trim\n",
			 __func__, cv_new_set, cv_ori);
		return -1;
	}

	cv_set = cv_new_set;
	if (cv_new_set - cv_ori > CV_TRIM_MAX) {
		cv_set = cv_ori + CV_TRIM_MAX;
		scharger_err("%s cv_new_set is too large, use %d\n", cv_set);
	}

	cv_cur = scharger_get_terminal_voltage();
	if (cv_cur != cv_ori) {
		g_cv_info->cv_trim_flag = 0;
		scharger_err("%s vterm changed cv_cur:%d, cv_ori:%d, stop cv trim\n", __func__, cv_cur, cv_ori);
		return -1;
	}

	g_cv_info->cv_ori_val = cv_ori;
	g_cv_info->cv_new_set_val = cv_set;

	ret = __set_terminal_voltage(cv_set);
	if (ret) {
		g_cv_info->cv_trim_flag = 0;
		scharger_err("%s set vterm fail\n", __func__);
		return -1;
	}

	g_cv_info->cv_trim_flag = 1;

	return 0;
}

struct scharger_cv_ops g_cv_ops = {
	.get_charge_done_state = scharger_get_charger_state,
	.get_chg_mode = get_chg_mode,
	.get_chg_enable = get_chg_enable,
	.get_vterm = scharger_get_terminal_voltage,
	.set_vterm = cv_set_vterm,
	.clear_cv_trim_flag = clear_cv_trim_flag,
	.reset_cv_value = reset_cv_value,
	.get_dieid = cv_get_dieid,
};

static int scharger_usb_notifier_call(struct notifier_block *usb_nb,
				    unsigned long event, void *data)
{
	struct scharger_buck *buck = container_of(usb_nb, struct scharger_buck, usb_nb);

	if (buck == NULL) {
		scharger_err("%s buck is NULL\n", __func__);
		return -EINVAL;
	}

	scharger_inf("%s + , event:%d\n", __func__, event);

	switch (event) {
	case CHARGER_TYPE_SDP:
		buck->usb_mode = CHG_TYPE_SDP;
		break;
	case CHARGER_TYPE_CDP:
		buck->usb_mode = CHG_TYPE_CDP;
		break;
	case CHARGER_TYPE_DCP:
		buck->usb_mode = CHG_TYPE_DCP;
		break;
	case CHARGER_TYPE_UNKNOWN:
		buck->usb_mode = CHG_TYPE_UNKNOWN;
		break;
	case CHARGER_TYPE_NONE:
		buck->usb_mode = CHG_TYPE_NONE;
		break;
	default:
		buck->usb_mode = CHG_TYPE_UNKNOWN;
		break;
	}

	switch (event) {
	case CHARGER_TYPE_SDP:
	case CHARGER_TYPE_CDP:
	case CHARGER_TYPE_DCP:
	case CHARGER_TYPE_UNKNOWN:
		charger_present_flag = 1;
		dual_batfet_forced_conduction(buck, BATFET_CONDUCTION_DISABLE);
		break;
	case CHARGER_TYPE_NONE:
		charger_present_flag = 0;
		if (buck->boost_en_flag == FALSE)
			dual_batfet_forced_conduction(buck, BATFET_CONDUCTION_ENABLE);
		break;
	default:
		return NOTIFY_OK;
	}

	return NOTIFY_OK;
}

#ifdef CONFIG_HUAWEI_DSM
static void one_battery_discharge_delay_work(struct work_struct *work)
{
	struct scharger_buck *buck = g_buck;
	int vbat_h, vbat_l;
	char buff[DSM_BUFF_SIZE_MAX] = {0};
	int ret = 0;
	static int err_count = 0;

	if (buck->bat_sel == SINGLE_BATTERY)
		return;

	if (charger_present_flag == 1)
		return;

	vbat_h = coul_merge_drv_battery_voltage_mv(BATT_H);
	vbat_l = coul_merge_drv_battery_voltage_mv(BATT_L);
	if (abs(vbat_h - vbat_l) > VOLTAGE_300MV)
		err_count++;
	else
		err_count = 0;

	if (err_count >= ONE_BATTERY_DISCHARGE_ERR_COUNT) {
		scharger_inf("one battery discharge happened!\n");
		ret = sprintf_s(buff, (size_t)DSM_BUFF_SIZE_MAX,
				"[one battery discharge]one battery discharge happened, vbat_h:%d, vbat_l:%d", vbat_h ,vbat_l);
		if (ret < 0)
			scharger_err("%s, sprintf_s error\n", __func__);
		scharger_dsm_report_fault_info(buff, POWER_DSM_CHARGE_SCHARGER_PARALLEL_ONE_BATTERY_DISCHARGE_WARNING);
		err_count = 0;
	}

	queue_delayed_work(system_power_efficient_wq, &buck->one_battery_discharge_work,
			   msecs_to_jiffies(ONE_BATTERY_DISCHARGE_MONITOR_DELAY));
}
#endif

static unsigned int scharger_get_bat_sel(struct scharger_buck *buck)
{
	int ret;
	unsigned int data = 0;

	ret = regmap_read(buck->regmap, BAT_SEL_REG, &data);
	if (ret < 0) {
		scharger_err("%s err\n", __func__);
		return -1;
	}

	scharger_inf("[%s] bat_sel:%u\n", __func__, data);

	return data;
}

static int scharger_buck_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct scharger_buck *buck = NULL;
	int ret;

#ifdef CONFIG_HUAWEI_DSM
	u8 ovp2_state, ovp3_state;
#endif

	buck = devm_kzalloc(dev, sizeof(*buck), GFP_KERNEL);
	if (!buck) {
		scharger_err("[%s]buck is null.\n", __func__);
		return -ENOMEM;
	}

	mutex_init(&buck->otg_lock);
	mutex_init(&buck->ibias_calc_lock);

	buck->dev = dev;
	buck->regmap = dev_get_regmap(dev->parent, NULL);
	if (!buck->regmap) {
		scharger_err("Parent regmap unavailable.\n");
		return -ENXIO;
	}

	g_cv_info = (struct cv_info *)devm_kzalloc(dev, sizeof(struct cv_info), GFP_KERNEL);
	if (!g_cv_info) {
		scharger_err("[%s]g_cv_info is null.\n", __func__);
		return -ENOMEM;
	}

	g_buck = buck;

	buck->bat_sel = (int)scharger_get_bat_sel(buck);
	if (buck->bat_sel == -1) {
		scharger_err("%s scharger_get_bat_sel failed\n", __func__);
		return -ENXIO;
	}

	buck_parse_dts(dev->of_node, buck);
	buck_ovpsw_register(buck);

	INIT_DELAYED_WORK(&buck->reverbst_work, reverbst_delay_work);
	INIT_DELAYED_WORK(&buck->vdropmin_check_work, vdropmin_check_delay_work);
	INIT_WORK(&buck->irq_work, buck_irq_work);
#ifdef CONFIG_HUAWEI_DSM
	INIT_DELAYED_WORK(&buck->one_battery_discharge_work, one_battery_discharge_delay_work);
#endif

	buck->scharger_version = scharger_get_device_version();

	dual_batfet_forced_conduction(buck, BATFET_CONDUCTION_ENABLE);

	buck->usb_nb.notifier_call = scharger_usb_notifier_call;
	ret = chip_charger_type_notifier_register(&buck->usb_nb);
	if (ret)
		scharger_err("%s notifier_register failed\n", __func__);

	batfet_init();
	vbat_lv_handle();
	scharger_mask_reset_n_disable(buck);

	/* factory problem debug */
	if (buck->scharger_version == CHIP_ID_V700) {
	} else {
#ifdef CONFIG_HUAWEI_DSM
		INIT_DELAYED_WORK(&buck->ovp_dmd_work, ovp_dmd_delay_work);
		ret = check_ovp2_state(&ovp2_state);
		ret += check_ovp3_state(&ovp3_state);
		if (ret < 0) {
			scharger_err("%s: check ovp2 ovp3 state fail\n", __func__);
		} else {
			if (ovp2_state || ovp3_state)
				ovp2_ovp3_err_flag = 1;
		}
#endif
		ret = scharger_psw_enable(0);
		ret += scharger_ovp3_enable(0);
		if (ret < 0)
			scharger_err("%s: init ovp2 ovp3 fail\n", __func__);
	}

	buck->term_vol_mv = scharger_get_terminal_voltage();

	buck_opt_param(VBUS_VSET_5V, buck);

	buck_irq_init(buck);
	others_irq_init(buck);
	ovp_psw_irq_init(buck);

	ret = charge_ops_register(&scharger_ops, BUCK_IC_TYPE_THIRDPARTY);
	if (ret)
		scharger_err("register charge ops failed!\n");

#ifdef CONFIG_HUAWEI_CHARGER_AP
	ret = charger_otg_ops_register(&scharger_otg_ops);
	if (ret)
		scharger_err("register charger_otg ops failed\n");
#endif

#ifdef CONFIG_SCHARGER_DYNAMIC_CV
	if (buck->scharger_version == CHIP_ID_V700) {
	} else {
		ret = scharger_cv_ops_register(&g_cv_ops);
		if (ret)
			scharger_err("register cv ops failed\n");
	}
#endif

	scharger_log_ops.dev_data = (void *)buck;
	power_log_ops_register(&scharger_log_ops);
#ifdef CONFIG_HUAWEI_DSM
	queue_delayed_work(system_power_efficient_wq, &buck->one_battery_discharge_work,
			   msecs_to_jiffies(ONE_BATTREY_DISCHARGE_WORK_DELAY));
#endif

	return 0;
}

static int scharger_buck_remove(struct platform_device *pdev)
{
	g_buck = NULL;
#ifdef CONFIG_HUAWEI_DSM
	ovp2_ovp3_err_flag = 0;
#endif

	return 0;
}

const static struct of_device_id of_scharger_buck_match_tbl[] = {
	{
		.compatible = "hisilicon,scharger-v700-buck",
		.data = NULL,
	},
	{},
};

static struct platform_driver scharger_buck_driver = {
	.driver = {
		.name	= "scharger_v700_buck",
		.owner  = THIS_MODULE,
		.of_match_table = of_scharger_buck_match_tbl,
	},
	.probe	= scharger_buck_probe,
	.remove	= scharger_buck_remove,
};

static int __init scharger_buck_init(void)
{
	return platform_driver_register(&scharger_buck_driver);
}

static void __exit scharger_buck_exit(void)
{
	platform_driver_unregister(&scharger_buck_driver);
}

fs_initcall(scharger_buck_init);
module_exit(scharger_buck_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("schargerV700 buck driver");
