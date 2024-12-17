// SPDX-License-Identifier: GPL-2.0
/*
 * rt9426_kirin.c
 *
 * driver for rt9426 kirin platform driver
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

#include "rt9426.h"

#define HWLOG_TAG rt9426_kirin
HWLOG_REGIST();

static int rt9426_get_property(struct power_supply *psy,
	enum power_supply_property psp,
	union power_supply_propval *val)
{
	struct rt9426_chip *chip = power_supply_get_drvdata(psy);
	int rc = 0;

	switch (psp) {
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = coul_interface_is_battery_exist(chip->pdata->ic_role);
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = chip->online;
		dev_info(chip->dev, "psp_online = %d\n", val->intval);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = rt9426_get_display_data(chip, RT9426_DISPLAY_VBAT);
		dev_info(chip->dev, "psp_volt_now = %d\n", val->intval);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN:
		val->intval = chip->pdata->battery_type;
		dev_info(chip->dev, "psp_volt_max_design = %d\n", val->intval);
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		chip->capacity = rt9426_get_display_data(chip, RT9426_DISPLAY_SOC);
		val->intval = chip->capacity;
		dev_info(chip->dev, "psp_capacity = %d\n", val->intval);
		break;
	case POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN:
		val->intval = rt9426_get_display_data(chip, RT9426_DISPLAY_DISIGN_FCC);
		dev_info(chip->dev,
			"psp_charge_full_design = %d\n", val->intval);
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		val->intval = rt9426_get_display_data(chip, RT9426_DISPLAY_IBAT);
		dev_info(chip->dev, "psp_curr_now = %d\n", val->intval);
		break;
	case POWER_SUPPLY_PROP_TEMP:
		val->intval = rt9426_get_display_data(chip, RT9426_DISPLAY_TEMP);
		dev_info(chip->dev, "psp_temp = %d\n", val->intval);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_OCV:
		val->intval = chip->ocv_index;
		dev_info(chip->dev, "ocv index = %d\n", chip->ocv_index);
		break;
	default:
		rc = -EINVAL;
		break;
	}
	return rc;
}

static int rt9426_set_property(struct power_supply *psy,
	enum power_supply_property psp,
	const union power_supply_propval *val)
{
	return 0;
}

static int rt9426_property_is_writeable(struct power_supply *psy,
	enum power_supply_property psp)
{
	return false;
}

static enum power_supply_property rt9426_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN,
	POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_VOLTAGE_OCV,
};

static struct power_supply_desc fg_psy_desc = {
	.name = "rt-fuelgauge",
	.type = POWER_SUPPLY_TYPE_UNKNOWN,
	.properties = rt9426_props,
	.num_properties = ARRAY_SIZE(rt9426_props),
	.get_property = rt9426_get_property,
	.set_property = rt9426_set_property,
	.property_is_writeable = rt9426_property_is_writeable,
};

static struct power_supply_desc fg_aux_psy_desc = {
	.name = "rt-fuelgauge_aux",
	.type = POWER_SUPPLY_TYPE_UNKNOWN,
	.properties = rt9426_props,
	.num_properties = ARRAY_SIZE(rt9426_props),
	.get_property = rt9426_get_property,
	.set_property = rt9426_set_property,
	.property_is_writeable = rt9426_property_is_writeable,
};

static void rt9426_register_psy_desc(struct rt9426_chip *chip)
{
	struct power_supply_config psy_config = {};
	struct rt9426_platform_data *pdata = chip->pdata;
	const char *bat_name = "battery_gauge";
	const char *bat_aux_name = "battery_gauge_aux";

	if (!pdata || !chip->i2c) {
		hwlog_err("register batt psy fail, pdata or i2c NULL\n");
		return;
	}

	psy_config.of_node = chip->i2c->dev.of_node;
	psy_config.drv_data = chip;
	if (pdata->ic_role == RT9426_IC_TYPE_MAIN) {
		fg_psy_desc.name = bat_name;
		chip->fg_psy = devm_power_supply_register(chip->dev,
			&fg_psy_desc, &psy_config);
	} else {
		fg_aux_psy_desc.name = bat_aux_name;
		chip->fg_psy = devm_power_supply_register(chip->dev,
			&fg_aux_psy_desc, &psy_config);
	}
	if (IS_ERR(chip->fg_psy))
		dev_notice(chip->dev, "register batt psy fail\n");
}

static int rt9426_is_connect_type_1s2p(void)
{
	struct rt9426_chip *chip = rt9426_get_chip(RT9426_IC_TYPE_MAIN);
	if (!chip || !chip->pdata)
		return false;

	if (chip->pdata->connect_type == BAT_CONNECT_1S2P)
		return true;

	return false;
}

static int rt9426_is_1s2p_ready(void)
{
	static int is_ready = 0;

	if (!is_ready)
		is_ready = coul_interface_is_coul_ready(COUL_TYPE_1S2P);

	return is_ready;
}

static int rt9426_get_battery_id_vol(void)
{
	int i;
	struct rt9426_chip *chip = rt9426_get_chip(RT9426_IC_TYPE_MAIN);
	struct rt9426_platform_data *pdata = NULL;

	if (!chip || !chip->pdata)
		return 0;

	pdata = chip->pdata;
	for (i = 0; i < ID_VOLT_PARA_LEVEL; i++)
		if (pdata->id_volt_data[i].id_index == RT9426_IC_TYPE_MAIN)
			return pdata->id_volt_data[i].batt_id_voltage;

	return 0;
}

static int rt9426_coul_is_ready(void)
{
	if (rt9426_is_connect_type_1s2p())
		return coul_interface_is_coul_ready(COUL_TYPE_1S2P);

	return coul_interface_is_coul_ready(COUL_TYPE_MAIN);
}

static int rt9426_coul_is_battery_exist(void)
{
	if (rt9426_is_connect_type_1s2p() && rt9426_is_1s2p_ready())
		return coul_interface_is_battery_exist(COUL_TYPE_1S2P);

	return coul_interface_is_battery_exist(COUL_TYPE_MAIN);
}

static int rt9426_is_battery_reach_threshold(void)
{
	int soc;

	if (!rt9426_coul_is_battery_exist())
		return 0;

	if (rt9426_is_connect_type_1s2p() && rt9426_is_1s2p_ready())
		soc = coul_interface_get_battery_capacity(COUL_TYPE_1S2P);
	else
		soc = coul_interface_get_battery_capacity(COUL_TYPE_MAIN);

	if (soc > RT9426_BATT_CAPACITY_WARNING_LVL)
		return 0;
	else if (soc > RT9426_BATT_CAPACITY_LOW_LVL)
		return BQ_FLAG_SOC1;

	return BQ_FLAG_LOCK;
}

static char *rt9426_get_battery_brand(void)
{
	return "SCUDATL";
}

static int rt9426_get_battery_vol(void)
{
	int vol;

	if (rt9426_is_connect_type_1s2p() && rt9426_is_1s2p_ready())
		vol = coul_interface_get_battery_voltage(COUL_TYPE_1S2P);
	else
		vol = coul_interface_get_battery_voltage(COUL_TYPE_MAIN);

	hwlog_info("battery_vol=%d\n", vol);
	return vol;
}

static int rt9426_get_battery_vol_uv(void)
{
	return POWER_UV_PER_MV * rt9426_get_battery_vol();
}

static int rt9426_get_battery_curr(void)
{
	int val;

	if (rt9426_is_connect_type_1s2p() && rt9426_is_1s2p_ready())
		val = coul_interface_get_battery_current(COUL_TYPE_1S2P);
	else
		val = coul_interface_get_battery_current(COUL_TYPE_MAIN);

	hwlog_info("battery_cur=%d\n", val);
	return val;
}

static int rt9426_get_battery_avgcurr(void)
{
	int val;

	if (rt9426_is_connect_type_1s2p() && rt9426_is_1s2p_ready())
		val = coul_interface_get_battery_avg_current(COUL_TYPE_1S2P);
	else
		val = coul_interface_get_battery_avg_current(COUL_TYPE_MAIN);

	hwlog_info("battery_avgcur=%d\n", val);
	return val;
}

static int rt9426_get_battery_soc(void)
{
	int soc;

	if (rt9426_is_connect_type_1s2p()) {
		soc = coul_interface_get_battery_capacity(COUL_TYPE_1S2P);
		if (soc >= 0)
			return soc;
		else
			hwlog_err("get coul_1s2p capacity error\n");
	}

	return coul_interface_get_battery_capacity(COUL_TYPE_MAIN);
}

static int rt9426_battery_unfiltered_soc(void)
{
	return rt9426_get_battery_soc();
}


static int rt9426_get_battery_temp(void)
{
	int temp;

	if (rt9426_is_connect_type_1s2p() && rt9426_is_1s2p_ready())
		temp = coul_interface_get_battery_temperature(COUL_TYPE_1S2P);
	else
		temp = coul_interface_get_battery_temperature(COUL_TYPE_MAIN);
	temp = temp / POWER_BASE_DEC;

	hwlog_info("battery_temp=%d\n", temp);
	return  temp;
}

static int rt9426_get_battery_rm(void)
{
	if (rt9426_is_connect_type_1s2p() && rt9426_is_1s2p_ready())
		return coul_interface_get_battery_rm(COUL_TYPE_1S2P);

	return coul_interface_get_battery_rm(COUL_TYPE_MAIN);
}

static int rt9426_get_battery_fcc(void)
{
	if (rt9426_is_connect_type_1s2p() && rt9426_is_1s2p_ready())
		return coul_interface_get_battery_fcc(COUL_TYPE_1S2P);

	return coul_interface_get_battery_fcc(COUL_TYPE_MAIN);
}

static int rt9426_get_battery_tte(void)
{
	struct rt9426_chip *chip = rt9426_get_chip(RT9426_IC_TYPE_MAIN);

	if (!chip)
		return 0;

	return rt9426_get_display_data(chip, RT9426_DISPLAY_TTE);
}

static int rt9426_get_battery_ttf(void)
{
	return 0;
}

static int rt9426_get_battery_health(void)
{
	int temp;
	int status = POWER_SUPPLY_HEALTH_GOOD;

	if (rt9426_is_connect_type_1s2p() && rt9426_is_1s2p_ready())
		temp = coul_interface_get_battery_temperature(COUL_TYPE_1S2P);
	else
		temp = coul_interface_get_battery_temperature(COUL_TYPE_MAIN);
	if (bat_fault_is_cutoff_vol())
		status = POWER_SUPPLY_HEALTH_UNDERVOLTAGE;
	else if (temp < RT9426_TEMP_ABR_LOW)
		status = POWER_SUPPLY_HEALTH_COLD;
	else if (temp > RT9426_TEMP_ABR_HIGH)
		status = POWER_SUPPLY_HEALTH_OVERHEAT;

	hwlog_info("battery_health=%d\n", status);
	return status;
}

static int rt9426_get_battery_capacity_level(void)
{
	int capacity;
	int level;

	if (!rt9426_coul_is_battery_exist())
		return 0;

	capacity = rt9426_get_battery_soc();
	if ((capacity < RT9426_BATT_CAPACITY_ZERO) ||
		(capacity > RT9426_BATT_CAPACITY_FULL))
		level = POWER_SUPPLY_CAPACITY_LEVEL_UNKNOWN;
	else if (capacity <= RT9426_BATT_CAPACITY_CRITICAL)
		level = POWER_SUPPLY_CAPACITY_LEVEL_CRITICAL;
	else if (capacity <= RT9426_BATT_CAPACITY_LOW)
		level = POWER_SUPPLY_CAPACITY_LEVEL_LOW;
	else if (capacity < RT9426_BATT_CAPACITY_HIGH)
		level = POWER_SUPPLY_CAPACITY_LEVEL_NORMAL;
	else if (capacity < RT9426_BATT_CAPACITY_FULL)
		level = POWER_SUPPLY_CAPACITY_LEVEL_HIGH;
	else
		level = POWER_SUPPLY_CAPACITY_LEVEL_FULL;

	hwlog_info("battery_cap_level=%d\n", level);
	return level;
}

static int rt9426_get_battery_technology(void)
{
	/* default technology is "Li-poly" */
	return POWER_SUPPLY_TECHNOLOGY_LIPO;
}

