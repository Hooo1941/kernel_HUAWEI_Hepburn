// SPDX-License-Identifier: GPL-2.0
/*
 * bq40z50_base.c
 *
 * bq40z50_base driver
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

#include "bq40z50.h"
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_log.h>
#include <chipset_common/hwpower/battery/battery_model_public.h>
#include <chipset_common/hwpower/coul/coul_interface.h>
#ifdef CONFIG_HUAWEI_BATTERY_VOLTAGE
#include <huawei_platform/power/battery_voltage.h>
#endif

#define HWLOG_TAG bq40z50
HWLOG_REGIST();

int bq40z50_is_ready_base(void *dev_data)
{
	return dev_data != NULL;
}

int bq40z50_get_battery_temp_base(void *dev_data)
{
	int ret;
	int temp_c;
	u16 temp_k = 0;
	struct bq40z50_device_info *di = dev_data;

	if (!di)
		return BQ40Z50_BATT_TEMP_ABNORMAL_LOW - 1;

	ret = bq40z50_read_word(di, BQ40Z50_REG_TEMP, &temp_k);
	if (ret) {
		temp_c = di->cache.temp;
	} else {
		temp_c = (int)temp_k / POWER_BASE_DEC - BQ40Z50_BATT_TEMP_ZERO;
		if (((temp_c <= BQ40Z50_BATT_TEMP_ABNORMAL_LOW) ||
			(temp_c >= BQ40Z50_BATT_TEMP_ABNORMAL_HIGH)) &&
			(di->abnormal_temp_count < BQ40Z50_BATT_TEMP_ABNORMAL_MAX_CNT)) {
			di->abnormal_temp_count++;
			temp_c = di->cache.temp;
		} else {
			di->abnormal_temp_count = 0;
			di->cache.temp = temp_c;
		}
	}

	/* unit degree * 10 */
	hwlog_info("battery_temp=%d\n", temp_c * 10);
	return temp_c * 10;
}

int bq40z50_get_cell_temp_base(struct bq40z50_device_info *di, u8 reg_offset, s16 *temp_cache)
{
	int ret;
	int temp_c;
	u16 temp_k;
	char temp_data[BQ40Z50_DASTATUS_BLOCK_LEN] = { 0 };
	u8 buf[BQ40Z50_MANUFACTURER_CMD_LEN] = { BQ40Z50_REG_DASTATUS2, 0x00 };

	if (!di || !temp_cache)
		return BQ40Z50_BATT_TEMP_ABNORMAL_LOW - 1;

	ret = bq40z50_write_block(di, BQ40Z50_REG_MANUFACTURER_SEND, buf, BQ40Z50_MANUFACTURER_CMD_LEN);
	if (!ret)
		ret = bq40z50_read_block(di, BQ40Z50_REG_MANUFACTURER_READ, (u8 *)&temp_data, BQ40Z50_DASTATUS_BLOCK_LEN);
	if (ret) {
		temp_c = *temp_cache;
	} else {
		temp_k = temp_data[reg_offset] + (temp_data[reg_offset + 1] << 8);
		temp_c = (int)temp_k / POWER_BASE_DEC - BQ40Z50_BATT_TEMP_ZERO;
		if (((temp_c <= BQ40Z50_BATT_TEMP_ABNORMAL_LOW) ||
			(temp_c >= BQ40Z50_BATT_TEMP_ABNORMAL_HIGH)) &&
			(di->abnormal_temp_count < BQ40Z50_BATT_TEMP_ABNORMAL_MAX_CNT)) {
			di->abnormal_temp_count++;
			temp_c = *temp_cache;
		} else {
			di->abnormal_temp_count = 0;
			*temp_cache = temp_c;
		}
	}

	hwlog_info("battery_cell_temp=%d, reg_offset:%u\n", temp_c, reg_offset);
	return temp_c;
}

