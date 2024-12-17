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
#ifndef __CLK_SEC_H_
#define __CLK_SEC_H_

#include "clk.h"
#include <linux/clk.h>
#include <securec.h>

enum SEC_CONTROL_FLAG {
	ENABLE_GATE_CLK,
	DISABLE_GATE_CLK,
};

enum sec_clk_type {
	CLK_GATE,
	CLK_SC_GATE,
	CLK_DIV,
	CLK_MUX,
	CLK_TYPE_MAX,
};

struct hi3xxx_sec_gate_clk {
	u32 id;
	struct clk_hw hw;
	void __iomem *base_addr;
	const char *parent_name;
	u32 clkgt_cfg[GATE_CFG_CNT]; /* offset value */
	spinlock_t *lock;
	u32 sec_clock_id;
	u32 always_on;
};

struct sec_gate_clock {
	u32 id;
	const char *name;
	const char *parent_name;
	u32 clkgt_cfg[GATE_CFG_CNT]; /* offset value */
	u32 always_on;
	u32 sec_clock_id;
	const char *alias;
};

void plat_clk_register_sec_gate(const struct sec_gate_clock *clks,
	int nums, struct clock_data *data);
#endif
