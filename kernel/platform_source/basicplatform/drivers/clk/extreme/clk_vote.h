/*
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
 */
#ifndef __CLK_VOTE_H_
#define __CLK_VOTE_H_

#include "clk.h"
#include <linux/clk.h>

struct hi3xxx_vote_clock {
	struct clk_hw hw;
	u32	id;
	const char *friend_clk;
	const char *parent_clk;
	unsigned long cur_freq;
};

#endif
