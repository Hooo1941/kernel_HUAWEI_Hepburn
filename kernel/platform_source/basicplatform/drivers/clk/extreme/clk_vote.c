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

#include "clk_vote.h"
#define max(x, y) ((x) < (y) ? (y) : (x))
static unsigned long vote_clk_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	struct hi3xxx_vote_clock *pclk = container_of(hw, struct hi3xxx_vote_clock, hw);
	return pclk->cur_freq;
}

static long vote_clk_round_rate(struct clk_hw *hw,
	unsigned long rate, unsigned long *prate)
{
	return rate;
}

static int vote_clk_update_rate(struct clk *friend_clk, struct clk *parent_clk,
	unsigned long new_rate)
{
	int ret = 0;
	unsigned long friend_rate = 0;
	unsigned long parent_rate = 0;
	unsigned long tmp_rate = 0;

	if (friend_clk == NULL || parent_clk == NULL)
		return -EINVAL;

	friend_rate = __clk_get_rate(friend_clk);
	parent_rate = __clk_get_rate(parent_clk);
	tmp_rate = max(friend_rate, new_rate);
	if (tmp_rate != parent_rate) {
		ret = clk_set_rate_nolock(parent_clk, tmp_rate);
	} else {
		pr_info("[%s] current rate %u not need change to %u \n",
			__func__, new_rate, parent_rate);
	}

	return ret;
}

static int vote_clk_set_rate(struct clk_hw *hw,
	unsigned long rate, unsigned long parent_rate)
{
	struct hi3xxx_vote_clock *pclk = container_of(hw, struct hi3xxx_vote_clock, hw);
	struct clk *clk_friend = NULL;
	struct clk *clk_parent = NULL;
	int ret = 0;

	clk_friend = __clk_lookup(pclk->friend_clk);
	if (IS_ERR_OR_NULL(clk_friend)) {
		pr_err("[%s] %s get friend clk failed!\n",
			__func__, __clk_get_name(hw->clk));
		return -ENODEV;
	}
	clk_parent = __clk_lookup(pclk->parent_clk);
	if (IS_ERR_OR_NULL(clk_parent)) {
		pr_err("[%s] %s get parent clk failed!\n",
			__func__, __clk_get_name(hw->clk));
		return -ENODEV;
	}

	ret = vote_clk_update_rate(clk_friend, clk_parent, rate);
	if (ret != 0) {
		pr_err("[%s] %s uadate rate failed! \n",
			__func__, __clk_get_name(hw->clk));
		return ret;
	}
	pclk->cur_freq = rate;
	return ret;
}

static int vote_clk_prepare(struct clk_hw *hw)
{
	struct hi3xxx_vote_clock *pclk = container_of(hw, struct hi3xxx_vote_clock, hw);
	struct clk *clk_parent = NULL;
	int ret = 0;

	clk_parent = __clk_lookup(pclk->parent_clk);
	if (IS_ERR_OR_NULL(clk_parent)) {
		pr_err("[%s] %s get parent failed!\n", __func__, __clk_get_name(hw->clk));
		return -ENODEV;
	}
	ret = plat_core_prepare(clk_parent);
	if (ret) {
		pr_err("[%s], friend clock prepare faild!\n", __func__);
		return ret;
	}

	return ret;
}

static void vote_clk_unprepare(struct clk_hw *hw)
{
	struct hi3xxx_vote_clock *pclk = container_of(hw, struct hi3xxx_vote_clock, hw);
	struct clk *clk_parent = NULL;

	clk_parent = __clk_lookup(pclk->parent_clk);
	if (IS_ERR_OR_NULL(clk_parent)) {
		pr_err("[%s] %s get parent failed!\n", __func__, __clk_get_name(hw->clk));
		return;
	}

	plat_core_unprepare(clk_parent);
}

static int vote_clk_enable(struct clk_hw *hw)
{
	struct hi3xxx_vote_clock *pclk = container_of(hw, struct hi3xxx_vote_clock, hw);
	struct clk *clk_parent = NULL;
	int ret = 0;

	clk_parent = __clk_lookup(pclk->parent_clk);
	if (IS_ERR_OR_NULL(clk_parent)) {
		pr_err("[%s] %s get parent failed!\n", __func__, __clk_get_name(hw->clk));
		return -EINVAL;
	}
	ret = __clk_enable(clk_parent);
	if (ret) {
		pr_err("[%s], %s friend clock enable faild!",
			__func__, __clk_get_name(hw->clk));
		return ret;
	}

	return ret;
}

static void vote_clk_disable(struct clk_hw *hw)
{
	struct hi3xxx_vote_clock *pclk = container_of(hw, struct hi3xxx_vote_clock, hw);
	struct clk *clk_parent = NULL;

	clk_parent = __clk_lookup(pclk->parent_clk);
	if (IS_ERR_OR_NULL(clk_parent)) {
		pr_err("[%s] %s get parent failed!\n", __func__, __clk_get_name(hw->clk));
		return;
	}
	__clk_disable(clk_parent);
}

static const struct clk_ops vote_clk_ops = {
	.recalc_rate = vote_clk_recalc_rate,
	.round_rate = vote_clk_round_rate,
	.set_rate = vote_clk_set_rate,
	.prepare = vote_clk_prepare,
	.unprepare = vote_clk_unprepare,
	.enable = vote_clk_enable,
	.disable = vote_clk_disable,
};

static struct clk *__clk_register_vote_clock(const struct vote_clock *vote_clk)
{
	struct hi3xxx_vote_clock *pclk = NULL;
	struct clk_init_data init;
	struct clk *clk = NULL;

	pclk = kzalloc(sizeof(*pclk), GFP_KERNEL);
	if (pclk == NULL) {
		pr_err("[%s] fail to alloc pclk!\n", __func__);
		return clk;
	}

	init.name = vote_clk->name;
	init.ops = &vote_clk_ops;
	init.parent_names = NULL;
	init.num_parents = 0;
	init.flags = CLK_IS_ROOT | CLK_GET_RATE_NOCACHE;

	pclk->id = vote_clk->id;
	pclk->friend_clk = vote_clk->friend_clk;
	pclk->parent_clk = vote_clk->parent_clk;
	pclk->cur_freq = 0;
	pclk->hw.init = &init;

	clk = clk_register(NULL, &pclk->hw);
	if (IS_ERR(clk)) {
		pr_err("[%s] fail to reigister clk %s!\n",
			__func__, vote_clk->name);
		goto err_init;
	}
	/* init is local variable, need set NULL before func */
	pclk->hw.init = NULL;
	return clk;

err_init:
	kfree(pclk);
	return clk;
}

void plat_clk_register_vote_clock(const struct vote_clock *clks,
	int nums, struct clock_data *data)
{
	struct clk *clk = NULL;
	int i;

	for (i = 0; i < nums; i++) {
		clk = __clk_register_vote_clock(&clks[i]);
		if (IS_ERR_OR_NULL(clk)) {
			pr_err("%s: failed to register clock %s\n",
				__func__, clks[i].name);
			continue;
		}
#ifdef CONFIG_CLK_DEBUG
		debug_clk_add(clk, CLOCK_VOTE);
#endif
		pr_err("clks id %d, nums %d, clks name = %s!\n",
			clks[i].id, nums, clks[i].name);
		clk_data_init(clk, clks[i].alias, clks[i].id, data);
	}
}