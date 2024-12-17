// SPDX-License-Identifier: GPL-2.0
/*
 * direct_charge_adapter.c
 *
 * adapter operate for direct charge
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#include <chipset_common/hwpower/direct_charge/direct_charge_adapter.h>
#include <huawei_platform/power/battery_voltage.h>
#include <chipset_common/hwpower/adapter/adapter_detect.h>
#include <chipset_common/hwpower/common_module/power_algorithm.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_dsm.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/common_module/power_time.h>
#include <chipset_common/hwpower/charger/charger_common_interface.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_charging_info.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_common.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_control.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_error_handle.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_ic_manager.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_vote.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_cable.h>

#define HWLOG_TAG direct_charge_adapter
HWLOG_REGIST();

/* define adapter power curve */
#define DC_ADP_PWR_CURVE_LEVEL              16
#define POWER_TH_UFCS_ILIMIT_ANTIFAKE       30000
#define ADP_ANTIFAKE_FAIL_MAX_CURRENT       2250
#define ADP_ANTIFAKE_CHECK_THLD             4
#define POWER_TH_IGNORE_ANTIFAKE            22500

#define DC_ANTI_22P5W_START_CHK_TIME        40
#define DC_ANTI_22P5W_CURR_ANOMALY_THRE     4000
#define DC_ANTI_22P5W_VOLT_ANOMALY_THRE     7500
#define DC_ANTI_22P5W_PWR                   BIT(0)
#define DC_ANTI_22P5W_CUR                   BIT(1)

static const char *g_prot_str[] = {
	[ADAPTER_PROTOCOL_SCP]    = "scp",
	[ADAPTER_PROTOCOL_FCP]    = "fcp",
	[ADAPTER_PROTOCOL_PD]     = "pd",
	[ADAPTER_PROTOCOL_UVDM]   = "uvdm",
	[ADAPTER_PROTOCOL_UFCS]   = "ufcs",
};

/* define iwatt lvc adapter limit info */
enum iwatt_adapter_limit_info {
	IWATT_SKIP_SECURITY_CHECK = 0,
	IWATT_LIMIT_IBAT,
	IWATT_LIMIT_INFO_END,
};

/*
 * define dc first cc charge time with max power
 * support up to 7 parameters list on dts
 */
#define DC_MAX_POWER_TIME_PARA_LEVEL    20
enum dc_max_power_time_info {
	DC_ADAPTER_TYPE,
	DC_MAX_POWER_TIME,
	DC_MAX_POWER_LIMIT_CURRENT,
	DC_MAX_POWER_PARA_MAX,
};

struct dc_max_power_time_para {
	int adatper_type;
	int max_power_time;
	int limit_current;
};

struct dc_limit_max_power_para {
	int limit_current;
	int start_time;
	int run_time;
	bool start_timer_flag;
	int max_power_timeout;
};

enum dc_antifake_22p5w_info {
	DC_22P5W_STRATEGY = 0,
	DC_IBUS_GAP,
	DC_IBUS_LOW_TIME,
	DC_22P5W_ALL,
};

struct dc_adpt_anti_22p5w_para {
	int strategy;
	int ibus_gap;
	int ibus_low_time;
};

struct dc_adpt_antifake_para {
	u32 adp_antifake_key_index;
	u32 adp_antifake_execute_enable;
	u32 adp_antifake_enable;
	struct dc_adpt_anti_22p5w_para anti_22p5w_para;
};

struct dc_adpt_dts_info {
	u32 gain_curr_10v2a;
	u32 delta_err_10v2p25a;
	int max_adapter_vset;
	int adapter_watchdog_time;
	int max_tadapt;
	int adaptor_detect_by_voltage;
	int iwatt_limit[IWATT_LIMIT_INFO_END];
	struct dc_max_power_time_para max_power_time[DC_MAX_POWER_TIME_PARA_LEVEL];
	struct dc_adpt_antifake_para antifake_para;
};

struct dc_adpt_mode_info {
	int max_adapter_iset;
};

struct dc_adpt_dev {
	struct dc_adpt_dts_info dts_info;
	struct dc_adpt_mode_info mode_info;
};

struct dc_adpt_inherent_para {
	struct dc_max_power_time_para max_pwr_para;
	int max_pwr_start_time;
	bool max_pwr_flag;
	bool is_abnormal_scp_adap;
	int max_cur;
	int adaptor_vendor_id;
	int adp_antifake_failed_cnt;
	int adp_antifake_result;
	bool adp_antifake_report_flag;
	int cur_err_cnt;
};

static struct dc_pps_ops *g_pps_ops;
static enum adapter_protocol_type g_prot = ADAPTER_PROTOCOL_SCP;
static struct dc_adpt_inherent_para g_adpt_inherent_info;
static struct dc_adpt_dev *g_dc_adpt_di[DC_MODE_TOTAL];

/* protocol power supply for direct charge */
int dc_pps_ops_register(struct dc_pps_ops *ops)
{
	int ret = 0;

	if (ops) {
		g_pps_ops = ops;
		hwlog_info("protocol power supply ops register ok\n");
	} else {
		hwlog_err("protocol power supply ops register fail\n");
		ret = -EINVAL;
	}

	return ret;
}

/* control power supply for adapter protocol communication */
void dc_adapter_protocol_power_supply(int enable)
{
	int ret;

	if (!g_pps_ops || !g_pps_ops->enable) {
		hwlog_err("pps_ops or power_supply_enable is null\n");
		return;
	}

	ret = g_pps_ops->enable(enable);
	if (ret) {
		hwlog_err("power supply fail, enable=%d\n", enable);
		return;
	}

	hwlog_info("power supply ok, enable=%d\n", enable);
}

static struct dc_adpt_dev *dc_adpt_get_di(int mode)
{
	int idx = dc_comm_get_mode_idx(mode);
	if ((idx < 0) || (idx >= DC_MODE_TOTAL))
		return NULL;
	return g_dc_adpt_di[idx];
}

