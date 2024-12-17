/*
 * echub_battery.c
 *
 * battery driver for echub
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
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

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/power_supply.h>
#include <linux/notifier.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/rtc.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/version.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/regmap.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <securec.h>

#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/ioctl.h>
#include <linux/of_gpio.h>
#include <linux/of_address.h>
#include <linux/interrupt.h>
#include "echub_battery.h"
#include "../echub.h"
#include "../echub_i2c.h"

#define WINDOW_LEN                   (10)
#define WORK_INTERVAL_QUICK                 (1000)

#define BATTERY_CAPACITY_FULL                 (100)
#define BATTERY_CAPACITY_LOW                  (10)
#define BATTERY_CAPACITY_CRITICAL             (3)
#define BATTERY_VOLTAGE_MAX_DESIGN_4S1P       (17600000)
#define BATTERY_VOLTAGE_MAX_DESIGN_2S2P       (8800000)

static unsigned int capacity_filter[WINDOW_LEN];
static unsigned int capacity_sum;
static unsigned int suspend_capacity;

struct echub_batt_device_info {
	int bat_cycle_count;
	int bat_voltage;
	int bat_volt_max_design;
	int bat_volt_avg;
	int bat_temperature;
	int bat_exist;
	int bat_health;
	int bat_capacity;
	int bat_capacity_level;
	int bat_technolog;
	int bat_design_fcc;
	int bat_rm;
	int bat_fcc;
	int bci_soc_at_term;
	int bat_current;
	int gpio_int;
	int irq_int;
	unsigned int bat_err;
	int ac_status_gpio;
	int charge_status;
	int power_supply_status;
	u8 usb_online;
	u8 ac_online;
	u8 chargedone_stat;
	u16 monitoring_interval;
	int watchdog_timer_status;
	unsigned long event;
	unsigned int capacity;
	unsigned int capacity_filter_count;
	unsigned int prev_capacity;
	unsigned int charge_full_count;
	unsigned int wakelock_enabled;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 13, 0))
	struct power_supply bat;
	struct power_supply usb;
	struct power_supply bat_google;
	struct power_supply ac;
	struct power_supply bk_bat;
#else
	struct power_supply    *bat;
	struct power_supply    *usb;
	struct power_supply    *bat_google;
	struct power_supply    *ac;
	struct power_supply    *wireless;
	struct power_supply    *bk_bat;
#endif
	struct device *dev;
	struct echub_i2c_dev *echub_dev;
	struct notifier_block nb;
	struct delayed_work echub_batt_monitor_work;
	struct delayed_work irq_work;
	struct work_interval_para interval_data[WORK_INTERVAL_PARA_LEVEL];
	struct hrtimer capacity_dec_timer;
	int capacity_dec_base_decimal;
	int capacity_dec_sample_nums;
	int capacity_dec_timer_interval;
	int capacity_dec_total_level;
	struct capacity_dec_para cap_dec_para[CAPATICY_DEC_PARA_LEVEL];
	unsigned int capacity_dec;
	struct cap_lock_para vth_correct_data[CAP_LOCK_PARA_LEVEL];
	unsigned int vth_correct_en;
	unsigned int bat_info_type;
	unsigned char low_battery_thres;
};

struct echub_batt_device_info *g_echub_bat_dev;

static enum power_supply_property echub_battery_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_CAPACITY_LEVEL,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN,
	POWER_SUPPLY_PROP_CHARGE_FULL,
	POWER_SUPPLY_PROP_CHARGE_NOW,
	POWER_SUPPLY_PROP_ENERGY_FULL_DESIGN,
	POWER_SUPPLY_PROP_ENERGY_FULL,
	POWER_SUPPLY_PROP_ENERGY_NOW,
	POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN,
	POWER_SUPPLY_PROP_CYCLE_COUNT,
};

static enum power_supply_property echub_ac_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_HEALTH,
};

static void echub_ac_irq_work(struct work_struct *work)
{
	struct echub_batt_device_info *di =
			container_of(work, struct echub_batt_device_info, irq_work.work);
	int ac_status = 0;

	if (di->ac_status_gpio > 0)
		ac_status = gpio_get_value(di->ac_status_gpio);

	if (ac_status) {
		if (di->ac_online != 1) {
			di->ac_online = 1;
			queue_delayed_work(system_power_efficient_wq, &di->echub_batt_monitor_work, msecs_to_jiffies(2000));
			power_supply_changed(di->ac);
			power_supply_changed(di->bat);
		}
		if (di->capacity == BATTERY_CAPACITY_FULL) {
			di->bat_current = 0;
			if (di->charge_status != POWER_SUPPLY_STATUS_FULL) {
				di->charge_status = POWER_SUPPLY_STATUS_FULL;
				queue_delayed_work(system_power_efficient_wq, &di->echub_batt_monitor_work, msecs_to_jiffies(2000));
				power_supply_changed(di->ac);
				power_supply_changed(di->bat);
			}
		} else if (di->bat_current == 0) {
			if (di->charge_status != POWER_SUPPLY_STATUS_NOT_CHARGING) {
				di->charge_status = POWER_SUPPLY_STATUS_NOT_CHARGING;
				queue_delayed_work(system_power_efficient_wq, &di->echub_batt_monitor_work, msecs_to_jiffies(2000));
				power_supply_changed(di->ac);
				power_supply_changed(di->bat);
			}
		} else if (di->charge_status != POWER_SUPPLY_STATUS_CHARGING) {
			di->charge_status = POWER_SUPPLY_STATUS_CHARGING;
			queue_delayed_work(system_power_efficient_wq, &di->echub_batt_monitor_work, msecs_to_jiffies(2000));
			power_supply_changed(di->ac);
			power_supply_changed(di->bat);
		}
	} else if ((ac_status <= 0) && (di->charge_status != POWER_SUPPLY_STATUS_DISCHARGING)) {
		di->charge_status = POWER_SUPPLY_STATUS_DISCHARGING;
		di->ac_online = 0;
		queue_delayed_work(system_power_efficient_wq, &di->echub_batt_monitor_work, msecs_to_jiffies(2000));
		power_supply_changed(di->ac);
		power_supply_changed(di->bat);
	}
	queue_delayed_work(system_power_efficient_wq, &di->irq_work, msecs_to_jiffies(WORK_INTERVAL_QUICK));
}

static void echub_battery_work(struct work_struct *work)
{
	struct echub_batt_device_info *di = container_of(work, struct echub_batt_device_info, echub_batt_monitor_work.work);
	char value[6] = {0};
	int ret = 0;

	ret = di->echub_dev->read_func(di->echub_dev, BATTERY_REG1, BATTERY_CAPACITY_OFFSET, value, 4);
	if (ret)
		goto echub_trans_failed;
	di->capacity = value[2];

	msleep(10);

	// bat_voltage
	ret = di->echub_dev->read_func(di->echub_dev, BATTERY_REG2, BATTERY_VOLTAGE_OFFSET, value, 5);
	if (ret) {
		echub_err("battery: get bat_voltage failed: %d\n", ret);
		goto echub_trans_failed;
	}
	di->bat_voltage = (u16)(value[3]<<8) + value[2];

	msleep(10);

	// bat_current
	ret = di->echub_dev->read_func(di->echub_dev, BATTERY_REG2, BATTERY_CURRENT_OFFSET, value, 5);
	if (ret) {
		echub_err("battery: get bat_current failed: %d\n", ret);
		goto echub_trans_failed;
	}
	di->bat_current = (short)((u16)(value[3]<<8) + (u16)value[2]);
	echub_info("battery: bat_current = %d\n", di->bat_current);

	msleep(10);

	// bat_remaining_capacity
	ret = di->echub_dev->read_func(di->echub_dev, BATTERY_REG2, BATTERY_REMAIN_CAPACITY_OFFSET, value, 5);
	if (ret) {
		echub_err("battery: get bat_rm failed: %d\n", ret);
		goto echub_trans_failed;
	}
	di->bat_rm = (u16)(value[3]<<8) + value[2];

	msleep(10);

	// di->bat_fcc
	ret = di->echub_dev->read_func(di->echub_dev, BATTERY_REG2, BATTERY_FCC_OFFSET, value, 5);
	if (ret) {
		echub_err("battery: get bat_fcc failed: %d\n", ret);
		goto echub_trans_failed;
	}
	di->bat_fcc = (u16)(value[3]<<8) + value[2];

	msleep(10);

	// di->bat_temperature
	ret = di->echub_dev->read_func(di->echub_dev, BATTERY_REG2, BATTERY_TEMPERATURE_OFFSET, value, 5);
	if (ret) {
		echub_err("battery: get bat_temperature failed: %d\n", ret);
		goto echub_trans_failed;
	}
	di->bat_temperature = ((u16)(value[3]<<8) + value[2] - 2730 + 5) / 10;

	msleep(10);

	// di->bat_health
	ret = di->echub_dev->read_func(di->echub_dev, BATTERY_REG1, BATTERY_HEALTH_OFFSET, value, 4);
	if (ret)
		goto echub_trans_failed;
	if ((value[2]>>4) % 2 == 0)
		di->bat_health = POWER_SUPPLY_HEALTH_GOOD;
	else
		di->bat_health = POWER_SUPPLY_HEALTH_UNSPEC_FAILURE;

	if ((value[2]>>5) % 2 == 1)
		di->bat_health = POWER_SUPPLY_HEALTH_OVERHEAT;
	if ((value[2]>>6) % 2 == 1)
		di->bat_health = POWER_SUPPLY_HEALTH_OVERVOLTAGE;

	msleep(10);

	// di->bat_exist
	ret = di->echub_dev->read_func(di->echub_dev, BATTERY_REG1, BATTERY_EXIST_OFFSET, value, 4);
	if (ret)
		goto echub_trans_failed;

	if ((value[2]>>1) % 2 == 1)
		di->bat_exist = 1;
	else
		di->bat_exist = 0;

	msleep(10);

	queue_delayed_work(system_power_efficient_wq, &di->echub_batt_monitor_work, msecs_to_jiffies(di->monitoring_interval));
	power_supply_changed(di->bat);
	return;

echub_trans_failed:
	queue_delayed_work(system_power_efficient_wq, &di->echub_batt_monitor_work, msecs_to_jiffies(di->monitoring_interval));
	return;
}

static int echub_ac_get_property(struct power_supply *psy,
				enum power_supply_property psp,
				union power_supply_propval *val)
{
	struct echub_batt_device_info *di = g_echub_bat_dev;

	if (!di) {
		echub_info("NULL point in [%s]\n", __func__);
		return -EINVAL;
	}

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		if (di->ac_online)
			val->intval = 1;
		else
			val->intval = 0;
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		val->intval = di->power_supply_status;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

/**********************************************************
* Function:		echub_battery_get_property
* Discription:		OS get battery property
* Parameters:		struct power_supply *psy,
					enum power_supply_property psp,
					union power_supply_propval *val,
* return value:	0-sucess or others-fail
**********************************************************/
static int echub_battery_get_property(struct power_supply *psy,
					 enum power_supply_property psp,
					 union power_supply_propval *val)
{
	struct echub_batt_device_info *di = g_echub_bat_dev;

