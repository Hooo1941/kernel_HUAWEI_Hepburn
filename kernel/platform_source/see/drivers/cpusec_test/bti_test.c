/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: el2 and el3 bti test
 * Create : 2023/2/8
 */
#include <platform_include/see/ffa/ffa.h>
#include <platform_include/see/bl31_smc.h>
#include <securec.h>
#include <asm/cputype.h>
#include <asm/compiler.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/arm-smccc.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/sched/mm.h>
#include <linux/atomic.h>
#include <linux/debugfs.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/timekeeping.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include "bti_test.h"

static inline struct arm_smccc_res smc_to_atf(u64 _fid, u64 _arg0, u64 _arg1, u64 _arg2)
{
	struct arm_smccc_res res = {0};

	arm_smccc_smc(_fid, _arg0, _arg1, _arg2, 0, 0, 0, 0, &res);

	return res;
};

void bl31_bti_test(void)
{
	smc_to_atf(FID_BL31_BTI_TEST, 0, 0, 0);
}

void hhee_bti_test(void)
{
	struct arm_smccc_res res;

	arm_smccc_hvc(HHEE_BTI_TEST_HVC_ID, 0, 0, 0, 0, 0, 0, 0, &res);
}

void kernel_bti_test(void)
{
	kernel_bti_testcase();
}

void thee_bti_test(void)
{
	struct arm_smccc_res ffa_ret = {0};

	ffa_ret = smc_to_atf(FFA_THEE_TEST_ID, THEE_BTI_TEST_ID, 0, 0);
	(void)ffa_ret;

	return;
}

