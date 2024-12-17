// SPDX-License-Identifier: GPL-2.0
/*
 * buck_charge_sysfs.c
 *
 * buck charge sysfs driver
 *
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
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

#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/sysfs.h>
#include <chipset_common/hwpower/buck_charge/buck_charge_ic_manager.h>
#include <chipset_common/hwpower/buck_charge/buck_charge_platform_interface.h>
#include <chipset_common/hwpower/buck_charge/buck_charge_sysfs.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_gpio_misc.h>
#include <chipset_common/hwpower/common_module/power_interface.h>
#include <chipset_common/hwpower/common_module/power_log.h>
#include <chipset_common/hwpower/common_module/power_nv.h>
#include <chipset_common/hwpower/common_module/power_supply_interface.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/hvdcp_charge/hvdcp_charge.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_plim.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_pmode.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <huawei_platform/power/huawei_charger.h>
#include <huawei_platform/usb/hw_pd_dev.h>

#define HWLOG_TAG buck_charge_sysfs
HWLOG_REGIST();

static struct charge_sysfs_dev *g_charge_sysfs_di;

static size_t g_charge_sysfs_offset_tbl[] = {
	[CHARGE_SYSFS_IIN_THERMAL] = { offsetof(struct charge_sysfs_dev, iin_thl) },
	[CHARGE_SYSFS_IIN_RUNNINGTEST] = { offsetof(struct charge_sysfs_dev, iin_rt) },
	[CHARGE_SYSFS_WATCHDOG_DISABLE] = { offsetof(struct charge_sysfs_dev, wdt_disable) },
	[CHARGE_SYSFS_CHARGE_DONE_STATUS] = { offsetof(struct charge_sysfs_dev, charge_done_status) },
	[CHARGE_SYSFS_ENABLE_CHARGER] = { offsetof(struct charge_sysfs_dev, charge_enable) },
	[CHARGE_SYSFS_DBC_CHARGE_CONTROL] = { offsetof(struct charge_sysfs_dev, dbc_charge_control) },
	[CHARGE_SYSFS_REGULATION_VOLTAGE] = { offsetof(struct charge_sysfs_dev, vterm_rt) },
	[CHARGE_SYSFS_ADC_CONV_RATE] = { offsetof(struct charge_sysfs_dev, adc_conv_rate) },
	[CHARGE_SYSFS_ICHG_THERMAL] = { offsetof(struct charge_sysfs_dev, ichg_thl) },
	[CHARGE_SYSFS_ICHG_RUNNINGTEST] = { offsetof(struct charge_sysfs_dev, ichg_rt) },
	[CHARGE_SYSFS_LIMIT_CHARGING] = { offsetof(struct charge_sysfs_dev, charge_limit) },
	[CHARGE_SYSFS_BATFET_DISABLE] = { offsetof(struct charge_sysfs_dev, batfet_disable) },
	[CHARGE_SYSFS_CHARGE_DONE_SLEEP_STATUS] = { offsetof(struct charge_sysfs_dev, charge_done_sleep_status) },
	[CHARGE_SYSFS_INPUTCURRENT] = { offsetof(struct charge_sysfs_dev, inputcurrent) },
	[CHARGE_SYSFS_VR_CHARGER_TYPE] = { offsetof(struct charge_sysfs_dev, vr_charger_type) },
	[CHARGE_SYSFS_FCP_CHARGE_ENABLE] = { offsetof(struct charge_sysfs_dev, fcp_charge_enable) },
};

static int charge_sysfs_get_value(struct charge_sysfs_dev *di, size_t offset)
{
	return *(int *)((void *)di + offset);
}

static void charge_sysfs_set_value(struct charge_sysfs_dev *di, size_t offset, int value)
{
	*(int *)((void *)di + offset) = value;
}

int charge_sysfs_get_sysfs_value(unsigned int info_name)
{
	struct charge_sysfs_dev *di = g_charge_sysfs_di;

	if (!di) {
		hwlog_err("get_sysfs_value di is null\n");
		return -EPERM;
	}

	if ((info_name < CHARGE_SYSFS_IIN_THERMAL) || (info_name > CHARGE_SYSFS_FCP_CHARGE_ENABLE))
		return -EINVAL;

	return charge_sysfs_get_value(di, g_charge_sysfs_offset_tbl[info_name]);
}

void charge_sysfs_set_sysfs_value(unsigned int info_name, unsigned int value)
{
	struct charge_sysfs_dev *di = g_charge_sysfs_di;

	if (!di) {
		hwlog_err("set_sysfs_value di is null\n");
		return;
	}

	if ((info_name < CHARGE_SYSFS_IIN_THERMAL) || (info_name > CHARGE_SYSFS_FCP_CHARGE_ENABLE))
		return;

	charge_sysfs_set_value(di, g_charge_sysfs_offset_tbl[info_name], value);
}

static int sdp_set_iin_limit(unsigned int val)
{
	struct charge_sysfs_dev *di = g_charge_sysfs_di;

	if (!di) {
		hwlog_err("sdp_set_iin_limit di is null\n");
		return -EPERM;
	}

	buck_charge_set_sdp_iin_limit(val);
	return 0;
}

static int dcp_set_enable_charger(unsigned int val)
{
	int ret;
	struct charge_sysfs_dev *di = g_charge_sysfs_di;

	if (!di) {
		hwlog_err("dcp_set_enable_charger di is null\n");
		return -EPERM;
	}

	di->chrg_config = val;
	ret = buck_charge_set_charge_en_flag(val, &di->charge_limit);
	if (ret)
		return ret;

	return buck_charge_set_charge_en(val, di->charge_enable);
}

static int dcp_get_enable_charger(unsigned int *val)
{
	struct charge_sysfs_dev *di = g_charge_sysfs_di;

	if (!di) {
		hwlog_err("dcp_get_enable_charger di is null\n");
		return -EPERM;
	}

	*val = di->charge_enable;
	return 0;
}

static int fcp_set_enable_charger(unsigned int val)
{
	struct charge_sysfs_dev *di = g_charge_sysfs_di;

	if (!di) {
		hwlog_err("fcp_set_enable_charger di is null\n");
		return -EPERM;
	}

	if ((val < 0) || (val > 1))
		return -EINVAL;

	di->fcp_charge_enable = val;
	hwlog_info("fcp runningtest set charge enable=%d\n", val);
	return 0;
}

static int fcp_get_enable_charger(unsigned int *val)
{
	struct charge_sysfs_dev *di = g_charge_sysfs_di;

	if (!di || !val) {
		hwlog_err("fcp_get_enable_charger di or val is null\n");
		return -EINVAL;
	}

	*val = di->fcp_charge_enable;
	return 0;
}

static int dcp_set_iin_limit(unsigned int val)
{
	struct charge_sysfs_dev *di = g_charge_sysfs_di;

	if (!di) {
		hwlog_err("dcp_set_iin_limit di is null\n");
		return -EPERM;
	}

	return buck_charge_set_iin_limit(val, &di->inputcurrent);;
}

static int dcp_get_iin_limit(unsigned int *val)
{
	struct charge_sysfs_dev *di = g_charge_sysfs_di;

	if (!di || !val) {
		hwlog_err("dcp_get_iin_limit di or val is null\n");
		return -EINVAL;
	}

	*val = di->inputcurrent;

	return 0;
}

static int dcp_set_ichg_limit(unsigned int val)
{
	struct charge_sysfs_dev *di = g_charge_sysfs_di;

	if (!di) {
		hwlog_err("dcp_set_ichg_limit di is null\n");
		return -EPERM;
	}

	return buck_charge_set_ichg_limit(val);
}

#ifndef CONFIG_HLTHERM_RUNTEST
static int dcp_set_ichg_thermal(unsigned int val)
{
	struct charge_sysfs_dev *di = g_charge_sysfs_di;

	if (!di || (val > di->ichg_max))
		return -EINVAL;

	if ((val == 0) || (val == 1))
		di->ichg_thl = di->ichg_max;
	else if ((val > 1) && (val <= CHARGE_SYSFS_MIN_ICHG_CURRENT))
		di->ichg_thl = CHARGE_SYSFS_MIN_ICHG_CURRENT;
	else
		di->ichg_thl = val;

	buck_charge_set_ichg_thermal(val, di->ichg_thl);
	return 0;
}
#else
static int dcp_set_ichg_thermal(unsigned int val)
{
	return 0;
}
#endif /* CONFIG_HLTHERM_RUNTEST */

