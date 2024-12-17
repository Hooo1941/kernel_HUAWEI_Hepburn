/* SPDX-License-Identifier: GPL-2.0 */
/*
 * buck_charge_jeita.h
 *
 * buck charge jeita module
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#ifndef _BUCK_CHARGE_JEITA_H_
#define _BUCK_CHARGE_JEITA_H_

#include <linux/of.h>

#define BC_JEITA_PARA_LEVEL     6

enum bc_jeita_info {
	BCJ_TEMP_MIN = 0,
	BCJ_TEMP_MAX,
	BCJ_IIN_LIMIT,
	BCJ_ICHG_LIMIT,
	BCJ_VTERM,
	BCJ_TEMP_BACK,
	BCJ_INFO_TOTAL,
};

struct bc_jeita_para {
	int temp_min;
	int temp_max;
	int iin_limit;
	int ichg_limit;
	int vterm;
	int temp_back;
};

struct bc_jeita_result {
	int iin;
	int ichg;
	int vterm;
};

int buck_charge_jeita_parse_jeita_table(struct device_node *np, void *p);
void buck_charge_jeita_tbatt_handler(int temp,
	struct bc_jeita_para jeita_table[], struct bc_jeita_result *data);

#endif /* _BUCK_CHARGE_JEITA_H_ */