struct chrg_para_lut *rt9426_get_battery_charge_params(void)
{
	struct rt9426_chip *chip = rt9426_get_chip(RT9426_IC_TYPE_MAIN);

	if (!chip)
		return NULL;

	return chip->para_batt_data;
}

static int rt9426_get_battery_vbat_max(void)
{
	struct rt9426_chip *chip = rt9426_get_chip(RT9426_IC_TYPE_MAIN);

	if (!chip || !chip->pdata)
		return 0;

	return chip->pdata->vbat_max;
}

static void rt9426_charger_event_process(struct rt9426_chip *chip, unsigned int event)
{
	if (!chip)
		return;

	hwlog_info("receive charge event=%u\n", event);
	switch (event) {
	case VCHRG_START_USB_CHARGING_EVENT:
	case VCHRG_START_AC_CHARGING_EVENT:
	case VCHRG_START_CHARGING_EVENT:
		chip->charge_status = RT9426_CHARGE_STATE_START_CHARGING;
		break;
	case VCHRG_STOP_CHARGING_EVENT:
		chip->charge_status = RT9426_CHARGE_STATE_STOP_CHARGING;
		break;
	case VCHRG_CHARGE_DONE_EVENT:
		chip->charge_status = RT9426_CHARGE_STATE_CHRG_DONE;
		break;
	case VCHRG_NOT_CHARGING_EVENT:
		chip->charge_status = RT9426_CHARGE_STATE_NOT_CHARGING;
		break;
	case VCHRG_POWER_SUPPLY_OVERVOLTAGE:
		chip->charge_status = RT9426_CHARGE_STATE_NOT_CHARGING;
		break;
	case VCHRG_POWER_SUPPLY_WEAKSOURCE:
		chip->charge_status = RT9426_CHARGE_STATE_NOT_CHARGING;
		break;
	default:
		chip->charge_status = RT9426_CHARGE_STATE_NOT_CHARGING;
		break;
	}
}