static int dcp_get_ichg_thermal(unsigned int *val)
{
	struct charge_sysfs_dev *di = g_charge_sysfs_di;

	if (!di || !val) {
		hwlog_err("dcp_get_ichg_thermal di or val is null\n");
		return -EINVAL;
	}

	*val = di->ichg_thl;
	return 0;
}

#ifndef CONFIG_HLTHERM_RUNTEST
static int dcp_set_iin_limit_array(unsigned int idx, unsigned int val)
{
	struct charge_sysfs_dev *di = g_charge_sysfs_di;

	if (!di || (val > di->iin_max))
		return -EINVAL;

	if ((val == 0) || (val == 1))
		di->iin_thl_array[idx] = di->iin_max;
	else if ((val > 1) && (val <= CHARGE_SYSFS_MIN_IIN_CURRENT))
		di->iin_thl_array[idx] = CHARGE_SYSFS_MIN_IIN_CURRENT;
	else
		di->iin_thl_array[idx] = val;

	hwlog_info("thermal set input current=%d, type=%u\n",
		di->iin_thl_array[idx], idx);

	if (idx != huawei_charger_get_iin_thermal_charge_type())
		return 0;

	di->iin_thl = di->iin_thl_array[idx];

	(void)buck_charge_set_iin_limit_array(di->iin_thl);
	return 0;
}
#else
static int dcp_set_iin_limit_array(unsigned int idx, unsigned int val)
{
	return 0;
}
#endif /* CONFIG_HLTHERM_RUNTEST */

static int dcp_set_iin_thermal(unsigned int index, unsigned int iin_thermal_value)
{
	if (index >= CHARGE_SYSFS_IIN_THERMAL_CHARGE_TYPE_END) {
		hwlog_err("error index=%u, out of boundary\n", index);
		return -EINVAL;
	}
	return dcp_set_iin_limit_array(index, iin_thermal_value);
}

static int dcp_set_iin_thermal_all(unsigned int value)
{
	int i;

	for (i = CHARGE_SYSFS_IIN_THERMAL_CHARGE_TYPE_BEGIN; i < CHARGE_SYSFS_IIN_THERMAL_CHARGE_TYPE_END; i++) {
		if (dcp_set_iin_limit_array(i, value))
			return -EINVAL;
	}
	return 0;
}

static int dcp_set_vterm_dec(unsigned int val)
{
	struct charge_sysfs_dev *di = g_charge_sysfs_di;

	if (!di) {
		hwlog_err("dcp_set_vterm_dec di is null\n");
		return -EPERM;
	}

	return buck_charge_set_vterm_dec(val);
}

static int dcp_get_hota_iin_limit(unsigned int *val)
{
	return huawei_charger_get_hota_iin_limit();
}

