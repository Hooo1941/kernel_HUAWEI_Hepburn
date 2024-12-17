/*
 * direct_charge_pmode.c
 *
 * direct charge pmode driver
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

#include <chipset_common/hwpower/common_module/power_algorithm.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_time.h>
#include <chipset_common/hwpower/direct_charge/direct_charger.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_common.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_ic.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_pmode.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_sysfs.h>
#include <chipset_common/hwpower/direct_charge/multi_bat_check.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <huawei_platform/power/battery_voltage.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_sysfs.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG direct_charge_pmode
HWLOG_REGIST();

#define DC_PMODE_SWITCH_HYSTERESIS_SEC                120
#define DC_PMODE_SWITCH_SYSFS_HYSTERESIS_SEC          30

struct pmode_table {
	unsigned int local_mode;
	unsigned int adp_mode;
	int base_volt;
	bool orig_local_en;
	bool local_en;
	bool adp_en;
	int max_cur_product;
	int max_cur_adp;
	int max_cur_cable;
	int max_pwr;
	enum dc_dis_flag dis_flag;
	u32 dis_time_begin;
};

struct pmode_info {
	unsigned int quick_mode_idx;
	unsigned int normal_mode_idx;
};

static struct pmode_table g_pmode_tbl[DC_MODE_TOTAL] = {
	[DC_MODE_LVC] = { LVC_MODE, ADAPTER_SUPPORT_LVC, 0, false, false, false, 0, 0, 0, 0, 0, 0 },
	[DC_MODE_SC] = { SC_MODE, ADAPTER_SUPPORT_SC, 0, false, false, false, 0, 0, 0, 0, 0, 0 },
	[DC_MODE_SC4] = { SC4_MODE, ADAPTER_SUPPORT_SC4, 0, false, false, false, 0, 0, 0, 0, 0, 0 },
};

static struct pmode_info g_pmode_di;
static struct pmode_table *dc_pmode_get_table(void)
{
	return g_pmode_tbl;
}

void dc_pmode_clear_data(void)
{
	int i;
	struct pmode_table *tbl = dc_pmode_get_table();

	hwlog_info("%s\n", __func__);
	for (i = 0; i < DC_MODE_TOTAL; i++) {
		tbl[i].base_volt = 0;
		tbl[i].orig_local_en = false;
		tbl[i].local_en = false;
		tbl[i].adp_en = false;
		tbl[i].max_cur_product = 0;
		tbl[i].max_cur_adp = 0;
		tbl[i].max_cur_cable = 0;
		tbl[i].max_pwr = 0;
	}
}

void dc_pmode_clear_data_in_exit(void)
{
	int i;
	struct pmode_table *tbl = dc_pmode_get_table();

	hwlog_info("%s\n", __func__);
	for (i = 0; i < DC_MODE_TOTAL; i++) {
		tbl[i].dis_flag = 0;
		tbl[i].dis_time_begin = 0;
	}
	memset(&g_pmode_di, 0, sizeof(g_pmode_di));
}

static void dc_pmode_updata_local_en(struct pmode_table *tbl, bool cc_safe, bool dfg_online)
{
	struct direct_charge_device *l_di = direct_charge_get_di_by_mode(tbl->local_mode);

	if (!l_di || l_di->dc_err_report_flag || l_di->pri_inversion || l_di->force_disable ||
		(l_di->dfg_disable && dfg_online))
		return;

	if ((((tbl->local_mode == LVC_MODE) && (!l_di->cc_protect || cc_safe)) ||
		((tbl->local_mode > LVC_MODE) && (l_di->cc_unsafe_sc_enable || cc_safe))) &&
		(l_di->dc_stage != DC_STAGE_CHARGE_DONE)) {
		l_di->cc_safe = cc_safe;
		tbl->local_en = true;
	}
}

void dc_pmode_enable_mode(unsigned int reason, unsigned int mode)
{
	int idx = dc_comm_get_mode_idx(mode);
	struct pmode_table *tbl = dc_pmode_get_table();

	if ((idx < 0) || (idx >= DC_MODE_TOTAL))
		return;

	tbl[idx].dis_flag &= ~reason;
	hwlog_info("mode=%d enable_reason=%d dis_flag=0x%x\n", mode, reason, tbl[idx].dis_flag);
}

void dc_pmode_disable_mode(unsigned int reason, unsigned int mode)
{
	int idx = dc_comm_get_mode_idx(mode);
	struct pmode_table *tbl = dc_pmode_get_table();

	if ((idx < 0) || (idx >= DC_MODE_TOTAL))
		return;

	tbl[idx].dis_flag |= reason;
	if (reason & (DC_DIS_BY_IBAT_TH_LOW | DC_DIS_BY_SYSFS_DISABLE))
		tbl[idx].dis_time_begin = power_get_current_kernel_time().tv_sec;
	hwlog_info("mode=%d disable_reason=%d dis_flag=0x%x\n", mode, reason, tbl[idx].dis_flag);
}

int dc_pmode_init_local_mode(void)
{
	unsigned int local_mode;
	bool cc_safe, dfg_online;
	bool enter_flag = false;
	int i;
	struct pmode_table *tbl = dc_pmode_get_table();

	if (direct_charge_check_charge_done())
		return -1;

	if (dc_adpt_get_inherent_info(ADAP_ANTIFAKE_FAILED_CNT) >= DC_ERR_CNT_MAX) {
		hwlog_info("antifake failed exceed %u times, dc is disabled\n",
			DC_ERR_CNT_MAX);
		return -1;
	}

	local_mode = direct_charge_get_local_mode();
	local_mode = direct_charge_update_local_mode(local_mode);
	hwlog_info("local_mode=0x%x\n", local_mode);

	cc_safe = direct_charge_check_port_fault();
	dfg_online = power_platform_check_online_status();
	for (i = 0; i < DC_MODE_TOTAL; i++) {
		if (tbl[i].local_mode & local_mode)
			tbl[i].orig_local_en = true;

		if (!(tbl[i].local_mode & local_mode))
			continue;
		dc_pmode_updata_local_en(&tbl[i], cc_safe, dfg_online);
		if (tbl[i].local_en)
			enter_flag = true;
	}

	if (enter_flag)
		return 0;

	direct_charge_set_can_enter_status(false);
	hwlog_info("neither sc4 nor sc nor lvc matched\n");
	return -1;
}

int dc_pmode_init_adp_mode(unsigned int *mode)
{
	int i;
	struct pmode_table *tbl = dc_pmode_get_table();

	if (!dc_adpt_detect_ping())
		*mode = dc_adpt_get_adapter_support_mode();

	hwlog_info("adp_mode=0x%x\n", *mode);
	power_event_bnc_notify(POWER_BNT_DC, POWER_NE_DC_ADAPTER_MODE, mode);

	if ((*mode == ADAPTER_TEST_MODE) ||
		(*mode == ADAPTER_SUPPORT_UNDEFINED)) {
		hwlog_info("adapter mode is test mode or undefined mode");
		return -1;
	}

	charge_set_fcp_disable_flag(false, FCP_DISABLE_PLATFORM_VOTER);
	for (i = 0; i < DC_MODE_TOTAL; i++) {
		if (*mode & tbl[i].adp_mode)
			tbl[i].adp_en = true;
	}

	return 0;
}

void dc_pmode_update_mode_power(void)
{
	int i, max_curr, ratio;
	struct pmode_table *tbl = dc_pmode_get_table();

	dc_update_cable_type();
	for (i = 0; i < DC_MODE_TOTAL; i++) {
		if (!(tbl[i].orig_local_en && tbl[i].adp_en))
			continue;

		ratio = dc_comm_get_mode_ratio(tbl[i].local_mode);
		if (ratio <= 0)
			continue;

		tbl[i].base_volt = hw_battery_get_series_num() * BAT_RATED_VOLT * ratio;
		tbl[i].max_cur_product = direct_charge_get_product_max_current(tbl[i].local_mode) / ratio;
		tbl[i].max_cur_adp = dc_get_adapter_max_current(tbl[i].local_mode, tbl[i].base_volt);
		tbl[i].max_cur_cable = dc_get_cable_max_current(tbl[i].local_mode) / ratio;
		dc_update_cable_max_current(&tbl[i].max_cur_cable);

		max_curr = power_min_positive(tbl[i].max_cur_adp, tbl[i].max_cur_product);
		max_curr = power_min_positive(tbl[i].max_cur_cable, max_curr);
		tbl[i].max_pwr = tbl[i].base_volt * max_curr / POWER_UW_PER_MW;
		hwlog_info("%s ratio=%d max_cur_product=%d max_cur_adp=%d max_cur_cable=%d max_pwr=%d\n",
			dc_comm_get_mode_name(tbl[i].local_mode),
			ratio, tbl[i].max_cur_product, tbl[i].max_cur_adp, tbl[i].max_cur_cable, tbl[i].max_pwr);
	}
	dc_update_cable_detect_ok();
}

int dc_pmode_get_max_cur_adp(void)
{
	int i;
	int max_cur = 0;
	struct pmode_table *tbl = dc_pmode_get_table();

	for (i = 0; i < DC_MODE_TOTAL; i++) {
		if (tbl[i].max_cur_adp > max_cur)
			max_cur = tbl[i].max_cur_adp;
	}

	hwlog_info("get max cur adp=%d\n", max_cur);
	return max_cur;
}

int dc_pmode_get_max_product_power(void)
{
	int i;
	int max_pwr = 0;
	struct pmode_table *tbl = dc_pmode_get_table();
	struct direct_charge_device *l_di = NULL;

	for (i = 0; i < DC_MODE_TOTAL; i++) {
		l_di = direct_charge_get_di_by_mode(tbl[i].local_mode);
		if (!l_di)
			continue;
		if (l_di->product_max_pwr > max_pwr)
			max_pwr = l_di->product_max_pwr;
	}

	hwlog_info("get max product power=%d\n", max_pwr);
	return max_pwr;
}

bool dc_pmode_is_cable_limit_power(void)
{
	int i, min_cur;
	int idx = -1;
	struct pmode_table *tbl = dc_pmode_get_table();

	for (i = 0; i < DC_MODE_TOTAL; i++) {
		if (!(tbl[i].orig_local_en && tbl[i].adp_en))
			continue;

		if ((idx < 0) || (tbl[i].max_pwr >= tbl[idx].max_pwr))
			idx = i;
	}

	if (idx < 0) {
		hwlog_err("%s idx error\n", __func__);
		return false;
	}

	min_cur = power_min_positive(tbl[idx].max_cur_product, tbl[idx].max_cur_adp);
	hwlog_info("%s idx=%d max_cur_product=%d max_cur_adp=%d max_cur_cable=%d\n", __func__,
		idx, tbl[idx].max_cur_product, tbl[idx].max_cur_adp, tbl[idx].max_cur_cable);
	if (tbl[idx].max_cur_cable && (tbl[idx].max_cur_cable < min_cur)) {
		hwlog_info("%s cable limit power\n", __func__);
		return true;
	}

	hwlog_info("%s cable not limit power\n", __func__);
	return false;
}

static unsigned int dc_pmode_get_quick_optimal_mode(void)
{
	int i;
	int idx = -1;
	struct pmode_table *tbl = dc_pmode_get_table();

	for (i = 0; i < DC_MODE_TOTAL; i++) {
		if (!(tbl[i].local_en && tbl[i].adp_en))
			continue;

		if ((idx < 0) || (tbl[i].max_pwr >= tbl[idx].max_pwr))
			idx = i;
	}

	if (idx < 0) {
		hwlog_err("get_quick_optimal_mode is failed, all mode is unsupport\n");
		return UNDEFINED_MODE;
	}

	hwlog_info("get_quick_optimal_mode=%s max_pwr=%d\n", dc_comm_get_mode_name(tbl[idx].local_mode),
		tbl[idx].max_pwr);
	g_pmode_di.quick_mode_idx = idx;
	return tbl[idx].local_mode;
}

static bool dc_pmode_ibat_th_check_failed(int mode)
{
	int ibat_th = dc_sysfs_get_iin_thermal(mode);
	int min_ibat_th = dc_info_get_val_with_mode(mode, MIN_IBAT_TH);
	if (ibat_th > min_ibat_th)
		return false;

	hwlog_err("%s ibat_th=%d is below min_ibat_th=%d\n", dc_comm_get_mode_name(mode),
		ibat_th, min_ibat_th);
	return true;
}

static bool dc_pmode_dis_flag_check_failed(struct pmode_table *tbl)
{
	int time;
	int time_th;

	if (tbl->dis_flag & DC_DIS_BY_PRIORITY) {
		hwlog_info("due to priority disable mode=%d has taken effect,now enable it\n", tbl->local_mode);
		dc_pmode_enable_mode(DC_DIS_BY_PRIORITY, tbl->local_mode);
		return true;
	}

	if (tbl->dis_flag & (DC_DIS_BY_IBAT_TH_LOW | DC_DIS_BY_SYSFS_DISABLE)) {
		time = power_get_current_kernel_time().tv_sec - tbl->dis_time_begin;
		time_th = (tbl->dis_flag & DC_DIS_BY_IBAT_TH_LOW) ?
			DC_PMODE_SWITCH_HYSTERESIS_SEC : DC_PMODE_SWITCH_SYSFS_HYSTERESIS_SEC;
		hwlog_info("due to %d disable mode=%d has taken effect for %ds when target is %ds,now %s it\n",
			tbl->dis_flag, tbl->local_mode, time, time_th, time >= time_th ? "enable" : "disable");

		if (time >= time_th)
			dc_pmode_enable_mode(tbl->dis_flag, tbl->local_mode);
		return true;
	}

	return false;
}

static unsigned int dc_pmode_get_normal_optimal_mode(int mode, bool cur_flag)
{
	int i;
	int idx = -1;
	struct pmode_table *tbl = dc_pmode_get_table();
	int bat_mode = multi_batchk_get_support_mode(mode, cur_flag);

	for (i = 0; i < DC_MODE_TOTAL; i++) {
		if (!(tbl[i].local_en && tbl[i].adp_en))
			continue;

		if ((bat_mode >= 0) && !(tbl[i].local_mode & bat_mode)) {
			if ((tbl[i].local_mode == mode) && cur_flag)
				dc_pmode_disable_mode(DC_DIS_BY_PRIORITY, mode);
			continue;
		}

		if (dc_pmode_ibat_th_check_failed(tbl[i].local_mode)) {
			if ((tbl[i].local_mode == mode) && cur_flag)
				dc_pmode_disable_mode(DC_DIS_BY_IBAT_TH_LOW, mode);
			continue;
		}

		if (dc_sysfs_get_disable_flag(tbl[i].local_mode)) {
			if (cur_flag)
				dc_pmode_disable_mode(DC_DIS_BY_SYSFS_DISABLE, tbl[i].local_mode);
			continue;
		}

		if (dc_pmode_dis_flag_check_failed(&tbl[i]))
			continue;

		if ((idx < 0) || (tbl[i].max_pwr >= tbl[idx].max_pwr))
			idx = i;
	}

	if (idx < 0) {
		hwlog_err("get_normal_optimal_mode is failed, all mode is unsupport\n");
		return UNDEFINED_MODE;
	}

	hwlog_info("get_normal_optimal_mode=%s max_pwr=%d\n", dc_comm_get_mode_name(tbl[idx].local_mode),
		tbl[idx].max_pwr);
	g_pmode_di.normal_mode_idx = idx;
	return tbl[idx].local_mode;
}

unsigned int dc_pmode_get_optimal_mode(int type, int mode, bool cur_flag)
{
	unsigned int ret;

	if (dc_mmi_get_test_flag())
		return dc_pmode_get_quick_optimal_mode();

	switch (type) {
	case QUICK_CHECK:
		return dc_pmode_get_quick_optimal_mode();
	case NORMAL_CHECK:
		return dc_pmode_get_normal_optimal_mode(mode, cur_flag);
	case DEFAULT_CHECK:
		/* default_check: the result of quick_check must be updated first due to chargedone, etc */
		ret = dc_pmode_get_quick_optimal_mode();
		if (dc_comm_get_first_check_completed())
			return dc_pmode_get_normal_optimal_mode(mode, cur_flag);
		dc_comm_set_first_check_completed(true);
		return ret;
	default:
		hwlog_err("pmode_check_type is illegal\n");
		return UNDEFINED_MODE;
	}
}

