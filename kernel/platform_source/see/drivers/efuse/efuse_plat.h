/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: efuse header file to adapt plat
 * Create: 2022/10/25
 */

#ifndef __EFUSE_PLAT_H__
#define __EFUSE_PLAT_H__

#include <linux/kernel.h>                     /* pr_err */

#define efuse_err(exp, ...)                   pr_err("[%s]: "exp, __func__, ##__VA_ARGS__)
#define efuse_info(exp, ...)                  pr_info("[%s]: "exp, __func__, ##__VA_ARGS__)

#endif /* __EFUSE_PLAT_H__ */