static int dcp_get_startup_iin_limit(unsigned int *val)
{
	return huawei_charger_get_startup_iin_limit();
}

static int set_adapter_voltage(int val)
{
	if (val < 0)
		return -EINVAL;

	buck_charge_set_adaptor_voltage(val);
	return 0;
}

static int get_adapter_voltage(int *val)
{
	if (!val)
		return -EINVAL;

	if (charge_get_reset_adapter_source())
		*val = ADAPTER_5V;
	else
		*val = ADAPTER_9V;
	return 0;
}

#ifdef CONFIG_SYSFS
#define charge_sysfs_field(_name, n, m, store) \
{ \
	.attr = __ATTR(_name, m, charge_sysfs_show, store), \
	.name = CHARGE_SYSFS_##n, \
}

#define charge_sysfs_field_rw(_name, n) \
	charge_sysfs_field(_name, n, S_IWUSR | S_IRUGO, charge_sysfs_store)

#define charge_sysfs_field_ro(_name, n) \
	charge_sysfs_field(_name, n, S_IRUGO, NULL)

static ssize_t charge_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);

static ssize_t charge_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info charge_sysfs_field_tbl[] = {
	charge_sysfs_field_rw(iin_thermal,              IIN_THERMAL),
	charge_sysfs_field_rw(iin_runningtest,          IIN_RUNNINGTEST),
	charge_sysfs_field_rw(shutdown_watchdog,        WATCHDOG_DISABLE),
	charge_sysfs_field_rw(enable_hiz,               HIZ),
	charge_sysfs_field_rw(adaptor_voltage,          ADAPTOR_VOLTAGE),
	charge_sysfs_field_rw(iin_rt_current,           IIN_RT_CURRENT),
	charge_sysfs_field_rw(enable_charger,           ENABLE_CHARGER),
	charge_sysfs_field_rw(factory_diag,             FACTORY_DIAG_CHARGER),
	charge_sysfs_field_rw(update_volt_now,          UPDATE_VOLT_NOW),
	charge_sysfs_field_rw(adc_conv_rate,            ADC_CONV_RATE),
	charge_sysfs_field_rw(ichg_thermal,             ICHG_THERMAL),
	charge_sysfs_field_rw(ichg_runningtest,         ICHG_RUNNINGTEST),
	charge_sysfs_field_rw(limit_charging,           LIMIT_CHARGING),
	charge_sysfs_field_rw(regulation_voltage,       REGULATION_VOLTAGE),
	charge_sysfs_field_rw(shutdown_q4,              BATFET_DISABLE),
	charge_sysfs_field_rw(charge_done_sleep_status, CHARGE_DONE_SLEEP_STATUS),
	charge_sysfs_field_rw(inputcurrent,             INPUTCURRENT),
	charge_sysfs_field_rw(vr_charger_type,          VR_CHARGER_TYPE),
	charge_sysfs_field_rw(dbc_charge_control,       DBC_CHARGE_CONTROL),
	charge_sysfs_field_rw(plugusb,                  PLUGUSB),
	charge_sysfs_field_rw(thermal_reason,           THERMAL_REASON),
	charge_sysfs_field_rw(charger_cvcal,            CHARGER_CVCAL),
	charge_sysfs_field_rw(charger_cvcal_clear,      CHARGER_CVCAL_CLEAR),
	charge_sysfs_field_ro(charger_online,           CHARGER_ONLINE),
	charge_sysfs_field_ro(chargerType,              CHARGE_TYPE),
	charge_sysfs_field_ro(voltage_sys,              VOLTAGE_SYS),
	charge_sysfs_field_ro(charge_done_status,       CHARGE_DONE_STATUS),
	charge_sysfs_field_ro(dbc_charge_done,          DBC_CHARGE_DONE),
	charge_sysfs_field_ro(ibus,                     IBUS_QCOM),
	charge_sysfs_field_ro(mains_online,             MAINS_ONLINE),
	charge_sysfs_field_ro(Ibus,                     IBUS),
	charge_sysfs_field_ro(Vbus,                     VBUS),
	charge_sysfs_field_ro(charger_get_cvset,        CHARGER_GET_CVSET),
	charge_sysfs_field_ro(charge_term_volt_design,  CHARGE_TERM_VOLT_DESIGN),
	charge_sysfs_field_ro(charge_term_curr_design,  CHARGE_TERM_CURR_DESIGN),
	charge_sysfs_field_ro(charge_term_volt_setting, CHARGE_TERM_VOLT_SETTING),
	charge_sysfs_field_ro(charge_term_curr_setting, CHARGE_TERM_CURR_SETTING),
	charge_sysfs_field_ro(is_trim_charger,          IS_TRIM_CHARGER),
};

static struct attribute *charge_sysfs_attrs[ARRAY_SIZE(charge_sysfs_field_tbl) + 1];

static const struct attribute_group charge_sysfs_attr_group = {
	.attrs = charge_sysfs_attrs,
};

static int charge_sysfs_common_show(struct charge_sysfs_dev *di, char *buf, size_t offset)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", *(int *)((void *)di + offset));
}

static int charge_sysfs_iin_thermal_show(struct charge_sysfs_dev *di, char *buf, size_t offset)
{
	int idx = huawei_charger_get_iin_thermal_charge_type();

	di->iin_thl = di->iin_thl_array[idx];
	return snprintf(buf, PAGE_SIZE, "%d\n", di->iin_thl);
}

static int charge_sysfs_hiz_show(struct charge_sysfs_dev *di, char *buf, size_t offset)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", buck_charge_get_hiz_mode(di->hiz_mode) ?
		HIZ_MODE_ENABLE : HIZ_MODE_DISABLE);;
}

