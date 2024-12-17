/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 * Description: pac test for bl31
 * Create : 2023/2/24
 */

#include <asm/cputype.h>
#include <platform_include/see/bl31_smc.h>
#include <platform_include/see/ffa/ffa.h>
#include <linux/arm-smccc.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/sched/mm.h>
#include <pac_test.h>

static int smc_to_atf(u64 _function_id, u64 _arg0, u64 _arg1, u64 _arg2)
{
	struct arm_smccc_res res = {0};

	arm_smccc_smc(_function_id, _arg0, _arg1, _arg2, 0, 0, 0, 0, &res);

	return (int)res.a0;
};


void test_bl31_pac(void)
{
	smc_to_atf(FID_BL31_PAC_TEST, 0, 0, 0);
}


void test_hhee_pac(void)
{
	struct arm_smccc_res res = {0};
	/* input hvc id, as arg0 */
	arm_smccc_hvc((u64)HHEE_PAC_TEST_HVC_INFO, 0, 0, 0, 0, 0, 0, 0, &res);
}

void test_kernel_pac(void)
{
	pr_err("KERNEL PAC ERROR\n");
	kernel_pac_test_handler();
}

void test_thee_pac(void)
{
	smc_to_atf(FFA_THEE_TEST_ID, THEE_PAC_TEST_ID, 0, 0);
}