static struct dc_adpt_dts_info *dc_adpt_get_dts_info(int mode)
{
	int idx = dc_comm_get_mode_idx(mode);
	if ((idx < 0) || (idx >= DC_MODE_TOTAL))
		return NULL;
	return &g_dc_adpt_di[idx]->dts_info;
}

static struct dc_adpt_inherent_para *dc_adpt_get_inherent_para(void)
{
	return &g_adpt_inherent_info;
}

void dc_adpt_clear_inherent_info(void)
{
	memset(&g_adpt_inherent_info, 0, sizeof(g_adpt_inherent_info));
}

int dc_adpt_get_inherent_info(unsigned int type)
{
	struct dc_adpt_inherent_para *para = dc_adpt_get_inherent_para();

	if (!para)
		return 0;

	switch (type) {
	case ADAP_MAX_CUR:
		return para->max_cur;
	case IS_ABNORMAL_ADP:
		return para->is_abnormal_scp_adap;
	case ADAP_ANTIFAKE_RESULT:
		return para->adp_antifake_result;
	case ADAP_ANTIFAKE_FAILED_CNT:
		return para->adp_antifake_failed_cnt;
	default:
		break;
	}

	return 0;
}

int dc_adpt_get_mode_info(int mode, unsigned int type)
{
	struct dc_adpt_dev *di = dc_adpt_get_di(mode);

	if (!di)
		return 0;

	switch (type) {
	case ADAP_MAX_TEMP:
		return di->dts_info.max_tadapt;
	case ADAP_ANTIFAKE_EXE_EN:
		return di->dts_info.antifake_para.adp_antifake_execute_enable;
	case ADAP_ANTI_22P5W_STRATEGY:
		return di->dts_info.antifake_para.anti_22p5w_para.strategy;
	default:
		break;
	}

	return 0;
}

void dc_adpt_set_mode_info(int mode, unsigned int type, int val)
{
	struct dc_adpt_dev *di = dc_adpt_get_di(mode);

	if (!di)
		return;

	switch (type) {
	case ADAP_MAX_ISET:
		di->mode_info.max_adapter_iset = val;
		break;
	default:
		break;
	}
}

int dc_init_adapter(int mode, int init_vset)
{
	struct adapter_init_data aid;
	struct dc_adpt_dev *di = dc_adpt_get_di(mode);

	if (!di)
		return -EPERM;

	aid.scp_mode_enable = 1;
	aid.vset_boundary = di->dts_info.max_adapter_vset;
	aid.iset_boundary = di->mode_info.max_adapter_iset;
	aid.init_voltage = init_vset;
	aid.watchdog_timer = di->dts_info.adapter_watchdog_time;

	return adapter_set_init_data(g_prot, &aid);
}

int dc_reset_operate(int type)
{
	switch (type) {
	case DC_RESET_ADAPTER:
		hwlog_info("soft reset adapter\n");
		return adapter_soft_reset_slave(g_prot);
	case DC_RESET_MASTER:
		hwlog_info("soft reset master\n");
		return adapter_soft_reset_master(g_prot);
	case DC_RESET_DPDM:
		hwlog_info("soft reset dpdm\n");
		return adapter_soft_reset_dpdm(g_prot);
	default:
		hwlog_info("reset operate invalid\n");
		return -EPERM;
	}
}

int dc_get_adapter_type(void)
{
	int adapter_type = ADAPTER_TYPE_UNKNOWN;

	adapter_get_adp_type(g_prot, &adapter_type);
	return adapter_type;
}

int dc_get_adapter_max_voltage(int *value)
{
	return adapter_get_max_voltage(g_prot, value);
}

int dc_get_adapter_min_voltage(int *value)
{
	return adapter_get_min_voltage(g_prot, value);
}

int dc_get_adapter_max_power(int *max_pwr)
{
	return adapter_get_max_power(g_prot, max_pwr);
}

int dc_get_adapter_current_set(int *value)
{
	if (dc_get_stop_charging_flag())
		return -EPERM;

	if (adapter_get_output_current_set(g_prot, value)) {
		*value = 0;
		dc_ctrl_fill_eh_buf_and_set_err_flag_true(DC_EH_GET_APT_CURR_SET, NULL);
		return -EPERM;
	}

	return 0;
}

static int dc_get_power_drop_current(int *value)
{
	if (dc_get_stop_charging_flag())
		return -EPERM;

	if (adapter_get_power_drop_current(g_prot, value)) {
		*value = 0;
		dc_ctrl_fill_eh_buf_and_set_err_flag_true(DC_EH_GET_APT_PWR_DROP_CURR, NULL);
		return -EPERM;
	}

	return 0;
}

static int dc_get_adapter_power_curve(struct adp_pwr_curve_para *para, int *size, int max_size)
{
	int num = 0;

	if (!para)
		return -EPERM;

	if (adapter_get_power_curve(g_prot, para, &num, max_size))
		return -EPERM;

	*size = num;
	return 0;
}

static int dc_get_adapter_max_cur_by_power_curve(int val)
{
	int i;
	int num = 0;
	struct adp_pwr_curve_para value[DC_ADP_PWR_CURVE_LEVEL] = { 0 };

	if (dc_get_adapter_power_curve(value, &num, DC_ADP_PWR_CURVE_LEVEL))
		return 0;

	for (i = 0; i < num; i++) {
		if (val <= value[i].volt)
			return value[i].cur;
	}

	return 0;
}

static int dc_get_adapter_max_cur_by_reg(int *value)
{
	if (dc_get_stop_charging_flag())
		return -EPERM;

	if (adapter_get_max_current(g_prot, value)) {
		*value = 0;
		dc_ctrl_fill_eh_buf_and_set_err_flag_true(DC_EH_GET_APT_MAX_CURR, NULL);
		return -EPERM;
	}

	return 0;
}