	if (di == NULL) {
		echub_info("NULL point in [%s]\n", __func__);
		return -EINVAL;
	}
	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		if (di->ac_online == 1) {
			if (di->capacity == BATTERY_CAPACITY_FULL) {
				di->bat_current = 0;
				di->charge_status = POWER_SUPPLY_STATUS_FULL;
			} else if (di->bat_current == 0) {
				di->charge_status = POWER_SUPPLY_STATUS_NOT_CHARGING;
			} else if (di->charge_status != POWER_SUPPLY_STATUS_CHARGING) {
				di->charge_status = POWER_SUPPLY_STATUS_CHARGING;
			}
		}
		val->intval = di->charge_status;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = di->bat_voltage * 1000;
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		val->intval = di->bat_info_type ? di->bat_current * 1000 : di->bat_current;
		break;
	case POWER_SUPPLY_PROP_TEMP:
		if (!di->bat_exist)
			val->intval = 0;
		else
			val->intval = di->bat_temperature * 10;
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = di->bat_exist;
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = di->ac_online;
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		if (!di->bat_exist)
			val->intval = POWER_SUPPLY_HEALTH_UNKNOWN;
		else
			val->intval = di->bat_health;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		val->intval = di->capacity;
		break;
	case POWER_SUPPLY_PROP_CAPACITY_LEVEL:
		if (!di->bat_exist)
			di->bat_capacity_level = POWER_SUPPLY_CAPACITY_LEVEL_UNKNOWN;
		else if (di->capacity <= BATTERY_CAPACITY_CRITICAL)
			di->bat_capacity_level = POWER_SUPPLY_CAPACITY_LEVEL_CRITICAL;
		else if (di->capacity <= BATTERY_CAPACITY_LOW)
			di->bat_capacity_level = POWER_SUPPLY_CAPACITY_LEVEL_LOW;
		else if (di->capacity == BATTERY_CAPACITY_FULL)
			di->bat_capacity_level = POWER_SUPPLY_CAPACITY_LEVEL_FULL;
		else
			di->bat_capacity_level = POWER_SUPPLY_CAPACITY_LEVEL_NORMAL;
		val->intval = di->bat_capacity_level;
		break;
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		if (!di->bat_exist)
			di->bat_technolog = POWER_SUPPLY_TECHNOLOGY_UNKNOWN;
		else
			di->bat_technolog = POWER_SUPPLY_TECHNOLOGY_LION;
		val->intval = di->bat_technolog;
		break;
	case POWER_SUPPLY_PROP_CHARGE_NOW:
		val->intval = di->bat_info_type ? di->bat_rm * 1000 : di->bat_rm;
		break;
	case POWER_SUPPLY_PROP_CHARGE_FULL:
		val->intval = di->bat_info_type ? di->bat_fcc * 1000 : di->bat_fcc;
		break;
	case POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN:
		if (!di->bat_exist)
			val->intval = 0;
		else
			val->intval = di->bat_info_type ? di->bat_design_fcc * 1000 : di->bat_design_fcc;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN:
		if (!di->bat_exist)
			val->intval = 0;
		else
			val->intval = di->bat_volt_max_design;
		break;
	case POWER_SUPPLY_PROP_ENERGY_FULL:
		val->intval = di->bat_fcc * di->bat_volt_avg;
		break;
	case POWER_SUPPLY_PROP_ENERGY_NOW:
		val->intval = di->bat_rm * di->bat_volt_avg;
		break;
	case POWER_SUPPLY_PROP_ENERGY_FULL_DESIGN:
		if (!di->bat_exist)
			val->intval = 0;
		else
			val->intval = di->bat_design_fcc * di->bat_volt_avg;
		break;
	case POWER_SUPPLY_PROP_CYCLE_COUNT:
		val->intval = di->bat_cycle_count;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static char *echub_supplied_to[] = {
	"echub_battery",
};

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 13, 0))
static const struct power_supply_config echub_battery_cfg = {
	.supplied_to = echub_supplied_to,
	.num_supplicants = ARRAY_SIZE(echub_supplied_to),
};

