/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: ioctrl cmd for fs crypto header
 * Author: Huawei OS Kernel Lab
 * Create : Thu Oct 20 11:29:30 2023
 */

#ifndef FBEX_FSCRYPTO_H
#define FBEX_FSCRYPTO_H

#include <linux/types.h>

typedef uint32_t (*fbex_func)(void __user *argp);

struct fbex_function_table {
	uint32_t cmd_id;
	fbex_func func;
};

#ifdef CONFIG_FILE_BASED_ENCRYPTO_FOR_HM_FSCRYPTO
int fbex_init_fscrypto_dev(void);
void fbex_cleanup_fscrypto_dev(void);
#else
static inline int fbex_init_fscrypto_dev(void) {return 0;}
static inline void fbex_cleanup_fscrypto_dev(void) {}
#endif
#endif