#ifdef CONFIG_HLTHERM_RUNTEST
int bq40z50_is_battery_exist_base(void *dev_data)
{
	return 0;
}
#else
int bq40z50_is_battery_exist_base(void *dev_data)
{
	int temp;
	struct bq40z50_device_info *di = dev_data;

	if (!di)
		return 0;

	temp = bq40z50_get_battery_temp_base(di) / 10;
	if ((temp <= BQ40Z50_BATT_TEMP_ABNORMAL_LOW) ||
		(temp >= BQ40Z50_BATT_TEMP_ABNORMAL_HIGH))
		return 0;

	return 1;
}
#endif /* CONFIG_HLTHERM_RUNTEST */

int bq40z50_get_battery_soc_base(void *dev_data)
{
	int ret;
	u16 soc = 0;
	struct bq40z50_device_info *di = dev_data;

	if (!di)
		return (int)soc;

	if (di->charge_status == BQ40Z50_CHARGE_STATE_CHRG_DONE) {
		hwlog_info("charge done, force soc to full\n");
		return BQ40Z50_BATT_CAPACITY_FULL;
	}

	ret = bq40z50_read_word(di, BQ40Z50_REG_SOC, &soc);
	if (ret)
		soc = di->cache.soc;
	else
		di->cache.soc = soc;

	hwlog_info("soc=%u\n", soc);
	return (int)soc;
}

int bq40z50_get_battery_vol_base(void *dev_data)
{
	u16 vol = 0;
	int ret;
	struct bq40z50_device_info *di = dev_data;

	if (!di)
		return (int)vol;

	ret = bq40z50_read_word(di, BQ40Z50_REG_VOLT, &vol);
	if (ret)
		vol = di->cache.vol;
	else
		di->cache.vol = vol;

	hwlog_info("battery_vol=%u\n", vol);
	return (int)vol;
}

int bq40z50_get_cell_vbat_mv_base(struct bq40z50_device_info *di, u8 reg, u16 *vol_cache)
{
	u16 vol = 0;
	int ret;

	if (!di || !vol_cache)
		return (int)vol;

	ret = bq40z50_read_word(di, reg, &vol);
	if (ret)
		vol = *vol_cache;
	else
		*vol_cache = vol;

	hwlog_info("cell_battery_vol=%u, reg=0x%x\n", vol, reg);
	return (int)vol;
}

int bq40z50_get_battery_curr_base(void *dev_data)
{
	short curr = 0;
	int ret;
	struct bq40z50_device_info *di = dev_data;

	if (!di)
		return (int)curr;

	ret = bq40z50_read_word(di, BQ40Z50_REG_CURR, &curr);
	if (ret)
		curr = di->cache.curr;
	else
		di->cache.curr = curr;

	hwlog_info("battery_cur=%d\n", curr);
	return (int)curr;
}

int bq40z50_get_battery_avgcurr_base(void *dev_data)
{
	short avg_curr = 0;
	int ret;
	struct bq40z50_device_info *di = dev_data;

	if (!di)
		return (int)avg_curr;

	ret = bq40z50_read_word(di, BQ40Z50_REG_AVRGCURR, &avg_curr);
	if (ret)
		avg_curr = di->cache.avg_curr;
	else
		di->cache.avg_curr = avg_curr;

	hwlog_info("battery_avgcur=%d\n", avg_curr);
	return (int)avg_curr;
}

int bq40z50_get_battery_rm_base(void *dev_data)
{
	u16 rm = 0;
	int ret;
	struct bq40z50_device_info *di = dev_data;

	if (!di)
		return (int)rm;

	ret = bq40z50_read_word(di, BQ40Z50_REG_RM, &rm);
	if (ret)
		rm = di->cache.rm;
	else
		di->cache.rm = rm;

	hwlog_info("battery_rm=%u\n", rm);
	return (int)rm;
}

int bq40z50_get_battery_dc_base(void *dev_data)
{
	u16 dc = 0;
	int ret;
	struct bq40z50_device_info *di = dev_data;

	if (!di)
		return (int)dc;

	ret = bq40z50_read_word(di, BQ40Z50_REG_DC, &dc);
	if (ret)
		dc = di->cache.dc;
	else
		di->cache.dc = dc;

	hwlog_info("battery_dc=%u\n", dc);
	return (int)dc;
}

