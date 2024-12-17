// SPDX-License-Identifier: GPL-2.0
/*
 * wireless_rx_sysfs.c
 *
 * authenticate for wireless rx charge
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

#include <chipset_common/hwpower/wireless_charge/wireless_rx_sysfs.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_ic_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_common.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_pctrl.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_status.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_acc.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_ui.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_fod.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_plim.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_interfere.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_pmode.h>
#include <chipset_common/hwpower/wireless_charge/wireless_power_supply.h>
#include <chipset_common/hwpower/protocol/wireless_protocol.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_interface.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <linux/kernel.h>
#include <securec.h>

#define HWLOG_TAG wireless_rx_sysfs
HWLOG_REGIST();

#define WLRX_SYSFS_U8_MAX      0xFF
#define SYSFS_CHECK_DELAY      (1 * 60 * 1000) /* 1min */

enum wlrx_sysfs_type {
	WLRX_SYSFS_CHIP_INFO = 0,
	WLRX_SYSFS_TX_ADAPTOR_TYPE,
	WLRX_SYSFS_RX_TEMP,
	WLRX_SYSFS_VOUT,
	WLRX_SYSFS_IOUT,
	WLRX_SYSFS_VRECT,
	WLRX_SYSFS_EN_ENABLE,
	WLRX_SYSFS_NORMAL_CHRG_SUCC,
	WLRX_SYSFS_FAST_CHRG_SUCC,
	WLRX_SYSFS_FOD_COEF,
	WLRX_SYSFS_INTERFERENCE_SETTING,
	WLRX_SYSFS_RX_SUPPORT_MODE,
	WLRX_SYSFS_THERMAL_CTRL,
	WLRX_SYSFS_IGNORE_FAN_CTRL,
	WLRX_SYSFS_CM_CTRL,
	WLRX_SYSFS_QVAL_CALI,
};

struct wlrx_sysfs_dev {
	struct device *dev;
	struct delayed_work check_work;
	unsigned int drv_type;
	bool en_enable;
	bool ignore_fan_ctrl;
	u8 support_mode;
	u8 thermal_ctrl;
};

static struct wlrx_sysfs_dev *g_rx_sysfs_di[WLTRX_DRV_MAX];
void wlrx_sysfs_deinit(unsigned int drv_type);

static struct wlrx_sysfs_dev *wlrx_sysfs_get_di(unsigned int drv_type)
{
	if (!wltrx_is_drv_type_valid(drv_type)) {
		hwlog_err("get_di: drv_type=%u err\n", drv_type);
		return NULL;
	}

	return g_rx_sysfs_di[drv_type];
}

static unsigned int wlrx_sysfs_get_ic_type(unsigned int drv_type)
{
	return wltrx_get_dflt_ic_type(drv_type);
}

#ifdef CONFIG_SYSFS
static ssize_t wlrx_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t wlrx_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);
static ssize_t wlrx_sysfs_info(unsigned int info_name, unsigned int ic_type,
	struct wlrx_sysfs_dev *di, char *buf);