static int dc_pmode_get_optimal_mode_idx(int type)
{
	if (dc_mmi_get_test_flag())
		return g_pmode_di.quick_mode_idx;

	switch (type) {
	case QUICK_CHECK:
		return g_pmode_di.quick_mode_idx;
	case NORMAL_CHECK:
		return g_pmode_di.normal_mode_idx;
	default:
		hwlog_err("pmode_check_type is illegal\n");
		return -EINVAL;
	}
}

int dc_pmode_get_optimal_mode_max_pwr(int type)
{
	int idx = dc_pmode_get_optimal_mode_idx(type);
	struct pmode_table *tbl = dc_pmode_get_table();

	if (idx < 0)
		return 0;

	hwlog_info("type=%d mode=%s max_pwr=%d\n", type, dc_comm_get_mode_name(tbl[idx].local_mode),
		tbl[idx].max_pwr);
	return tbl[idx].max_pwr;
}

int dc_pmode_get_optimal_mode_max_cur(int type)
{
	int max_cur;
	int idx = dc_pmode_get_optimal_mode_idx(type);
	struct pmode_table *tbl = dc_pmode_get_table();

	if (idx < 0)
		return 0;

	max_cur = tbl[idx].max_pwr * POWER_UW_PER_MW / hw_battery_get_series_num() / BAT_RATED_VOLT;
	hwlog_info("type=%d mode=%s max_cur=%d\n", type, dc_comm_get_mode_name(tbl[idx].local_mode),
		max_cur);
	return max_cur;
}