int bq40z50_get_battery_fcc_base(void *dev_data)
{
	u16 fcc = 0;
	int ret;
	struct bq40z50_device_info *di = dev_data;

	if (!di)
		return (int)fcc;

	ret = bq40z50_read_word(di, BQ40Z50_REG_FCC, &fcc);
	if (ret)
		fcc = di->cache.fcc;
	else
		di->cache.fcc = fcc;

	hwlog_info("battery_fcc=%u\n", fcc);
	return (int)fcc;
}

int bq40z50_get_battery_cycle_base(void *dev_data)
{
	u16 cycle = 0;
	int ret;
	struct bq40z50_device_info *di = dev_data;

	if (!di)
		return (int)cycle;

	ret = bq40z50_read_word(di, BQ40Z50_REG_CYCLE, &cycle);
	if (ret)
		cycle = di->cache.cycle;
	else
		di->cache.cycle = cycle;

	hwlog_info("battery_cycle=%u\n", cycle);
	return (int)cycle;
}

int bq40z50_set_battery_low_voltage_base(int val, void *dev_data)
{
	struct bq40z50_device_info *di = dev_data;

	if (!di)
		return -EPERM;

	return 0;
}

int bq40z50_set_last_capacity_base(int capacity, void *dev_data)
{
	return 0;
}

int bq40z50_get_last_capacity_base(void *dev_data)
{
	int soc_at_term, ui_cap_zero_offset, cap;
	struct bq40z50_device_info *di = dev_data;

	if (!di)
		return -EPERM;

	(void)power_dts_read_u32_compatible(power_dts_tag(HWLOG_TAG),
		"huawei,battery_ui_capacity", "soc_at_term", &soc_at_term, BQ40Z50_BATT_CAPACITY_FULL);
	(void)power_dts_read_u32_compatible(power_dts_tag(HWLOG_TAG),
		"huawei,battery_ui_capacity", "ui_cap_zero_offset", &ui_cap_zero_offset, 0);

	/* no customize register, use current soc and terminate soc algorithm */
	cap = bq40z50_get_battery_soc_base(di);
	/* uisoc = soc/soc_at_term - ui_cap_zero_offset */
	if (soc_at_term)
		cap = DIV_ROUND_CLOSEST((cap * BQ40Z50_BATT_CAPACITY_FULL), soc_at_term) - ui_cap_zero_offset;

	if (cap < 0)
		cap = 0;
	else if (cap > BQ40Z50_BATT_CAPACITY_FULL)
		cap = BQ40Z50_BATT_CAPACITY_FULL;

	return cap;
}

static int bq40z50_get_battery_charge_counter(void *dev_data)
{
	return bq40z50_get_battery_rm_base(dev_data);
}

int bq40z50_get_desired_charging_current_base(void *dev_data)
{
	u16 curr = 0;
	int ret;
	struct bq40z50_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = bq40z50_read_word(di, BQ40Z50_REG_CHARGING_CURRENT, &curr);
	if (ret)
		curr = di->cache.charge_current;
	else
		di->cache.charge_current = curr;

	hwlog_info("battery charging current=%u\n", curr);
	return (int)curr;
}

int bq40z50_get_desired_charging_voltage_base(void *dev_data)
{
	u16 vol = 0;
	int ret;
	struct bq40z50_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	ret = bq40z50_read_word(di, BQ40Z50_REG_CHARGING_VOLTAGE, &vol);
	if (ret)
		vol = di->cache.charge_voltage;
	else
		di->cache.charge_voltage = vol;

	hwlog_info("battery charging voltage=%u\n", vol);
	return (int)vol;
}

bool bq40z50_is_smart_battery_base(void *dev_data)
{
	struct bq40z50_device_info *di = dev_data;

	if (!di)
		return false;

	return di->is_smart_battery;
}

