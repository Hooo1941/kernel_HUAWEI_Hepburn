// SPDX-License-Identifier: GPL-2.0
/*
 * direct_charge_cable.c
 *
 * cable detect for direct charge module
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

#include <huawei_platform/usb/hw_pd_dev.h>
#include <huawei_platform/power/battery_voltage.h>
#include <chipset_common/hwpower/common_module/power_common_macro.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_common.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_pmode.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_vote.h>

#define HWLOG_TAG direct_charge_cable
HWLOG_REGIST();

/* for second resistance check */
#define DC_2ND_RESIST_IBUS_TH           4000
#define DC_CTC_EMARK_CURR_5A            5000
#define DC_CTC_EMARK_CURR_6A            6000
#define DC_CTC_EMARK_CURR_7A            7000
#define DC_CABLE_DETECT_CURRENT_THLD    3000

#define DC_CTC_DET_ADP_POWER_THLD       35000
#define DC_CTC_DET_PRODUCT_POWER_THLD   40000

struct dc_resist_data {
	int vadapt;
	int iadapt;
	int vbus;
	int ibus;
};

struct dc_mode_cable_info {
	int full_path_res_thld;
	int full_path_resistance;
	bool second_resist_check_ok;
};

/*
 * define resistance threshold with maximum current
 * support up to 5 parameters list on dts
 */
#define DC_RESIST_LEVEL         5

enum dc_cable_resist_info {
	DC_RESIST_MIN = 0,
	DC_RESIST_MAX,
	DC_RESIST_CUR_MAX,
	DC_RESIST_TOTAL,
};

struct dc_cable_resist_para {
	int resist_min;
	int resist_max;
	int resist_cur_max;
};

struct dc_dts_cable_info {
	u32 is_show_ico_first;
	u32 cc_cable_detect_enable;
	u32 is_send_cable_type;
	int second_resist_check_en;
	int second_path_res_report_th;
	int std_cable_full_path_res_max;
	int ctc_cable_full_path_res_max;
	int nonstd_cable_full_path_res_max;
	int max_current_for_ctc_cable;
	int max_current_for_nonstd_cable;
	struct dc_cable_resist_para ctc_resist_para[DC_RESIST_LEVEL];
	struct dc_cable_resist_para ctc_emark_resist_para[DC_RESIST_LEVEL];
	struct dc_cable_resist_para nonstd_resist_para[DC_RESIST_LEVEL];
	struct dc_cable_resist_para std_resist_para[DC_RESIST_LEVEL];
	struct dc_cable_resist_para ctc_second_resist_para[DC_RESIST_LEVEL];
	struct dc_cable_resist_para second_resist_para[DC_RESIST_LEVEL];
};

struct dc_cable_dev {
	struct device *dev;
	struct dc_dts_cable_info dts_info;
	struct dc_mode_cable_info mode_info;
};

struct dc_cable_inherent_para {
	unsigned int cable_detect_ok;
	unsigned int cable_type;
	unsigned int ctc_cable_type;
	unsigned int ui_cable_type;
	int cable_max_curr;
	bool is_ctc_cable;
	bool is_ufcs_cable;
	bool cable_type_send_flag;
};

static struct dc_resist_data g_resist_base_data;
static struct dc_cable_ops *g_cable_ops;
static struct dc_cable_dev *g_dc_cable_di[DC_MODE_TOTAL];
static struct dc_cable_inherent_para g_cable_inherent_info;

static struct dc_cable_dev *dc_cable_get_di(int mode)
{
	int idx = dc_comm_get_mode_idx(mode);
	if ((idx < 0) || (idx >= DC_MODE_TOTAL))
		return NULL;
	return g_dc_cable_di[idx];
}

int dc_cable_ops_register(struct dc_cable_ops *ops)
{
	int ret = 0;

	if (ops) {
		g_cable_ops = ops;
		hwlog_info("cable ops register ok\n");
	} else {
		hwlog_err("cable ops register fail\n");
		ret = -EINVAL;
	}

	return ret;
}

