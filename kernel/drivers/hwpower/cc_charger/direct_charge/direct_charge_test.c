/*
 * direct_charge_test.c
 *
 * direct charge test driver
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

#include <chipset_common/hwpower/adapter/adapter_test.h>
#include <chipset_common/hwpower/battery/battery_temp.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_supply.h>
#include <chipset_common/hwpower/common_module/power_supply_interface.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_ic_manager.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_test.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <huawei_platform/hwpower/common_module/power_platform_macro.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG direct_charge_test
HWLOG_REGIST();

static const int g_at_mode_tbl[] = {
	[UNDEFINED_MODE] = AT_TYPE_OTHER,
	[LVC_MODE] = AT_TYPE_LVC,
	[SC_MODE] = AT_TYPE_SC,
	[SC4_MODE] = AT_TYPE_SC4,
};

static const char *coul_psy[] = {
	"battery_gauge",
	"battery_gauge_aux",
	"battery_gauge_third"
};

void dc_test_set_adapter_test_result(int mode, int result)
{
	adapter_test_set_result(g_at_mode_tbl[mode], result);
}

static int dc_check_coul_status(void)
{
	int i;
	int coul_num = 0;
	struct power_supply *psy = NULL;

	for (i = 0; i < COUL_IC_NUM; i++) {
		psy = NULL;
		if (power_supply_check_psy_available(coul_psy[i], &psy))
			coul_num |= BIT(i);
	}
	return coul_num;
}

static int dc_update_battery_info(struct dc_test_info *info, int coul_vbat[],
	int coul_ibat[], int len, int *coul_vbat_max)
{
	int ret;
	int i;

	info->coul_check_flag = dc_check_coul_status();
	ret = power_supply_get_int_property_value(POWER_PLATFORM_BAT_PSY_NAME,
		POWER_SUPPLY_PROP_VOLTAGE_NOW, coul_vbat_max);
	for (i = 0; i < len; i++) {
		if (!(info->coul_check_flag & BIT(i)))
			continue;
		ret += power_supply_get_int_property_value(coul_psy[i],
			POWER_SUPPLY_PROP_VOLTAGE_NOW, &coul_vbat[i]);
		if (!ret)
			coul_vbat[i] /= POWER_UV_PER_MV;
		ret += power_supply_get_int_property_value(coul_psy[i],
			POWER_SUPPLY_PROP_CURRENT_NOW, &coul_ibat[i]);
		if (ret || (coul_ibat[i] < info->coul_ibat[i]))
			return -1;
	}
	return 0;
}

void dc_update_charging_info(int mode, struct dc_test_info *info, int *vbat_comp)
{
	int i;
	int ret;
	int coul_ibat_max = 0;
	int coul_vbat_max = 0;
	int ibat[CHARGE_IC_TYPE_MAX] = { 0 };
	int ibus[CHARGE_IC_TYPE_MAX] = { 0 };
	int coul_ibat[COUL_IC_NUM] = { 0 };
	int coul_vbat[COUL_IC_NUM] = { 0 };

	if (!info || !vbat_comp)
		return;

	coul_ibat_max = -power_platform_get_battery_current();
	if (coul_ibat_max < info->coul_ibat_max)
		return;

	for (i = 0; i < info->channel_num; i++) {
		ret = dcm_get_ic_ibat(mode, BIT(i), &ibat[i]);
		ret += dcm_get_ic_ibus(mode, BIT(i), &ibus[i]);
		if (ret || (ibus[i] < info->ibus[i]) || (ibat[i] < info->ibat[i]))
			return;
	}
	if (dc_update_battery_info(info, coul_vbat, coul_ibat, ARRAY_SIZE(coul_vbat),
		&coul_vbat_max))
		return;

	direct_charge_get_bat_current(&info->ibat_max);
	for (i = 0; i < info->channel_num; i++) {
		info->ibat[i] = ibat[i];
		info->ibus[i] = ibus[i];
		info->ic_name[i] = dcm_get_ic_name(mode, BIT(i));
		info->vbat[i] = dcm_get_ic_vbtb_with_comp(mode, BIT(i), vbat_comp);
		dcm_get_ic_vout(mode, BIT(i), &info->vout[i]);
		bat_temp_get_temperature(i, &info->tbat[i]);
	}

	info->coul_ibat_max = coul_ibat_max;
	info->coul_vbat_max = coul_vbat_max;
	for (i = 0; i < COUL_IC_NUM; i++) {
		if (!(info->coul_check_flag & BIT(i)))
			continue;
		info->coul_vbat[i] = coul_vbat[i];
		info->coul_ibat[i] = coul_ibat[i];
	}
}

void dc_show_realtime_charging_info(int mode, unsigned int path)
{
	int vbat;
	int vbus = 0;
	int ibus = 0;
	int ibat = 0;
	struct adapter_source_info source_info = { 0 };
	unsigned int flag = BIT(ADAPTER_OUTPUT_VOLT) | BIT(ADAPTER_OUTPUT_CURR);

	dc_get_adapter_source_info(flag, &source_info);
	dcm_get_ic_vbus(mode, path, &vbus);
	dcm_get_ic_ibus(mode, path, &ibus);
	vbat = dcm_get_ic_vbtb(mode, path);
	dcm_get_total_ibat(mode, path, &ibat);

	hwlog_info("Vadp=%d, Iadp=%d, Vbus=%d, Ibus=%d, Vbat=%d, Ibat=%d\n",
		source_info.output_volt, source_info.output_curr, vbus, ibus, vbat, ibat);
}