static void dc_revises_adapter_max_current(int *value)
{
	/*
	 * Avoid charger current accuracy problems
	 * value is 3200mA, when the actual current is 3250mA
	*/
	if (*value == 3200)
		*value = 3250;
}

int dc_get_adapter_max_current(int mode, int value)
{
	int max_cur = 0;
	struct dc_adpt_dev *di = dc_adpt_get_di(mode);
	struct dc_adpt_inherent_para *para = dc_adpt_get_inherent_para();

	if (!di || !para)
		return 0;

	if ((di->dts_info.antifake_para.adp_antifake_execute_enable == 0) &&
		(para->adp_antifake_failed_cnt >= ADP_ANTIFAKE_CHECK_THLD)) {
		hwlog_info("antifake check failed, set max current to %d\n", ADP_ANTIFAKE_FAIL_MAX_CURRENT);
		return ADP_ANTIFAKE_FAIL_MAX_CURRENT;
	}

	max_cur = dc_get_adapter_max_cur_by_power_curve(value);
	if (max_cur <= 0)
		dc_get_adapter_max_cur_by_reg(&max_cur);

	dc_revises_adapter_max_current(&max_cur);
	return max_cur;
}

static int dc_get_adapter_voltage(int *value)
{
	if (adapter_get_output_voltage(g_prot, value)) {
		*value = 0;
		dc_ctrl_fill_eh_buf_and_set_err_flag_true(DC_EH_GET_VAPT, NULL);
		return -EPERM;
	}

	return 0;
}

static int dc_get_adapter_current(int *value)
{
	int adapter_type = ADAPTER_TYPE_UNKNOWN;
	struct dc_adpt_inherent_para *para = dc_adpt_get_inherent_para();

	if (!para || dc_get_stop_charging_flag())
		return -EPERM;

	adapter_type = dc_get_adapter_type();
	if ((para->adaptor_vendor_id == ADAPTER_CHIP_IWATT) &&
		((adapter_type == ADAPTER_TYPE_5V4P5A) ||
		(adapter_type == ADAPTER_TYPE_10V2A) ||
		(adapter_type == ADAPTER_TYPE_10V2P25A)))
		return dc_get_device_ibus(value);

	if (adapter_get_output_current(g_prot, value)) {
		*value = 0;
		dc_ctrl_fill_eh_buf_and_set_err_flag_true(DC_EH_GET_IAPT, NULL);
		return -EPERM;
	}

	return 0;
}

static int dc_get_adapter_temp(int *value)
{
	if (dc_get_stop_charging_flag())
		return -EPERM;

	if (adapter_get_inside_temp(g_prot, value)) {
		*value = 0;
		dc_ctrl_fill_eh_buf_and_set_err_flag_true(DC_EH_GET_APT_TEMP, NULL);
		return -EPERM;
	}

	return 0;
}

bool dc_adpt_cable_undetached(void)
{
	return adapter_is_undetach_cable(g_prot);
}

int dc_get_adapter_cable_info(int *curr)
{
	if (!curr || dc_get_stop_charging_flag())
		return -EPERM;

	return adapter_get_cable_info(g_prot, curr);
}

int dc_get_adapter_source_info(unsigned int flag, struct adapter_source_info *data)
{
	int ret = 0;
	struct adapter_source_info source_info = { 0 };

	if (dc_get_stop_charging_flag())
		return -EPERM;

	if (!adapter_get_source_info(g_prot, &source_info)) {
		data->output_volt = source_info.output_volt;
		data->output_curr = source_info.output_curr;
		data->port_temp = source_info.port_temp;
		data->dev_temp = source_info.dev_temp;
		return ret;
	}

	if (flag & BIT(ADAPTER_OUTPUT_VOLT))
		ret = dc_get_adapter_voltage(&data->output_volt);

	if (!ret && (flag & BIT(ADAPTER_OUTPUT_CURR)))
		ret += dc_get_adapter_current(&data->output_curr);

	if (!ret && (flag & BIT(ADAPTER_DEV_TEMP)))
		ret += dc_get_adapter_temp(&data->dev_temp);

	return ret;
}

int dc_get_adapter_source_id(int *source_id)
{
	if (!source_id || dc_get_stop_charging_flag())
		return -EPERM;

	return adapter_get_source_id(g_prot, source_id);
}

int dc_get_adapter_scpb_pwr(struct adp_scpb_pwr_data *scpb_pwr, int *size, int max_size)
{
	if (!scpb_pwr || !size || dc_get_stop_charging_flag())
		return -EPERM;

	return adapter_get_scpb_pwr(g_prot, scpb_pwr, size, max_size);
}

bool dc_is_scp_superior(void)
{
	return adapter_is_scp_superior(g_prot);
}

int dc_get_protocol_register_state(void)
{
	if (adapter_get_protocol_register_state(g_prot)) {
		hwlog_err("adapter protocol not ready\n");
		return -EPERM;
	}

	return 0;
}

int dc_set_adapter_voltage(int mode, int *value)
{
	struct dc_adpt_dev *di = dc_adpt_get_di(mode);

	if (!di)
		return -EPERM;

	hwlog_info("set adapter_volt=%d,max_volt=%d\n", *value, di->dts_info.max_adapter_vset);
	*value = power_min_positive(*value, di->dts_info.max_adapter_vset);
	if (adapter_set_output_voltage(g_prot, *value)) {
		dc_ctrl_fill_eh_buf_and_set_err_flag_true(DC_EH_SET_APT_VOLT, NULL);
		return -EPERM;
	}

	return 0;
}

