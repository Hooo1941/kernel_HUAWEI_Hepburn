/*
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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
#include "clk-smc.h"
#include <linux/arm-smccc.h>
#include <linux/types.h>
#include <linux/spinlock_types.h>
#include <linux/spinlock.h>
#ifdef CONFIG_CLOCK_SUPPORT_FFA
#include <platform_include/see/ffa/ffa_plat_drv.h>
#endif

#define clk_smc_e(fmt, args ...) pr_err("[CLK_SMC]"fmt, ##args)

#define clk_smc_i(fmt, args ...) pr_info("[CLK_SMC]"fmt, ##args)

#define clk_smc_d(fmt, args ...) pr_debug("[CLK_SMC]"fmt, ##args)

static DEFINE_SPINLOCK(clk_smc_lock);

#ifdef CONFIG_CLOCK_SUPPORT_FFA
noinline int atfd_service_clk_smc(u64 _arg1, u64 _arg2, u64 _arg3, u64 _arg4)
{
	int ret;
	unsigned long flags;
	struct ffa_send_direct_data args = {0};

	args.data0 = CLK_REGISTER_FN_ID;
	args.data1 = _arg1;
	args.data2 = _arg2;
	args.data3 = _arg3;
	args.data4 = _arg4;

	spin_lock_irqsave(&clk_smc_lock, flags);
	ret = ffa_platdrv_send_msg(&args);
	spin_unlock_irqrestore(&clk_smc_lock, flags);

	if (ret != 0) {
		pr_err("%s failed to send_msg, ret:%d!\n", __func__, ret);
		return ret;
	}

	return (int)args.data0;
}
#else
/* atf entrance function support */
noinline int atfd_service_clk_smc(u64 _arg1, u64 _arg2, u64 _arg3, u64 _arg4)
{
	struct arm_smccc_res res;
	unsigned long flags;

	spin_lock_irqsave(&clk_smc_lock, flags);
#ifdef CONFIG_LIBLINUX_CDC
	arm_smccc_1_1_smc(CLK_REGISTER_FN_ID, _arg1, _arg2, _arg3, _arg4, 0, 0, 0, &res);
#else
	arm_smccc_smc(CLK_REGISTER_FN_ID, _arg1, _arg2, _arg3, _arg4, 0, 0, 0, &res);
#endif
	spin_unlock_irqrestore(&clk_smc_lock, flags);

	return (int)res.a0;
}
#endif /* CONFIG_CLOCK_SUPPORT_FFA */

noinline struct smc_ppll atfd_ppll_clk_smc(struct smc_ppll args)
{
	struct arm_smccc_res res;
#ifdef CONFIG_LIBLINUX_CDC
	arm_smccc_1_1_smc(CLK_REGISTER_FN_ID, args.arg1,
		args.arg2, args.arg3, args.arg4, 0, 0, 0, &res);
#else
	arm_smccc_smc(CLK_REGISTER_FN_ID, args.arg1,
		args.arg2, args.arg3, args.arg4, 0, 0, 0, &res);
#endif
	return (struct smc_ppll) {
		.smc_fid = CLK_REGISTER_FN_ID,
		.arg1 = args.arg1,
		.arg2 = args.arg2,
		.arg3 = args.arg3,
		.arg4 = args.arg4,
	};
}

