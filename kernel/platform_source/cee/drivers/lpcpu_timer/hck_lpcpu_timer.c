/*
 *
 * hck timer module
 *
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
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

#include <linux/init.h>
#include <linux/io.h>
#include "timer-sp.h"
#include <platform_include/cee/linux/hck/lpcpu/timer/hck_lpcpu_timer.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))

static void lpcpu_timer_sp804_clock_init(struct sp804_clkevt *clkevt, int *hook_flag)
{
	*hook_flag = 0;
#ifdef CONFIG_LPCPU_HITIMER
	writel(0xffff0000, clkevt->ctrl);
	writel(0xffffffff, clkevt->load);
	writel(LPCPU_CLK_CTRL_VAL, clkevt->clk_ctrl);
	writel((TIMER_CTRL_32BIT | TIMER_CTRL_ENABLE | TIMER_CTRL_PERIODIC) | TIMER_CTRL_INT_MASK,
		clkevt->ctrl);
	*hook_flag = 1;
#endif
}

static void lpcpu_timer_sp804_shutdown(struct sp804_clkevt *common_clkevt, int *hook_flag)
{
	*hook_flag = 0;
#ifdef CONFIG_LPCPU_HITIMER
	writel(0xffff0000, common_clkevt->ctrl);
	*hook_flag = 1;
#endif
}

static void lpcpu_timer_sp804_clockevents_init(struct sp804_clkevt *common_clkevt, int *hook_flag)
{
	*hook_flag = 0;
#ifdef CONFIG_LPCPU_HITIMER
	writel(0xffff0000, common_clkevt->ctrl);
	writel(LPCPU_CLK_CTRL_VAL, common_clkevt->clk_ctrl);
	*hook_flag = 1;
#endif
}

static void lpcpu_timer_sp804_timers_disable(void __iomem *timer_base, int timer_ctrl,
					     int *hook_flag)
{
	*hook_flag = 0;
#ifdef CONFIG_LPCPU_HITIMER
	writel(0xffff0000, timer_base + timer_ctrl);
	*hook_flag = 1;
#endif
}

static void lpcpu_timer_sp804_of_timers_disable(void __iomem *timer1_base,
						void __iomem *timer2_base,
						int timer_ctrl, int *hook_flag)
{
	*hook_flag = 0;
#ifdef CONFIG_LPCPU_HITIMER
	lpcpu_timer_sp804_timers_disable(timer1_base, timer_ctrl, hook_flag);
	lpcpu_timer_sp804_timers_disable(timer2_base, timer_ctrl, hook_flag);
	*hook_flag = 1;
#endif
}

static void lpcpu_timer_sp804_clkevt_init(struct sp804_clkevt *clkevt, void __iomem *timer_base,
					  struct sp804_timer *timer)
{
#ifdef CONFIG_LPCPU_HITIMER
	clkevt->clk_ctrl = timer_base + timer->clk_ctrl;
#endif
}

void __init hck_lpcpu_timer_register(void)
{
	static int init = 0;

	if (init != 0)
		return;

	REGISTER_HCK_VH(lpcpu_timer_sp804_clock_init, lpcpu_timer_sp804_clock_init);
	REGISTER_HCK_VH(lpcpu_timer_sp804_shutdown, lpcpu_timer_sp804_shutdown);
	REGISTER_HCK_VH(lpcpu_timer_sp804_clockevents_init, lpcpu_timer_sp804_clockevents_init);
	REGISTER_HCK_VH(lpcpu_timer_sp804_timers_disable, lpcpu_timer_sp804_timers_disable);
	REGISTER_HCK_VH(lpcpu_timer_sp804_of_timers_disable, lpcpu_timer_sp804_of_timers_disable);
	REGISTER_HCK_VH(lpcpu_timer_sp804_clkevt_init, lpcpu_timer_sp804_clkevt_init);
	init++;
}

#endif