int dc_set_adapter_voltage_step_by_step(int working_mode, int cur_mode, int start, int end)
{
	int vol;
	int step = 3000; /* step in 3000mv */
	int cnt = 10; /* max times */

	vol = end;
	if (!dcm_is_ic_support_prepare(working_mode, cur_mode))
		return dc_set_adapter_voltage(working_mode, &vol);

	vol = start;
	if (dc_set_adapter_voltage(working_mode, &vol))
		return -EPERM;

	if (dcm_prepare_enable_ic(working_mode, cur_mode))
		return -EPERM;

	while (cnt--) {
		vol += step;
		hwlog_info("vol step open=%d\n", vol);

		if (vol < end) {
			if (dc_set_adapter_voltage(working_mode, &vol))
				return -EPERM;
			power_usleep(DT_USLEEP_5MS);
		} else {
			vol = end;
			return dc_set_adapter_voltage(working_mode, &vol);
		}
	}

	return -EPERM;
}

int dc_set_adapter_current(int mode, int value)
{
	int l_value = value;
	struct dc_adpt_dev *di = dc_adpt_get_di(mode);

	if (!di)
		return -EPERM;

	if (dc_get_stop_charging_flag())
		return -EPERM;

	hwlog_info("set adapter_cur=%d,max_cur=%d\n",
		l_value, di->mode_info.max_adapter_iset);

	l_value = power_min_positive(di->mode_info.max_adapter_iset, l_value);
	if (adapter_set_output_current(g_prot, l_value)) {
		dc_ctrl_fill_eh_buf_and_set_err_flag_true(DC_EH_SET_APT_CURR, NULL);
		return -EPERM;
	}

	return 0;
}

int dc_set_adapter_output_capability(int vol, int cur, int wdt_time)
{
	struct adapter_init_data sid;

	sid.scp_mode_enable = 1;
	sid.vset_boundary = vol;
	sid.iset_boundary = cur;
	sid.init_voltage = vol;
	sid.watchdog_timer = wdt_time;

	if (adapter_set_init_data(g_prot, &sid))
		return -EIO;
	if (adapter_set_output_voltage(g_prot, vol))
		return -EIO;
	if (adapter_set_output_current(g_prot, cur))
		return -EIO;

	return 0;
}

int dc_set_adapter_output_enable(int enable)
{
	int ret, i;
	int retry = 3; /* retry 3 times */

	hwlog_info("set adapter_output_enable=%d\n", enable);
	for (i = 0; i < retry; i++) {
		ret = adapter_set_output_enable(g_prot, enable);
		if (!ret)
			break;
	}

	return ret;
}

void dc_set_adapter_default_param(void)
{
	adapter_set_default_param(g_prot);
}

int dc_set_adapter_default(void)
{
	return adapter_set_default_state(g_prot);
}

int dc_update_adapter_info(void)
{
	int ret;
	struct dc_adpt_inherent_para *para = dc_adpt_get_inherent_para();

	if (!para)
		return -EPERM;

	ret = adapter_get_device_info(g_prot);
	if (ret) {
		hwlog_err("get adapter info fail\n");
		return -EPERM;
	}
	adapter_show_device_info(g_prot);

	ret = adapter_get_chip_vendor_id(g_prot, &para->adaptor_vendor_id);
	if (ret) {
		hwlog_err("get adapter vendor id fail\n");
		return -EPERM;
	}

	return 0;
}

static int dc_get_adapter_support_mode(void)
{
	int adapter_mode = ADAPTER_SUPPORT_UNDEFINED;

	adapter_get_support_mode(g_prot, &adapter_mode);

	return adapter_mode;
}

static unsigned int dc_update_adapter_support_mode(void)
{
	unsigned int adp_mode = ADAPTER_SUPPORT_UNDEFINED;

	adapter_update_adapter_support_mode(g_prot, &adp_mode);

	return adp_mode;
}

struct adpt_redetect_info {
	int ratio;
	int delta;
	int max_vbat;
	int min_vbat;
};

static bool dc_adpt_voltage_within_range(int mode, int max_vadp, int min_vadp)
{
	int bat_num = hw_battery_get_series_num();
	int max_cfg_volt;
	int min_cfg_volt;
	struct adpt_redetect_info info;

	info.ratio = dc_info_get_val_with_mode(mode, VOLT_RATIO);
	info.delta = dc_info_get_val_with_mode(mode, INIT_DELT_VSET);
	info.max_vbat = dc_info_get_val_with_mode(mode, MAX_VBAT);
	info.min_vbat = dc_info_get_val_with_mode(mode, MIN_VBAT);
	hwlog_info("max_vadp=%d, max_vbat=%d, bat_num=%d, ratio=%d, delta=%d\n",
		max_vadp, info.max_vbat, bat_num, info.ratio, info.delta);
	hwlog_info("min_vadp=%d, min_vbat=%d, bat_num=%d, ratio=%d, delta=%d\n",
		min_vadp, info.min_vbat, bat_num, info.ratio, info.delta);

	/*
	 * how to calculate max configure voltage as below:
	 * max battery voltage * battery number * ratio + delta
	 */
	max_cfg_volt = info.max_vbat * bat_num * info.ratio + info.delta;
	min_cfg_volt = info.min_vbat * bat_num * info.ratio + info.delta;
	if ((max_vadp > max_cfg_volt) && (min_vadp < min_cfg_volt))
		return true;

	return false;
}