static const struct power_supply_desc echub_battery_desc = {
	.name			= "Battery",
	.type			= POWER_SUPPLY_TYPE_BATTERY,
	.properties		= echub_battery_props,
	.num_properties		= ARRAY_SIZE(echub_battery_props),
	.get_property		= echub_battery_get_property,
};

static const struct power_supply_desc echub_mains_desc = {
	.name			= "Mains",
	.type			= POWER_SUPPLY_TYPE_MAINS,
	.properties		= echub_ac_props,
	.num_properties		= ARRAY_SIZE(echub_ac_props),
	.get_property		= echub_ac_get_property,
};
#endif
/**********************************************************
*  Function:       parse_cap_dec_dts
*  Discription:    parse the module dts config value of capacity decimal
*  Parameters:   np:device_node
*                      di:echub_batt_device_info
*  return value:  null
**********************************************************/
static void parse_cap_dec_dts(struct echub_batt_device_info *di)
{
	int ret = 0;
	int i = 0;
	int array_len = 0;
	u32 cap_dec_tmp[CAP_DEC_PARA_TOTAL * CAPATICY_DEC_PARA_LEVEL] = {0};
	struct device_node *np = di->dev->of_node;

	array_len = of_property_count_u32_elems(np, "capacity_dec_para");
	if ((array_len <= 0) || (array_len % CAP_DEC_PARA_TOTAL != 0)) {
		echub_err("capacity_dec_para is invaild, please check capacity_dec_para number!!\n");
		goto FuncEnd;
	} else if (array_len > CAPATICY_DEC_PARA_LEVEL * CAP_DEC_PARA_TOTAL) {
		echub_err("capacity_dec_para is too long, please check capacity_dec_para number!!\n");
		goto FuncEnd;
	} else {
		ret = of_property_read_u32_array(np, "capacity_dec_para", cap_dec_tmp, array_len);
		if (ret) {
			echub_err("dts:get capacity_dec_para fail!\n");
			goto FuncEnd;
		} else {
			di->capacity_dec_total_level = array_len / CAP_DEC_PARA_TOTAL;
			for (i = 0; i < di->capacity_dec_total_level; i++) {
				di->cap_dec_para[i].cap_dec_base_decimal =
					cap_dec_tmp[(int)(CAP_DEC_BASE_DECIMAL + CAP_DEC_PARA_TOTAL * i)];
				di->cap_dec_para[i].cap_dec_sample_nums =
					cap_dec_tmp[(int)(CAP_DEC_SAMPLE_NUMS + CAP_DEC_PARA_TOTAL * i)];
				di->cap_dec_para[i].cap_dec_timer_interval =
					cap_dec_tmp[(int)(CAP_DEC_TIMER_INTERVAL + CAP_DEC_PARA_TOTAL * i)];
				echub_info("cap_dec_para[%d], base_decimal: %-3d sample_nums: %-3d timer_interval: %-3d\n",
						i, di->cap_dec_para[i].cap_dec_base_decimal,
						di->cap_dec_para[i].cap_dec_sample_nums, di->cap_dec_para[i].cap_dec_timer_interval);
			}
			return;
		}
	}

FuncEnd:
	di->capacity_dec_total_level = 1;
	di->cap_dec_para[0].cap_dec_base_decimal = BASE_DECIMAL;
	di->cap_dec_para[0].cap_dec_sample_nums = CAPACITY_DEC_SAMPLE_NUMS;
	di->cap_dec_para[0].cap_dec_timer_interval = CAPATICY_DEC_TIMER_INTERVAL;
	echub_info("cap_dec_para[0], base_decimal: %-3d sample_nums: %-3d timer_interval: %-3d\n",
			di->cap_dec_para[0].cap_dec_base_decimal,
			di->cap_dec_para[0].cap_dec_sample_nums, di->cap_dec_para[0].cap_dec_timer_interval);
}
/**********************************************************
*  Function:       parse_cap_lock_dts
*  Discription:    parse the module dts config value
*  Parameters:   np:device_node
*                      di:echub_batt_device_info
*  return value:  0-sucess or others-fail
**********************************************************/
static int parse_vth_correct_dts(struct echub_batt_device_info *di)
{
	int i = 0;
	struct device_node *np = di->dev->of_node;

	if (of_property_read_u32(np, "vth_correct_en", (u32 *)&di->vth_correct_en)) {
		di->vth_correct_en = 0;
		echub_err("error:get vth_correct_en value failed, used default value!\n");
	}

	if (of_property_read_u32_array(np, "vth_correct_para",
			(u32 *)&di->vth_correct_data[0],
			sizeof(di->vth_correct_data) / sizeof(int))) {
		di->vth_correct_en = 0;
		echub_err("get vth_correct_para fail!!\n");
		return -EINVAL;
	}

	for (i = 0; i < CAP_LOCK_PARA_LEVEL; i++)
		echub_info("di->vth_correct_data[%d]:%d,%d\n", i,
			di->vth_correct_data[i].cap, di->vth_correct_data[i].level_vol);

	return 0;
}