static int charge_sysfs_thermal_reason_show(struct charge_sysfs_dev *di, char *buf, size_t offset)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", (char *)((void *)di + offset));
}

static int charge_sysfs_charge_type_show(struct charge_sysfs_dev *di, char *buf, size_t offset)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", charge_get_charger_type());
}

static int charge_sysfs_charge_done_status_show(struct charge_sysfs_dev *di, char *buf, size_t offset)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", charge_get_done_type());
}

static int charge_sysfs_voltage_sys_show(struct charge_sysfs_dev *di, char *buf, size_t offset)
{
	int vsys = 0;

	vsys = buck_charge_get_vsys();
	return snprintf(buf, PAGE_SIZE, "%d\n", (vsys * POWER_UA_PER_MA));
}

static int charge_sysfs_adaptor_voltage_show(struct charge_sysfs_dev *di, char *buf, size_t offset)
{
	if (charge_get_reset_adapter_source())
		return snprintf(buf, PAGE_SIZE, "%d\n", ADAPTER_5V);
	else
		return snprintf(buf, PAGE_SIZE, "%d\n", ADAPTER_9V);
}

static int charge_sysfs_update_volt_now_show(struct charge_sysfs_dev *di, char *buf, size_t offset)
{
	/* always return 1 when reading UPDATE_VOLT_NOW property */
	return snprintf(buf, PAGE_SIZE, "%d\n", 1);
}

static int charge_sysfs_ibus_show(struct charge_sysfs_dev *di, char *buf, size_t offset)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", charge_get_ibus());
}

static int charge_sysfs_main_online_show(struct charge_sysfs_dev *di, char *buf, size_t offset)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", buck_charge_get_mains_online());
}

static int charge_sysfs_vbus_show(struct charge_sysfs_dev *di, char *buf, size_t offset)
{
	charge_get_vbus(&di->vbus);
	return snprintf(buf, PAGE_SIZE, "%d\n", di->vbus);
}

static int charge_sysfs_charge_term_volt_show(struct charge_sysfs_dev *di, char *buf, size_t offset)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", ((di->vterm < di->vterm_rt) ? di->vterm : di->vterm_rt));
}

static int charge_sysfs_dbc_charge_control_show(struct charge_sysfs_dev *di, char *buf, size_t offset)
{
	if (charge_ops_valid(set_force_term_enable))
		return snprintf(buf, PAGE_SIZE, "%d\n", di->dbc_charge_control);

	return 0;
}

static int charge_sysfs_dbc_charge_done_show(struct charge_sysfs_dev *di, char *buf, size_t offset)
{
	if (charge_ops_valid(get_charger_state))
		return snprintf(buf, PAGE_SIZE, "%d\n", charge_get_charger_state());

	return 0;
}

static int charge_sysfs_plugusb_show(struct charge_sysfs_dev *di, char *buf, size_t offset)
{
	return buck_charge_get_plugusb(buf);
}

static int charge_sysfs_charger_online_show(struct charge_sysfs_dev *di, char *buf, size_t offset)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", charge_get_charger_online());
}

static int charge_sysfs_charger_get_cvset_show(struct charge_sysfs_dev *di, char *buf, size_t offset)
{
	di->charger_get_cvset = 0;
	if (charge_ops_valid(get_terminal_voltage))
		di->charger_get_cvset = charge_get_terminal_voltage();

	return snprintf(buf, PAGE_SIZE, "%d\n", di->charger_get_cvset);
}

static int charge_sysfs_is_trim_charger_show(struct charge_sysfs_dev *di, char *buf, size_t offset)
{
	return scnprintf(buf, PAGE_SIZE, "%d\n", power_gpio_misc_is_trim_charger());
}

static int charge_sysfs_iin_thermal_store(struct charge_sysfs_dev *di, const char *buf)
{
	int idx = 0;
	long val = 0;

	if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) ||
		(val < 0) || (val > di->iin_max))
		return -EINVAL;

	idx = huawei_charger_get_iin_thermal_charge_type();
	buck_charge_set_iin_thermal(val, di->iin_max, &di->iin_thl_array[idx]);
	return 0;
}

static int charge_sysfs_iin_runningtest_store(struct charge_sysfs_dev *di, const char *buf)
{
	long val = 0;

	if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) ||
		(val < 0) || (val > di->iin_max))
		return -EINVAL;

	hwlog_info("thermal set running test val=%ld\n", val);
	buck_charge_set_iin_runningtest(val, di->iin_max, &di->iin_rt);
	return 0;
}

static int charge_sysfs_watchdog_disable_store(struct charge_sysfs_dev *di, const char *buf)
{
	long val = 0;

	if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) || (val < 0) || (val > 1))
		return -EINVAL;

	di->wdt_disable = val;
	if (val == 1)
		charge_set_watchdog(CHAGRE_WDT_DISABLE);

	hwlog_info("runningtest set wdt_disable=%d\n", di->wdt_disable);
	return 0;
}

static int charge_sysfs_hiz_store(struct charge_sysfs_dev *di, const char *buf)
{
	long val = 0;

	if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) || (val < 0) || (val > 1))
		return -EINVAL;

	hwlog_info("enable_charger set hz mode val=%ld\n", val);
	di->hiz_mode = val;
	if (!val && huawei_charger_get_dc_enable_hiz_status())
		hwlog_info("in direct charging, ignore set hiz\n");
	else
		charge_set_hiz_enable(val ? HIZ_MODE_ENABLE : HIZ_MODE_DISABLE);

	return 0;
}

