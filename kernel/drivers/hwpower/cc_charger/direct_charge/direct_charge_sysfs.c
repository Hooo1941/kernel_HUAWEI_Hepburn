// SPDX-License-Identifier: GPL-2.0
/*
 * direct_charge_sysfs.c
 *
 * direct charger sysfs driver
 *
 * Copyright (c) 2012-2022 Huawei Technologies Co., Ltd.
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

#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_cmdline.h>
#include <chipset_common/hwpower/direct_charge/direct_charger.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_sysfs.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_common.h>
#include <linux/atomic.h>

#define HWLOG_TAG direct_charge_sysfs
HWLOG_REGIST();

#define DC_SYSFS_RD_BUF_SIZE     64
#define DC_SYSFS_OUT_BUF_SIZE    256
#define DC_SYSFS_TMP_BUF_SIZE    32

static const char * const g_dc_sysfs_op_user_table[DC_SYSFS_OP_USER_END] = {
	[DC_SYSFS_OP_USER_CAMERA] = "camera",
};

static struct dc_sysfs_dev *g_dc_sysfs_di[DC_MODE_TOTAL];

static struct dc_sysfs_dev *dc_sysfs_get_di(int mode)
{
	int idx = dc_comm_get_mode_idx(mode);
	if ((idx < 0) || (idx >= DC_MODE_TOTAL))
		return NULL;

	return g_dc_sysfs_di[idx];
}

static int dc_sysfs_get_op_user(const char *str)
{
	unsigned int i;

	for (i = 0; i < DC_SYSFS_OP_USER_END; i++) {
		if (!strcmp(str, g_dc_sysfs_op_user_table[i]))
			return i;
	}

	hwlog_err("invalid user_str=%s\n", str);
	return -EPERM;
}

int dc_sysfs_set_iin_thermal(int local_mode, int val)
{
	struct direct_charge_device *di = direct_charge_get_di_by_mode(local_mode);
	struct dc_sysfs_dev *l_dev = dc_sysfs_get_di(local_mode);
	int index;
	int cur_low;
	unsigned int idx;

	if (!di || !l_dev) {
		hwlog_err("di is null\n");
		return -EPERM;
	}

	if (val > di->iin_thermal_default) {
		hwlog_err("val=%d greater than default=%d, use default val\n", val, di->iin_thermal_default);
		val = di->iin_thermal_default;
	}

	if ((di->stage_size < 1) || (di->stage_size > DC_VOLT_LEVEL)) {
		hwlog_err("invalid stage_size=%d\n", di->stage_size);
		return -EPERM;
	}

	index = di->orig_volt_para_p[0].stage_size - 1;
	cur_low = di->orig_volt_para_p[0].volt_info[index].cur_th_low;
	idx = (di->cur_mode == CHARGE_MULTI_IC) ? DC_DUAL_CHANNEL : DC_SINGLE_CHANNEL;
	if (val == 0)
		l_dev->iin_thermal_array[idx] = di->iin_thermal_default;
	else if (val < cur_low)
		l_dev->iin_thermal_array[idx] = cur_low;
	else
		l_dev->iin_thermal_array[idx] = val;

	hwlog_info("set iin_thermal: %u, limit current: %d, current channel type: %u\n",
		val, l_dev->iin_thermal_array[idx], idx);
	return 0;
}

int dc_sysfs_set_iin_thermal_array(int local_mode, unsigned int idx, int val)
{
	struct dc_sysfs_dev *l_dev = dc_sysfs_get_di(local_mode);
	struct direct_charge_device *di = direct_charge_get_di_by_mode(local_mode);
	int index;
	int cur_low;

	if (!di || !l_dev) {
		hwlog_err("di is null\n");
		return -EPERM;
	}

	if (val > di->iin_thermal_default) {
		hwlog_err("val=%d greater than default=%d, use default val\n", val, di->iin_thermal_default);
		val = di->iin_thermal_default;
	}

	if ((di->stage_size < 1) || (di->stage_size > DC_VOLT_LEVEL)) {
		hwlog_err("invalid stage_size=%d\n", di->stage_size);
		return -EPERM;
	}

	index = di->orig_volt_para_p[0].stage_size - 1;
	cur_low = di->orig_volt_para_p[0].volt_info[index].cur_th_low;

	if (val == 0)
		l_dev->iin_thermal_array[idx] = di->iin_thermal_default;
	else if (val < cur_low)
		l_dev->iin_thermal_array[idx] = cur_low;
	else
		l_dev->iin_thermal_array[idx] = val;
	l_dev->iin_thermal = l_dev->iin_thermal_array[idx];

	hwlog_info("set iin_thermal: %d, limit current: %d, channel type: %u\n",
		val, l_dev->iin_thermal_array[idx], idx);
	return 0;
}

void dc_sysfs_set_ignore_full_path_res(int mode, int val)
{
	struct dc_sysfs_dev *di = dc_sysfs_get_di(mode);

	if (!di)
		return;

	dc_sysfs_get_di(mode)->ignore_full_path_res = val;
	hwlog_info("dc_set_ignore_full_path_res %d\n", val);
}

int dc_sysfs_get_iin_thermal(int mode)
{
	struct dc_sysfs_dev *di = dc_sysfs_get_di(mode);

	if (!di)
		return 0;

	return di->iin_thermal;
}

void dc_sysfs_get_iin_thermal_limit(int local_mode, int cur_mode, unsigned int *val)
{
	struct dc_sysfs_dev *di = dc_sysfs_get_di(local_mode);
	unsigned int idx;

	if (!di) {
		*val = 0;
		hwlog_err("di is null\n");
		return;
	}

	idx = (cur_mode == CHARGE_MULTI_IC) ? DC_DUAL_CHANNEL : DC_SINGLE_CHANNEL;
	*val = di->iin_thermal_array[idx];
}

int dc_sysfs_get_ichg_thermal(int local_mode, int cur_mode)
{
	struct dc_sysfs_dev *di = dc_sysfs_get_di(local_mode);
	int ichg_thermal_cur;
	unsigned int idx;

	if (!di)
		return 0;

	idx = (cur_mode == CHARGE_MULTI_IC) ? DC_DUAL_CHANNEL : DC_SINGLE_CHANNEL;
	di->iin_thermal = di->iin_thermal_array[idx];

	if (di->ichg_control_enable)
		ichg_thermal_cur = di->ichg_thermal;
	else
		ichg_thermal_cur = di->iin_thermal;

	hwlog_info("ichg_thermal_cur=%d\n", ichg_thermal_cur);
	return ichg_thermal_cur;
}

int dc_sysfs_get_ignore_full_path_res(int mode)
{
	struct dc_sysfs_dev *di = dc_sysfs_get_di(mode);

	if (!di)
		return 0;

	hwlog_err("dc_get_ignore_full_path_res %d\n", di->ignore_full_path_res);
	return di->ignore_full_path_res;
}

bool dc_sysfs_get_disable_flag(int mode)
{
	struct dc_sysfs_dev *di = dc_sysfs_get_di(mode);
	int i;

	if (!di)
		return false;

	for (i = 0; i < DC_SYSFS_OP_USER_END; i++) {
		if (atomic_read(&di->disable_charger[i]) > 0)
			return true;
	}

	return false;
}

#ifdef CONFIG_SYSFS
static ssize_t dc_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t dc_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info g_dc_sysfs_field_tbl[] = {
	power_sysfs_attr_rw(dc, 0644, DC_SYSFS_IIN_THERMAL, iin_thermal),
	power_sysfs_attr_rw(dc, 0644, DC_SYSFS_IIN_THERMAL_ICHG_CONTROL, iin_thermal_ichg_control),
	power_sysfs_attr_rw(dc, 0644, DC_SYSFS_ICHG_CONTROL_ENABLE, ichg_control_enable),
	power_sysfs_attr_rw(dc, 0644, DC_SYSFS_THERMAL_REASON, thermal_reason),
	power_sysfs_attr_ro(dc, 0444, DC_SYSFS_ADAPTER_DETECT, adaptor_detect),
	power_sysfs_attr_ro(dc, 0444, DC_SYSFS_IADAPT, iadapt),
	power_sysfs_attr_ro(dc, 0444, DC_SYSFS_FULL_PATH_RESISTANCE, full_path_resistance),
	power_sysfs_attr_ro(dc, 0444, DC_SYSFS_DIRECT_CHARGE_SUCC, direct_charge_succ),
	power_sysfs_attr_ro(dc, 0444, DC_SYSFS_ADAPTER_ANTIFAKE_STATE, adaptor_antifake_state),
	power_sysfs_attr_ro(dc, 0444, DC_SYSFS_CABLE_TYPE, cable_type),
	power_sysfs_attr_rw(dc, 0644, DC_SYSFS_SET_RESISTANCE_THRESHOLD, set_resistance_threshold),
	power_sysfs_attr_rw(dc, 0644, DC_SYSFS_SET_CHARGETYPE_PRIORITY, set_chargetype_priority),
	power_sysfs_attr_ro(dc, 0444, DC_SYSFS_MULTI_SC_CUR, multi_sc_cur),
	power_sysfs_attr_ro(dc, 0444, DC_SYSFS_MULTI_IBAT, multi_ibat),
	power_sysfs_attr_ro(dc, 0444, DC_SYSFS_CHARGING_INFO, charging_info),
	power_sysfs_attr_ro(dc, 0444, DC_SYSFS_SC_STATE, sc_state),
	power_sysfs_attr_rw(dc, 0664, DC_SYSFS_SC_FREQ, sc_freq),
	power_sysfs_attr_ro(dc, 0444, DC_SYSFS_DC_IBUS, ibus),
	power_sysfs_attr_ro(dc, 0444, DC_SYSFS_ANTI_22P5W_STRATEGY, anti_22p5w_strategy),
	power_sysfs_attr_rw(dc, 0664, DC_SYSFS_DISABLE_CHARGER, disable_charger),
};

#define DC_SYSFS_ATTRS_SIZE   ARRAY_SIZE(g_dc_sysfs_field_tbl)

static struct attribute *g_dc_sysfs_attrs[DC_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group g_dc_sysfs_attr_group = {
	.attrs = g_dc_sysfs_attrs,
};

static const char *g_dc_sysfs_link_name[] = {
	"direct_charger",
	"direct_charger_sc",
	"direct_charger_sc4",
};

static int dc_sysfs_get_multi_cur(struct dc_sysfs_dev *l_dev, int local_mode, char *buf, size_t offset)
{
	struct direct_charge_device *di = direct_charge_get_di_by_mode(local_mode);
	int ret;
	int i = 0;
	char temp_buf[DC_SC_CUR_LEN] = {0};

	if (!power_cmdline_is_factory_mode())
		return 0;

	if (!di || (di->multi_ic_mode_para.support_multi_ic == 0)) {
		hwlog_err("di is null or not support multi ic\n");
		return 0;
	}

	ret = snprintf(buf, PAGE_SIZE, "%d,%d", di->curr_info.channel_num,
		di->curr_info.coul_ibat_max);
	while ((ret > 0) && (i < di->curr_info.channel_num)) {
		memset(temp_buf, 0, sizeof(temp_buf));
		ret += snprintf(temp_buf, DC_SC_CUR_LEN,
			"\r\n^MULTICHARGER:%s,%d,%d,%d,%d",
			di->curr_info.ic_name[i], di->curr_info.ibus[i],
			di->curr_info.vout[i], di->curr_info.vbat[i],
			di->curr_info.tbat[i]);
		strncat(buf, temp_buf, strlen(temp_buf));
		i++;
	}

	return ret;
}

static int dc_sysfs_get_multi_ibat(struct dc_sysfs_dev *l_dev, int local_mode, char *buf, size_t offset)
{
	struct direct_charge_device *di = direct_charge_get_di_by_mode(local_mode);
	int ret;
	int i = 0;
	char temp_buf[DC_SC_CUR_LEN] = {0};

	if (!power_cmdline_is_factory_mode())
		return 0;

	if (!di || (di->multi_ic_mode_para.support_multi_ic == 0)) {
		hwlog_err("di is null or not support multi ic\n");
		return 0;
	}

	ret = snprintf(buf, PAGE_SIZE, "%d,%d\r\n^GETCHARGEINFO:PMU,%d",
		di->curr_info.channel_num, di->curr_info.coul_ibat_max, di->curr_info.coul_ibat_max);

	while ((ret > 0) && (i < di->curr_info.channel_num)) {
		memset(temp_buf, 0, sizeof(temp_buf));
		ret += snprintf(temp_buf, DC_SC_CUR_LEN, "\r\n^GETCHARGEINFO:%s,%d",
			di->curr_info.ic_name[i], di->curr_info.ibat[i]);
		if ((PAGE_SIZE - strlen(buf)) < strlen(temp_buf))
			return 0;
		strncat(buf, temp_buf, strlen(temp_buf));
		i++;
	}

	return ret;
}

static int dc_sysfs_get_valid_coul_num(int coul_check_flag)
{
	int count = 0;
	int i;

	for (i = 0; i < COUL_IC_NUM; i++) {
		if (!!(coul_check_flag & BIT(i)))
			count++;
	}
	return count;
}

static int dc_sysfs_get_charging_info(struct dc_sysfs_dev *l_dev, int local_mode, char *buf, size_t offset)
{
	struct direct_charge_device *di = direct_charge_get_di_by_mode(local_mode);
	int ret;
	int i = 0;
	char temp_buf[DC_SC_CUR_LEN] = {0};
	int coul_count;

	if (!di || !power_cmdline_is_factory_mode())
		return 0;

	coul_count = dc_sysfs_get_valid_coul_num(di->curr_info.coul_check_flag);
	ret = snprintf(buf, PAGE_SIZE, "^MULTICHARGER:%d,%d\r\n", di->curr_info.channel_num,
		di->curr_info.ibat_max);
	for (i = 0; (ret > 0) && (i < di->curr_info.channel_num); i++) {
		ret += snprintf(temp_buf, DC_SC_CUR_LEN,
			"^MULTICHARGER:%s,%d,%d,%d,%d,%d\r\n",
			di->curr_info.ic_name[i], di->curr_info.ibus[i],
			di->curr_info.vout[i], di->curr_info.vbat[i],
			di->curr_info.ibat[i], di->curr_info.tbat[i]);
		if ((PAGE_SIZE - strlen(buf)) < strlen(temp_buf))
			return 0;
		strncat(buf, temp_buf, strlen(temp_buf));
		memset(temp_buf, 0, sizeof(temp_buf));
	}

	ret += snprintf(temp_buf, DC_SC_CUR_LEN, "^MULTICHARGER:%d,%d,%d\r\n", coul_count,
		di->curr_info.coul_vbat_max, di->curr_info.coul_ibat_max);
	if ((PAGE_SIZE - strlen(buf)) < strlen(temp_buf))
			return 0;
	strncat(buf, temp_buf, strlen(temp_buf));
	for (i = 0; (ret > 0) && (i < COUL_IC_NUM); i++) {
		if (!(di->curr_info.coul_check_flag & BIT(i)))
			continue;
		memset(temp_buf, 0, sizeof(temp_buf));
		ret += snprintf(temp_buf, DC_SC_CUR_LEN, "^MULTICHARGER:%d,%d\r\n",
			di->curr_info.coul_vbat[i], di->curr_info.coul_ibat[i]);
		if ((PAGE_SIZE - strlen(buf)) < strlen(temp_buf))
			return 0;
		strncat(buf, temp_buf, strlen(temp_buf));
	}

	return ret;
}

static int dc_sysfs_iin_thermal_show(struct dc_sysfs_dev *l_dev, int local_mode, char *buf, size_t offset)
{
	struct direct_charge_device *di = direct_charge_get_di_by_mode(local_mode);
	int idx;

	if (!di)
		return 0;

	idx = (di->cur_mode == CHARGE_MULTI_IC) ? DC_DUAL_CHANNEL : DC_SINGLE_CHANNEL;
	return snprintf(buf, PAGE_SIZE, "%d\n", l_dev->iin_thermal_array[idx]);
}

static int dc_sysfs_thermal_reason_show(struct dc_sysfs_dev *l_dev, int local_mode, char *buf, size_t offset)
{
	return snprintf(buf, PAGE_SIZE, "%s\n", (char *)((void *)l_dev + offset));
}

static int dc_sysfs_common_show(struct dc_sysfs_dev *l_dev, int local_mode, char *buf, size_t offset)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", *(int *)((void *)l_dev + offset));
}

static int dc_sysfs_adapter_detect(struct dc_sysfs_dev *l_dev, int local_mode, char *buf, size_t offset)
{
	struct direct_charge_device *di = direct_charge_get_di_by_mode(local_mode);
	unsigned int type = charge_get_charger_type();
	unsigned int chg_state = direct_charge_get_stage_status();
	int work_mode = direct_charge_get_working_mode();
	int ret = ADAPTER_DETECT_FAIL;

	if (!di)
		return 0;

	if (adapter_get_protocol_register_state(ADAPTER_PROTOCOL_SCP) ||
		(di->working_mode != work_mode))
		return snprintf(buf, PAGE_SIZE, "%d\n", ret);

	if (((type == CHARGER_TYPE_STANDARD) && (chg_state >= DC_STAGE_ADAPTER_DETECT)) ||
		((type == CHARGER_REMOVED) && (chg_state == DC_STAGE_CHARGING)))
		ret = ADAPTER_DETECT_SUCC;

	return snprintf(buf, PAGE_SIZE, "%d\n", ret);
}

static int dc_sysfs_state_show(struct dc_sysfs_dev *l_dev, int local_mode, char *buf, size_t offset)
{
	struct direct_charge_device *di = direct_charge_get_di_by_mode(local_mode);
	unsigned int idx;

	if (!di)
		return 0;

	idx = (di->cur_mode == CHARGE_MULTI_IC) ? DC_DUAL_CHANNEL : DC_SINGLE_CHANNEL;
	return snprintf(buf, PAGE_SIZE, "%u\n", idx);
}

static int dc_sysfs_freq_show(struct dc_sysfs_dev *l_dev, int local_mode, char *buf, size_t offset)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", dcm_get_ic_freq(local_mode, CHARGE_IC_MAIN));
}

static int dc_sysfs_disable_charger_show(struct dc_sysfs_dev *l_dev, int local_mode, char *buf, size_t offset)
{
	char out_buf[DC_SYSFS_OUT_BUF_SIZE] = { 0 };
	char tmp_buf[DC_SYSFS_TMP_BUF_SIZE] = { 0 };
	int i;

	for (i = 0; i < DC_SYSFS_OP_USER_END; i++) {
		memset(tmp_buf, 0, DC_SYSFS_TMP_BUF_SIZE);
		scnprintf(tmp_buf, DC_SYSFS_TMP_BUF_SIZE, "%8s %d\n",
			g_dc_sysfs_op_user_table[i], atomic_read(&l_dev->disable_charger[i]));
		strncat(out_buf, tmp_buf, strlen(tmp_buf));
	}

	return snprintf(buf, PAGE_SIZE, "%s", out_buf);
}

static int dc_sysfs_dc_ibus_show(struct dc_sysfs_dev *l_dev, int local_mode, char *buf, size_t offset)
{
	int ibus = 0;

	dc_get_device_ibus(&ibus);
	return snprintf(buf, PAGE_SIZE, "%d\n", ibus);
}

static int dc_sysfs_anti_22p5w_strategy_show(struct dc_sysfs_dev *l_dev, int local_mode, char *buf, size_t offset)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", dc_adpt_get_mode_info(local_mode, ADAP_ANTI_22P5W_STRATEGY));
}

static int dc_sysfs_cable_type_info_show(struct dc_sysfs_dev *l_dev, int local_mode, char *buf, size_t offset)
{
	unsigned int cable_type;

	if (dc_get_cable_inherent_info(DC_IS_CTC_CABLE))
		cable_type = DC_STD_CABLE;
	else
		cable_type = dc_get_cable_inherent_info(DC_CABLE_TYPE);
	return snprintf(buf, PAGE_SIZE, "%u\n", cable_type);
}

static int dc_sysfs_iadapt_show(struct dc_sysfs_dev *l_dev, int local_mode, char *buf, size_t offset)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", direct_charge_get_ibus());
}

static int dc_sysfs_succ_flag_show(struct dc_sysfs_dev *l_dev, int local_mode, char *buf, size_t offset)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", direct_charge_get_succ_flag());
}

static int dc_sysfs_adp_antifake_failed_cnt_show(struct dc_sysfs_dev *l_dev, int local_mode, char *buf, size_t offset)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", dc_adpt_get_inherent_info(ADAP_ANTIFAKE_FAILED_CNT));
}

static int dc_get_full_path_resistance(struct dc_sysfs_dev *l_dev, int local_mode, char *buf, size_t offset)
{
	return snprintf(buf, PAGE_SIZE, "%u\n", dc_get_cable_mode_info(local_mode, CABLE_FULL_PATH_RES));
}

static int dc_set_ichg_thermal(struct dc_sysfs_dev *l_dev, int local_mode, int val)
{
	struct direct_charge_device *di = direct_charge_get_di_by_mode(local_mode);
	int index;
	int cur_low;

	if (!di)
		return 0;

	if (val > di->iin_thermal_default) {
		hwlog_err("val=%d greater than default=%d\n", val, di->iin_thermal_default);
		return -EPERM;
	}

	if ((di->stage_size < 1) || (di->stage_size > DC_VOLT_LEVEL)) {
		hwlog_err("invalid stage_size=%d\n", di->stage_size);
		return -EPERM;
	}

	index = di->orig_volt_para_p[0].stage_size - 1;
	cur_low = di->orig_volt_para_p[0].volt_info[index].cur_th_low;

	if (val == 0)
		l_dev->ichg_thermal = di->iin_thermal_default;
	else if (val < cur_low)
		l_dev->ichg_thermal = cur_low;
	else
		l_dev->ichg_thermal = val;

	hwlog_info("ichg_thermal set input current: %u, limit current: %d\n",
		val, l_dev->ichg_thermal);
	return 0;
}

static int dc_sysfs_iin_thermal_store(struct dc_sysfs_dev *l_dev, int local_mode, const char *buf)
{
	long val = 0;

	if (kstrtol(buf, POWER_BASE_DEC, &val) < 0)
		return -EINVAL;

	dc_sysfs_set_iin_thermal(local_mode, (int)val);
	return 0;
}

static int dc_sysfs_iin_thermal_ichg_control_store(struct dc_sysfs_dev *l_dev, int local_mode, const char *buf)
{
	long val = 0;

	if (kstrtol(buf, POWER_BASE_DEC, &val) < 0)
		return -EINVAL;
	dc_set_ichg_thermal(l_dev, local_mode, (int)val);
	return 0;
}

static int dc_sysfs_ichg_control_enable_store(struct dc_sysfs_dev *l_dev, int local_mode, const char *buf)
{
	long val = 0;

	if (kstrtol(buf, POWER_BASE_DEC, &val) < 0)
		return -EINVAL;
	l_dev->ichg_control_enable = val;
	return 0;
}

static int dc_sysfs_thermal_reason_store(struct dc_sysfs_dev *l_dev, int local_mode, const char *buf)
{
	struct direct_charge_device *di = direct_charge_get_di_by_mode(local_mode);

	if (!di)
		return 0;

	if (strlen(buf) >= DC_THERMAL_REASON_SIZE)
		return -EINVAL;
	snprintf(l_dev->thermal_reason, strlen(buf), "%s", buf);
	power_event_notify_sysfs(&di->dev->kobj, NULL, "thermal_reason");
	hwlog_info("THERMAL set reason = %s, buf = %s\n", l_dev->thermal_reason, buf);
	return 0;
}

static int dc_sysfs_ignore_full_path_res_store(struct dc_sysfs_dev *l_dev, int local_mode, const char *buf)
{
	l_dev->ignore_full_path_res = true;
	hwlog_info("set ignore_full_path_res true\n");

	return 0;
}

static int dc_sysfs_charge_type_priority_store(struct dc_sysfs_dev *l_dev, int local_mode, const char *buf)
{
	long val = 0;

	if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) ||
		(val < 0) || (val > DC_MAX_RESISTANCE))
		return -EINVAL;

	hwlog_info("set chargertype_priority=%ld %d\n", val, local_mode);

	if (val == DC_CHARGE_TYPE_SC)
		direct_charge_set_local_mode(OR_SET, SC_MODE);
	else if (val == DC_CHARGE_TYPE_LVC)
		direct_charge_set_local_mode(AND_SET, LVC_MODE);
	else
		hwlog_info("invalid chargertype priority\n");

	return 0;
}

static int dc_sysfs_freq_store(struct dc_sysfs_dev *l_dev, int local_mode, const char *buf)
{
	long val = 0;

	if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) ||
		(val < 0) || (val > 1))
		return -EINVAL;
	hwlog_info("set freq=%ld\n", val);
	(void)dcm_set_ic_freq(local_mode, CHARGE_IC_MAIN, val);

	return 0;
}

static int dc_sysfs_disable_charger_store(struct dc_sysfs_dev *l_dev, int local_mode, const char *buf)
{
	int disable_flag = 0;
	char user_name[DC_SYSFS_RD_BUF_SIZE] = {0};
	struct direct_charge_device *di = direct_charge_get_di_by_mode(local_mode);
	int user;

	if (!di || !di->sysfs_disable) {
		hwlog_err("unsupport disable charger\n");
		return -EINVAL;
	}

	if ((sscanf(buf, "%s %d", user_name, &disable_flag) != 2) ||
		(disable_flag < 0) || (disable_flag > 1)) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	user = dc_sysfs_get_op_user(user_name);
	if (user < 0)
		return -EINVAL;

	if (disable_flag)
		atomic_inc(&l_dev->disable_charger[user]);
	else
		atomic_dec(&l_dev->disable_charger[user]);

	if (atomic_read(&l_dev->disable_charger[user]) < 0)
		atomic_set(&l_dev->disable_charger[user], 0);

	hwlog_info("%s set %s disable_flag=%d\n", user_name,
		dc_comm_get_mode_name(local_mode), atomic_read(&l_dev->disable_charger[user]));

	return 0;
}

static struct dc_sysfs_show_struct g_dc_sysfs_show_tbl[] = {
	[DC_SYSFS_IIN_THERMAL] = { dc_sysfs_iin_thermal_show, 0 },
	[DC_SYSFS_IIN_THERMAL_ICHG_CONTROL] = { dc_sysfs_common_show, offsetof(struct dc_sysfs_dev, ichg_thermal) },
	[DC_SYSFS_ICHG_CONTROL_ENABLE] = { dc_sysfs_common_show, offsetof(struct dc_sysfs_dev, ichg_control_enable) },
	[DC_SYSFS_ADAPTER_DETECT] = { dc_sysfs_adapter_detect, 0 },
	[DC_SYSFS_IADAPT] = { dc_sysfs_iadapt_show, 0 },
	[DC_SYSFS_FULL_PATH_RESISTANCE] = { dc_get_full_path_resistance, 0 },
	[DC_SYSFS_DIRECT_CHARGE_SUCC] = {dc_sysfs_succ_flag_show, 0 },
	[DC_SYSFS_ADAPTER_ANTIFAKE_STATE] = { dc_sysfs_adp_antifake_failed_cnt_show, 0 },
	[DC_SYSFS_CABLE_TYPE] = { dc_sysfs_cable_type_info_show, 0 },
	[DC_SYSFS_THERMAL_REASON] = { dc_sysfs_thermal_reason_show, offsetof(struct dc_sysfs_dev, thermal_reason) },
	[DC_SYSFS_MULTI_SC_CUR] = { dc_sysfs_get_multi_cur, 0 },
	[DC_SYSFS_MULTI_IBAT] = { dc_sysfs_get_multi_ibat, 0 },
	[DC_SYSFS_CHARGING_INFO] = { dc_sysfs_get_charging_info, 0 },
	[DC_SYSFS_SC_STATE] = { dc_sysfs_state_show, 0 },
	[DC_SYSFS_SC_FREQ] = { dc_sysfs_freq_show, 0 },
	[DC_SYSFS_DC_IBUS] = { dc_sysfs_dc_ibus_show, 0 },
	[DC_SYSFS_ANTI_22P5W_STRATEGY] = { dc_sysfs_anti_22p5w_strategy_show, 0 },
	[DC_SYSFS_DISABLE_CHARGER] = { dc_sysfs_disable_charger_show, 0 },
};

static struct dc_sysfs_store_struct g_dc_sysfs_store_tbl[] = {
	[DC_SYSFS_IIN_THERMAL] = { dc_sysfs_iin_thermal_store },
	[DC_SYSFS_IIN_THERMAL_ICHG_CONTROL] = { dc_sysfs_iin_thermal_ichg_control_store },
	[DC_SYSFS_ICHG_CONTROL_ENABLE] = { dc_sysfs_ichg_control_enable_store },
	[DC_SYSFS_SET_RESISTANCE_THRESHOLD] = { dc_sysfs_ignore_full_path_res_store },
	[DC_SYSFS_SET_CHARGETYPE_PRIORITY] = { dc_sysfs_charge_type_priority_store },
	[DC_SYSFS_THERMAL_REASON] = { dc_sysfs_thermal_reason_store },
	[DC_SYSFS_SC_FREQ] = { dc_sysfs_freq_store },
	[DC_SYSFS_DISABLE_CHARGER] = { dc_sysfs_disable_charger_store },
};

static ssize_t dc_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	struct dc_sysfs_dev *l_dev = NULL;
	struct direct_charge_device *di = dev_get_drvdata(dev);

	info = power_sysfs_lookup_attr(attr->attr.name,
		g_dc_sysfs_field_tbl, DC_SYSFS_ATTRS_SIZE);
	if (!info || !di)
		return -EINVAL;

	l_dev = dc_sysfs_get_di(di->local_mode);
	if (!l_dev)
		return -EPERM;

	if (!g_dc_sysfs_show_tbl[info->name].process)
		return -EPERM;

	return g_dc_sysfs_show_tbl[info->name].process(l_dev, di->local_mode, buf, g_dc_sysfs_show_tbl[info->name].offset);
}

static ssize_t dc_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct power_sysfs_attr_info *info = NULL;
	struct dc_sysfs_dev *l_dev = NULL;
	struct direct_charge_device *di = dev_get_drvdata(dev);
	int ret;

	info = power_sysfs_lookup_attr(attr->attr.name,
		g_dc_sysfs_field_tbl, DC_SYSFS_ATTRS_SIZE);
	if (!info || !di)
		return -EINVAL;

	l_dev = dc_sysfs_get_di(di->local_mode);
	if (!l_dev)
		return -EPERM;

	if (!g_dc_sysfs_store_tbl[info->name].process)
		return -EPERM;

	ret = g_dc_sysfs_store_tbl[info->name].process(l_dev, di->local_mode, buf);

	return (ret == 0) ? count : ret;
}

static void dc_sysfs_create_group(struct device *dev, int mode)
{
	power_sysfs_init_attrs(g_dc_sysfs_attrs,
		g_dc_sysfs_field_tbl, DC_SYSFS_ATTRS_SIZE);
	power_sysfs_create_link_group("hw_power", "charger", g_dc_sysfs_link_name[mode],
		dev, &g_dc_sysfs_attr_group);
}

static void dc_sysfs_remove_group(struct device *dev, int mode)
{
	power_sysfs_remove_link_group("hw_power", "charger", g_dc_sysfs_link_name[mode],
		dev, &g_dc_sysfs_attr_group);
}
#else
static inline void dc_sysfs_create_group(struct device *dev, int mode)
{
}

static inline void dc_sysfs_remove_group(struct device *dev, int mode)
{
}
#endif /* CONFIG_SYSFS */

