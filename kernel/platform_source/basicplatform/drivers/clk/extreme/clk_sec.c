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

#include "clk_sec.h"
#include <securec.h>
#include "clk-smc.h"

static int sec_clk_enable(struct clk_hw *hw)
{
	struct hi3xxx_sec_gate_clk *pclk = NULL;
	int ret = 0;

	pclk = container_of(hw, struct hi3xxx_sec_gate_clk, hw);
	if (pclk == NULL) {
		pr_err("[%s] fail to get pclk!\n", __func__);
		return -EINVAL;
	}

	/* SMC into ATF to open the gate */
	ret = atfd_service_clk_smc(CLK_SEC_SET, pclk->sec_clock_id, CLK_GATE, ENABLE_GATE_CLK);
	if (ret) {
		pr_err("clock flag %u atf enable gate error, ret = %d\n",
			pclk->sec_clock_id, ret);
		return ret;
	}

	return 0;
}

static void sec_clk_disable(struct clk_hw *hw)
{
#ifndef CONFIG_CLK_ALWAYS_ON
	struct hi3xxx_sec_gate_clk *pclk = NULL;
	int ret = 0;

	pclk = container_of(hw, struct hi3xxx_sec_gate_clk, hw);
	if (pclk == NULL) {
		pr_err("[%s] fail to get pclk!\n", __func__);
		return;
	}

	if (!pclk->always_on) {
		/* SMC into ATF to disable the gate */
		ret = atfd_service_clk_smc(CLK_SEC_SET, pclk->sec_clock_id, CLK_GATE, DISABLE_GATE_CLK);
		if (ret) {
			pr_err("clock flag %u atf disable gate error, ret = %d\n",
				pclk->sec_clock_id, ret);
			return;
		}
	}
#endif
}

static const struct clk_ops sec_clkgate_ops = {
	.enable		= sec_clk_enable,
	.disable	= sec_clk_disable,
};

static struct clk *__sec_clk_register_gate(const struct sec_gate_clock *gate,
	struct clock_data *data)
{
	struct hi3xxx_sec_gate_clk *pclk = NULL;
	struct clk_init_data init;
	struct clk *clk = NULL;
	struct hs_clk *hs_clk = get_hs_clk_info();
	int i;

	pclk = kzalloc(sizeof(*pclk), GFP_KERNEL);
	if (pclk == NULL) {
		pr_err("[%s] fail to alloc pclk!\n", __func__);
		return clk;
	}

	init.name = gate->name;
	init.ops  = &sec_clkgate_ops;
	init.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED;
	init.parent_names = &(gate->parent_name);
	init.num_parents = 1;

	pclk->sec_clock_id = gate->sec_clock_id;
	pclk->always_on	= gate->always_on;
	pclk->lock	= &hs_clk->lock;
	pclk->hw.init	= &init;
	pclk->base_addr	= data->base;
	for (i = 0; i < GATE_CFG_CNT; i++)
		pclk->clkgt_cfg[i] = gate->clkgt_cfg[i];

	clk = clk_register(NULL, &pclk->hw);
	if (IS_ERR(clk)) {
		pr_err("[%s] fail to reigister clk %s!\n",
			__func__, gate->name);
		goto err_init;
	}

	/* init is local variable, need set NULL before func */
	pclk->hw.init = NULL;
	return clk;

err_init:
	kfree(pclk);
	return clk;
}

void plat_clk_register_sec_gate(const struct sec_gate_clock *clks,
	int nums, struct clock_data *data)
{
	struct clk *clk = NULL;
	int i;

	for (i = 0; i < nums; i++) {
		clk = __sec_clk_register_gate(&clks[i], data);
		if (IS_ERR_OR_NULL(clk)) {
			pr_err("%s: failed to register clock %s\n",
				__func__, clks[i].name);
			continue;
		}

		clk_log_dbg("clks id %d, nums %d, clks name = %s!\n",
			clks[i].id, nums, clks[i].name);
		clk_data_init(clk, clks[i].alias, clks[i].id, data);
	}
}
