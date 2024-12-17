/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Device driver for schargerV700 dc
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
#include "scharger_v700_dc.h"
#include "scharger_v700_fcp.h"
#include <linux/power_supply.h>
#include "scharger_v700_batctl.h"
#include <chipset_common/hwpower/common_module/power_event_ne.h>

#define ADAPTER_VENDOR_ID      0x82
#define ADAPTER_VENDOR_ID_SIZE 2
#define IWATT_ADAPTER_ID       0x80A2
#define ADAPTER_TYPE           0x8d
#define ADAPTER_CURRENT        0xc9
#define ADAPTER_CURRENT_UNIT   50

struct scharger_dc {
	struct device *dev;
	struct regmap *regmap;
	struct notifier_block nb;
	int scp_support;
	int gpio_wldc_en;
	int gpio_wldc_en_valid_val;
	char batt_l_name[BATT_NAME_SIZE_MAX];
	char batt_h_name[BATT_NAME_SIZE_MAX];
	struct delayed_work dc_vdrop_ovp_work;
	enum chg_mode_state dc_chg_mode;
	unsigned int scharger_version;
	bool iwatt_lvc_adap;
	bool lvc_init_flag;
	bool sc_en_flag;
};

#ifndef unused
#define unused(x) ((void)(x))
#endif

static struct scharger_dc *g_scharger_dc;

enum chg_mode_state dc_get_direct_charge_mode(void)
{
	if (g_scharger_dc == NULL)
		return -1;
	return g_scharger_dc->dc_chg_mode;
}

static int switch_to_buck_mode(struct scharger_dc *dc)
{
	int ret = 0;

	if (dc == NULL) {
		scharger_err("%s scharger_dc is NULL!\n", __func__);
		return -EINVAL;
	}

	scharger_inf("%s\n", __func__);

	ret = bat_cnct_ctrl(BAT_PARALLEL_MODE);
	if (ret) {
		scharger_err("%s battery switched to parallel mode fail!\n", __func__);
		return ret;
	}

	ret = scharger_set_buck_mode_enable(CHG_ENABLE);
	if (ret != 0)
		pr_err("%s scharger_set_buck_mode_enable err!\n", __func__);

	/* usb plugin enable */
	ret += regmap_update_bits(dc->regmap, REG_USB_OVP_CFG_REG_2, USB_PLUGIN_EN_MSK,
			 USB_PLUGIN_EN << USB_PLUGIN_EN_SHIFT);
	/* set usb ovp 11v */
	ret += set_vusb_ovp(USB_OVP_11V);

	ret = scharger_set_charge_enable(CHG_DISABLE);
	if (ret != 0)
		pr_err("%s set_charge_enable err!\n", __func__);
	config_opt_param(VBUS_VSET_5V);
	set_fast_safe_timer(CHG_FASTCHG_TIMER_20H);
	ret = dpm_init();
	if (ret != 0)
		pr_err("%s dpm_init err!\n", __func__);
	ret = scharger_set_term_enable(CHG_TERM_DIS);
	if (ret != 0)
		pr_err("%s set_term_enable err!\n", __func__);
	ret = scharger_set_input_current(CHG_ILIMIT_475);
	if (ret != 0)
		pr_err("%s set_input_current err!\n", __func__);
	ret = scharger_set_charge_current(CHG_FAST_ICHG_500MA);
	if (ret != 0)
		pr_err("%s set_input_current err!\n", __func__);
	ret = scharger_set_terminal_voltage(CHG_FAST_VCHG_4430);
	if (ret != 0)
		pr_err("%s set_input_current err!\n", __func__);
	(void)scharger_set_terminal_current(CHG_TERM_ICHG_150MA);
	(void)scharger_set_watchdog_timer(WATCHDOG_TIMER_05_S);

	return 0;
}

static int dc_get_chip_temp(int *temp, void *dev_data)
{
	unused(dev_data);
	*temp = scharger_get_tdie();
	return *temp;
}

static int dc_get_coul_ibat(const char *batt_name, int *ibat)
{
	int ret;
	struct power_supply *psy = NULL;
	union power_supply_propval val = {0};

	if (ibat == NULL)
		return -EINVAL;

	psy = power_supply_get_by_name(batt_name);
	if (!psy) {
		scharger_err("power supply %s not exist.\n", batt_name);
		return -EINVAL;
	}

	ret = power_supply_get_property(psy, POWER_SUPPLY_PROP_CURRENT_NOW, &val);
	power_supply_put(psy);

	*ibat = val.intval;

	return ret;
}