static int dc_adpt_redetect_by_voltage(unsigned int mode)
{
	unsigned int new_mode = ADAPTER_SUPPORT_UNDEFINED;
	int max_vadp = 0;
	int min_vadp = 0;
	struct dc_adpt_dev *lvc_di = dc_adpt_get_di(LVC_MODE);
	struct dc_adpt_dev *sc_di = dc_adpt_get_di(SC_MODE);
	struct dc_adpt_dev *sc4_di = dc_adpt_get_di(SC4_MODE);
	struct dc_adpt_inherent_para *para = dc_adpt_get_inherent_para();

	if (!(lvc_di && lvc_di->dts_info.adaptor_detect_by_voltage) &&
		!(sc_di && sc_di->dts_info.adaptor_detect_by_voltage) &&
		!(sc4_di && sc4_di->dts_info.adaptor_detect_by_voltage))
		return mode;

	if (dc_get_adapter_max_voltage(&max_vadp)) {
		hwlog_err("get adapter max voltage fail\n");
		return mode;
	}

	if (dc_get_adapter_min_voltage(&min_vadp)) {
		hwlog_err("get adapter min voltage fail\n");
		return mode;
	}

	if (lvc_di && lvc_di->dts_info.adaptor_detect_by_voltage &&
		dc_adpt_voltage_within_range(LVC_MODE, max_vadp, min_vadp))
		new_mode |= ADAPTER_SUPPORT_LVC;
	if (sc_di && sc_di->dts_info.adaptor_detect_by_voltage &&
		dc_adpt_voltage_within_range(SC_MODE, max_vadp, min_vadp))
		new_mode |= ADAPTER_SUPPORT_SC;
	if (sc4_di && sc4_di->dts_info.adaptor_detect_by_voltage &&
		dc_adpt_voltage_within_range(SC4_MODE, max_vadp, min_vadp))
		new_mode |= ADAPTER_SUPPORT_SC4;
	if (!new_mode)
		para->is_abnormal_scp_adap = true;

	hwlog_info("new_adapter_mode=%x\n", new_mode);
	return new_mode;
}

unsigned int dc_adpt_get_adapter_support_mode(void)
{
	unsigned int adp_mode = dc_get_adapter_support_mode();
	unsigned int adp_update_mode = dc_update_adapter_support_mode();
	if (adp_update_mode != ADAPTER_SUPPORT_UNDEFINED)
		adp_mode &= adp_update_mode;

	if (!(adp_mode & (ADAPTER_SUPPORT_LVC | ADAPTER_SUPPORT_SC | ADAPTER_SUPPORT_SC4)))
		return adp_mode;

	return dc_adpt_redetect_by_voltage(adp_mode);
}

static int dc_select_adapter_protocol_type(unsigned int prot_type)
{
	if (prot_type & BIT(ADAPTER_PROTOCOL_UFCS)) {
		g_prot = ADAPTER_PROTOCOL_UFCS;
		hwlog_info("select ufcs protocol\n");
		return 0;
	}

	if (prot_type & BIT(ADAPTER_PROTOCOL_SCP)) {
		g_prot = ADAPTER_PROTOCOL_SCP;
		hwlog_info("select scp protocol\n");
		return 0;
	}

	/* default is scp */
	g_prot = ADAPTER_PROTOCOL_SCP;
	hwlog_info("default select scp protocol\n");
	return -EPERM;
}

int dc_adpt_detect_ping(void)
{
	unsigned int prot_type;
	int ret;

	prot_type = adapter_detect_ping_protocol_type();
	ret = dc_select_adapter_protocol_type(prot_type);
	if (ret != 0)
		power_event_bnc_notify(POWER_BNT_DC, POWER_NE_DC_PING_FAIL, NULL);

	hwlog_info("adapter ping: prot_type=%x ret=%d\n", prot_type, ret);
	return ret;
}

static const char *dc_get_protocol_str(int prot)
{
	if (prot >= ADAPTER_PROTOCOL_END || prot < ADAPTER_PROTOCOL_BEGIN)
		return "scp";

	return g_prot_str[prot];
}

struct adpt_bigdata_info {
	int vendor_id;
	int serial_num;
	int adp_code;
	unsigned int adp_mode;
	int cable_type;
	int cable_cur;
};

static void dc_adpt_update_bigdata_info(struct adpt_bigdata_info *info, int ratio)
{
	if (adapter_get_chip_vendor_id(g_prot, &info->vendor_id))
		hwlog_info("get vendor id fail\n");

	if (adapter_get_chip_serial_num(g_prot, &info->serial_num))
		hwlog_info("get serial num fail\n");

	if (adapter_get_adp_code(g_prot, &info->adp_code))
		hwlog_info("get adapter type fail\n");

	info->adp_mode = dc_adpt_get_adapter_support_mode();

	/* 0: DC_CABLE_ATC 1: DC_CABLE_CTC */
	info->cable_type = dc_get_cable_inherent_info(DC_IS_CTC_CABLE);
	info->cable_cur = dc_get_cable_curr() / ratio;
}

static void dc_report_antifake_result(int mode, int res, int cur, int pwr, int ratio)
{
	int len;
	struct adpt_bigdata_info info = { 0 };
	char auth_buf[POWER_EVENT_NOTIFY_SIZE] = { 0 };
	struct power_event_notify_data n_data;
	struct dc_adpt_dev *l_di = dc_adpt_get_di(mode);
	struct dc_adpt_inherent_para *para = dc_adpt_get_inherent_para();
	int auth_state = (res == ADAPTER_ANTIFAKE_FAIL) ? res : ADAPTER_ANTIFAKE_SUCC;

	if (!l_di || !para || para->adp_antifake_report_flag)
		return;

	if (charge_get_charger_online() == 0) {
		hwlog_info("charger has been removed, no need report event\n");
		return;
	}

	dc_adpt_update_bigdata_info(&info, ratio);
	len = snprintf(auth_buf, POWER_EVENT_NOTIFY_SIZE - 1,
		"BMS_EVT=EVT_ADAPTER_AUTH_UPDATE@vendor_id:%d,serial_no:%d,protocol:%s,adp_type:%d,adp_mode:%u,"
		"max_cur:%d,max_pwr:%d,auth_state:%d,auth_state_extra:%d,cable_type:%d,cable_cur:%d\n",
		info.vendor_id, info.serial_num, dc_get_protocol_str(g_prot), info.adp_code, info.adp_mode,
		cur, pwr, auth_state, res, info.cable_type, info.cable_cur);
	if (len <= 0)
		return;

	n_data.event_len = len;
	n_data.event = auth_buf;
	if (l_di->dts_info.antifake_para.adp_antifake_execute_enable &&
		!((g_prot == ADAPTER_PROTOCOL_UFCS) && (res == ADAPTER_ANTIFAKE_FAIL)))
		power_event_report_uevent(&n_data);
	else
		hwlog_info("adapter antifake result:%s\n", auth_buf);

	if (res == ADAPTER_ANTIFAKE_FAIL)
		power_dsm_report_dmd(POWER_DSM_BATTERY, POWER_DSM_ERROR_NON_STANDARD_CHARGER_PLUGGED, auth_buf);
	para->adp_antifake_report_flag = true;
}