static int rt9426_battery_charger_event_rcv(unsigned int evt)
{
	struct rt9426_chip *chip = rt9426_get_chip(RT9426_IC_TYPE_MAIN);

	if (!rt9426_coul_is_battery_exist())
		return 0;

	rt9426_charger_event_process(chip, evt);
	return 0;
}

static int rt9426_is_fcc_debounce(void)
{
	return 0;
}

static int rt9426_get_battery_cycle(void)
{
	if (rt9426_is_connect_type_1s2p() && rt9426_is_1s2p_ready())
		return coul_interface_get_battery_cycle(COUL_TYPE_1S2P);

	return coul_interface_get_battery_cycle(COUL_TYPE_MAIN);
}

static int rt9426_get_battery_dc(void)
{
	int dc_main;
	int dc_aux = 0;
	int dc_sum = 0;
	struct rt9426_chip *chip_main = rt9426_get_chip(RT9426_IC_TYPE_MAIN);
	struct rt9426_chip *chip_aux = rt9426_get_chip(RT9426_IC_TYPE_AUX);

	if (rt9426_is_connect_type_1s2p()) {
		dc_main = rt9426_get_display_data(chip_main, RT9426_DISPLAY_DC);
		if (chip_aux)
			dc_aux = rt9426_get_display_data(chip_aux, RT9426_DISPLAY_DC);
	} else {
		dc_main = rt9426_get_display_data(chip_main, RT9426_DISPLAY_DC);
	}

	if ((dc_main < 0) && (dc_aux < 0))
		return -1;
	if (dc_main >= 0)
		dc_sum += dc_main;
	if (dc_aux >= 0)
		dc_sum += dc_aux;

	return dc_sum;
}

