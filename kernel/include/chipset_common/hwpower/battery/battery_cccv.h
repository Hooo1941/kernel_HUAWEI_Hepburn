/* SPDX-License-Identifier: GPL-2.0 */
/*
 * battery_cccv.h
 *
 * head file for battery cccv driver
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

#ifndef _BATTERY_CCCV_H_
#define _BATTERY_CCCV_H_

#define BAT_CCCV_TBL_LEN            10
#define BAT_CCCV_MAX_TEMP_NUM       10
#define BAT_CCCV_MAX_STAGE          10
#define BAT_CCCV_RATIO_UTIL         100
#define BAT_CCCV_RATIO_DEFAULT      100
#define BAT_CCCV_RATIO_MIN          70
#define BAT_CCCV_BUF_MAX_SIZE       256
#define BAT_CCCV_TMP_HYSTERESIS     20

enum bat_cccv_sysfs_type {
	BAT_CCCV_SYSFS_BEGIN = 0,
	BAT_CCCV_SYSFS_UPDATE_ICHG_RATIO = BAT_CCCV_SYSFS_BEGIN,
	BAT_CCCV_SYSFS_END,
};

enum bat_cccv_bat_tbl_type {
	BAT_CCCV_BAT_TBL_BEGIN = 0,
	BAT_CCCV_BAT_TBL_BRAND = BAT_CCCV_BAT_TBL_BEGIN,
	BAT_CCCV_BAT_TBL_PARAM,
	BAT_CCCV_BAT_TBL_END
};

enum bat_cccv_tmp_tbl_type {
	BAT_CCCV_TMP_TBL_BEGIN = 0,
	BAT_CCCV_TMP_TBL_TBAT = BAT_CCCV_TMP_TBL_BEGIN,
	BAT_CCCV_TMP_TBL_CCCV,
	BAT_CCCV_TMP_TBL_END
};

enum bat_cccv_tbl_type {
	BAT_CCCV_TBL_BEGIN = 0,
	BAT_CCCV_TBL_VBAT = BAT_CCCV_TBL_BEGIN,
	BAT_CCCV_TBL_ICHG,
	BAT_CCCV_TBL_TIME,
	BAT_CCCV_TBL_END
};

#ifdef CONFIG_HUAWEI_BATTERY_CCCV
int bat_cccv_get_ichg(int vbat);
#else
static inline int bat_cccv_get_ichg(int vbat)
{
	return -1;
}
#endif /* CONFIG_HUAWEI_BATTERY_CCCV */

#endif /* _BATTERY_CCCV_H_ */