/**********************************************************
*  Function:       get_ac_status_gpio
*  Discription:    get and init the ac_status_gpio
*  Parameters:   np:device_node
*                      di:echub_batt_device_info
*  return value:  0-sucess or others-fail
**********************************************************/
static int get_ac_status_gpio(struct echub_batt_device_info *di)
{
	struct device_node *np = di->dev->of_node;

	di->ac_status_gpio = of_get_named_gpio(np, "ac_status_gpio", 0);
	if (di->ac_status_gpio < 0) {
		echub_err("Get ac_status_gpio failed\n");
		return -EINVAL;
	}

	if (!gpio_is_valid(di->ac_status_gpio)) {
		echub_err("ac_status_gpio is not valid\n");
		return -EINVAL;
	}

	if (devm_gpio_request(di->dev, di->ac_status_gpio, "ac_status_gpio")) {
		echub_err("Could not request %d gpio\n", di->ac_status_gpio);
		return -EINVAL;
	} else if (gpio_direction_input(di->ac_status_gpio)) {
		echub_err("Could not set gpio %d as input\n", di->ac_status_gpio);
		return -EINVAL;
	}

	echub_info("ac_status_gpio = %d\n", di->ac_status_gpio);
	return 0;
}

static void get_battery_static_paras_from_ec(struct echub_batt_device_info *di)
{
	int ret = 0;
	char temp = 0;
	char value[4] = {0};

	if (di == NULL)
		return;

	// bat_design_fcc
	ret = di->echub_dev->read_func(di->echub_dev, BATTERY_REG1, BATTERY_DESIGN_FCC_OFFSET1, value, 4);
	if (ret)
		echub_info("get bat_design_fcc failed %d\n", ret);
	temp = value[2];
	ret = di->echub_dev->read_func(di->echub_dev, BATTERY_REG1, BATTERY_DESIGN_FCC_OFFSET2, value, 4);
	if (ret)
		echub_info("get bat_design_fcc failed %d\n", ret);
	di->bat_design_fcc = (u16)(value[2]<<8) + temp;
	echub_info("di->bat_design_fcc = %d\n", di->bat_design_fcc);

	// get bat_volt_avg
	ret = di->echub_dev->read_func(di->echub_dev, BATTERY_REG1, BATTERY_VOLT_AVG_OFFSET1, value, 4);
	if (ret)
		echub_info("get bat_volt_avg failed %d\n", ret);
	temp = value[2];
	ret = di->echub_dev->read_func(di->echub_dev, BATTERY_REG1, BATTERY_VOLT_AVG_OFFSET2, value, 4);
	if (ret)
		echub_info("get bat_volt_avg failed %d\n", ret);
	di->bat_volt_avg = (u16)(value[2]<<8) + temp;
	echub_info("di->bat_volt_avg = %d\n", di->bat_volt_avg);

	// bat_cycle_count
	ret = di->echub_dev->read_func(di->echub_dev, BATTERY_REG1, BATTERY_CYCLE_COUNT_OFFSET1, value, 4);
	if (ret)
		echub_info("get bat_cycle_count failed %d\n", ret);
	temp = value[2];
	ret = di->echub_dev->read_func(di->echub_dev, BATTERY_REG1, BATTERY_CYCLE_COUNT_OFFSET2, value, 4);
	if (ret)
		echub_info("get bat_cycle_count failed %d\n", ret);
	di->bat_cycle_count = (short)((u16)(value[2]<<8) + (u16)temp);
	echub_info("di->bat_cycle_count = %d\n", di->bat_cycle_count);
}