static int rt9426_device_check(void)
{
	return 0;
}

static int rt9426_update_basp_policy(unsigned int level, unsigned int nondc_volt_dec)
{
	hwlog_info("basp vterm_dec set: %u\n", nondc_volt_dec);
	if (rt9426_is_connect_type_1s2p())
		return coul_interface_set_vterm_dec(COUL_TYPE_1S2P, nondc_volt_dec);

	return coul_interface_set_vterm_dec(COUL_TYPE_MAIN, nondc_volt_dec);
}

static int rt9426_get_id_volt_para(struct rt9426_platform_data *pdata)
{
	int i, len, temp_data;
	const char *temp_string = NULL;
	struct device_node *np = of_find_compatible_node(NULL, NULL, "rt9426_batt");

	len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"id_volt_para", ID_VOLT_PARA_LEVEL, ID_VOLT_PARA_TOTAL);
	for (i = 0; i < len; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG), np,
			"id_volt_para", i, &temp_string))
			return -EINVAL;

		if (kstrtoint(temp_string, 0, &temp_data))
			return -EINVAL;

		switch (i % ID_VOLT_PARA_TOTAL) {
		case ID_VOLT_INDEX:
			pdata->id_volt_data[i / ID_VOLT_PARA_TOTAL].id_index = temp_data;
			break;
		case ID_VOLT_ADC_CHANNEL:
			pdata->id_volt_data[i / ID_VOLT_PARA_TOTAL].id_adc_channel = temp_data;
			break;
		case ID_VOLT_MIN:
			pdata->id_volt_data[i / ID_VOLT_PARA_TOTAL].id_volt_min = temp_data;
			break;
		case ID_VOLT_MAX:
			pdata->id_volt_data[i / ID_VOLT_PARA_TOTAL].id_volt_max = temp_data;
			break;
		default:
			break;
		}
	}

	return len;
}