static struct power_sysfs_attr_info wlrx_sysfs_field_tbl[] = {
	power_sysfs_attr_ro(wlrx, 0444, WLRX_SYSFS_CHIP_INFO, chip_info),
	power_sysfs_attr_ro(wlrx, 0444,
		WLRX_SYSFS_TX_ADAPTOR_TYPE, tx_adaptor_type),
	power_sysfs_attr_rw(wlrx, 0644, WLRX_SYSFS_RX_TEMP, rx_temp),
	power_sysfs_attr_rw(wlrx, 0644, WLRX_SYSFS_VOUT, vout),
	power_sysfs_attr_rw(wlrx, 0644, WLRX_SYSFS_IOUT, iout),
	power_sysfs_attr_rw(wlrx, 0644, WLRX_SYSFS_VRECT, vrect),
	power_sysfs_attr_rw(wlrx, 0644, WLRX_SYSFS_EN_ENABLE, en_enable),
	power_sysfs_attr_ro(wlrx, 0444,
		WLRX_SYSFS_NORMAL_CHRG_SUCC, normal_chrg_succ),
	power_sysfs_attr_ro(wlrx, 0444,
		WLRX_SYSFS_FAST_CHRG_SUCC, fast_chrg_succ),
	power_sysfs_attr_rw(wlrx, 0644, WLRX_SYSFS_FOD_COEF, fod_coef),
	power_sysfs_attr_rw(wlrx, 0644,
		WLRX_SYSFS_INTERFERENCE_SETTING, interference_setting),
	power_sysfs_attr_rw(wlrx, 0644,
		WLRX_SYSFS_RX_SUPPORT_MODE, rx_support_mode),
	power_sysfs_attr_rw(wlrx, 0644,
		WLRX_SYSFS_THERMAL_CTRL, thermal_ctrl),
	power_sysfs_attr_rw(wlrx, 0644,
		WLRX_SYSFS_IGNORE_FAN_CTRL, ignore_fan_ctrl),
	power_sysfs_attr_rw(wlrx, 0644,
		WLRX_SYSFS_CM_CTRL, cm_ctrl),
	power_sysfs_attr_rw(wlrx, 0644, WLRX_SYSFS_QVAL_CALI, qval),
};

static struct attribute *wlrx_sysfs_attrs[ARRAY_SIZE(wlrx_sysfs_field_tbl) + 1];
static const struct attribute_group wlrx_sysfs_attr_group = {
	.attrs = wlrx_sysfs_attrs,
};

static void wlrx_sysfs_create_group(struct device *dev)
{
	power_sysfs_init_attrs(wlrx_sysfs_attrs, wlrx_sysfs_field_tbl,
		ARRAY_SIZE(wlrx_sysfs_field_tbl));
	power_sysfs_create_link_group("hw_power", "charger", "wireless_charger",
		dev, &wlrx_sysfs_attr_group);
}

static void wlrx_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_link_group("hw_power", "charger", "wireless_charger",
		dev, &wlrx_sysfs_attr_group);
}
#else
static inline void wlrx_sysfs_create_group(struct device *dev)
{
}

static inline void wlrx_sysfs_remove_group(struct device *dev)
{
}
#endif

void wlrx_sysfs_charge_para_init(unsigned int drv_type)
{
	struct wlrx_sysfs_dev *di = wlrx_sysfs_get_di(drv_type);

	if (!di)
		return;

	di->en_enable = 0;
}

u8 wlrx_sysfs_get_support_mode(unsigned int drv_type)
{
	struct wlrx_sysfs_dev *di = wlrx_sysfs_get_di(drv_type);

	if (!di)
		return 0;

	return di->support_mode;
}

u8 wlrx_sysfs_get_thermal_ctrl(unsigned int drv_type)
{
	struct wlrx_sysfs_dev *di = wlrx_sysfs_get_di(drv_type);

	if (!di)
		return 0;

	return di->thermal_ctrl;
}

bool wlrx_sysfs_ignore_fan_ctrl(unsigned int drv_type)
{
	struct wlrx_sysfs_dev *di = wlrx_sysfs_get_di(drv_type);

	if (!di)
		return false;

	return di->ignore_fan_ctrl;
}

static ssize_t wlrx_syfs_show_adp_type(struct wlrx_sysfs_dev *di, char *buf)
{
	struct wlprot_acc_cap *acc_cap = wlrx_acc_get_cap(di->drv_type);

	if (!acc_cap)
		return -EINVAL;

	return snprintf(buf, PAGE_SIZE, "%u\n", acc_cap->adp_type);
}

static ssize_t wlrx_syfs_show_rx_temp(struct wlrx_sysfs_dev *di, char *buf)
{
	int temp = 0;

	(void)wlrx_ic_get_temp(wlrx_sysfs_get_ic_type(di->drv_type), &temp);
	return snprintf(buf, PAGE_SIZE, "%d\n", temp);
}

static ssize_t wlrx_syfs_show_rx_vout(struct wlrx_sysfs_dev *di, char *buf)
{
	int vout = 0;

	(void)wlrx_ic_get_vout(wlrx_sysfs_get_ic_type(di->drv_type), &vout);
	return snprintf(buf, PAGE_SIZE, "%d\n", vout);
}