static bool dc_is_iwatt_lvc_adapter(void)
{
	u32 value[ADAPTER_VENDOR_ID_SIZE] = { 0 };
	int adp_type = -1;
	int id;

	scharger_v700_scp_read_block(ADAPTER_VENDOR_ID, value, ADAPTER_VENDOR_ID_SIZE);
	id = (value[0] << 8) | value[1];
	if (id != IWATT_ADAPTER_ID)
		return false;

	scharger_v700_scp_read_block(ADAPTER_TYPE, &adp_type, 1);
	if (adp_type != 0)
		return false;

	scharger_inf("use iwatt lvc adapter\n");
	return true;
}

static int dc_get_iadpter(void)
{
	int value = 0;

	scharger_v700_scp_read_block(ADAPTER_CURRENT, &value, 1);
	return value * ADAPTER_CURRENT_UNIT;
}

static int dc_get_lvc_ibus_ma(int value, struct scharger_dc *dc)
{
	if (!dc->lvc_init_flag || dc->iwatt_lvc_adap ||
		(charge_get_charger_type() == CHARGER_TYPE_WIRELESS))
		return value;

	return dc_get_iadpter();
}

static int dc_get_ibus_ma(int *ibus, void *dev_data)
{
	int mode;
	int ibat_l = 0;
	int ibat_h = 0;
	int ret = 0;
	struct scharger_dc *dc = dev_data;

	if (dc == NULL) {
		scharger_err("%s scharger_dc is NULL!\n", __func__);
		return -EINVAL;
	}
	if (ibus == NULL)
		return -EINVAL;

	ret += dc_get_coul_ibat(dc->batt_l_name, &ibat_l);
	ret += dc_get_coul_ibat(dc->batt_h_name, &ibat_h);

	mode = bat_cnct_mode_get();
	if (mode == BAT_SERIES_MODE)
		*ibus = ibat_h;
	else
		*ibus = dc_get_lvc_ibus_ma(ibat_h + ibat_l, dc);

	scharger_err("%s ibat_l=%d, ibat_h=%d, ibus=%d\n", __func__, ibat_l, ibat_h, *ibus);
	if (*ibus < 0)
		*ibus = 0;

	return ret;
}

static int dc_get_ibat_ma(int *ibat, void *dev_data)
{
	int ibat_l = 0;
	int ibat_h = 0;
	int ret = 0;
	struct scharger_dc *dc = dev_data;

	if (dc == NULL) {
		scharger_err("%s scharger_dc is NULL!\n", __func__);
		return -EINVAL;
	}
	if (ibat == NULL)
		return -EINVAL;

	ret += dc_get_coul_ibat(dc->batt_l_name, &ibat_l);
	ret += dc_get_coul_ibat(dc->batt_h_name, &ibat_h);
	*ibat = (ibat_l + ibat_h);

	scharger_err("%s ibat_l=%d, ibat_h=%d, ibat=%d.\n", __func__, ibat_l, ibat_h, *ibat);
	if (*ibat < 0)
		*ibat = 0;
	return 0;
}

static int dc_get_coul_vbat(const char *batt_name, int *vbat)
{
	int ret;
	struct power_supply *psy = NULL;
	union power_supply_propval val = {0};

	if (vbat == NULL)
		return -EINVAL;

	psy = power_supply_get_by_name(batt_name);
	if (!psy) {
		scharger_err("power supply %s not exist\n", batt_name);
		return -EINVAL;
	}

	ret = power_supply_get_property(psy, POWER_SUPPLY_PROP_VOLTAGE_NOW, &val);
	power_supply_put(psy);

	*vbat = val.intval;
	return ret;
}

static int dc_get_vbat_mv(void *dev_data)
{
	int ret = 0;
	int vbat_l = 0;
	int vbat_h = 0;
	struct scharger_dc *dc = dev_data;

	if (dc == NULL) {
		scharger_err("%s scharger_dc is NULL!\n", __func__);
		return -EINVAL;
	}

	ret += dc_get_coul_vbat(dc->batt_l_name, &vbat_l);
	ret += dc_get_coul_vbat(dc->batt_h_name, &vbat_h);
	if (ret) {
		scharger_err("%s error\n", __func__);
		return 0;
	}
	scharger_err("%s vbat_l=%d, vbat_h=%d\n", __func__, vbat_l, vbat_h);

	return vbat_l > vbat_h ? (vbat_l / 1000) : (vbat_h / 1000);
}