bool dc_is_support_cable_detect(void)
{
	if (!g_cable_ops || !g_cable_ops->detect)
		return false;
	return true;
}

int dc_cable_detect(void)
{
	if (!g_cable_ops || !g_cable_ops->detect)
		return -EINVAL;

	return g_cable_ops->detect();
}

static int dc_cable_auth_detect(void)
{
	if (!g_cable_ops || !g_cable_ops->cable_auth_det)
		return -EINVAL;

	return g_cable_ops->cable_auth_det();
}

static struct dc_cable_inherent_para *dc_get_cable_inherent_para(void)
{
	return &g_cable_inherent_info;
}

unsigned int dc_get_cable_inherent_info(unsigned int type)
{
	struct dc_cable_inherent_para *para = dc_get_cable_inherent_para();

	if (!para)
		return 0;

	switch (type) {
	case DC_CABLE_DETECT_OK:
		return para->cable_detect_ok;
	case DC_CABLE_TYPE:
		return para->cable_type;
	case DC_IS_CTC_CABLE:
		return (unsigned int)para->is_ctc_cable;
	case DC_CTC_CABLE_TYPE:
		return para->ctc_cable_type;
	case DC_UI_CABLE_TYPE:
		return para->ui_cable_type;
	default:
		break;
	}

	return 0;
}

unsigned int dc_get_cable_mode_info(int mode, int type)
{
	struct dc_cable_dev *l_di = dc_cable_get_di(mode);

	if (!l_di)
		return 0;

	switch (type) {
	case CABLE_FULL_PATH_RES:
		return (unsigned int)l_di->mode_info.full_path_resistance;
	case CABLE_FULL_ATH_RES_THLD:
		return (unsigned int)l_di->mode_info.full_path_res_thld;
	case MAX_CURR_FOR_NONSTD_CABLE:
		return (unsigned int)l_di->dts_info.max_current_for_nonstd_cable;
	case SEC_RES_CHECK_EN:
		return (unsigned int)l_di->dts_info.second_resist_check_en;
	case SEC_RES_CHECK_OK:
		return (unsigned int)l_di->mode_info.second_resist_check_ok;
	default:
		break;
	}

	return 0;
}

void dc_clear_cable_inherent_info_in_exit(void)
{
	memset(&g_cable_inherent_info, 0, sizeof(g_cable_inherent_info));
}

void dc_clear_cable_inherent_info(void)
{
	g_cable_inherent_info.cable_detect_ok = 0;
	g_cable_inherent_info.cable_type = 0;
	g_cable_inherent_info.ctc_cable_type = 0;
	g_cable_inherent_info.ui_cable_type = 0;
	g_cable_inherent_info.cable_max_curr = 0;
	g_cable_inherent_info.is_ctc_cable = false;
	g_cable_inherent_info.is_ufcs_cable = false;
}

void dc_clear_cable_mode_info(int mode)
{
	struct dc_cable_dev *l_di = dc_cable_get_di(mode);

	if (!l_di)
		return;

	memset(&l_di->mode_info, 0, sizeof(l_di->mode_info));
}