static ssize_t wlrx_syfs_show_rx_iout(struct wlrx_sysfs_dev *di, char *buf)
{
	int iout = 0;

	(void)wlrx_ic_get_iout(wlrx_sysfs_get_ic_type(di->drv_type), &iout);
	return snprintf(buf, PAGE_SIZE, "%d\n", iout);
}

static ssize_t wlrx_syfs_show_rx_vrect(struct wlrx_sysfs_dev *di, char *buf)
{
	int vrect = 0;

	(void)wlrx_ic_get_vrect(wlrx_sysfs_get_ic_type(di->drv_type), &vrect);
	return snprintf(buf, PAGE_SIZE, "%d\n", vrect);
}

static ssize_t wlrx_syfs_show_normal_charge_succ(struct wlrx_sysfs_dev *di, char *buf)
{
	enum wlrx_ui_icon_type type;
	int chrg_succ = 1; /* 0: succ 1: fail */

	type = wlrx_ui_get_icon_type(di->drv_type);
	if (!wlrx_is_err_tx(di->drv_type) && (type == WLRX_UI_NORMAL_CHARGE) &&
		(wlrx_get_charge_stage() >= WLRX_STAGE_CHARGING))
		chrg_succ = 0;

	return snprintf(buf, PAGE_SIZE, "%d\n", chrg_succ);
}

static ssize_t wlrx_syfs_show_fast_charge_succ(struct wlrx_sysfs_dev *di, char *buf)
{
	enum wlrx_ui_icon_type type;
	int chrg_succ = 1; /* 0: succ 1: fail */

	type = wlrx_ui_get_icon_type(di->drv_type);
	if (((type == WLRX_UI_FAST_CHARGE) || (type == WLRX_UI_SUPER_CHARGE)) &&
		(wlrx_get_charge_stage() >= WLRX_STAGE_CHARGING))
		chrg_succ = 0;

	return snprintf(buf, PAGE_SIZE, "%d\n", chrg_succ);
}

static ssize_t wlrx_syfs_show_fod_qval(struct wlrx_sysfs_dev *di, char *buf)
{
	int ret;
	struct power_nv_fod_qval qval = { 0 };

	(void)wlrx_fod_get_cali_para(di->drv_type, &qval);

	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%u,%u,%u,%u\n",
		qval.km, qval.qr, qval.fr, qval.kz);
}

static ssize_t wlrx_syfs_store_en_enable(struct wlrx_sysfs_dev *di,
	const char *buf, size_t count)
{
	long val = 0;
	unsigned int ic_type;

	if (!buf || (kstrtol(buf, POWER_BASE_DEC, &val) < 0) || (val < 0) || (val > 1))
		return -EINVAL;

	di->en_enable = val;
	hwlog_info("[store_en_enable] en=%d\n", di->en_enable);
	ic_type = wlrx_sysfs_get_ic_type(di->drv_type);
	wlrx_ic_sleep_enable(ic_type, val);
	wlps_control(ic_type, WLPS_SYSFS_EN_PWR, di->en_enable ? true : false);

	return count;
}

static ssize_t wlrx_syfs_store_fod_coef(struct wlrx_sysfs_dev *di,
	const char *buf, size_t count)
{
	if (!buf)
		return -EINVAL;

	hwlog_info("[store_fod_coef] fod_coef: %s\n", buf);
	(void)wlrx_ic_set_fod_coef(wlrx_sysfs_get_ic_type(di->drv_type), buf);

	return count;
}

static ssize_t wlrx_syfs_store_interference_setting(struct wlrx_sysfs_dev *di,
	const char *buf, size_t count)
{
	long val = 0;

	if (!buf || (kstrtol(buf, POWER_BASE_DEC, &val) < 0) ||
		(val < 0) || (val > WLRX_SYSFS_U8_MAX))
		return -EINVAL;

	hwlog_info("[store_interference_setting] settings=0x%x\n", val);
	wlrx_intfr_handle_settings(di->drv_type, (u8)val);

	return count;
}