static int dc_get_vbus_mv(int *vbus, void *dev_data)
{
	unused(dev_data);
	if (vbus == NULL)
		return -EINVAL;

	*vbus = scharger_get_vbus();

	scharger_inf("%s vbus=%d\n", __func__, *vbus);
	return 0;
}

static int dc_get_vusb_mv(int *vusb, void *dev_data)
{
	unused(dev_data);
	if (!vusb)
		return -EINVAL;

	*vusb = scharger_get_vusb();

	return 0;
}

static int get_sc_device_id(void *dev_data)
{
	unused(dev_data);
	return SWITCHCAP_SCHARGERV700;
}

static int sc_charge_init(void *dev_data)
{
	int ret = 0;
	struct scharger_dc *dc = dev_data;

	scharger_err("%s +\n", __func__);

	if (dc == NULL) {
		scharger_err("%s scharger_dc is NULL!\n", __func__);
		return -EINVAL;
	}

	ret = scharger_set_buck_mode_enable(CHG_DISABLE);
	if (ret != 0)
		scharger_err("%s scharger_set_buck_mode_enable err!\n", __func__);

	/* disable usb plugin */
	ret += regmap_update_bits(dc->regmap, REG_USB_OVP_CFG_REG_2, USB_PLUGIN_EN_MSK,
			 USB_PLUGIN_DIS << USB_PLUGIN_EN_SHIFT);
	/* set usb ovp 11v */
	ret += set_vusb_ovp(USB_OVP_11V);
	if (ret) {
		scharger_err("%s disable ovp fail!\n", __func__);
		return ret;
	}

	return ret;
}

static int lvc_charge_init(void *dev_data)
{
	int ret = 0;
	struct scharger_dc *dc = dev_data;

	scharger_err("%s +\n", __func__);

	if (dc == NULL) {
		scharger_err("%s scharger_dc is NULL!\n", __func__);
		return -EINVAL;
	}
	if (dc->scharger_version == CHIP_ID_V700)
		return ret;

	ret = scharger_set_buck_mode_enable(CHG_DISABLE);
	if (ret != 0)
		scharger_err("%s scharger_set_buck_mode_enable err!\n", __func__);

	/* disable usb plugin */
	ret += regmap_update_bits(dc->regmap, REG_USB_OVP_CFG_REG_2, USB_PLUGIN_EN_MSK,
			USB_PLUGIN_DIS << USB_PLUGIN_EN_SHIFT);

	/* set usb ovp 24v */
	ret += set_vusb_ovp(USB_OVP_24V);
	if (ret) {
		scharger_err("%s disable ovp fail!\n", __func__);
		return ret;
	}

	dc->iwatt_lvc_adap = dc_is_iwatt_lvc_adapter();
	dc->lvc_init_flag = true;
	return ret;
}

static int dc_charge_exit(void *dev_data)
{
	struct scharger_dc *dc = dev_data;
	int ret = 0;

	if (dc == NULL) {
		scharger_err("%s scharger_dc is NULL!\n", __func__);
		return -EINVAL;
	}

	ret = switch_to_buck_mode(dc);
	dc->dc_chg_mode = NONE;
	dc->iwatt_lvc_adap = false;
	dc->lvc_init_flag = false;

	return ret;
}

static int scharger_wldc_enable(struct scharger_dc *dc, int enable)
{
	int gpio_set;
	int gpio_get;

	if (dc->gpio_wldc_en < 0)
		return 0;

	gpio_set = enable ? dc->gpio_wldc_en_valid_val : !dc->gpio_wldc_en_valid_val;
	gpio_direction_output(dc->gpio_wldc_en, gpio_set);
	gpio_get = gpio_get_value(dc->gpio_wldc_en);
	scharger_inf("wldc_enable gpio %s now\n", gpio_get ? "high" : "low");

	/* delay 20ms for avoid irq_batsw2_scp wrong report */
	if (!enable)
		mdelay(20);

	return gpio_get == gpio_set ? 0 : -EIO;
}