static int rt9426_check_batt_id_voltage(struct rt9426_chip *chip)
{
	int i, len, id_voltage;
	struct rt9426_platform_data *pdata = chip->pdata;

	if (!pdata)
		return -EINVAL;

	len = rt9426_get_id_volt_para(pdata);
	if (len < 0)
		return len;

	for (i = 0; i < len / ID_VOLT_PARA_TOTAL; i++) {
		if ((pdata->id_volt_data[i].id_volt_min >=
			pdata->id_volt_data[i].id_volt_max) ||
			(pdata->id_volt_data[i].id_volt_min < 0)) {
			hwlog_err("batt id voltage params error, para_level: %d\n", i);
			continue;
		}
		id_voltage = power_platform_get_adc_voltage(
			pdata->id_volt_data[i].id_adc_channel);
		pdata->id_volt_data[i].batt_id_voltage = id_voltage;
		if ((id_voltage < 0) ||
			((id_voltage < pdata->id_volt_data[i].id_volt_min) ||
			(id_voltage > pdata->id_volt_data[i].id_volt_max))) {
			hwlog_err("para_level: %d, batt id voltage %d mv, check fail\n",
				i, id_voltage);
			continue;
		}
		hwlog_info("batt id:%d, id_voltage is %d mv, check ok\n",
			pdata->id_volt_data[i].id_index, id_voltage);
		return 0;
	}

	return -EINVAL;
}

static int rt9426_read_dts_string_array(const char *prop,
	long *data, u32 row, u32 col)
{
	int i, len;
	const char *tmp_string = NULL;

	len = power_dts_read_count_strings_compatible(power_dts_tag(HWLOG_TAG),
		"rt9426_batt", prop, row, col);
	if (len < 0)
		return -EINVAL;

	for (i = 0; i < len; i++) {
		if (power_dts_read_string_index_compatible(power_dts_tag(HWLOG_TAG),
			"rt9426_batt", prop, i, &tmp_string))
			return -EINVAL;

		if (kstrtol(tmp_string, 0, &data[i]))
			return -EINVAL;
		hwlog_info("data[%d] = %ld\n", i, data[i]);
	}

	return len;
}

static int rt9426_get_temp_para_data(struct chrg_para_lut *pdata)
{
	int len;

	if (!pdata)
		return -EINVAL;

	len = rt9426_read_dts_string_array("temp_para", &pdata->temp_data[0][0],
		TEMP_PARA_LEVEL, TEMP_PARA_TOTAL);
	if (len < 0)
		return -EINVAL;

	pdata->temp_len = len;
	return 0;
}