static int device_info_interval_data_init(struct echub_batt_device_info *di, int array_len)
{
	int idata = 0;
	int i = 0;
	int ret = 0;
	const char *bci_data_string = NULL;

	if (!di || !di->dev)
		return -EINVAL;

	for (i = 0; i < array_len; i++) {
		ret = of_property_read_string_index(di->dev->of_node, "bci_work_interval_para", i, &bci_data_string);
		if (ret) {
			echub_err("get bci_work_interval_para failed\n");
			return -EINVAL;
		}

		idata = simple_strtol(bci_data_string, NULL, 10);
		switch (i % WORK_INTERVAL_PARA_TOTAL) {
		case WORK_INTERVAL_CAP_MIN:
			if ((idata < CAPACITY_MIN) || (idata > CAPACITY_MAX)) {
				echub_err("the bci_work_interval_para cap_min is out of range!!\n");
				return -EINVAL;
			}
			di->interval_data[i / (WORK_INTERVAL_PARA_TOTAL)].cap_min = idata;
			break;
		case WORK_INTERVAL_CAP_MAX:
			if ((idata < CAPACITY_MIN) || (idata > CAPACITY_MAX)) {
				echub_err("the bci_work_interval_para cap_max is out of range!!\n");
				return -EINVAL;
			}
			di->interval_data[i / (WORK_INTERVAL_PARA_TOTAL)].cap_max = idata;
			break;
		case WORK_INTERVAL_VALUE:
			if ((idata < WORK_INTERVAL_MIN) || (idata > WORK_INTERVAL_MAX)) {
				echub_err("the bci_work_interval_para work_interval is out of range!!\n");
				return -EINVAL;
			}
			di->interval_data[i / (WORK_INTERVAL_PARA_TOTAL)].work_interval = idata;
			break;
		}
		echub_info("di->interval_data[%d][%d] = %d\n",
					i / (WORK_INTERVAL_PARA_TOTAL), i % (WORK_INTERVAL_PARA_TOTAL), idata);
	}

	return 0;
}