void set_qsw2_rcp_protect(int enable)
{
	int ret = 0;
	struct scharger_dc *dc = g_scharger_dc;

	if (dc == NULL) {
		scharger_err("%s scharger_dc is NULL!\n", __func__);
		return;
	}

	if (enable) {
		ret += regmap_update_bits(dc->regmap, SCHG_LOGIC_CFG_REG_1, QSW2_RCP_FAST_PROTECT_MSK,
				QSW2_RCP_PROTECT_ENANLE << QSW2_RCP_FAST_PROTECT_SHIFT);
		ret += regmap_update_bits(dc->regmap, SCHG_LOGIC_CFG_REG_1, QSW2_RCP_SLOW_PROTECT_MSK,
				QSW2_RCP_PROTECT_ENANLE << QSW2_RCP_SLOW_PROTECT_SHIFT);
	} else {
		ret += regmap_update_bits(dc->regmap, SCHG_LOGIC_CFG_REG_1, QSW2_RCP_FAST_PROTECT_MSK,
				QSW2_RCP_PROTECT_DISABLE << QSW2_RCP_FAST_PROTECT_SHIFT);
		ret += regmap_update_bits(dc->regmap, SCHG_LOGIC_CFG_REG_1, QSW2_RCP_SLOW_PROTECT_MSK,
				QSW2_RCP_PROTECT_DISABLE << QSW2_RCP_SLOW_PROTECT_SHIFT);
	}
	if (ret)
		scharger_err("%s: set_qsw2_rcp_protect %d fail\n", __func__, enable);
}

static void ovp2_ovp3_drop_config(struct scharger_dc *dc, int enable)
{
	if (dc->dc_chg_mode == SC) {
		if (enable) {
			regmap_update_bits(dc->regmap, SCHG_LOGIC_CFG_REG_4, DROPMIN_OVP_TIME_MSK,
				DROPMIN_OVP_TIME_10MS << DROPMIN_OVP_TIME_SHIFT);
			regmap_update_bits(dc->regmap, PSW_OVP_CFG_REG_1, OVP2_DROPOVP_MIN_EN_MSK,
				OVP2_DROPOVP_ENABLE << OVP2_DROPOVP_MIN_EN_SHIFT);
		} else {
			regmap_update_bits(dc->regmap, SCHG_LOGIC_CFG_REG_4, DROPMIN_OVP_TIME_MSK,
				DROPMIN_OVP_TIME_32US << DROPMIN_OVP_TIME_SHIFT);
			regmap_update_bits(dc->regmap, PSW_OVP_CFG_REG_3, OVP2_DROPOVP_EN_MSK,
				OVP2_DROPOVP_DISABLE << OVP2_DROPOVP_EN_SHIFT);
			regmap_update_bits(dc->regmap, PSW_OVP_CFG_REG_1, OVP2_DROPOVP_MIN_EN_MSK,
				OVP2_DROPOVP_DISABLE << OVP2_DROPOVP_MIN_EN_SHIFT);
		}
	} else if (dc->dc_chg_mode == LVC) {
		if (enable) {
			regmap_update_bits(dc->regmap, SCHG_LOGIC_CFG_REG_4, DROPMIN_OVP_TIME_MSK,
				DROPMIN_OVP_TIME_10MS << DROPMIN_OVP_TIME_SHIFT);
			regmap_update_bits(dc->regmap, PSW_OVP_CFG_REG_3, OVP2_DROPOVP_EN_MSK,
				OVP2_DROPOVP_ENABLE << OVP2_DROPOVP_EN_SHIFT);
		} else {
			regmap_update_bits(dc->regmap, SCHG_LOGIC_CFG_REG_4, DROPMIN_OVP_TIME_MSK,
				DROPMIN_OVP_TIME_32US << DROPMIN_OVP_TIME_SHIFT);
			regmap_update_bits(dc->regmap, PSW_OVP_CFG_REG_3, OVP2_DROPOVP_EN_MSK,
				OVP2_DROPOVP_DISABLE << OVP2_DROPOVP_EN_SHIFT);
			regmap_update_bits(dc->regmap, PSW_OVP_CFG_REG_1, OVP2_DROPOVP_MIN_EN_MSK,
				OVP2_DROPOVP_DISABLE << OVP2_DROPOVP_MIN_EN_SHIFT);
		}
	} else {
		scharger_err("%s dc charge mode err %d!\n", __func__, dc->dc_chg_mode);
	}
}

