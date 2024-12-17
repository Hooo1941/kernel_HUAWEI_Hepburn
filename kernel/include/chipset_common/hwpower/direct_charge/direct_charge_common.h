/*
 * direct_charge_common.h
 *
 * direct charge common driver
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

#ifndef _DIRECT_CHARGE_COMMON_H_
#define _DIRECT_CHARGE_COMMON_H_

#include <linux/device.h>

#define BAT_RATED_VOLT                5000
#define DC_NEED_ANOMALY_DET           1
#define DC_NO_NEED_ANOMALY_DET        0
#define DC_ANOMALY_DET_INTERVAL       5000

enum dc_mode {
	DC_MODE_LVC = 0,
	DC_MODE_SC,
	DC_MODE_SC4,
	DC_MODE_TOTAL,
};

#ifdef CONFIG_DIRECT_CHARGER
int dc_comm_get_mode_idx(int mode);
const char *dc_comm_get_mode_name(int mode);
int dc_comm_get_mode_ratio(int mode);
void dc_comm_set_mode_ratio(int mode, int ratio);
void dc_comm_set_need_recheck_flag(bool flag);
bool dc_comm_get_need_recheck_flag(void);
void dc_comm_set_first_check_completed(bool flag);
bool dc_comm_get_first_check_completed(void);
#else
static inline int dc_comm_get_mode_idx(int mode)
{
	return -EPERM;
}

static inline const char *dc_comm_get_mode_name(int mode)
{
	return "invalid mode";
}

static inline int dc_comm_get_mode_ratio(int mode)
{
	return 0;
}

static inline void dc_comm_set_mode_ratio(int mode, int ratio)
{
}

static inline void dc_comm_set_need_recheck_flag(bool flag)
{
}

static inline bool dc_comm_get_need_recheck_flag(void)
{
	return false;
}

static inline void dc_comm_set_first_check_completed(bool flag)
{
}

static inline bool dc_comm_get_first_check_completed(void)
{
	return false;
}
#endif /* CONFIG_DIRECT_CHARGER */

#endif /* _DIRECT_CHARGE_COMMON_H_ */