int dc_update_cable_type(void)
{
	int curr = 0;
	struct dc_cable_inherent_para *para = dc_get_cable_inherent_para();

	if (!para)
		return -EINVAL;

	if (para->cable_type == DC_STD_CABLE) {
		hwlog_info("cable type detect is finished\n");
		return 0;
	}

	if (dc_adpt_cable_undetached()) {
		para->cable_type = DC_STD_CABLE;
		hwlog_info("undetached cable detect ok\n");
		return 0;
	}

	if (!dc_get_adapter_cable_info(&curr)) {
		para->is_ufcs_cable = true;
		para->cable_max_curr = curr;
		para->cable_type = DC_STD_CABLE;
		return 0;
	}

	para->is_ctc_cable = pd_dpm_get_ctc_cable_flag();
	para->ctc_cable_type = pd_dpm_get_cvdo_cur_cap();
	if (para->is_ctc_cable) {
		if (para->ctc_cable_type >= PD_DPM_CURR_5A)
			para->cable_type = DC_STD_CABLE;
		else
			para->cable_type = DC_NONSTD_CABLE;
		hwlog_info("ctc cable detect, cable_type=%u\n", para->cable_type);
		return 0;
	}

	if (!dc_is_support_cable_detect())
		return -EPERM;

	if (dc_cable_detect())
		para->cable_type = DC_NONSTD_CABLE;
	else
		para->cable_type = DC_STD_CABLE;
	hwlog_info("atc cable detect, cable_type=%u\n", para->cable_type);
	return 0;
}

void dc_update_cable_detect_ok(void)
{
	int max_cur_adp, adapter_type;
	struct dc_cable_inherent_para *para = dc_get_cable_inherent_para();

	if (!para)
		return;

	if (para->cable_type == DC_STD_CABLE) {
		para->cable_detect_ok = CABLE_DETECT_OK;
		return;
	}

	max_cur_adp = dc_pmode_get_max_cur_adp();
	adapter_type = dc_get_adapter_type();
	if (((max_cur_adp > 0) && (max_cur_adp <= DC_CABLE_DETECT_CURRENT_THLD)) ||
		(adapter_type == ADAPTER_TYPE_10V2A) ||
		(adapter_type == ADAPTER_TYPE_10V2P25A)) {
		para->cable_detect_ok = CABLE_DETECT_OK;
		hwlog_info("adp max cur below 3A, set cable_detect_ok flag\n");
	}
}

void dc_update_ui_cable_type(void)
{
	int adp_max_power = 0;
	int product_max_power;
	struct dc_cable_inherent_para *para = dc_get_cable_inherent_para();

	if (!para)
		return;

	if (!para->is_ctc_cable) {
		para->ui_cable_type = (para->cable_detect_ok == CABLE_DETECT_OK) ?
			DC_UI_STD_CABLE : DC_UI_NONSTD_ATC_CABLE;
		hwlog_info("atc cable, ui cable type=%u\n", para->ui_cable_type);
		return;
	}

	if (!pd_dpm_support_cable_auth()) {
		para->ui_cable_type = DC_UI_STD_CABLE;
		hwlog_info("not support cable auth, ctc cable is STD\n");
		return;
	}

	(void)dc_get_adapter_max_power(&adp_max_power);
	product_max_power = dc_pmode_get_max_product_power();
	if (((adp_max_power > 0) && (adp_max_power <= DC_CTC_DET_ADP_POWER_THLD)) ||
		(product_max_power < DC_CTC_DET_PRODUCT_POWER_THLD)) {
		para->ui_cable_type = DC_UI_STD_CABLE;
		hwlog_info("ui cable type is STD, adp_max_power=%d, product_max_power=%d\n",
			adp_max_power, product_max_power);
		return;
	}

	if ((dc_cable_auth_detect() == PD_DPM_RESULT_THIRDPARTY) &&
		(para->ctc_cable_type >= PD_DPM_CURR_5A)) {
		para->ui_cable_type = DC_UI_NONSTD_CTC_CABLE;
		hwlog_info("ctc thirdparty cable, ui cable type is NONSTD\n");
		return;
	}

	if (dc_pmode_is_cable_limit_power())
		para->ui_cable_type = DC_UI_NONSTD_CTC_CABLE;
	else
		para->ui_cable_type = DC_UI_STD_CABLE;
	hwlog_info("ctc cable, ui cable type=%u\n", para->ui_cable_type);
}