static int charge_sysfs_adaptor_voltage_store(struct charge_sysfs_dev *di, const char *buf)
{
	long val = 0;

	if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) ||
		(val < 0) || (val > POWER_BASE_DEC))
		return -EINVAL;

	buck_charge_set_adaptor_voltage(val);
	return 0;
}

static int charge_sysfs_iin_rt_current_store(struct charge_sysfs_dev *di, const char *buf)
{
	long val = 0;

	if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) ||
		(val < 0) || (val > di->iin_max))
		return -EINVAL;

	hwlog_info("thermal set rt test val=%ld\n", val);
	buck_charge_set_iin_rt_current(val, &di->iin_rt_curr);
	return 0;
}

static int charge_sysfs_enable_charger_store(struct charge_sysfs_dev *di, const char *buf)
{
#ifndef CONFIG_HLTHERM_RUNTEST
	long val = 0;

	if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) || (val < 0) || (val > 1))
		return -EINVAL;

	hwlog_info("enable_charger set enable charger val=%ld\n", val);
	buck_charge_set_chrg_config(val, &di->chrg_config);
#endif
	return 0;
}

static int charge_sysfs_factory_diag_store(struct charge_sysfs_dev *di, const char *buf)
{
	long val = 0;

	if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) || (val < 0) || (val > 1))
		return -EINVAL;

	buck_charge_set_factory_diag(val);
	di->factory_diag = val;
	hwlog_info("enable_charger set factory_diag=%d\n", di->factory_diag);
	return 0;
}

static int charge_sysfs_update_volt_now_store(struct charge_sysfs_dev *di, const char *buf)
{
	long val = 0;

	/* update volt now valid input is 1 */
	if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) || (val != 1))
		return -EINVAL;

	return 0;
}

static int charge_sysfs_adc_conv_rate_store(struct charge_sysfs_dev *di, const char *buf)
{
	long val = 0;

	if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) || (val < 0) || (val > 1))
		return -EINVAL;

	di->adc_conv_rate = val;
	if (charge_ops_valid(set_adc_conv_rate))
		charge_set_adc_conv_rate(di->adc_conv_rate);
	hwlog_info("set adc conversion rate mode=%d\n", di->adc_conv_rate);
	return 0;
}

static int charge_sysfs_ichg_thermal_store(struct charge_sysfs_dev *di, const char *buf)
{
#ifndef CONFIG_HLTHERM_RUNTEST
	long val = 0;

	if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) ||
		(val < 0) || (val > di->ichg_max))
		return -EINVAL;

	dcp_set_ichg_thermal(val);
#endif

	return 0;
}

static int charge_sysfs_ichg_runningtest_store(struct charge_sysfs_dev *di, const char *buf)
{
	long val = 0;

	if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) ||
		(val < 0) || (val > di->ichg_max))
		return -EINVAL;

	if ((val == 0) || (val == 1))
		di->ichg_rt = di->ichg_max;
	else if ((val > 1) && (val <= CHARGE_SYSFS_MIN_ICHG_RT))
		di->ichg_rt = CHARGE_SYSFS_MIN_ICHG_RT;
	else
		di->ichg_rt = val;

	buck_charge_set_ichg_runningtest(di->ichg_rt);
	return 0;
}

static int charge_sysfs_charge_limit_store(struct charge_sysfs_dev *di, const char *buf)
{
	long val = 0;

	if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) || (val < 0) || (val > 1))
		return -EINVAL;

	di->charge_limit = val;
	hwlog_info("projectmune set limit charge enable=%d\n", di->charge_limit);
	return 0;
}

static int charge_sysfs_vterm_rt_store(struct charge_sysfs_dev *di, const char *buf)
{
	long val = 0;

	if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) || (val < CHARGE_SYSFS_MIN_VTERM_RT) ||
		(val > CHARGE_SYSFS_MAX_VTERM_RT))
		return -EINVAL;

	di->vterm_rt = val;
	buck_charge_set_vterm_rt(di->vterm_rt);
	return 0;
}

static int charge_sysfs_batfet_disable_store(struct charge_sysfs_dev *di, const char *buf)
{
	long val = 0;

	if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) || (val < 0) || (val > 1))
		return -EINVAL;

	di->batfet_disable = val;
	buck_charge_set_batfet_disable(val, di->batfet_disable);
	return 0;
}

static int charge_sysfs_charge_done_status_store(struct charge_sysfs_dev *di, const char *buf)
{
	long val = 0;

	if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) || (val < 0) || (val > 1))
		return -EINVAL;

	di->charge_done_status = val;
	return 0;
}

static int charge_sysfs_charge_done_sleep_status_store(struct charge_sysfs_dev *di, const char *buf)
{
	long val = 0;

	if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) || (val < 0) || (val > 1))
		return -EINVAL;

	di->charge_done_sleep_status = val;
	return 0;
}

static int charge_sysfs_inputcurrent_store(struct charge_sysfs_dev *di, const char *buf)
{
	long val = 0;

	if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) || (val < CHARGE_SYSFS_MIN_INPUTCURRENT) ||
		(val > CHARGE_SYSFS_MAX_INPUTCURRENT))
		return -EINVAL;

	di->inputcurrent = val;
	buck_charge_set_inputcurrent(di->inputcurrent);
	return 0;
}

static int charge_sysfs_vr_charger_type_store(struct charge_sysfs_dev *di, const char *buf)
{
	long val = 0;

	if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) || (val < 0) || (val > CHARGER_TYPE_NONE))
		return -EINVAL;

	di->vr_charger_type = val;
	hwlog_info("set vr_charger_type=%d\n", di->vr_charger_type);
	buck_charge_set_vr_charge_type();
	return 0;
}