static bool dc_adpt_antifake_22p5w_check_failed(int cur)
{
	int max_vol = 0;

	dc_get_adapter_max_voltage(&max_vol);
	if ((cur <= DC_ANTI_22P5W_CURR_ANOMALY_THRE) && (max_vol < DC_ANTI_22P5W_VOLT_ANOMALY_THRE)) {
		hwlog_err("22p5w antifake fail, can not do direct charge\n");
		return true;
	}

	return false;
}

static bool dc_adpt_need_auth_encrypt(int max_pwr)
{
	if ((max_pwr <= POWER_TH_IGNORE_ANTIFAKE) ||
		((g_prot == ADAPTER_PROTOCOL_UFCS) && (max_pwr <= POWER_TH_UFCS_ILIMIT_ANTIFAKE)))
		return false;

	return true;
}

int dc_check_adapter_antifake(int mode, int ratio)
{
	int max_cur, max_pwr;
	int bat_vol = hw_battery_get_series_num() * BAT_RATED_VOLT;
	struct dc_adpt_dts_info *dts_info = dc_adpt_get_dts_info(mode);
	struct dc_adpt_inherent_para *para = dc_adpt_get_inherent_para();
	int antifake_res = ADAPTER_ANTIFAKE_INIT;

	if (!dts_info || !para || !ratio)
		return 0;

	if ((para->adp_antifake_result != ADAPTER_ANTIFAKE_FAIL) && para->adp_antifake_report_flag) {
		hwlog_info("antifake already checked\n");
		return 0;
	}

	max_cur = dc_get_adapter_max_current(mode, bat_vol * ratio);
	max_pwr = max_cur * bat_vol * ratio / POWER_UW_PER_MW; /* unit: mw */
	hwlog_info("max_cur=%d, max_pwr=%d\n", max_cur, max_pwr);

	if (max_cur <= 0)
		return -EPERM;

	if ((dts_info->antifake_para.anti_22p5w_para.strategy & DC_ANTI_22P5W_PWR)
		&& dc_adpt_antifake_22p5w_check_failed(max_cur))
			goto fail_end;

	if (dts_info->antifake_para.adp_antifake_enable && dc_adpt_need_auth_encrypt(max_pwr)) {
		if (adapter_auth_encrypt_start(g_prot, dts_info->antifake_para.adp_antifake_key_index))
			goto fail_end;
		else
			antifake_res = ADAPTER_ANTIFAKE_SUCC;
	}
	goto end;

fail_end:
	antifake_res = ADAPTER_ANTIFAKE_FAIL;
end:
	para->adp_antifake_result = antifake_res;
	dc_report_antifake_result(mode, antifake_res, max_cur, max_pwr, ratio);
	if (g_prot == ADAPTER_PROTOCOL_UFCS || antifake_res != ADAPTER_ANTIFAKE_FAIL)
		return 0;

	para->adp_antifake_failed_cnt++;
	return -EPERM;
}

static bool dc_adpt_is_iwatt_lvc(void)
{
	int adapter_type;
	struct dc_adpt_inherent_para *para = dc_adpt_get_inherent_para();

	if (para->adaptor_vendor_id != ADAPTER_CHIP_IWATT)
		return false;

	adapter_type = dc_get_adapter_type();
	if (adapter_type != ADAPTER_TYPE_5V4P5A)
		return false;

	return true;
}

bool dc_adpt_skip_ileak_check(void)
{
	if (dc_adpt_is_iwatt_lvc()) {
		hwlog_info("iwatt lvc skip check ileak\n");
		return true;
	}
	return false;
}

bool dc_adpt_skip_security_check(int mode)
{
	struct dc_adpt_dev *di = dc_adpt_get_di(mode);

	if (!di)
		return false;

	if (di->dts_info.iwatt_limit[IWATT_SKIP_SECURITY_CHECK] &&
		dc_adpt_is_iwatt_lvc())
		return true;

	return false;
}

static int dc_adpt_get_gain_current(int mode)
{
	int adapter_type = dc_get_adapter_type();
	struct dc_adpt_dev *di = dc_adpt_get_di(mode);

	if (!di)
		return 0;

	switch (adapter_type) {
	case ADAPTER_TYPE_10V2A:
		return (int)di->dts_info.gain_curr_10v2a;
	default:
		return 0;
	}
}

int dc_adpt_update_delta_err(int mode, int delta_err)
{
	int adapter_type = dc_get_adapter_type();
	struct dc_adpt_dev *di = dc_adpt_get_di(mode);
	int ret = delta_err;

	if (!di)
		return ret;

	switch (adapter_type) {
	case ADAPTER_TYPE_10V2P25A:
		if (di->dts_info.delta_err_10v2p25a)
			ret = (int)di->dts_info.delta_err_10v2p25a;
		break;
	default:
		break;
	}

	return ret;
}

static void dc_adpt_update_max_pwr_time_para(struct dc_max_power_time_para *dts_para,
	struct dc_max_power_time_para *para)
{
	int i, adapter_type;

	if (!dts_para[0].adatper_type)
		return;