/**********************************************************
*  Function:       echub_batt_parse_dts
*  Discription:    parse the module dts config value
*  Parameters:   np:device_node
*                      di:echub_batt_device_info
*  return value:  0-sucess or others-fail
**********************************************************/
static int echub_batt_parse_dts(struct echub_batt_device_info *di)
{
	int ret = 0;
	int array_len = 0;
	struct device_node *np = di->dev->of_node;

	if (of_property_read_u32(np, "bci_soc_at_term", (u32 *)&di->bci_soc_at_term)) {
		di->bci_soc_at_term = 100;
		echub_err("error:get bci_soc_at_term value failed, no early term in bci !\n");
	}
	echub_info("bci_soc_at_term =%d", di->bci_soc_at_term);

	if (get_ac_status_gpio(di))
		echub_err("get ac_status_gpio failed\n");

	/* bci_work_interval_para */
	array_len = of_property_count_strings(np, "bci_work_interval_para");
	if ((array_len <= 0) || (array_len % WORK_INTERVAL_PARA_TOTAL != 0)) {
		echub_err("bci_work_interval_para is invaild,please check iput number!!\n");
		return -EINVAL;
	}

	if (array_len > WORK_INTERVAL_PARA_LEVEL * WORK_INTERVAL_PARA_TOTAL) {
		echub_err("bci_work_interval_para is too long,use only front %d paras!!\n", array_len);
		return -EINVAL;
	}

	ret = memset_s(di->interval_data, sizeof(di->interval_data),
			0, WORK_INTERVAL_PARA_LEVEL * sizeof(struct work_interval_para));
	if (ret) {
		echub_err("memset_s failed\n");
		return -EINVAL;
	}

	ret = device_info_interval_data_init(di, array_len);
	if (ret)
		return ret;

	if (of_property_read_u32(np, "battery_info_type", (u32 *)&di->bat_info_type))
		di->bat_info_type = 1;

	parse_cap_dec_dts(di);
	ret |= parse_vth_correct_dts(di);
	return ret;
}