static void scharger_dc_after_direct_charger(struct scharger_dc *dc, int enable)
{
	if (dc->scharger_version == CHIP_ID_V700)
		return;

	if (dc->dc_chg_mode == SC) {
		set_qsw2_rcp_protect(enable);
		if (enable) {
			mdelay(50);
			regmap_update_bits(dc->regmap, PSW_OVP_CFG_REG_3, OVP2_DROPOVP_EN_MSK,
				 OVP2_DROPOVP_ENABLE << OVP2_DROPOVP_EN_SHIFT);
			queue_delayed_work(system_power_efficient_wq, &dc->dc_vdrop_ovp_work,
				msecs_to_jiffies(IBAT_ABNORMAL_TIME));
		} else {
			ovp2_ovp3_drop_config(dc, enable);
		}
	} else if (dc->dc_chg_mode == LVC) {
		mdelay(50);
		ovp2_ovp3_drop_config(dc, enable);
		if (enable)
			queue_delayed_work(system_power_efficient_wq, &dc->dc_vdrop_ovp_work,
				msecs_to_jiffies(IBAT_ABNORMAL_TIME));
	} else {
		scharger_err("%s dc charge mode err %d!\n", __func__, dc->dc_chg_mode);
	}
}

static int lvc_charge_enable(int enable, void *dev_data)
{
	struct scharger_dc *dc = dev_data;
	int en;
	u8 err_flag = 0;

	if (dc == NULL) {
		scharger_err("%s scharger_dc is NULL!\n", __func__);
		return -EINVAL;
	}

	en = !!enable;
	scharger_inf("%s en %d\n", __func__, en);

	err_flag = scharger_get_charge_err_flag();
	if (en == 1 && ((err_flag & CHARGE_ERR_FLAG) || (err_flag & LVC_CHG_ERR_FLAG))) {
		scharger_err("%s err_flag:%u\n", __func__, err_flag);
		return -1;
	}

	if (dc->scharger_version == CHIP_ID_V700)
		return 0;

	if (en)
		dc->dc_chg_mode = LVC;
	else
		dc->lvc_init_flag = false;

	if (charge_get_charger_type() == CHARGER_TYPE_WIRELESS)
		scharger_wldc_enable(dc, en);
	else
		scharger_psw_enable(en);

	if (!en)
		cancel_delayed_work(&dc->dc_vdrop_ovp_work);
	scharger_dc_after_direct_charger(dc, en);

	return 0;
}

int scharger_lvc_enable(int enable)
{
	int en;
	int ret = 0;

	scharger_inf("%s enable:%d\n", __func__, enable);

	if (g_scharger_dc == NULL)
		return -1;

	en = !!enable;
	ret = lvc_charge_enable(en, g_scharger_dc);

	return ret;
}

static int dc_discharge(int val, void *dev_data)
{
	unused(dev_data);

	return 0;
}

static int dc_charge_wireless_is_close(struct scharger_dc *dc)
{
	int gpio_val;

	if (dc->gpio_wldc_en < 0)
		return 0;

	gpio_val = gpio_get_value(dc->gpio_wldc_en);
	if (gpio_val == dc->gpio_wldc_en_valid_val)
		return 0;

	return 1;
}

static int lvc_charge_is_close(void *dev_data)
{
	struct scharger_dc *dc = dev_data;
	u8 psw_state = 0;
	int ret = 0;

	if (dc == NULL) {
		scharger_err("%s scharger_dc is NULL!\n", __func__);
		return -EINVAL;
	}

	if (charge_get_charger_type() == CHARGER_TYPE_WIRELESS)
		return dc_charge_wireless_is_close(dc);

	ret = check_ovp2_state(&psw_state);
	if (ret) {
		scharger_err("%s: check ovp2 state fail\n", __func__);
		return -1;
	}

	if (psw_state == PSW_ENABLE)
		return 0;

	return 1;
}

static int get_loadswitch_id(void *dev_data)
{
	unused(dev_data);
	return 0;
}

static int dc_config_watchdog_ms(int time, void *dev_data)
{
	unused(dev_data);
	return scharger_set_watchdog_timer_ms(time);
}

static int dc_kick_watchdog_timer(void *dev_data)
{
	unused(dev_data);
	return scharger_reset_watchdog_timer();
}

