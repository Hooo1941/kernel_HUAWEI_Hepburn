/* SPDX-License-Identifier: GPL-2.0 */
/*
 * ground_loop_compensate.h
 *
 * algorithm (ground loop temperature compensation) interface for fuel guage module
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

#ifndef _GROUND_LOOP_COMPENSATE_H_
#define _GROUND_LOOP_COMPENSATE_H_

#include <linux/slab.h>
#include <linux/bitops.h>
#include <linux/types.h>
#include <linux/math64.h>
#include <linux/ctype.h>

struct glc_temp_comp_data {
	int ibat; /* mA */
	int vadc; /* mv */
	int vpullup; /* mv */
	int rpullup; /* Ohm */
	int rcomp; /* mOhm */
};

int ground_loop_compensate_get_temp(struct glc_temp_comp_data *glc_data);

#endif /* _GROUND_LOOP_COMPENSATE_H_ */
