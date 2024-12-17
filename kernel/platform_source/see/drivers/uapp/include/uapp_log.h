/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: header file to provide log
 * Create: 2023/09/06
 */

#ifndef __UAPP_LOG_H__
#define __UAPP_LOG_H__

#include <linux/kernel.h> /* pr_err & min & ARRAY_SIZE */

#define uapp_err(exp, ...)                       pr_err("[%s]: "exp, __func__, ##__VA_ARGS__)
#define uapp_info(exp, ...)                      pr_info("[%s]: "exp, __func__, ##__VA_ARGS__)

#endif /* __UAPP_LOG_H__ */