void dc_update_cable_resistance_thld(int mode)
{
	struct dc_cable_inherent_para *para = dc_get_cable_inherent_para();
	struct dc_cable_dev *l_di = dc_cable_get_di(mode);

	if (!l_di || !para)
		return;

	if (para->cable_detect_ok) {
		l_di->mode_info.full_path_res_thld = l_di->dts_info.std_cable_full_path_res_max;
		dc_send_icon_uevent();
		return;
	}

	if (para->is_ctc_cable) {
		l_di->mode_info.full_path_res_thld = l_di->dts_info.ctc_cable_full_path_res_max;
		dc_send_icon_uevent();
	} else {
		l_di->mode_info.full_path_res_thld = l_di->dts_info.nonstd_cable_full_path_res_max;
		if (l_di->dts_info.is_show_ico_first)
			dc_send_icon_uevent();
	}
}

static int dc_get_emark_cable_max_current(unsigned int type)
{
	switch (type) {
	case PD_DPM_CURR_5A:
		return DC_CTC_EMARK_CURR_5A;
	case PD_DPM_CURR_6A:
		return DC_CTC_EMARK_CURR_6A;
	case PD_DPM_CURR_7A:
		return DC_CTC_EMARK_CURR_7A;
	default:
		return 0;
	}
}

int dc_get_cable_max_current(int mode)
{
	int ratio;
	int cable_limit = 0;
	struct dc_cable_inherent_para *para = dc_get_cable_inherent_para();
	struct dc_cable_dev *l_di = dc_cable_get_di(mode);

	if (!l_di || !l_di->dts_info.cc_cable_detect_enable || !para)
		return 0;

	ratio = (int)dc_info_get_val_with_mode(mode, VOLT_RATIO);
	if (para->is_ufcs_cable)
		return ratio * para->cable_max_curr;

	if (para->is_ctc_cable) {
		if (para->ctc_cable_type >= PD_DPM_CURR_5A)
			cable_limit = dc_get_emark_cable_max_current(para->ctc_cable_type) * ratio;
		else
			cable_limit = l_di->dts_info.max_current_for_ctc_cable;
	} else {
		if (para->cable_type == DC_STD_CABLE)
			cable_limit = 0;
		else
			cable_limit = l_di->dts_info.max_current_for_nonstd_cable;
	}

	return cable_limit;
}

void dc_update_cable_max_current(int *cable_max_cur)
{
	int max_cur;
	struct dc_cable_inherent_para *para = dc_get_cable_inherent_para();

	if (!para)
		return;

	/* avoid 55W/66W display issues on 5A/6A c2c cable */
	max_cur = dc_get_emark_cable_max_current(para->ctc_cable_type);
	if (para->is_ctc_cable && ((max_cur == DC_CTC_EMARK_CURR_5A) || (max_cur == DC_CTC_EMARK_CURR_6A)))
		*cable_max_cur = *cable_max_cur * 11 / POWER_BASE_DEC; /* amplified 1.1 times. */
}

void dc_init_cable_curr(int working_mode)
{
	int curr = dc_get_cable_max_current(working_mode);

	dc_vote_clear(VOTE_OBJ_CABLE_CURR);
	if (curr)
		dc_vote_set(VOTE_OBJ_CABLE_CURR, VOTE_CLIENT_CABLE_TYPE, true, curr);
}

int dc_get_cable_curr()
{
	int curr = dc_vote_get_effective_result_with_lock(VOTE_OBJ_CABLE_CURR);

	return curr < 0 ? 0 : curr;
}

static int dc_get_path_resistance_info(struct dc_resist_data *data, int working_mode)
{
	struct adapter_source_info source_info = { 0 };
	unsigned int flag = BIT(ADAPTER_OUTPUT_VOLT) | BIT(ADAPTER_OUTPUT_CURR);

	if (dc_get_adapter_source_info(flag, &source_info))
		return -EPERM;
	data->vadapt = source_info.output_volt;
	data->iadapt = source_info.output_curr;

	if (direct_charge_get_device_vbus(&(data->vbus)))
		return -EPERM;

	if (dc_get_device_ibus(&(data->ibus)))
		return -EPERM;
	data->ibus += dc_get_gain_ibus();
	if (data->ibus == 0) {
		hwlog_err("ibus is zero\n");
		return -EPERM;
	}

	return 0;
}