static int rt9426_get_vbat_para_data(struct chrg_para_lut *pdata)
{
	int len;

	if (!pdata)
		return -EINVAL;

	len = rt9426_read_dts_string_array("vbat_para", &pdata->volt_data[0][0],
		VOLT_PARA_LEVEL, VOLT_PARA_TOTAL);
	if (len < 0)
		return -EINVAL;

	pdata->volt_len = len;
	return 0;
}

static int rt9426_get_segment_para_data(struct chrg_para_lut *pdata)
{
	int len;

	if (!pdata)
		return -EINVAL;

	len = rt9426_read_dts_string_array("segment_para", &pdata->segment_data[0][0],
		SEGMENT_PARA_LEVEL, SEGMENT_PARA_TOTAL);
	if (len < 0)
		return -EINVAL;

	pdata->segment_len = len;
	return 0;
}

static int rt9426_get_batt_para(struct chrg_para_lut *pdata)
{
	int ret;

	ret = rt9426_get_temp_para_data(pdata);
	if (ret)
		return -EINVAL;
	ret = rt9426_get_vbat_para_data(pdata);
	if (ret)
		return -EINVAL;
	ret = rt9426_get_segment_para_data(pdata);
	if (ret)
		return -EINVAL;

	return 0;
}

static struct coulometer_ops rt9426_coul_drv_ops = {
	.battery_id_voltage = rt9426_get_battery_id_vol,
	.is_coul_ready = rt9426_coul_is_ready,
	.is_battery_exist = rt9426_coul_is_battery_exist,
	.is_battery_reach_threshold = rt9426_is_battery_reach_threshold,
	.battery_brand = rt9426_get_battery_brand,
	.battery_voltage = rt9426_get_battery_vol,
	.battery_voltage_uv = rt9426_get_battery_vol_uv,
	.battery_current = rt9426_get_battery_curr,
	.fifo_avg_current = rt9426_get_battery_curr,
	.battery_current_avg = rt9426_get_battery_avgcurr,
	.battery_unfiltered_capacity = rt9426_battery_unfiltered_soc,
	.battery_capacity = rt9426_get_battery_soc,
	.battery_temperature = rt9426_get_battery_temp,
	.battery_rm = rt9426_get_battery_rm,
	.battery_fcc = rt9426_get_battery_fcc,
	.battery_tte = rt9426_get_battery_tte,
	.battery_ttf = rt9426_get_battery_ttf,
	.battery_health = rt9426_get_battery_health,
	.battery_capacity_level = rt9426_get_battery_capacity_level,
	.battery_technology = rt9426_get_battery_technology,
	.battery_charge_params = rt9426_get_battery_charge_params,
	.battery_vbat_max = rt9426_get_battery_vbat_max,
	.charger_event_rcv = rt9426_battery_charger_event_rcv,
	.coul_is_fcc_debounce = rt9426_is_fcc_debounce,
	.battery_cycle_count = rt9426_get_battery_cycle,
	.battery_fcc_design = rt9426_get_battery_dc,
	.dev_check = rt9426_device_check,
	.battery_temperature_for_charger = rt9426_get_battery_temp,
	.update_basp_policy = rt9426_update_basp_policy,
};

int rt9426_adapt_kirin_platform(struct rt9426_chip *chip)
{
	int ret;

	rt9426_register_psy_desc(chip);
	if (!chip->pdata || (chip->pdata->ic_role != RT9426_IC_TYPE_MAIN))
		return -EINVAL;

	ret = rt9426_check_batt_id_voltage(chip);
	if (ret)
		goto out_fail;
	chip->para_batt_data = kzalloc(sizeof(struct chrg_para_lut), GFP_KERNEL);
	if (!chip->para_batt_data)
		goto out_fail;

	ret = rt9426_get_batt_para(chip->para_batt_data);
	if (ret)
		goto get_para_fail;

	ret = coul_drv_coul_ops_register(&rt9426_coul_drv_ops, COUL_RT9426);
	if (ret)
		goto register_coul_ops_fail;

	return ret;
register_coul_ops_fail:
get_para_fail:
	kfree(chip->para_batt_data);
	chip->para_batt_data = NULL;
out_fail:
	hwlog_err("adapt kirin platform fail\n");
	return ret;
}