static ssize_t wlrx_syfs_store_rx_support_mode(struct wlrx_sysfs_dev *di,
	const char *buf, size_t count)
{
	long val = 0;

	if (!buf || (kstrtol(buf, POWER_BASE_HEX, &val) < 0) ||
		(val < 0) || (val > WLRX_SUPP_PMODE_ALL))
		return -EINVAL;

	if (!val)
		di->support_mode = WLRX_SUPP_PMODE_ALL;
	else
		di->support_mode = val;
	hwlog_info("[store_rx_support_mode] mode=0x%x\n", di->support_mode);

	return count;
}

static void wlrx_syfs_set_thermal_ctrl(struct wlrx_sysfs_dev *di, unsigned char val)
{
	u8 thermal_status;

	di->thermal_ctrl = val;
	hwlog_info("[store_thermal_ctrl] 0x%x", di->thermal_ctrl);
	thermal_status = di->thermal_ctrl & WLRX_SYSFS_THERMAL_EXIT_SC_MODE;
	if ((thermal_status == WLRX_SYSFS_THERMAL_EXIT_SC_MODE) &&
		!wlrx_in_high_pwr_test(di->drv_type))
		wlrx_plim_set_src(di->drv_type, WLRX_PLIM_SRC_THERMAL);
	else
		wlrx_plim_clear_src(di->drv_type, WLRX_PLIM_SRC_THERMAL);
}

static ssize_t wlrx_syfs_store_thermal_ctrl(struct wlrx_sysfs_dev *di,
	const char *buf, size_t count)
{
	long val = 0;

	if (!buf || (kstrtol(buf, POWER_BASE_DEC, &val) < 0) ||
		(val < 0) || (val > WLRX_SYSFS_U8_MAX))
		return -EINVAL;

	wlrx_syfs_set_thermal_ctrl(di, (unsigned char)val);

	return count;
}

static ssize_t wlrx_syfs_store_ingnore_fan_ctrl(struct wlrx_sysfs_dev *di,
	const char *buf, size_t count)
{
	long val = 0;

	if (!buf || (kstrtol(buf, POWER_BASE_DEC, &val) < 0) ||
		(val < 0) || (val > 1)) /* 1: ignore 0: otherwise */
		return -EINVAL;

	hwlog_info("[store_ingnore_fan_ctrl] flag=0x%x", val);
	di->ignore_fan_ctrl = val;

	return count;
}

static ssize_t wlrx_syfs_store_cm_ctrl(struct wlrx_sysfs_dev *di,
	const char *buf, size_t count)
{
	int type = 0;
	int val = 0;

	if (!buf || (sscanf(buf, "%d %d", &type, &val) != 2)) /* 2: type val */
		return -EINVAL;

	if ((type < 0) || (type > WLRX_SYSFS_U8_MAX) || (val < 0) || (val > 1)) {
		hwlog_err("store_cm_ctrl: unkown para, type=0x%d val=0x%d\n", type, val);
		return -EINVAL;
	}

	hwlog_info("[store_cm_ctrl] type=0x%d, val=0x%d\n", type, val);
	wlrx_pctrl_handle_cm_ctrl(di->drv_type, (u8)type, (u8)val);

	return count;
}

static ssize_t wlrx_syfs_store_qval(struct wlrx_sysfs_dev *di,
	const char *buf, size_t count)
{
	int ret;
	struct power_nv_fod_qval qval = { 0 };

	/* 4: km qr fr kz */
	if (!buf || (sscanf_s(buf, "%u,%u,%u,%u",
		&qval.km, &qval.qr, &qval.fr, &qval.kz) != 4))
		return -EINVAL;

	hwlog_info("[store_qval] km=%u qr=%u fr=%u kz=%u\n",
		qval.km, qval.qr, qval.fr, qval.kz);
	ret = wlrx_fod_set_cali_para(di->drv_type, qval);
	if (ret)
		return ret;

	return count;
}