/* get the maximum current allowed by direct charging at specified resist */
int dc_resist_handler(int mode, int value)
{
	int i, ret;
	struct dc_cable_resist_para *resist_para = NULL;
	struct dc_cable_inherent_para *para = dc_get_cable_inherent_para();
	struct dc_cable_dev *l_di = dc_cable_get_di(mode);

	if (!l_di || !para)
		return 0;

	if (para->is_ctc_cable) {
		if (para->ctc_cable_type >= PD_DPM_CURR_5A)
			resist_para = l_di->dts_info.ctc_emark_resist_para;
		else
			resist_para = l_di->dts_info.ctc_resist_para;
	} else {
		if (para->cable_type == DC_STD_CABLE)
			resist_para = l_di->dts_info.std_resist_para;
		else
			resist_para = l_di->dts_info.nonstd_resist_para;
	}

	for (i = 0; i < DC_RESIST_LEVEL; ++i) {
		if ((value >= resist_para[i].resist_min) &&
			(value < resist_para[i].resist_max))
			break;
	}

	ret = i < DC_RESIST_LEVEL ? resist_para[i].resist_cur_max : 0;
	hwlog_info("resist=%d, cable_type=%u, is_ctc_cable=%u, max_curr=%d\n", value,
		para->cable_type, para->is_ctc_cable, ret);
	if (ret)
		dc_vote_set(VOTE_OBJ_CABLE_CURR, VOTE_CLIENT_CABLE_1ST_RES, true, ret);
	return ret;
}

int dc_calculate_path_resistance(int working_mode, int *rpath)
{
	struct dc_resist_data tmp;
	int sum = 0;
	int i, ruem;
	int retry = 3; /* 3 : retry times */
	struct dc_cable_dev *l_di = dc_cable_get_di(working_mode);

	if (!l_di)
		return -ENODEV;

	g_resist_base_data.iadapt = 0;
	g_resist_base_data.vadapt = 0;
	g_resist_base_data.ibus = 0;
	g_resist_base_data.vbus = 0;
	for (i = 0; i < retry; ++i) {
		if (dc_get_path_resistance_info(&tmp, working_mode))
			return -EPERM;

		/* r = v/i, unit is mohm */
		*rpath = (tmp.vadapt - tmp.vbus) * POWER_MO_PER_O / tmp.ibus;
		sum += *rpath;

		g_resist_base_data.ibus += tmp.ibus;
		g_resist_base_data.vbus += tmp.vbus;
		g_resist_base_data.vadapt += tmp.vadapt;

		hwlog_info("[%d]: Rpath=%d, Vapt=%d, Vbus=%d, Iapt=%d, Ibus=%d\n",
			i, *rpath, tmp.vadapt, tmp.vbus, tmp.iadapt, tmp.ibus);
	}

	g_resist_base_data.ibus /= retry;
	g_resist_base_data.vadapt /= retry;
	g_resist_base_data.vbus /= retry;

	*rpath = sum / retry;
	*rpath = *rpath > 0 ? *rpath : -*rpath;
	hwlog_info("Rpath=%d, Vapt=%d, Vbus=%d, Ibus=%d\n",
		*rpath, g_resist_base_data.vadapt, g_resist_base_data.vbus, g_resist_base_data.ibus);

	ruem = (int)power_platform_get_uem_resistance();
	*rpath -= ruem;
	if (ruem)
		hwlog_info("Ruem=%d, new Rpath=%d\n", ruem, *rpath);
	l_di->mode_info.full_path_resistance = *rpath;
	return 0;
}

