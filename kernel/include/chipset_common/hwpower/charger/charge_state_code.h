/* SPDX-License-Identifier: GPL-2.0 */
/*
 * charge_state_code.h
 *
 * status process interface for charge
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

#ifndef _CHARGE_STATE_CODE_H_
#define _CHARGE_STATE_CODE_H_

#include <linux/kernel.h>
#include <linux/stat.h>

#define CSC_DC_BASE           10
#define CSC_WL_BASE           100
#define CSC_FG_BASE           200
#define CSC_BUCK_BASE         300
#define CSC_BC12_BASE         350

enum charge_csc_code_list {
	CSC_BEGIN,
	CSC_CHARGER_INSERT = CSC_BEGIN,
	CSC_BC12_RESULT,
	CSC_BUCK_CHARGE,
	CSC_START_CHECK = CSC_DC_BASE, /* 10 */
	CSC_MODE_MATCH,
	CSC_MODE_CHECK,
	CSC_FSM_STATE,
	CSC_DC_SUCC,
	/* for wireless charge */
	CSC_WL_START = CSC_WL_BASE, /* 100 */
	/* for fuel gauge */
	CSC_FG_START = CSC_FG_BASE, /* 200 */
	/* for buck charge */
	CSC_BUCK_START = CSC_BUCK_BASE, /* 300 */
};

void charge_state_code_set(unsigned int code, char *para);

#endif /* _CHARGE_STATE_CODE_H_ */