static void scharger_dc_vdrop_ovp_delay_work(struct work_struct *work)
{
	struct scharger_dc *dc = container_of(work, struct scharger_dc, dc_vdrop_ovp_work.work);
	int ibat_l = 0;
	int ibat_h = 0;

	if (dc == NULL) {
		scharger_err("%s scharger_dc is NULL!\n", __func__);
		return;
	}

	if (dc->dc_chg_mode != SC && dc->dc_chg_mode != LVC)
		return;

	dc_get_coul_ibat(dc->batt_l_name, &ibat_l);
	dc_get_coul_ibat(dc->batt_h_name, &ibat_h);

	if (dc->dc_chg_mode == SC) {
		if (ibat_h >= CHG_CURRENT_1300MA) {
			ovp2_ovp3_drop_config(dc, OVP2_DROPOVP_ENABLE);
			scharger_inf("%s enable ovp2 drop ovp!\n", __func__);
		} else {
			queue_delayed_work(system_power_efficient_wq, &dc->dc_vdrop_ovp_work,
				msecs_to_jiffies(IBAT_ABNORMAL_TIME));
		}
	} else if (dc->dc_chg_mode == LVC) {
		if (ibat_h + ibat_l >= CHG_CURRENT_1300MA) {
			scharger_inf("%s enable ovp2 drop min!\n", __func__);
			regmap_update_bits(dc->regmap, PSW_OVP_CFG_REG_1, OVP2_DROPOVP_MIN_EN_MSK,
				OVP2_DROPOVP_ENABLE << OVP2_DROPOVP_MIN_EN_SHIFT);
		} else {
			queue_delayed_work(system_power_efficient_wq, &dc->dc_vdrop_ovp_work,
				msecs_to_jiffies(IBAT_ABNORMAL_TIME));
		}
	}
}

static int sc_charge_enable(int enable, void *dev_data)
{
	struct scharger_dc *dc = dev_data;
	int ret = 0;
	int en;
	u8 err_flag = 0;

	if (dc == NULL) {
		scharger_err("%s scharger_dc is NULL!\n", __func__);
		return -EINVAL;
	}

	en = !!enable;
	scharger_inf("%s en %d\n", __func__, en);

	err_flag = scharger_get_charge_err_flag();
	if (en == 1 && ((err_flag & CHARGE_ERR_FLAG) || (err_flag & SC_CHG_ERR_FLAG))) {
		scharger_err("%s err_flag:%u\n", __func__, err_flag);
		return -1;
	}

	dc->sc_en_flag = !!en;
	if (en) {
		dc->dc_chg_mode = SC;
		/* battery switched to series mode */
		ret = bat_cnct_ctrl(BAT_SERIES_MODE);
		scharger_inf("swith to series mode ret=%d\n", ret);
		if (ret) {
			scharger_err("%s battery switched to series mode fail\n", __func__);
			return ret;
		}
		/* delay 20ms for battery switch to serials mode */
		mdelay(20);
		if (charge_get_charger_type() == CHARGER_TYPE_WIRELESS)
			ret = scharger_wldc_enable(dc, en);
		else
			ret = scharger_psw_enable(en);
		scharger_inf("open ovp ret=%d\n", ret);
		scharger_dc_after_direct_charger(dc, en);
	} else {
		ret = scharger_psw_enable(en) + scharger_wldc_enable(dc, en);
		scharger_inf("close ovp ret=%d\n", ret);
		ret += bat_cnct_ctrl(BAT_PARALLEL_MODE);
		scharger_inf("swith to parallel mode ret=%d\n", ret);
		if (ret)
			scharger_err("%s close ovp or switch to parallel mode fail\n", __func__);
		cancel_delayed_work(&dc->dc_vdrop_ovp_work);
		scharger_dc_after_direct_charger(dc, en);
		return ret;
	}

	return ret;
}

int scharger_sc_enable(int enable)
{
	int ret = 0;
	int en;

	scharger_inf("%s enable:%d\n", __func__, enable);

	if (g_scharger_dc == NULL)
		return -1;

	en = !!enable;
	ret = sc_charge_enable(en, g_scharger_dc);

	return ret;
}

static int sc_charge_wired_is_close(struct scharger_dc *dc)
{
	int bat_mode;
	u8 psw_state = 0;
	int ret = 0;

	bat_mode = bat_cnct_mode_get();
	ret = check_ovp2_state(&psw_state);
	if (ret) {
		scharger_err("%s: check ovp2 state fail\n", __func__);
		return -1;
	}

	if (!dc->sc_en_flag || ((bat_mode == BAT_SERIES_MODE) && (psw_state == PSW_ENABLE)))
		return 0;

	return 1;
}