struct echub_batt_device_info *create_battery_device_info(struct platform_device *pdev, struct echub_i2c_dev *echub_dev)
{
	struct echub_batt_device_info *di = NULL;

	if (!pdev || !echub_dev)
		return NULL;

	di = devm_kzalloc(&pdev->dev, sizeof(struct echub_batt_device_info), GFP_KERNEL);
	if (!di)
		return NULL;

	di->monitoring_interval = WORK_INTERVAL_NOARMAL;
	di->dev = &pdev->dev;
	di->echub_dev = echub_dev;
	di->bat_health = POWER_SUPPLY_HEALTH_GOOD;
	di->bat_err = 0;
	di->power_supply_status = POWER_SUPPLY_HEALTH_GOOD;
	di->charge_status = POWER_SUPPLY_STATUS_NOT_CHARGING;

	get_battery_static_paras_from_ec(di);

	/* Battery is 4S1S or 2S2S */
	if (di->bat_volt_avg < 8000) {
		echub_info("Battery is 2S\n");
		di->bat_volt_max_design = BATTERY_VOLTAGE_MAX_DESIGN_2S2P;
	} else if (di->bat_volt_avg < 12000) {
		echub_info("Battery is 3S\n");
		di->bat_volt_max_design = BATTERY_VOLTAGE_MAX_DESIGN_4S1P;
	} else {
		echub_info("Battery is 4S\n");
		di->bat_volt_max_design = BATTERY_VOLTAGE_MAX_DESIGN_4S1P;
	}

	di->capacity = -1;
	di->capacity_filter_count = 0;
	di->charge_full_count = 0;

	return di;
}

static ssize_t low_battery_thres_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "low battery thres:%d\n", g_echub_bat_dev->low_battery_thres);
}

static ssize_t low_battery_thres_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	struct echub_batt_device_info *di = g_echub_bat_dev;
	int battery_thres = 0x0;

	ret = kstrtouint(buf, 0, &battery_thres);
	if (ret < 0)
		return ret;

	if ((battery_thres < CAPACITY_MIN) || (battery_thres > CAPACITY_MAX)) {
		echub_err("battery_thres is invalid\n");
		return -EINVAL;
	}
	di->low_battery_thres = battery_thres;
	echub_info("low battery thres : %d\n", di->low_battery_thres);

	ret = di->echub_dev->write_func(di->echub_dev, SET_LOW_BATTERY_ADDR, di->low_battery_thres);
	if (ret) {
		echub_err("set low battery thres failed: %d\n", ret);
		return ret;
	}

	return count;
}
static DEVICE_ATTR(low_battery_thres, 0660, low_battery_thres_show, low_battery_thres_store);

static ssize_t wakeup_cause_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int ret;
	char value[0x4] = {0};
	char wakeup_cause = 0x0;
	struct echub_batt_device_info *di = g_echub_bat_dev;

	ret = di->echub_dev->read_func(di->echub_dev, GET_WAKEUP_CAUSE_ADDR, 0x0, value, 0x4);
	if (ret) {
		echub_err("get wakeup cause failed %d\n", ret);
		return ret;
	}
	wakeup_cause = value[0x2];
	echub_info("wakeup cause %d\n", wakeup_cause);
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n", wakeup_cause);
}
static DEVICE_ATTR(wakeup_cause, 0440, wakeup_cause_show, NULL);

static struct attribute *echub_battery_dev_attrs[] = {
	&dev_attr_low_battery_thres.attr,
	&dev_attr_wakeup_cause.attr,
	NULL,
};

static const struct attribute_group echub_battery_sysfs_attr_group = {
	.attrs = echub_battery_dev_attrs,
};