static int charge_sysfs_dbc_charge_control_store(struct charge_sysfs_dev *di, const char *buf)
{
	long val = 0;

	if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) || (val < 0) ||
		(val > CHARGE_SYSFS_CHARGER_NOT_DBC_CONTROL))
		return -EINVAL;

	if (!charge_ops_valid(set_force_term_enable)) {
		if (charge_set_force_term_enable(val))
			hwlog_err("set force term enable failed\n");
		di->dbc_charge_control = (unsigned int)val;
		hwlog_info("set charge term switch %d\n", di->dbc_charge_control);
	}
	return 0;
}

static int charge_sysfs_plugusb_store(struct charge_sysfs_dev *di, const char *buf)
{
	buck_charge_set_plugusb(buf);
	return 0;
}

static int charge_sysfs_thermal_reason_store(struct charge_sysfs_dev *di, const char *buf)
{
	if (strlen(buf) < CHARGE_SYSFS_THERMAL_REASON_SIZE)
		snprintf(di->thermal_reason, strlen(buf), "%s", buf);

	hwlog_info("thermal set reason=%s, buf=%s\n", di->thermal_reason, buf);
	return 0;
}

static int charge_sysfs_charger_cvcal_store(struct charge_sysfs_dev *di, const char *buf)
{
	long val = 0;

	if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) || (val < 0)) {
		hwlog_err("cvcal:invaild para\n");
		return -EINVAL;
	}

	buck_charge_set_charger_cvcal(val, &di->charger_cvcal_value);
	return 0;
}

static int charge_sysfs_charger_cvcal_clear_store(struct charge_sysfs_dev *di, const char *buf)
{
	long val = 0;

	if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) || (val < 0)) {
		hwlog_err("cvcal_clear:invaild para\n");
		return -EINVAL;
	}

	buck_charge_set_charger_cvcal_clear(val, &di->charger_cvcal_clear);
	return 0;
}

static struct charge_sysfs_show_struct g_charge_sysfs_show_tbl[] = {
	[CHARGE_SYSFS_IIN_THERMAL] = { charge_sysfs_iin_thermal_show, 0 },
	[CHARGE_SYSFS_IIN_RUNNINGTEST] = { charge_sysfs_common_show, offsetof(struct charge_sysfs_dev, iin_rt) },
	[CHARGE_SYSFS_WATCHDOG_DISABLE] = { charge_sysfs_common_show, offsetof(struct charge_sysfs_dev, wdt_disable) },
	[CHARGE_SYSFS_HIZ] = { charge_sysfs_hiz_show, 0 },
	[CHARGE_SYSFS_CHARGE_TYPE] = { charge_sysfs_charge_type_show, 0 },
	[CHARGE_SYSFS_CHARGE_DONE_STATUS] = { charge_sysfs_charge_done_status_show, 0 },
	[CHARGE_SYSFS_VOLTAGE_SYS] = { charge_sysfs_voltage_sys_show, 0 },
	[CHARGE_SYSFS_ADAPTOR_VOLTAGE] = { charge_sysfs_adaptor_voltage_show, 0 },
	[CHARGE_SYSFS_IIN_RT_CURRENT] = { charge_sysfs_common_show, offsetof(struct charge_sysfs_dev, iin_rt_curr) },
	[CHARGE_SYSFS_ENABLE_CHARGER] = { charge_sysfs_common_show, offsetof(struct charge_sysfs_dev, chrg_config) },
	[CHARGE_SYSFS_FACTORY_DIAG_CHARGER] = { charge_sysfs_common_show, offsetof(struct charge_sysfs_dev, factory_diag) },
	[CHARGE_SYSFS_UPDATE_VOLT_NOW] = { charge_sysfs_update_volt_now_show, 0 },
	[CHARGE_SYSFS_IBUS] = { charge_sysfs_ibus_show, 0 },
	[CHARGE_SYSFS_IBUS_QCOM] = { charge_sysfs_ibus_show, 0 },
	[CHARGE_SYSFS_MAINS_ONLINE] = { charge_sysfs_main_online_show, 0 },
	[CHARGE_SYSFS_ADC_CONV_RATE] = { charge_sysfs_common_show, offsetof(struct charge_sysfs_dev, adc_conv_rate) },
	[CHARGE_SYSFS_ICHG_THERMAL] = { charge_sysfs_common_show, offsetof(struct charge_sysfs_dev, ichg_thl) },
	[CHARGE_SYSFS_ICHG_RUNNINGTEST] = { charge_sysfs_common_show, offsetof(struct charge_sysfs_dev, ichg_rt) },
	[CHARGE_SYSFS_LIMIT_CHARGING] = { charge_sysfs_common_show, offsetof(struct charge_sysfs_dev, charge_limit) },
	[CHARGE_SYSFS_REGULATION_VOLTAGE] = { charge_sysfs_common_show, offsetof(struct charge_sysfs_dev, vterm_rt) },
	[CHARGE_SYSFS_BATFET_DISABLE] = { charge_sysfs_common_show, offsetof(struct charge_sysfs_dev, batfet_disable) },
	[CHARGE_SYSFS_VBUS] = { charge_sysfs_vbus_show, 0 },
	[CHARGE_SYSFS_CHARGE_DONE_SLEEP_STATUS] = { charge_sysfs_common_show, offsetof(struct charge_sysfs_dev, charge_done_sleep_status) },
	[CHARGE_SYSFS_INPUTCURRENT] = { charge_sysfs_common_show, offsetof(struct charge_sysfs_dev, inputcurrent) },
	[CHARGE_SYSFS_VR_CHARGER_TYPE] = { charge_sysfs_common_show, offsetof(struct charge_sysfs_dev, vr_charger_type) },
	[CHARGE_SYSFS_CHARGE_TERM_VOLT_DESIGN] = { charge_sysfs_common_show, offsetof(struct charge_sysfs_dev, vterm) },
	[CHARGE_SYSFS_CHARGE_TERM_CURR_DESIGN] = { charge_sysfs_common_show, offsetof(struct charge_sysfs_dev, iterm) },
	[CHARGE_SYSFS_CHARGE_TERM_VOLT_SETTING] = { charge_sysfs_charge_term_volt_show, 0 },
	[CHARGE_SYSFS_CHARGE_TERM_CURR_SETTING] = { charge_sysfs_common_show, offsetof(struct charge_sysfs_dev, iterm) },
	[CHARGE_SYSFS_DBC_CHARGE_CONTROL] = { charge_sysfs_dbc_charge_control_show, 0 },
	[CHARGE_SYSFS_DBC_CHARGE_DONE] = { charge_sysfs_dbc_charge_done_show, 0 },
	[CHARGE_SYSFS_PLUGUSB] = { charge_sysfs_plugusb_show, 0 },
	[CHARGE_SYSFS_THERMAL_REASON] = { charge_sysfs_thermal_reason_show, offsetof(struct charge_sysfs_dev, thermal_reason) },
	[CHARGE_SYSFS_CHARGER_ONLINE] = { charge_sysfs_charger_online_show, 0 },
	[CHARGE_SYSFS_CHARGER_CVCAL] = { charge_sysfs_common_show, offsetof(struct charge_sysfs_dev, charger_cvcal_value) },
	[CHARGE_SYSFS_CHARGER_CVCAL_CLEAR] = { charge_sysfs_common_show, offsetof(struct charge_sysfs_dev, charger_cvcal_clear) },
	[CHARGE_SYSFS_CHARGER_GET_CVSET] = { charge_sysfs_charger_get_cvset_show, 0 },
	[CHARGE_SYSFS_IS_TRIM_CHARGER] = { charge_sysfs_is_trim_charger_show, 0 },
};

