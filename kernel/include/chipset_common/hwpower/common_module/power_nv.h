/* SPDX-License-Identifier: GPL-2.0 */
/*
 * power_nv.h
 *
 * nv(non-volatile) interface for power module
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

#ifndef _POWER_NV_H_
#define _POWER_NV_H_

#include <linux/types.h>

enum power_nv_type {
	POWER_NV_TYPE_BEGIN = 0,
	POWER_NV_BLIMSW = POWER_NV_TYPE_BEGIN,
	POWER_NV_BBINFO,
	POWER_NV_COMMON, /* B[0-3]: BATHEAT, B[4-19]: QVAL_CALI */
	POWER_NV_BATCAP,
	POWER_NV_DEVCOLR,
	POWER_NV_TERMVOL,
	POWER_NV_CHGDIEID,
	POWER_NV_SCCALCUR,
	POWER_NV_CUROFFSET,
	POWER_NV_HWCOUL,
	POWER_NV_QCOUL,
	POWER_NV_BATACT1, /* single batt or first batt in multiple batt */
	POWER_NV_BATACT2, /* second batt in multiple batt */
	POWER_NV_BATCYC,
	POWER_NV_TYPE_END,
};

struct power_nv_data_info {
	enum power_nv_type type;
	unsigned int id;
	const char *name;
};

enum power_nv_common_type {
	POWER_NV_COMMON_BEGIN = 0,
	POWER_NV_COMMON_BATHEAT = POWER_NV_COMMON_BEGIN,
	POWER_NV_COMMON_QVAL,
	POWER_NV_COMMON_END,
};

struct power_nv_fod_qval {
	u32 km;
	u32 qr;
	u32 fr;
	u32 kz;
};

/* power_nv_common_info 104 bytes at most */
struct power_nv_common_info {
	u32 bat_heat;
	struct power_nv_fod_qval qval; /* 16 bytes */
};

int power_nv_write(enum power_nv_type type, const void *data, uint32_t data_len);
int power_nv_read(enum power_nv_type type, void *data, uint32_t data_len);

/* for POWER_NV_COMMON */
int power_common_nv_write(enum power_nv_common_type nv_common_type, void *value);
int power_common_nv_read(enum power_nv_common_type nv_common_type, void *value);

#endif /* _POWER_NV_H_ */
