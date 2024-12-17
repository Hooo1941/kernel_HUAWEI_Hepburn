/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: efuse header file to provide utils
 * Create: 2023/01/30
 */

#ifndef __EFUSE_UTILS_H__
#define __EFUSE_UTILS_H__

#include <linux/kernel.h> /* pr_err & min & ARRAY_SIZE */

#define efuse_min(a, b)                          min((a), (b))
#define efuse_array_size(a)                      ARRAY_SIZE(a)
#define efuse_unused(a)                          (void)(a)

#endif /* __EFUSE_UTILS_H__ */