static struct charge_sysfs_store_struct g_charge_sysfs_store_tbl[] = {
	[CHARGE_SYSFS_IIN_THERMAL] = { charge_sysfs_iin_thermal_store },
	[CHARGE_SYSFS_IIN_RUNNINGTEST] = { charge_sysfs_iin_runningtest_store },
	[CHARGE_SYSFS_WATCHDOG_DISABLE] = { charge_sysfs_watchdog_disable_store },
	[CHARGE_SYSFS_HIZ] = { charge_sysfs_hiz_store },
	[CHARGE_SYSFS_ADAPTOR_VOLTAGE] = { charge_sysfs_adaptor_voltage_store },
	[CHARGE_SYSFS_IIN_RT_CURRENT] = { charge_sysfs_iin_rt_current_store },
	[CHARGE_SYSFS_ENABLE_CHARGER] = { charge_sysfs_enable_charger_store },
	[CHARGE_SYSFS_FACTORY_DIAG_CHARGER] = { charge_sysfs_factory_diag_store },
	[CHARGE_SYSFS_UPDATE_VOLT_NOW] = { charge_sysfs_update_volt_now_store },
	[CHARGE_SYSFS_ADC_CONV_RATE] = { charge_sysfs_adc_conv_rate_store },
	[CHARGE_SYSFS_ICHG_THERMAL] = { charge_sysfs_ichg_thermal_store },
	[CHARGE_SYSFS_ICHG_RUNNINGTEST] = { charge_sysfs_ichg_runningtest_store },
	[CHARGE_SYSFS_LIMIT_CHARGING] = { charge_sysfs_charge_limit_store },
	[CHARGE_SYSFS_REGULATION_VOLTAGE] = { charge_sysfs_vterm_rt_store },
	[CHARGE_SYSFS_BATFET_DISABLE] = { charge_sysfs_batfet_disable_store },
	[CHARGE_SYSFS_CHARGE_DONE_STATUS] = { charge_sysfs_charge_done_status_store },
	[CHARGE_SYSFS_CHARGE_DONE_SLEEP_STATUS] = { charge_sysfs_charge_done_sleep_status_store },
	[CHARGE_SYSFS_INPUTCURRENT] = { charge_sysfs_inputcurrent_store },
	[CHARGE_SYSFS_VR_CHARGER_TYPE] = { charge_sysfs_vr_charger_type_store },
	[CHARGE_SYSFS_DBC_CHARGE_CONTROL] = { charge_sysfs_dbc_charge_control_store },
	[CHARGE_SYSFS_PLUGUSB] = { charge_sysfs_plugusb_store },
	[CHARGE_SYSFS_THERMAL_REASON] = { charge_sysfs_thermal_reason_store },
	[CHARGE_SYSFS_CHARGER_CVCAL] = { charge_sysfs_charger_cvcal_store },
	[CHARGE_SYSFS_CHARGER_CVCAL_CLEAR] = { charge_sysfs_charger_cvcal_clear_store },
};

/* show the value for all charge device's node */
static ssize_t charge_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	struct charge_sysfs_dev *di = g_charge_sysfs_di;

	if (!dev || !attr || !buf) {
		hwlog_err("charge_sysfs_show invalid param, fatal error\n");
		return -EINVAL;
	}

	info = power_sysfs_lookup_attr(attr->attr.name,
		charge_sysfs_field_tbl, ARRAY_SIZE(charge_sysfs_field_tbl));
	if (!di || !info)
		return -EINVAL;

	if (!g_charge_sysfs_show_tbl[info->name].show_process)
		return -EPERM;

	return  g_charge_sysfs_show_tbl[info->name].show_process(di, buf,
		g_charge_sysfs_show_tbl[info->name].offset);
}