static int sc_charge_is_close(void *dev_data)
{
	struct scharger_dc *dc = dev_data;

	if (dc == NULL) {
		scharger_err("%s scharger_dc is NULL!\n", __func__);
		return -EINVAL;
	}

	if (charge_get_charger_type() == CHARGER_TYPE_WIRELESS)
		return dc_charge_wireless_is_close(dc);

	return sc_charge_wired_is_close(dc);
}

static struct dc_ic_ops lvc_ops = {
	.dev_name = "hi6526",
	.ic_init = lvc_charge_init,
	.ic_exit = dc_charge_exit,
	.ic_enable = lvc_charge_enable,
	.ic_discharge = dc_discharge,
	.is_ic_close = lvc_charge_is_close,
	.get_ic_id = get_loadswitch_id,
	.config_ic_watchdog = dc_config_watchdog_ms,
	.kick_ic_watchdog = dc_kick_watchdog_timer,
};

static struct dc_ic_ops sc_ops = {
	.dev_name = "hi6526",
	.ic_init = sc_charge_init,
	.ic_exit = dc_charge_exit,
	.ic_enable = sc_charge_enable,
	.ic_discharge = dc_discharge,
	.is_ic_close = sc_charge_is_close,
	.get_ic_id = get_sc_device_id,
	.config_ic_watchdog = dc_config_watchdog_ms,
	.kick_ic_watchdog = dc_kick_watchdog_timer,
};

static int dc_batinfo_init(void *dev_data)
{
	return 0;
}

static int dc_batinfo_exit(void *dev_data)
{
	return 0;
}

static struct dc_batinfo_ops batinfo_ops = {
	.init = dc_batinfo_init,
	.exit = dc_batinfo_exit,
	.get_bat_btb_voltage = dc_get_vbat_mv,
	.get_bat_package_voltage = dc_get_vbat_mv,
	.get_vbus_voltage = dc_get_vbus_mv,
	.get_bat_current = dc_get_ibat_ma,
	.get_ic_ibus = dc_get_ibus_ma,
	.get_ic_temp = dc_get_chip_temp,
	.get_ic_vusb = dc_get_vusb_mv,
};

static int is_support_scp(struct scharger_dc *dc)
{
	if (!dc || !dc->scp_support)
		return -1;

	return 0;
}

static void scp_ops_register(struct scharger_dc *dc)
{
	int i;
	int ret[REGISTER_RET_SIZE] = { 0 };

	/* if chip support scp,register scp adapter ops */

	lvc_ops.dev_data = (void *)dc;
	sc_ops.dev_data = (void *)dc;
	batinfo_ops.dev_data = (void *)dc;

	ret[0] = 0;
	ret[1] = dc_ic_ops_register(LVC_MODE, IC_ONE,
		&lvc_ops);
	ret[2] = dc_ic_ops_register(SC_MODE, IC_ONE,
		&sc_ops);
	ret[3] = dc_batinfo_ops_register(IC_ONE,
		&batinfo_ops, SWITCHCAP_SCHARGERV700);

	for (i = 0; i < REGISTER_RET_SIZE; i++) {
		if (ret[i])
			dev_err(dc->dev, "register scp adapter ops failed i %d!\n", i);
	}
}

static int dc_init_wldc_en_gpio(struct device_node *np, struct scharger_dc *dc)
{
	const char *prop = "gpio_wldc_en";

	if (np == NULL || dc == NULL) {
		scharger_err("%s input is NULL\n", __func__);
		return -ENODEV;
	}

	dc->gpio_wldc_en = of_get_named_gpio(np, prop, 0);
	scharger_inf("%s=%d\n", prop, dc->gpio_wldc_en);
	if (dc->gpio_wldc_en <= 0)
		return 0;

	if (gpio_request(dc->gpio_wldc_en, prop)) {
		scharger_err("gpio %d request fail\n", dc->gpio_wldc_en);
		goto err;
	}

	dc->gpio_wldc_en_valid_val = 1;
	if (gpio_direction_output(dc->gpio_wldc_en, !dc->gpio_wldc_en_valid_val)) {
		gpio_free(dc->gpio_wldc_en);
		scharger_err("gpio %d set output fail\n", dc->gpio_wldc_en);
		goto err;
	}

	return 0;

err:
	dc->gpio_wldc_en = -1;
	return -EINVAL;
}

