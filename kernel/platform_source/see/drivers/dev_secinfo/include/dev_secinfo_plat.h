/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: header file to provide plat utils
 * Create: 2023/03/31
 */

#ifndef __DEV_SECINFO_PLAT_H__
#define __DEV_SECINFO_PLAT_H__

#include <linux/kernel.h> /* pr_err & min & ARRAY_SIZE */

#define dev_secinfo_min(a, b)                    min((a), (b))
#define dev_secinfo_array_size(a)                ARRAY_SIZE(a)

#define dev_secinfo_err(exp, ...)                pr_err("[%s]: "exp, __func__, ##__VA_ARGS__)
#define dev_secinfo_info(exp, ...)               pr_info("[%s]: "exp, __func__, ##__VA_ARGS__)

#endif /* __DEV_SECINFO_PLAT_H__ */