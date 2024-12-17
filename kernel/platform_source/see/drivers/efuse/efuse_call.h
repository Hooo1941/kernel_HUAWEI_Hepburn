/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: efuse header file to provide sub-layer call
 * Create: 2023/04/26
 */

#ifndef __EFUSE_CALL_H__
#define __EFUSE_CALL_H__

#include <linux/types.h>
#include <platform_include/see/efuse_driver.h>

struct efuse_call_args {
	uintptr_t arg0;
	uintptr_t arg1;
	uintptr_t arg2;
	uintptr_t arg3;
	uintptr_t arg4;
};

#ifdef CONFIG_EFUSE_SUPPORT_FFA
uint32_t efuse_ffa(struct efuse_call_args *arg);

uint32_t efuse_ffa_rd_shmem(struct efuse_call_args *arg, uint8_t *buf);

uint32_t efuse_ffa_wr_shmem(struct efuse_call_args *arg, uint8_t *buf);

#else
uint32_t efuse_smc(struct efuse_call_args *arg);

#endif /* CONFIG_EFUSE_SUPPORT_FFA */
#endif /* __EFUSE_CALL_H__ */