/* get the maximum current allowed by direct charging at specified secondary resist */
static void dc_second_resist_handler(int mode, int value)
{
	int i, ret;
	struct dc_cable_resist_para *resist_para = NULL;
	struct dc_cable_inherent_para *para = dc_get_cable_inherent_para();
	struct dc_cable_dev *l_di = dc_cable_get_di(mode);

	if (!l_di || !l_di->mode_info.second_resist_check_ok || !para)
		return;

	if (para->is_ctc_cable)
		resist_para = l_di->dts_info.ctc_second_resist_para;
	else
		resist_para = l_di->dts_info.second_resist_para;

	for (i = 0; i < DC_RESIST_LEVEL; ++i) {
		if ((value >= resist_para[i].resist_min) &&
			(value < resist_para[i].resist_max))
			break;
	}

	ret = i < DC_RESIST_LEVEL ? resist_para[i].resist_cur_max : 0;
	hwlog_info("second path resist=%d, cur=%d\n", value, ret);
	if (ret)
		dc_vote_set(VOTE_OBJ_CABLE_CURR, VOTE_CLIENT_CABLE_2ND_RES, true, ret);
}

static bool dc_need_calculate_second_path_resistance(int working_mode, int ibus)
{
	int ibus_th = DC_2ND_RESIST_IBUS_TH;

	if (ibus < ibus_th)
		return false;

	return true;
}

void dc_calculate_second_path_resistance(int working_mode, int ibus)
{
	struct dc_resist_data tmp;
	struct dc_resist_data resist = { 0 };
	int i, rpath, ruem;
	int retry = 5; /* 5 : retry times */
	char tmp_buf[ERR_NO_STRING_SIZE] = { 0 };
	struct dc_cable_dev *l_di = dc_cable_get_di(working_mode);

	if (!l_di || !dc_need_calculate_second_path_resistance(working_mode, ibus))
		return;

	for (i = 0; i < retry; ++i) {
		if (dc_get_path_resistance_info(&tmp, working_mode))
			return;

		resist.ibus += tmp.ibus;
		resist.vbus += tmp.vbus;
		resist.vadapt += tmp.vadapt;

		hwlog_info("[%d]: Vapt=%d, Vbus=%d, Iapt=%d, Ibus=%d\n",
			i, tmp.vadapt, tmp.vbus, tmp.iadapt, tmp.ibus);
	}

	resist.ibus = resist.ibus / retry;
	resist.vadapt = resist.vadapt / retry;
	resist.vbus = resist.vbus / retry;

	if (resist.ibus - g_resist_base_data.ibus == 0)
		return;

	/* r = v/i, unit is mohm */
	rpath = ((resist.vadapt - g_resist_base_data.vadapt) -
		(resist.vbus - g_resist_base_data.vbus)) * POWER_MO_PER_O /
		(resist.ibus - g_resist_base_data.ibus);
	rpath = rpath > 0 ? rpath : -rpath;

	snprintf(tmp_buf, sizeof(tmp_buf),
		"f_Rpath=%d,S_Rpath=%d,Vapt=%d,Vbus=%d,Ibus=%d,Vapt0=%d,Vbus0=%d,Ibus0=%d,adp_type=%d,ctc_cable=%d\n",
		l_di->mode_info.full_path_resistance, rpath, resist.vadapt, resist.vbus,
		resist.ibus, g_resist_base_data.vadapt,
		g_resist_base_data.vbus, g_resist_base_data.ibus,
		dc_get_adapter_type(), pd_dpm_get_ctc_cable_flag());

	hwlog_info("%s\n", tmp_buf);

	ruem = (int)power_platform_get_uem_resistance();
	rpath -= ruem;
	if (ruem)
		hwlog_info("Ruem=%d, new S_Rpath=%d\n", ruem, rpath);

	if (rpath >= l_di->dts_info.second_path_res_report_th)
		power_dsm_report_dmd(POWER_DSM_BATTERY,
			POWER_DSM_DIRECT_CHARGE_FULL_PATH_RESISTANCE_2ND, tmp_buf);

	l_di->mode_info.second_resist_check_ok = true;
	dc_second_resist_handler(working_mode, rpath);
}