const char *bq40z50_get_battery_brand_base(void *dev_data)
{
	struct bq40z50_device_info *di = dev_data;

	if (!di)
		return "UNKNOWN";

	switch (di->device_id) {
	case BATTERY_FUEL_BQ40Z50:
		return "BITCGLT";
	case BATTERY_FUEL_SH366006:
		return "BITCGLS";
	default:
		return "UNKNOWN";
	}
}

int bq40z50_get_battery_vbat_max_base(void *dev_data)
{
	struct bq40z50_device_info *di = dev_data;

	if (!di)
		return -ENODEV;

	hwlog_info("battery_vbat_max=%d\n", di->vbat_max);
	return di->vbat_max;
}

const char *bq40z50_get_coul_model_base(void *dev_data)
{
	return bq40z50_get_battery_brand_base(dev_data);
}

int bq40z50_get_battery_status_base(struct bq40z50_device_info *di)
{
	u16 status = 0;
	int ret;

	if (!di)
		return -ENODEV;

	ret = bq40z50_read_word(di, BQ40Z50_REG_BATTERY_STATUS, &status);
	if (ret)
		status = di->cache.status;
	else
		di->cache.status = status;

	return (int)status;
}

int bq40z50_get_safety_status_base(struct bq40z50_device_info *di)
{
	int ret;
	u8 data[BQ40Z50_STATUS_BLOCK_LEN] = { 0 };
	u8 buf[BQ40Z50_MANUFACTURER_CMD_LEN] = { BQ40Z50_REG_SAFETY_STATUS, 0x00 };

	ret = bq40z50_write_block(di, BQ40Z50_REG_MANUFACTURER_SEND, buf, BQ40Z50_MANUFACTURER_CMD_LEN);
	if (ret < 0) {
		hwlog_err("write command 0x%x fail\n", BQ40Z50_REG_SAFETY_STATUS);
		return ret;
	}
	ret = bq40z50_read_block(di, BQ40Z50_REG_MANUFACTURER_READ, (u8 *)&data, BQ40Z50_STATUS_BLOCK_LEN);
	if (ret < 0) {
		hwlog_err("read safety status fail\n");
		return ret;
	}

	return (int)conver2u32(data);
}

int bq40z50_get_pf_status_base(struct bq40z50_device_info *di)
{
	int ret;
	u8 data[BQ40Z50_STATUS_BLOCK_LEN] = { 0 };
	u8 buf[BQ40Z50_MANUFACTURER_CMD_LEN] = { BQ40Z50_REG_PF_STATUS, 0x00 };

	ret = bq40z50_write_block(di, BQ40Z50_REG_MANUFACTURER_SEND, buf, BQ40Z50_MANUFACTURER_CMD_LEN);
	if (ret < 0) {
		hwlog_err("write command 0x%x fail\n", BQ40Z50_REG_PF_STATUS);
		return ret;
	}
	ret = bq40z50_read_block(di, BQ40Z50_REG_MANUFACTURER_READ, (u8 *)&data, BQ40Z50_STATUS_BLOCK_LEN);
	if (ret < 0) {
		hwlog_err("read pf status fail\n");
		return ret;
	}

	return (int)conver2u32(data);
}

int bq40z50_get_operation_status_base(struct bq40z50_device_info *di)
{
	int ret;
	u8 data[BQ40Z50_STATUS_BLOCK_LEN] = { 0 };
	u8 buf[BQ40Z50_MANUFACTURER_CMD_LEN] = { BQ40Z50_REG_OP_STATUS, 0x00 };

	ret = bq40z50_write_block(di, BQ40Z50_REG_MANUFACTURER_SEND, buf, BQ40Z50_MANUFACTURER_CMD_LEN);
	if (ret < 0) {
		hwlog_err("write command 0x%x fail\n", BQ40Z50_REG_OP_STATUS);
		return ret;
	}
	ret = bq40z50_read_block(di, BQ40Z50_REG_MANUFACTURER_READ, (u8 *)&data, BQ40Z50_STATUS_BLOCK_LEN);
	if (ret < 0) {
		hwlog_err("read op status fail\n");
		return ret;
	}

	return (int)conver2u32(data);
}

