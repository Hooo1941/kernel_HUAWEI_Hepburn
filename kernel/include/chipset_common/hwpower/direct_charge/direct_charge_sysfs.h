/*
 * direct_charge_sysfs.h
 *
 * direct charge sysfs driver
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

#ifndef _DIRECT_CHARGE_SYSFS_H_
#define _DIRECT_CHARGE_SYSFS_H_

#define DC_THERMAL_REASON_SIZE        16

enum dc_sysfs_op_user {
	DC_SYSFS_OP_USER_BEGIN = 0,
	DC_SYSFS_OP_USER_CAMERA = DC_SYSFS_OP_USER_BEGIN, /* for camera */
	DC_SYSFS_OP_USER_END,
};

/*
 * define sysfs type with direct charge
 * DC is simplified identifier with direct-charge
 */
enum dc_sysfs_type {
	DC_SYSFS_BEGIN = 0,
	DC_SYSFS_IIN_THERMAL = DC_SYSFS_BEGIN,
	DC_SYSFS_IIN_THERMAL_ICHG_CONTROL,
	DC_SYSFS_ICHG_CONTROL_ENABLE,
	DC_SYSFS_ADAPTER_DETECT,
	DC_SYSFS_IADAPT,
	DC_SYSFS_FULL_PATH_RESISTANCE,
	DC_SYSFS_DIRECT_CHARGE_SUCC,
	DC_SYSFS_ADAPTER_ANTIFAKE_STATE,
	DC_SYSFS_CABLE_TYPE,
	DC_SYSFS_SET_RESISTANCE_THRESHOLD,
	DC_SYSFS_SET_CHARGETYPE_PRIORITY,
	DC_SYSFS_THERMAL_REASON,
	DC_SYSFS_MULTI_SC_CUR,
	DC_SYSFS_MULTI_IBAT,
	DC_SYSFS_CHARGING_INFO,
	DC_SYSFS_SC_STATE,
	DC_SYSFS_SC_FREQ,
	DC_SYSFS_DC_IBUS,
	DC_SYSFS_ANTI_22P5W_STRATEGY,
	DC_SYSFS_DISABLE_CHARGER,
	DC_SYSFS_END,
};

enum dc_sysfs_mode {
	DC_SYSFS_MODE_LVC = 0,
	DC_SYSFS_MODE_SC,
	DC_SYSFS_MODE_SC4,
	DC_SYSFS_MODE_TOTAL,
};

enum dc_sysfs_iin_thermal_channel_type {
	DC_SYSFS_CHANNEL_TYPE_BEGIN = 0,
	DC_SYSFS_SINGLE_CHANNEL = DC_SYSFS_CHANNEL_TYPE_BEGIN,
	DC_SYSFS_DUAL_CHANNEL,
	DC_SYSFS_CHANNEL_TYPE_END,
};

struct dc_sysfs_dev {
	struct device *dev;
	int iin_thermal;
	int iin_thermal_array[DC_SYSFS_CHANNEL_TYPE_END];
	int ichg_thermal;
	int ichg_control_enable;
	int ignore_full_path_res;
	char thermal_reason[DC_THERMAL_REASON_SIZE];
	atomic_t disable_charger[DC_SYSFS_OP_USER_END];
};

struct dc_sysfs_struct {
	struct attribute **attrs;
	struct power_sysfs_attr_info *attr_info;
	int size;
	const char *link_name;
	const struct attribute_group *group;
};

struct dc_sysfs_show_struct {
	int (*process)(struct dc_sysfs_dev *di, int local_mode, char *buf, size_t offset);
	size_t offset;
};

struct dc_sysfs_store_struct {
	int (*process)(struct dc_sysfs_dev *di, int local_mode, const char *buf);
};

int dc_sysfs_set_iin_thermal(int local_mode, int val);
int dc_sysfs_set_iin_thermal_array(int local_mode, unsigned int idx, int val);
void dc_sysfs_set_ignore_full_path_res(int mode, int val);
int dc_sysfs_get_iin_thermal(int mode);
void dc_sysfs_get_iin_thermal_limit(int local_mode, int cur_mode, unsigned int *val);
int dc_sysfs_get_ichg_thermal(int local_mode, int cur_mode);
int dc_sysfs_get_ignore_full_path_res(int mode);
bool dc_sysfs_get_disable_flag(int mode);
void dc_sysfs_remove(struct device *dev, int mode);
void dc_sysfs_init(struct device *dev, int mode);
#endif
