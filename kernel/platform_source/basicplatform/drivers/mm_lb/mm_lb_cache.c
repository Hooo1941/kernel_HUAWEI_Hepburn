/*
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,4,0))
#include <linux/arm-smccc.h>
#else
#include <asm/compiler.h>
#endif
#include <linux/mm_types.h>

#include "mm_lb_priv.h"
#include "platform_include/see/bl31_smc.h"

#define MM_LB_QUOTA_VALUE        0xa13584
#define MM_LB_PLC_VALUE          0x4135a4
#define MM_LB_EFUSE              0xa358767
#define MM_LB_LITE               0x435912f

#define CMD_PARAM_SHIFT           16

static noinline u64 kernel_notify_atf(u64 fid, u32 spid, u32 quota, u32 plc)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,4,0))
	struct arm_smccc_res res;

	arm_smccc_smc(fid, spid, quota, plc,
		0, 0, 0, 0, &res);
	return (int)res.a0;
#else
	register u64 x0 asm("x0") = fid;
	register u64 x1 asm("x1") = spid;
	register u64 x2 asm("x2") = quota;
	register u64 x3 asm("x3") = plc;

	asm volatile (
		__asmeq("%0", "x0")
		__asmeq("%1", "x1")
		__asmeq("%2", "x2")
		__asmeq("%3", "x3")
		"smc    #0\n"
		: "+r" (x0)
		: "r" (x1), "r" (x2), "r"(x3));

	return x0;
#endif
}

int power_control(u32 flag)
{
	int ret;

	ret = kernel_notify_atf(MM_LB_FID_VALUE, flag,
			MM_LB_QUOTA_VALUE, MM_LB_PLC_VALUE);
	if (ret) {
		lb_print(LB_ERROR, "%s fail flag: 0x%x, ret: 0x%x\n",
			__func__, flag, ret);
		WARN_ON(1);
	}

	return ret;
}

int gid_bypass_control(u32 cmd, u32 gid, u32 quota, u32 plc)
{
	int ret;

	ret = (int)kernel_notify_atf(MM_LB_FID_VALUE,
			cmd | (gid << CMD_PARAM_SHIFT), quota, plc);
	if (ret) {
		lb_print(LB_ERROR, "%s fail 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n",
			__func__, cmd, gid, quota, plc, ret);
		WARN_ON(1);
	}

	return ret;
}

int sec_gid_set(u32 cmd, u32 gid, u32 quota)
{
	int ret;

	ret = (int)kernel_notify_atf(MM_LB_FID_VALUE,
			cmd | (gid << CMD_PARAM_SHIFT), quota, 0);
	if (ret) {
		lb_print(LB_ERROR, "%s fail cmd: 0x%x, gid: 0x%x, ret: 0x%x\n",
			__func__, cmd, gid, ret);
		WARN_ON(1);
	}

	return ret;
}

unsigned int get_lb_efuse(void)
{
	unsigned int ret;

	ret = kernel_notify_atf(MM_LB_FID_VALUE, MM_LB_EFUSE,
		MM_LB_QUOTA_VALUE, MM_LB_PLC_VALUE);

	return ret;
}

unsigned int get_lb_lite(void)
{
	unsigned int ret;

	ret = kernel_notify_atf(MM_LB_FID_VALUE, MM_LB_LITE,
		MM_LB_QUOTA_VALUE, MM_LB_PLC_VALUE);

	return ret;
}