static void get_battery_name(struct scharger_dc *dc)
{
	int ret = 0;
	struct device_node *coul_np = NULL;
	struct device_node *batt_l_np = NULL;
	struct device_node *batt_h_np = NULL;
	const char *batt_l_name = NULL;
	const char *batt_h_name = NULL;

	if (dc == NULL) {
		scharger_err("%s scharger_dc is NULL!\n", __func__);
		return;
	}
	coul_np = of_find_compatible_node(NULL, NULL, "hisilicon,schargerV700_coul");
	if (coul_np == NULL) {
		scharger_err("%s hisilicon,schargerV700_coul unfind\n", __func__);
		return;
	}
	batt_l_np = of_find_node_by_name(coul_np, "batt_l");
	if (batt_l_np == NULL) {
		scharger_err("%s batt_l not define in dts tree!", __func__);
		return;
	}
	batt_h_np = of_find_node_by_name(coul_np, "batt_h");
	if (batt_h_np == NULL) {
		scharger_err("%s batt_h not define in dts tree!", __func__);
		return;
	}

	ret = of_property_read_string(batt_l_np, "batt_name", &batt_l_name);
	if (ret) {
		(void)strcpy_s(dc->batt_l_name, BATT_NAME_SIZE_MAX, "battery_gauge_aux");
		scharger_err("%s get batt_l_name fail, set default value %s\n", __func__, "battery_gauge_aux");
	} else {
		(void)strcpy_s(dc->batt_l_name, BATT_NAME_SIZE_MAX, batt_l_name);
	}
	ret = of_property_read_string(batt_h_np, "batt_name", &batt_h_name);
	if (ret) {
		(void)strcpy_s(dc->batt_h_name, BATT_NAME_SIZE_MAX, "battery_gauge");
		scharger_err("%s get batt_h_name fail, set default value %s\n", __func__, "battery_gauge");
	} else {
		(void)strcpy_s(dc->batt_h_name, BATT_NAME_SIZE_MAX, batt_h_name);
	}
}

static int dc_event_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct scharger_dc *dc = g_scharger_dc;

	if (!dc)
		return NOTIFY_OK;

	switch (event) {
	case POWER_NE_USB_DISCONNECT:
		dc->lvc_init_flag = false;
		scharger_inf("usb disconnect, set init flag false\n");
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

static int scharger_dc_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;
	struct scharger_dc *dc = NULL;

	pr_err("[%s]+\n", __func__);

	dc = devm_kzalloc(dev, sizeof(*dc), GFP_KERNEL);
	if (!dc) {
		dev_err(dev, "[%s]dc is null.\n", __func__);
		return -ENOMEM;
	}

	dc->dev = dev;
	dc->regmap = dev_get_regmap(dev->parent, NULL);
	if (!dc->regmap) {
		dev_err(dev, "Parent regmap unavailable.\n");
		return -ENXIO;
	}

	get_battery_name(dc);
	INIT_DELAYED_WORK(&dc->dc_vdrop_ovp_work, scharger_dc_vdrop_ovp_delay_work);

	ret = dc_init_wldc_en_gpio(dev->of_node, dc);
	if (ret) {
		devm_kfree(&pdev->dev, dc);
		return ret;
	}

	dc->nb.notifier_call = dc_event_notifier_call;
	(void)power_event_bnc_register(POWER_BNT_CONNECT, &dc->nb);
	scp_ops_register(dc);
	dc->scharger_version = scharger_get_device_version();
	dc->dc_chg_mode = NONE;
	g_scharger_dc = dc;

	return 0;
}

static int scharger_dc_remove(struct platform_device *pdev)
{
	struct scharger_dc *dc = platform_get_drvdata(pdev);

	if (!dc)
		return -ENODEV;

	power_event_bnc_unregister(POWER_BNT_CONNECT, &dc->nb);
	g_scharger_dc = NULL;
	return 0;
}

const static struct of_device_id of_scharger_dc_match_tbl[] = {
	{
		.compatible = "hisilicon,scharger-v700-dc",
		.data = NULL,
	},
	{},
};

static struct platform_driver scharger_dc_driver = {
	.driver = {
		.name	= "scharger_v700_dc",
		.owner  = THIS_MODULE,
		.of_match_table = of_scharger_dc_match_tbl,
	},
	.probe	= scharger_dc_probe,
	.remove	= scharger_dc_remove,
};

static int __init scharger_dc_init(void)
{
	return platform_driver_register(&scharger_dc_driver);
}

static void __exit scharger_dc_exit(void)
{
	platform_driver_unregister(&scharger_dc_driver);
}

fs_initcall(scharger_dc_init);
module_exit(scharger_dc_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("schargerV700 direct charge driver");