static int bq40z50_get_battery_technology_base(void *dev_data)
{
	/* default technology is "Li-poly" */
	return POWER_SUPPLY_TECHNOLOGY_LIPO;
}

static int bq40z50_get_cell1_vbat_mv(void *dev_data)
{
	struct bq40z50_device_info *di = dev_data;
	u16 vol = 0;
	int ret;

	if (!di)
		return (int)vol;

	ret = bq40z50_read_word(di, BQ40Z50_REG_CELL1_VOLT, &vol);
	if (ret)
		vol = di->cache.cell1_vol;
	else
		di->cache.cell1_vol = vol;

	hwlog_info("cell1_battery_vol=%u\n", vol);
	return (int)vol;
}

static int bq40z50_get_cell2_vbat_mv(void *dev_data)
{
	struct bq40z50_device_info *di = dev_data;
	u16 vol = 0;
	int ret;

	if (!di)
		return (int)vol;

	ret = bq40z50_read_word(di, BQ40Z50_REG_CELL2_VOLT, &vol);
	if (ret)
		vol = di->cache.cell2_vol;
	else
		di->cache.cell2_vol = vol;

	hwlog_info("cell2_battery_vol=%u\n", vol);
	return (int)vol;
}

static int bq40z50_get_all_vbat_mv(void *dev_data)
{
	return bq40z50_get_battery_vol_base(dev_data);
}

static int bq40z50_dump_log_data(char *buffer, int size, void *dev_data)
{
	struct bq40z50_device_info *di = dev_data;
	struct bq40z50_log_data log_data;

	if (!buffer || !di)
		return -EPERM;

	log_data.vbat = bq40z50_get_battery_vol_base(di);
	log_data.ibat = bq40z50_get_battery_curr_base(di);
	log_data.avg_ibat = bq40z50_get_battery_avgcurr_base(di);
	log_data.rm = bq40z50_get_battery_rm_base(di);
	log_data.temp = bq40z50_get_battery_temp_base(di);
	log_data.soc = bq40z50_get_battery_soc_base(di);
	log_data.fcc = bq40z50_get_battery_fcc_base(di);
	log_data.ibat_th = bq40z50_get_desired_charging_current_base(di);
	log_data.vbat_th = bq40z50_get_desired_charging_voltage_base(di);
	log_data.cycle_count = bq40z50_get_battery_cycle_base(di);
	log_data.cell1_vol = bq40z50_get_cell1_vbat_mv(di);
	log_data.cell2_vol = bq40z50_get_cell2_vbat_mv(di);
	log_data.bat_status = bq40z50_get_battery_status_base(di);
	log_data.safe_status = bq40z50_get_safety_status_base(di);
	log_data.pf_status = bq40z50_get_pf_status_base(di);
	log_data.op_status = bq40z50_get_operation_status_base(di);

	snprintf(buffer, size, "%-7d%-7d%-7d%-7d%-7d%-7d%-7d%-7d%-7d"
		"%-7d%-7d%-7d%-7d%-10x%-10x%-10x",
		log_data.temp, log_data.vbat, log_data.ibat,
		log_data.avg_ibat, log_data.rm, log_data.soc,
		log_data.fcc, log_data.ibat_th, log_data.vbat_th, log_data.cycle_count,
		log_data.cell1_vol, log_data.cell2_vol, log_data.bat_status, log_data.safe_status,
		log_data.pf_status, log_data.op_status);

	return 0;
}

static int bq40z50_get_log_head(char *buffer, int size, void *dev_data)
{
	struct bq40z50_device_info *di = dev_data;

	if (!buffer || !di)
		return -EPERM;

	snprintf(buffer, size,
		"Temp   Vbat   Ibat   AIbat  Rm     Soc    Fcc    dIbat  dVbat  cycle"
		"  Vcell1 Vcell2 batS   safeS     pfS       opS       ");

	return 0;
}