	adapter_type = dc_get_adapter_type();
	for (i = 0; i < DC_MAX_POWER_TIME_PARA_LEVEL; i++) {
		if (adapter_type != dts_para[i].adatper_type)
			continue;
		para->adatper_type = adapter_type;
		para->limit_current = dts_para[i].limit_current;
		para->max_power_time = dts_para[i].max_power_time;
		hwlog_info("max_power_timeout=%d,max_power_limit_current=%d\n",
			para->max_power_time, para->limit_current);
		return;
	}
}

static int dc_adpt_get_max_pwr_limit_by_time(int ibat_th, int ratio)
{
	int run_time, current_time;
	struct dc_adpt_inherent_para *para = dc_adpt_get_inherent_para();

	if (!para || !ratio)
		return 0;

	current_time = power_get_current_kernel_time().tv_sec;
	run_time = current_time - para->max_pwr_start_time;
	if (para->max_pwr_flag && (run_time >= para->max_pwr_para.max_power_time))
		return para->max_pwr_para.limit_current / ratio;

	if (ibat_th / ratio < para->max_cur) {
		para->max_pwr_flag = false;
		return 0;
	}

	if (!para->max_pwr_flag) {
		para->max_pwr_start_time = current_time;
		para->max_pwr_flag = true;
	}

	return 0;
}

static int dc_adpt_get_runtime_max_curr(int mode, int adap_vset)
{
	int max_curr;

	max_curr = dc_get_adapter_max_current(mode, adap_vset);
	max_curr += dc_adpt_get_gain_current(mode);
	/* some adapters support power drop, will reduce power when OTP */
	(void)dc_get_power_drop_current(&max_curr);

	return max_curr;
}

void dc_adpt_init_curr(int mode, int adap_vset, int ratio)
{
	int bat_volt = hw_battery_get_series_num() * BAT_RATED_VOLT;
	struct dc_adpt_dev *di = dc_adpt_get_di(mode);
	struct dc_adpt_inherent_para *para = dc_adpt_get_inherent_para();

	if (!di || !ratio || !bat_volt)
		return;

	dc_vote_clear(VOTE_OBJ_ADPT_CURR);
	dc_vote_set(VOTE_OBJ_ADPT_CURR, VOTE_CLIENT_ADPT_MAX_CURR, true,
		dc_adpt_get_runtime_max_curr(mode, adap_vset));

	if (dc_adpt_is_iwatt_lvc() && di->dts_info.iwatt_limit[IWATT_LIMIT_IBAT])
		dc_vote_set(VOTE_OBJ_ADPT_CURR, VOTE_CLIENT_ADPT_IWATT, true,
			di->dts_info.iwatt_limit[IWATT_LIMIT_IBAT]);

	/* limit power to 30w when ufcs adapter antifake failed */
	if ((g_prot == ADAPTER_PROTOCOL_UFCS) &&
		(para->adp_antifake_result == ADAPTER_ANTIFAKE_FAIL))
		dc_vote_set(VOTE_OBJ_ADPT_CURR, VOTE_CLIENT_ADPT_ANTIFAKE, true,
			POWER_TH_UFCS_ILIMIT_ANTIFAKE * POWER_UW_PER_MW /
			(bat_volt * ratio));

	para->max_cur = dc_vote_get_effective_result_with_lock(VOTE_OBJ_ADPT_CURR);
	hwlog_info("adp_max_cur=%d\n", para->max_cur);
	dc_adpt_update_max_pwr_time_para(di->dts_info.max_power_time, &para->max_pwr_para);
}

int dc_adpt_get_curr(int mode, int adap_vset)
{
	int curr;
	int ratio = dc_info_get_val_with_mode(mode, VOLT_RATIO);
	int ibat_th = dc_info_get_val_with_mode(mode, IBAT_TH_HIGH);

	curr = dc_adpt_get_runtime_max_curr(mode, adap_vset);
	dc_vote_set(VOTE_OBJ_ADPT_CURR, VOTE_CLIENT_ADPT_MAX_CURR, true, curr);

	curr = dc_adpt_get_max_pwr_limit_by_time(ibat_th, ratio);
	if (curr)
		dc_vote_set(VOTE_OBJ_ADPT_CURR, VOTE_CLIENT_ADPT_TIME, true, curr);

	return dc_vote_get_effective_result_with_lock(VOTE_OBJ_ADPT_CURR);
}

static void dc_adpt_report_22p5w_antifake_result(int ibus, int ibus_gap, int ibus_gap_th)
{
	int len;
	char auth_buf[POWER_EVENT_NOTIFY_SIZE] = { 0 };
	struct power_event_notify_data n_data;

	if (charge_get_charger_online() == 0) {
		hwlog_info("charger has been removed, no need report event\n");
		return;
	}

	len = snprintf(auth_buf, POWER_EVENT_NOTIFY_SIZE - 1,
		"BMS_EVT=EVT_REAL_DEL_CUR_UPDATE@ibus:%d,ibus_gap:%d,ibus_gap_th:%d\n",
		ibus, ibus_gap, ibus_gap_th);
	if (len <= 0)
		return;

	n_data.event_len = len;
	n_data.event = auth_buf;
	power_event_report_uevent(&n_data);
}

