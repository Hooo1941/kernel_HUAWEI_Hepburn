/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: header file to provide utils
 * Create: 2023/09/06
 */

#ifndef __UAPP_UTILS_H__
#define __UAPP_UTILS_H__

#include <uapp_log.h>
#include <uapp_errno.h>
#include <linux/kernel.h> /* pr_err & min & ARRAY_SIZE */

#define UAPP_BITS_PER_BYTE                       8

#define uapp_unused(x)                           (void)(x)
#define uapp_min(a, b)                           min((a), (b))
#define uapp_array_size(a)                       ARRAY_SIZE(a)
#define uapp_div_round_up(nr, d)                 (((nr) + (d) - 1) / (d))

#define uapp_return_err_if_null(__ptr)\
	do { \
		if ((__ptr) == NULL) { \
			uapp_err("error, %s is NULL\n", #__ptr); \
			return UAPP_NULL_PTR_ERR; \
		} \
	} while (0)

#define uapp_return_err_if_size_zero(__size) \
	do { \
		if ((__size) == 0) { \
			uapp_err("error, size zero\n"); \
			return UAPP_SIZE_ZERO_ERR; \
		} \
	} while (0)

#define uapp_expect_true(cond, ret) \
	do { \
		if (!(cond)) { \
			uapp_err("error, cond %s not fit\n", #cond); \
			return ret;\
		} \
	} while(0)

#endif /* __UAPP_UTILS_H__ */