static struct power_log_ops bq40z50_log_ops = {
	.dev_name = "bq40z50",
	.dump_log_head = bq40z50_get_log_head,
	.dump_log_content = bq40z50_dump_log_data,
};

static struct coul_interface_ops bq40z50_ops = {
	.type_name = "main",
	.is_coul_ready = bq40z50_is_ready_base,
	.is_battery_exist = bq40z50_is_battery_exist_base,
	.get_battery_capacity = bq40z50_get_battery_soc_base,
	.get_battery_voltage = bq40z50_get_battery_vol_base,
	.get_battery_current = bq40z50_get_battery_curr_base,
	.get_battery_avg_current = bq40z50_get_battery_avgcurr_base,
	.get_battery_temperature = bq40z50_get_battery_temp_base,
	.get_battery_fcc = bq40z50_get_battery_fcc_base,
	.get_battery_cycle = bq40z50_get_battery_cycle_base,
	.set_battery_low_voltage = bq40z50_set_battery_low_voltage_base,
	.set_battery_last_capacity = bq40z50_set_last_capacity_base,
	.get_battery_last_capacity = bq40z50_get_last_capacity_base,
	.get_battery_charge_counter =  bq40z50_get_battery_charge_counter,
	.get_desired_charging_current = bq40z50_get_desired_charging_current_base,
	.get_desired_charging_voltage = bq40z50_get_desired_charging_voltage_base,
	.is_smart_battery = bq40z50_is_smart_battery_base,
	.get_battery_rm = bq40z50_get_battery_rm_base,
	.get_coul_model = bq40z50_get_coul_model_base,
};

static struct bat_model_ops bq40z50_bat_model_ops = {
	.get_technology = bq40z50_get_battery_technology_base,
	.get_brand = bq40z50_get_battery_brand_base,
	.get_vbat_max = bq40z50_get_battery_vbat_max_base,
	.get_design_fcc = bq40z50_get_battery_dc_base,
};

#if defined(CONFIG_HUAWEI_BATTERY_VOLTAGE) && !defined(CONFIG_COUL_DRV)
struct hw_batt_vol_ops bq40z50_cell1_vbatt_ops = {
	.get_batt_vol = bq40z50_get_cell1_vbat_mv,
};

struct hw_batt_vol_ops bq40z50_cell2_vbatt_ops = {
	.get_batt_vol = bq40z50_get_cell2_vbat_mv,
};

struct hw_batt_vol_ops bq40z50_all_vbatt_ops = {
	.get_batt_vol = bq40z50_get_all_vbat_mv,
};
#endif

int bq40z50_ops_register_base(struct bq40z50_device_info *di)
{
	int ret = -ENODEV;

	bq40z50_ops.dev_data = (void *)di;
	ret = coul_interface_ops_register(&bq40z50_ops);

	bq40z50_bat_model_ops.dev_data = (void *)di;
	ret += bat_model_register_ops(&bq40z50_bat_model_ops);

#if defined(CONFIG_HUAWEI_BATTERY_VOLTAGE) && !defined(CONFIG_COUL_DRV)
	bq40z50_cell1_vbatt_ops.dev_data = di;
	bq40z50_cell2_vbatt_ops.dev_data = di;
	bq40z50_all_vbatt_ops.dev_data = di;
	ret += hw_battery_voltage_ops_register(&bq40z50_cell1_vbatt_ops, "cell1");
	ret += hw_battery_voltage_ops_register(&bq40z50_cell2_vbatt_ops, "cell2");
	ret += hw_battery_voltage_ops_register(&bq40z50_all_vbatt_ops, "cell_all");
#endif

	bq40z50_log_ops.dev_data = (void *)di;
	ret += power_log_ops_register(&bq40z50_log_ops);

	return ret;
}