/**********************************************************
*  Function:       echub_battery_probe
*  Discription:    module probe
*  Parameters:     platform_device, i2c_device_id
*  return value:   0-sucess or others-fail
**********************************************************/
static int echub_battery_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct echub_batt_device_info *di = NULL;
	struct echub_i2c_dev *echub_dev = dev_get_drvdata(pdev->dev.parent);

	if (echub_dev == NULL) {
		echub_err("echub_dev is null, get echub_dev form tmp\n");
		echub_dev = get_echub_dev();
		if (echub_dev == NULL) {
			echub_err("echub_dev is NULL return\n");
			return -ENODEV;
		}
	}

	echub_info("echub_battery Probe start\n");

	di = create_battery_device_info(pdev, echub_dev);
	if (!di)
		return -ENOMEM;

	g_echub_bat_dev = di;

	hrtimer_init(&di->capacity_dec_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);

	platform_set_drvdata(pdev, di);

	ret = echub_batt_parse_dts(di);
	if (ret)
		echub_err("battery parse dts failed, please check\n");

	di->bat = power_supply_register(&pdev->dev, &echub_battery_desc, &echub_battery_cfg);
	if (IS_ERR(di->bat)) {
		echub_info("failed to register main battery\n");
		goto batt_failed;
	}

	di->ac = power_supply_register(&pdev->dev, &echub_mains_desc, NULL);
	if (IS_ERR(di->ac)) {
		echub_info("failed to register ac power supply\n");
		goto ac_failed;
	}

	INIT_DELAYED_WORK(&di->irq_work, echub_ac_irq_work);
	INIT_DELAYED_WORK(&di->echub_batt_monitor_work, echub_battery_work);
	queue_delayed_work(system_power_efficient_wq, &di->irq_work, 0);
	queue_delayed_work(system_power_efficient_wq, &di->echub_batt_monitor_work, 0);

	ret = sysfs_create_group(&pdev->dev.kobj, &echub_battery_sysfs_attr_group);
	if (ret < 0)
		echub_info("register sysfs echub_battery failed\n");

	echub_info("echub_batt probe ok!\n");

	return 0;

ac_failed:
	power_supply_unregister(di->usb);
batt_failed:
	platform_set_drvdata(pdev, NULL);
	kfree(di);
	di = NULL;
	return ret;
}

static int echub_battery_remove(struct platform_device *pdev)
{
	struct echub_batt_device_info *di = platform_get_drvdata(pdev);

	pr_info("remove\n");

	if (di == NULL) {
		echub_err("di is NULL!\n");
		return -ENODEV;
	}

	sysfs_remove_group(&pdev->dev.kobj, &echub_battery_sysfs_attr_group);
	cancel_delayed_work(&di->echub_batt_monitor_work);
	platform_set_drvdata(pdev, NULL);
	kfree(di);
	di = NULL;

	return 0;
}

#ifdef CONFIG_PM
static int echub_battery_suspend(struct platform_device *pdev,
				    pm_message_t state)
{
	struct echub_batt_device_info *di = platform_get_drvdata(pdev);

	if (di == NULL) {
		echub_err("di is NULL!\n");
		return -ENODEV;
	}
	echub_info("%s:+\n", __func__);
	cancel_delayed_work(&di->echub_batt_monitor_work);
	echub_info("%s:-\n", __func__);
	return 0;
}

static int echub_battery_resume(struct platform_device *pdev)
{
	struct echub_batt_device_info *di = platform_get_drvdata(pdev);
	int i = 0;
	int resume_capacity = 0;

	if (di == NULL) {
		echub_err("di is NULL!\n");
		return -ENODEV;
	}

	echub_info("%s:+\n", __func__);

	if (di->charge_status == POWER_SUPPLY_STATUS_DISCHARGING
	    || di->charge_status == POWER_SUPPLY_STATUS_NOT_CHARGING) {
		if ((suspend_capacity - resume_capacity) >= 2) {
			capacity_sum = 0;
			for (i = 0; i < WINDOW_LEN; i++) {
				capacity_filter[i] = resume_capacity;
				capacity_sum += capacity_filter[i];
			}
		}
	}
	queue_delayed_work(system_power_efficient_wq, &di->echub_batt_monitor_work, 0);
	echub_info("%s:-\n", __func__);
	return 0;
}
#endif /* CONFIG_PM */

static struct of_device_id echub_battery_match_table[] = {
	{ .compatible = "huawei,echub_battery", .data = NULL },
	{},
};

static struct platform_driver echub_battery_driver = {
	.probe = echub_battery_probe,
	.remove = echub_battery_remove,
#ifdef CONFIG_PM
	.suspend = echub_battery_suspend,
	.resume = echub_battery_resume,
#endif
	.driver = {
		   .name = "huawei,echub_battery",
		   .owner = THIS_MODULE,
		   .of_match_table = of_match_ptr(echub_battery_match_table),
		   },
};

static int __init echub_battery_init(void)
{
	return platform_driver_register(&echub_battery_driver);
}

module_init(echub_battery_init);

static void __exit echub_battery_exit(void)
{
	platform_driver_unregister(&echub_battery_driver);
}

module_exit(echub_battery_exit);

MODULE_LICENSE("GPL");
MODULE_ALIAS("armpc echub_battery");