static ssize_t wlrx_sysfs_info(unsigned int info_name, unsigned int ic_type,
	struct wlrx_sysfs_dev *di, char *buf)
{
	switch (info_name) {
	case WLRX_SYSFS_CHIP_INFO:
		return wlrx_ic_get_chip_info(ic_type, buf, PAGE_SIZE);
	case WLRX_SYSFS_TX_ADAPTOR_TYPE:
		return wlrx_syfs_show_adp_type(di, buf);
	case WLRX_SYSFS_RX_TEMP:
		return wlrx_syfs_show_rx_temp(di, buf);
	case WLRX_SYSFS_VOUT:
		return wlrx_syfs_show_rx_vout(di, buf);
	case WLRX_SYSFS_IOUT:
		return wlrx_syfs_show_rx_iout(di, buf);
	case WLRX_SYSFS_VRECT:
		return wlrx_syfs_show_rx_vrect(di, buf);
	case WLRX_SYSFS_EN_ENABLE:
		return snprintf(buf, PAGE_SIZE, "%d\n", di->en_enable);
	case WLRX_SYSFS_NORMAL_CHRG_SUCC:
		return wlrx_syfs_show_normal_charge_succ(di, buf);
	case WLRX_SYSFS_FAST_CHRG_SUCC:
		return wlrx_syfs_show_fast_charge_succ(di, buf);
	case WLRX_SYSFS_FOD_COEF:
		return wlrx_ic_get_fod_coef(ic_type, buf, PAGE_SIZE);
	case WLRX_SYSFS_INTERFERENCE_SETTING:
		return snprintf(buf, PAGE_SIZE, "%u\n",
			wlrx_intfr_get_src(di->drv_type));
	case WLRX_SYSFS_RX_SUPPORT_MODE:
		return snprintf(buf, PAGE_SIZE, "0x%x\n", di->support_mode);
	case WLRX_SYSFS_THERMAL_CTRL:
		return snprintf(buf, PAGE_SIZE, "%u\n", di->thermal_ctrl);
	case WLRX_SYSFS_IGNORE_FAN_CTRL:
		return snprintf(buf, PAGE_SIZE, "%d\n", di->ignore_fan_ctrl);
	case WLRX_SYSFS_CM_CTRL:
		return snprintf(buf, PAGE_SIZE, "%u\n",
			wlrx_pctrl_get_cm_ctrl_src(di->drv_type));
	case WLRX_SYSFS_QVAL_CALI:
		return wlrx_syfs_show_fod_qval(di, buf);
	default:
		break;
	}

	return 0;
}

static ssize_t wlrx_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	unsigned int ic_type;
	struct wlrx_sysfs_dev *di = wlrx_sysfs_get_di(WLTRX_DRV_MAIN);
	struct power_sysfs_attr_info *info = power_sysfs_lookup_attr(
		attr->attr.name, wlrx_sysfs_field_tbl,
		ARRAY_SIZE(wlrx_sysfs_field_tbl));

	if (!di || !info) {
		hwlog_err("di null\n");
		return -ENODEV;
	}

	ic_type = wlrx_sysfs_get_ic_type(di->drv_type);
	if (!wlrx_ic_is_ops_registered(ic_type)) {
		hwlog_err("sysfs_show: ic not registered\n");
		return -ENODEV;
	}

	return wlrx_sysfs_info(info->name, ic_type, di, buf);
}