static void dc_sysfs_para_init(struct dc_sysfs_dev *di, int iin_thermal_default)
{
	int i;

	di->iin_thermal = iin_thermal_default;
	di->iin_thermal_array[DC_SINGLE_CHANNEL] = iin_thermal_default;
	di->iin_thermal_array[DC_DUAL_CHANNEL] = iin_thermal_default;
	di->ichg_thermal = iin_thermal_default;
	di->ichg_control_enable = 0;
	di->ignore_full_path_res = false;

	for (i = 0; i < DC_SYSFS_OP_USER_END; i++)
		atomic_set(&di->disable_charger[i], 0);
}

void dc_sysfs_init(struct device *dev, int mode)
{
	int idx = dc_comm_get_mode_idx(mode);
	struct direct_charge_device *l_dev = direct_charge_get_di_by_mode(mode);
	struct dc_sysfs_dev *di = NULL;

	if (idx < 0)
		return;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di || !l_dev)
		return;

	hwlog_info("dc_sysfs_init mode %d\n", mode);
	dc_sysfs_create_group(dev, idx);
	g_dc_sysfs_di[idx] = di;

	dc_sysfs_para_init(di, l_dev->iin_thermal_default);
}

void dc_sysfs_remove(struct device *dev, int mode)
{
	struct dc_sysfs_dev *di = dc_sysfs_get_di(mode);
	int idx;

	if (!di)
		return;

	hwlog_info("dc_sysfs_remove mode %d\n", mode);
	idx = dc_comm_get_mode_idx(mode);
	dc_sysfs_remove_group(dev, idx);
	kfree(di);
	di = NULL;
}