/* set the value for charge_data's node which is can be written */
static ssize_t charge_sysfs_store(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	int ret = 0;
	struct power_sysfs_attr_info *info = NULL;
	struct charge_sysfs_dev *di = g_charge_sysfs_di;

	if (!dev || !attr || !buf) {
		hwlog_err("charge_sysfs_store invalid param, fatal error\n");
		return -EINVAL;
	}

	info = power_sysfs_lookup_attr(attr->attr.name,
		charge_sysfs_field_tbl, ARRAY_SIZE(charge_sysfs_field_tbl));
	if (!di || !info)
		return -EINVAL;

	if (!g_charge_sysfs_store_tbl[info->name].store_process)
		return -EPERM;

	ret = g_charge_sysfs_store_tbl[info->name].store_process(di, buf);
	return (ret == 0) ? count : ret;
}

static void charge_sysfs_create_group(struct device *dev)
{
	power_sysfs_init_attrs(charge_sysfs_attrs,
		charge_sysfs_field_tbl, ARRAY_SIZE(charge_sysfs_field_tbl));
	power_sysfs_create_link_group("hw_power", "charger", "charge_data",
		dev, &charge_sysfs_attr_group);
}

static void charge_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_link_group("hw_power", "charger", "charge_data",
		dev, &charge_sysfs_attr_group);
}
#else
static inline void charge_sysfs_create_group(struct device *dev)
{
}

static inline void charge_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static struct power_if_ops sdp_if_ops = {
	.type_name = "sdp",
	.set_iin_limit = sdp_set_iin_limit,
};

static struct power_if_ops dcp_if_ops = {
	.type_name = "dcp",
	.set_enable_charger = dcp_set_enable_charger,
	.get_enable_charger = dcp_get_enable_charger,
	.set_iin_limit = dcp_set_iin_limit,
	.get_iin_limit = dcp_get_iin_limit,
	.set_ichg_limit = dcp_set_ichg_limit,
	.set_vterm_dec = dcp_set_vterm_dec,
	.get_hota_iin_limit = dcp_get_hota_iin_limit,
	.get_startup_iin_limit = dcp_get_startup_iin_limit,
	.set_iin_thermal = dcp_set_iin_thermal,
	.set_iin_thermal_all = dcp_set_iin_thermal_all,
	.set_ichg_thermal = dcp_set_ichg_thermal,
	.get_ichg_thermal = dcp_get_ichg_thermal,
	.set_adap_volt = set_adapter_voltage,
	.get_adap_volt = get_adapter_voltage,
};

static struct power_if_ops fcp_if_ops = {
	.type_name = "hvc",
	.set_enable_charger = fcp_set_enable_charger,
	.get_enable_charger = fcp_get_enable_charger,
};

static void charge_sysfs_params_init(struct charge_sysfs_dev *di)
{
	di->iin_max = buck_charge_get_charge_core_data(BUCK_CHARGE_CORE_IIN_MAX);
	di->ichg_max = buck_charge_get_charge_core_data(BUCK_CHARGE_CORE_ICHG_MAX);

	di->iin_thl = di->iin_max;
	di->iin_thl_array[CHARGE_SYSFS_IIN_THERMAL_WCURRENT_5V] = di->iin_max;
	di->iin_thl_array[CHARGE_SYSFS_IIN_THERMAL_WCURRENT_9V] = di->iin_max;
	di->iin_thl_array[CHARGE_SYSFS_IIN_THERMAL_WLCURRENT_5V] = di->iin_max;
	di->iin_thl_array[CHARGE_SYSFS_IIN_THERMAL_WLCURRENT_9V] = di->iin_max;
	di->iin_rt = di->iin_max;
	di->ichg_thl = di->ichg_max;
	di->ichg_rt = di->ichg_max;
	di->vterm_rt = buck_charge_get_charge_core_data(BUCK_CHARGE_CORE_VTERM_RT);
	di->iterm = buck_charge_get_charge_core_data(BUCK_CHARGE_CORE_ITERM);
	di->vterm = buck_charge_get_charge_core_data(BUCK_CHARGE_CORE_VTERM);
	di->iin_rt_curr = CHARGE_SYSFS_DEFAULT_IIN_CURRENT;
	di->hiz_mode = 0;
	di->chrg_config = 1;
	di->factory_diag = 1;
	di->adc_conv_rate = 0;
	di->charge_enable = TRUE;
	di->fcp_charge_enable = TRUE;
	di->batfet_disable = FALSE;
	di->wdt_disable = FALSE;
	di->charge_limit = TRUE;
	di->charge_done_status = CHARGE_DONE_NON;
	di->charge_done_sleep_status = CHARGE_DONE_SLEEP_DISABLED;
	di->vr_charger_type = CHARGER_TYPE_NONE;
	di->dbc_charge_control = CHARGE_SYSFS_CHARGER_NOT_DBC_CONTROL;
}

void charge_sysfs_init(struct device *dev)
{
	struct charge_sysfs_dev *di = NULL;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di || !dev)
		return;

	if (buck_charge_init_charge_core_data()) {
		kfree(di);
		return;
	}

	charge_sysfs_params_init(di);
	charge_sysfs_create_group(dev);
	power_if_ops_register(&sdp_if_ops);
	power_if_ops_register(&dcp_if_ops);
	power_if_ops_register(&fcp_if_ops);
	g_charge_sysfs_di = di;
}

void charge_sysfs_remove(struct device *dev)
{
	charge_sysfs_remove_group(dev);
	kfree(g_charge_sysfs_di);
	g_charge_sysfs_di = NULL;
}