static ssize_t wlrx_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret = count;
	unsigned int ic_type;
	struct wlrx_sysfs_dev *di = wlrx_sysfs_get_di(WLTRX_DRV_MAIN);
	struct power_sysfs_attr_info *info = power_sysfs_lookup_attr(
		attr->attr.name, wlrx_sysfs_field_tbl,
		ARRAY_SIZE(wlrx_sysfs_field_tbl));

	if (!di || !info)
		return -ENODEV;

	ic_type = wlrx_sysfs_get_ic_type(di->drv_type);
	if (!wlrx_ic_is_ops_registered(ic_type)) {
		hwlog_err("sysfs_store: ic not registered\n");
		return -ENODEV;
	}

	switch (info->name) {
	case WLRX_SYSFS_EN_ENABLE:
		ret = wlrx_syfs_store_en_enable(di, buf, count);
		break;
	case WLRX_SYSFS_FOD_COEF:
		ret = wlrx_syfs_store_fod_coef(di, buf, count);
		break;
	case WLRX_SYSFS_INTERFERENCE_SETTING:
		ret = wlrx_syfs_store_interference_setting(di, buf, count);
		break;
	case WLRX_SYSFS_RX_SUPPORT_MODE:
		ret = wlrx_syfs_store_rx_support_mode(di, buf, count);
		break;
	case WLRX_SYSFS_THERMAL_CTRL:
		ret = wlrx_syfs_store_thermal_ctrl(di, buf, count);
		break;
	case WLRX_SYSFS_IGNORE_FAN_CTRL:
		ret = wlrx_syfs_store_ingnore_fan_ctrl(di, buf, count);
		break;
	case WLRX_SYSFS_CM_CTRL:
		ret = wlrx_syfs_store_cm_ctrl(di, buf, count);
		break;
	case WLRX_SYSFS_QVAL_CALI:
		ret = wlrx_syfs_store_qval(di, buf, count);
		break;
	default:
		break;
	}

	return ret;
}

static int wlrx_itf_set_thermal_ctrl(unsigned char val)
{
	struct wlrx_sysfs_dev *di = wlrx_sysfs_get_di(WLTRX_DRV_MAIN);

	if (!di || (val > WLRX_SYSFS_U8_MAX))
		return -EINVAL;

	wlrx_syfs_set_thermal_ctrl(di, val);
	return 0;
}

static int wlrx_itf_get_thermal_ctrl(unsigned char *val)
{
	if (!val)
		return -EINVAL;

	*val = wlrx_sysfs_get_thermal_ctrl(WLTRX_DRV_MAIN);
	return 0;
}

static struct power_if_ops wlrx_itf_ops = {
	.set_wl_thermal_ctrl = wlrx_itf_set_thermal_ctrl,
	.get_wl_thermal_ctrl = wlrx_itf_get_thermal_ctrl,
	.type_name = "wl",
};

static void wlrx_sysfs_kfree_dev(struct wlrx_sysfs_dev *di)
{
	if (!di)
		return;

	cancel_delayed_work(&di->check_work);
	wlrx_sysfs_remove_group(di->dev);
	kfree(di);
}

static void wlrx_sysfs_check_work(struct work_struct *work)
{
	struct wlrx_sysfs_dev *di = wlrx_sysfs_get_di(WLTRX_DRV_MAIN);

	if (!di)
		return;

	if (!wlrx_ic_is_ops_registered(wlrx_sysfs_get_ic_type(di->drv_type))) {
		hwlog_err("sysfs_check_work: ic not registered, remove sysfs\n");
		wlrx_sysfs_deinit(di->drv_type);
	}
}

int wlrx_sysfs_init(unsigned int drv_type, struct device *dev)
{
	struct wlrx_sysfs_dev *di = NULL;

	if (!dev || !wltrx_is_drv_type_valid(drv_type))
		return -ENODEV;

	if (g_rx_sysfs_di[drv_type])
		return 0;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = dev;
	di->drv_type = drv_type;
	di->support_mode = WLRX_SUPP_PMODE_ALL;
	if (power_cmdline_is_factory_mode())
		di->support_mode &= ~WLRX_SUPP_PMODE_SC2;

	wlrx_sysfs_create_group(dev);
	INIT_DELAYED_WORK(&di->check_work, wlrx_sysfs_check_work);
	schedule_delayed_work(&di->check_work, msecs_to_jiffies(SYSFS_CHECK_DELAY));
	power_if_ops_register(&wlrx_itf_ops);
	g_rx_sysfs_di[drv_type] = di;
	return 0;
}

void wlrx_sysfs_deinit(unsigned int drv_type)
{
	if (!wltrx_is_drv_type_valid(drv_type))
		return;

	wlrx_sysfs_kfree_dev(g_rx_sysfs_di[drv_type]);
	g_rx_sysfs_di[drv_type] = NULL;
}