int dc_adpt_22p5w_det(int mode, int ibat_th_high, int ratio, int cur_stage)
{
	int ibus = 0;
	int ibus_gap, err_cnt_max, ibus_gap_th;
	time64_t delta_time;
	struct dc_adpt_dts_info *dts_info = dc_adpt_get_dts_info(mode);
	struct dc_adpt_inherent_para *para = dc_adpt_get_inherent_para();

	if (!dts_info || !para || !ratio ||
		!(dts_info->antifake_para.anti_22p5w_para.strategy & DC_ANTI_22P5W_CUR))
		return DC_NO_NEED_ANOMALY_DET;

	err_cnt_max = dts_info->antifake_para.anti_22p5w_para.ibus_low_time / DC_ANOMALY_DET_INTERVAL;
	if (para->cur_err_cnt >= err_cnt_max)
		return DC_NO_NEED_ANOMALY_DET;

	dc_get_device_ibus(&ibus);
	ibus_gap = ibat_th_high / ratio - ibus;
	delta_time = power_get_monotonic_boottime() - dc_ctrl_get_start_time();
	if ((cur_stage % 2) || (delta_time < DC_ANTI_22P5W_START_CHK_TIME))
		return DC_NEED_ANOMALY_DET;

	ibus_gap_th = dts_info->antifake_para.anti_22p5w_para.ibus_gap;
	if (ibus_gap < ibus_gap_th) {
		para->cur_err_cnt = 0;
		return DC_NEED_ANOMALY_DET;
	}

	para->cur_err_cnt += 1;
	if (para->cur_err_cnt >= err_cnt_max) {
		/* report the charge ability not enough and stop 22p5w adapter detect */
		dc_adpt_report_22p5w_antifake_result(ibus, ibus_gap, ibus_gap_th);
		return DC_NO_NEED_ANOMALY_DET;
	}

	return DC_NEED_ANOMALY_DET;
}

static void dc_adpt_parse_max_power_time_para(
	struct device_node *np, struct dc_max_power_time_para *para)
{
	int len, row, col;
	int idata[DC_MAX_POWER_TIME_PARA_LEVEL * DC_MAX_POWER_PARA_MAX] = { 0 };

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		"adapter_max_power_time_para", idata, DC_MAX_POWER_TIME_PARA_LEVEL,
		DC_MAX_POWER_PARA_MAX);
	if (len < 0)
		return;

	for (row = 0; row < len / DC_MAX_POWER_PARA_MAX; row++) {
		col = row * DC_MAX_POWER_PARA_MAX + DC_ADAPTER_TYPE;
		para[row].adatper_type = idata[col];
		col = row * DC_MAX_POWER_PARA_MAX + DC_MAX_POWER_TIME;
		para[row].max_power_time = idata[col];
		col = row * DC_MAX_POWER_PARA_MAX + DC_MAX_POWER_LIMIT_CURRENT;
		para[row].limit_current = idata[col];
	}
}

static void dc_adpt_parse_iwatt_evasive_info(struct device_node *np,
	int *para)
{
	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"iwatt_limit", para, IWATT_LIMIT_INFO_END))
		return;

	hwlog_info("iwatt_limit %d %d\n", para[IWATT_SKIP_SECURITY_CHECK],
		para[IWATT_LIMIT_IBAT]);
}

static void dc_adpt_parse_antifake_para(struct device_node *np,
	struct dc_adpt_antifake_para *para)
{
	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"adaptor_antifake_check_enable", &para->adp_antifake_enable, 0))
		return;

	if (para->adp_antifake_enable != 1) {
		para->adp_antifake_enable = 0;
		return;
	}

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"adaptor_antifake_execute_enable", &para->adp_antifake_execute_enable, 0);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"adaptor_antifake_key_index", &para->adp_antifake_key_index, 1);

	/* set public key as default key in factory mode */
	if (power_cmdline_is_factory_mode())
		para->adp_antifake_key_index = 1;

	hwlog_info("adp_antifake_key_index=%d\n", para->adp_antifake_key_index);
}

static void dc_adpt_parse_antifake_22p5w_para(struct device_node *np,
	struct dc_adpt_antifake_para *para)
{
	u32 idata[DC_22P5W_ALL] = { 0 };

	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np, "adaptor_antifake_22p5w", idata, DC_22P5W_ALL))
		return;

	para->anti_22p5w_para.strategy = (int)idata[DC_22P5W_STRATEGY];
	para->anti_22p5w_para.ibus_gap = (int)idata[DC_IBUS_GAP];
	para->anti_22p5w_para.ibus_low_time = (int)idata[DC_IBUS_LOW_TIME];

	if ((para->anti_22p5w_para.strategy & DC_ANTI_22P5W_PWR) && !para->adp_antifake_enable)
		(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
			"adaptor_antifake_execute_enable", &para->adp_antifake_execute_enable, 0);

	hwlog_info("[22p5w_adapter_antifake_para] strategy=%d, ibus_gap=%d, ibus_low_time=%d\n",
		para->anti_22p5w_para.strategy, para->anti_22p5w_para.ibus_gap, para->anti_22p5w_para.ibus_low_time);
}

static void dc_adpt_parse_para(struct device_node *np,
	struct dc_adpt_dts_info *info)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"max_adaptor_vset", (u32 *)&info->max_adapter_vset, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"adapter_watchdog_time", &info->adapter_watchdog_time, 3); /* default is 3s */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"max_tadapt", (u32 *)&info->max_tadapt, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"adaptor_detect_by_voltage", &info->adaptor_detect_by_voltage, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"delta_err_10v2p25a", (u32 *)&info->delta_err_10v2p25a, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"gain_curr_10v2a", &info->gain_curr_10v2a, 0);

	dc_adpt_parse_iwatt_evasive_info(np, info->iwatt_limit);
	dc_adpt_parse_antifake_para(np, &info->antifake_para);
	dc_adpt_parse_antifake_22p5w_para(np, &info->antifake_para);
	dc_adpt_parse_max_power_time_para(np, info->max_power_time);
}

void dc_adpt_init(struct device *dev, int mode)
{
	int idx;
	struct dc_adpt_dev *di = NULL;

	idx = dc_comm_get_mode_idx(mode);
	if ((idx < 0) || (idx >= DC_MODE_TOTAL))
		return;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return;

	hwlog_info("dc_adpt_init mode %d\n", mode);
	g_dc_adpt_di[idx] = di;
	dc_adpt_parse_para(dev->of_node, &di->dts_info);
}

void dc_adpt_remove(struct device *dev, int mode)
{
	struct dc_adpt_dev *di = dc_adpt_get_di(mode);

	if (!di)
		return;

	hwlog_info("dc_adpt_remove mode %d\n", mode);
	kfree(di);
	di = NULL;
}