void dc_send_cable_type_uevent(int mode)
{
	unsigned int ui_cable_type;
	struct dc_cable_dev *l_di = dc_cable_get_di(mode);
	struct dc_cable_inherent_para *para = dc_get_cable_inherent_para();

	if (!l_di || !para || l_di->dts_info.is_send_cable_type == 0)
		return;

	if (para->cable_type_send_flag)
		return;

	ui_cable_type = dc_get_cable_inherent_info(DC_UI_CABLE_TYPE);
	power_ui_event_notify(POWER_UI_NE_CABLE_TYPE, &ui_cable_type);
	para->cable_type_send_flag = true;
}

static void dc_cable_parse_resist_para(struct device_node *np,
	struct dc_cable_resist_para *data, const char *name)
{
	int row, col, len;
	int idata[DC_RESIST_LEVEL * DC_RESIST_TOTAL] = { 0 };

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		name, idata, DC_RESIST_LEVEL, DC_RESIST_TOTAL);
	if (len < 0)
		return;

	for (row = 0; row < len / DC_RESIST_TOTAL; row++) {
		col = row * DC_RESIST_TOTAL + DC_RESIST_MIN;
		data[row].resist_min = idata[col];
		col = row * DC_RESIST_TOTAL + DC_RESIST_MAX;
		data[row].resist_max = idata[col];
		col = row * DC_RESIST_TOTAL + DC_RESIST_CUR_MAX;
		data[row].resist_cur_max = idata[col];
	}
}

static void dc_cable_parse_para(struct device_node *np,
	struct dc_dts_cable_info *info)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "standard_cable_full_path_res_max",
		(u32 *)&info->std_cable_full_path_res_max, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "full_path_res_max",
		(u32 *)&info->nonstd_cable_full_path_res_max, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "ctc_cable_full_path_res_max",
		(u32 *)&info->ctc_cable_full_path_res_max, 320); /* default is 320mohm */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"is_show_ico_first", &info->is_show_ico_first, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"is_send_cable_type", &info->is_send_cable_type, 1);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "max_current_for_none_standard_cable",
		(u32 *)&info->max_current_for_nonstd_cable, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "max_current_for_ctc_cable",
		(u32 *)&info->max_current_for_ctc_cable, info->max_current_for_nonstd_cable);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"second_resist_check_en", (u32 *)&info->second_resist_check_en, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "second_path_res_report_th",
		(u32 *)&info->second_path_res_report_th, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"cc_cable_detect_enable", &info->cc_cable_detect_enable, 0);
	dc_cable_parse_resist_para(np, info->std_resist_para, "std_resist_para");
	dc_cable_parse_resist_para(np, info->second_resist_para, "second_resist_para");
	dc_cable_parse_resist_para(np, info->ctc_second_resist_para, "ctc_second_resist_para");
	dc_cable_parse_resist_para(np, info->nonstd_resist_para, "resist_para");
	dc_cable_parse_resist_para(np, info->ctc_resist_para, "ctc_resist_para");
	dc_cable_parse_resist_para(np, info->ctc_emark_resist_para, "ctc_emark_resist_para");
}

void dc_cable_init(struct device *dev, int mode)
{
	int idx;
	struct dc_cable_dev *di = NULL;

	idx = dc_comm_get_mode_idx(mode);
	if (idx < 0)
		return;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return;

	hwlog_info("dc_cable_init mode %d\n", mode);
	g_dc_cable_di[idx] = di;
	dc_cable_parse_para(dev->of_node, &di->dts_info);
}

void dc_cable_remove(struct device *dev, int mode)
{
	struct dc_cable_dev *di = dc_cable_get_di(mode);

	if (!di)
		return;

	hwlog_info("dc_cable_remove mode %d\n", mode);
	kfree(di);
	di = NULL;
}
