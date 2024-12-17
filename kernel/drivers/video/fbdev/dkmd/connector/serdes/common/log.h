/* Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
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

#ifndef __COMMON_LOG_H__
#define __COMMON_LOG_H__

#include <linux/types.h>
#include <linux/cdev.h>

enum {
	CDC_DEBUG_EMERG = 1 << 7,
	CDC_DEBUG_ALERT = 1 << 6,
	CDC_DEBUG_CRIT = 1 << 5,
	CDC_DEBUG_ERR = 1 << 4,
	CDC_DEBUG_WARING = 1 << 3,
	CDC_DEBUG_NOTICE = 1 << 2,
	CDC_DEBUG_INFO = 1 << 1,
	CDC_DEBUG_DEBUG = 1 << 0,
};

static uint32_t cdc_debug_mask = CDC_DEBUG_DEBUG | CDC_DEBUG_INFO | CDC_DEBUG_WARING | \
	CDC_DEBUG_EMERG | CDC_DEBUG_ALERT | CDC_DEBUG_ERR;

#define CDC_LOG_TAG "CDC_UNDEFINE"

#define CDC_LOG_DEBUG
#ifdef CDC_LOG_DEBUG
#define cdc_debug(fmt, ...) \
	do { \
		if (cdc_debug_mask & CDC_DEBUG_DEBUG) \
			printk(KERN_DEBUG "[%s]" "[DEBUG] [%u %s]" fmt "\n", CDC_LOG_TAG, __LINE__, __func__, ##__VA_ARGS__); \
	} while (0)

#define cdc_info(fmt, ...) \
	do { \
		if (cdc_debug_mask & CDC_DEBUG_INFO) \
			printk(KERN_INFO "[%s]" "[INFO] [%u %s]" fmt "\n", CDC_LOG_TAG, __LINE__, __func__, ##__VA_ARGS__); \
	} while (0)

#define cdc_notice(fmt, ...) \
	do { \
		if (cdc_debug_mask & CDC_DEBUG_NOTICE) \
			printk(KERN_NOTICE "[%s]" "[NOTICE] [%u %s]" fmt "\n", CDC_LOG_TAG, __LINE__, __func__, ##__VA_ARGS__); \
	} while (0)

#define cdc_warn(fmt, ...) \
	do { \
		if (cdc_debug_mask & CDC_DEBUG_WARING) \
			printk(KERN_WARNING "[%s]" "[WARN] [%u %s]" fmt "\n", CDC_LOG_TAG, __LINE__, __func__, ##__VA_ARGS__); \
	} while (0)

#define cdc_err(fmt, ...) \
	do { \
		if (cdc_debug_mask & CDC_DEBUG_ERR) \
			printk(KERN_ERR "[%s]" "[ERROR] [%u %s]" fmt "\n", CDC_LOG_TAG, __LINE__, __func__, ##__VA_ARGS__); \
	} while (0)

#define cdc_crit(fmt, ...) \
	do { \
		if (cdc_debug_mask & CDC_DEBUG_CRIT) \
			printk(KERN_CRIT "[%s]" "[CRIT] [%u %s]" fmt "\n", CDC_LOG_TAG, __LINE__, __func__, ##__VA_ARGS__); \
	} while (0)

#define cdc_alert(fmt, ...) \
	do { \
		if (cdc_debug_mask & CDC_DEBUG_ALERT) \
			printk(KERN_ALERT "[%s]" "[ALERT] [%u %s]" fmt "\n", CDC_LOG_TAG, __LINE__, __func__, ##__VA_ARGS__); \
	} while (0)

#define cdc_emerg(fmt, ...) \
	do { \
		if (cdc_debug_mask & CDC_DEBUG_EMERG) \
			printk(KERN_EMERY "[%s]" "[EMERG] [%u %s]" fmt "\n", CDC_LOG_TAG, __LINE__, __func__, ##__VA_ARGS__); \
	} while (0)
#else
#define cdc_debug(fmt, ...)

#define cdc_info(fmt, ...)

#define cdc_notice(fmt, ...)

#define cdc_warn(fmt, ...) \
	do { \
		if (cdc_debug_mask & CDC_DEBUG_WARING) \
			printk("[%s]" "[WARN] [%u %s]" fmt "\n", CDC_LOG_TAG, __LINE__, __func__, ##__VA_ARGS__); \
	} while (0)

#define cdc_err(fmt, ...) \
	do { \
		if (cdc_debug_mask & CDC_DEBUG_ERR) \
			printk("[%s]" "[ERROR] [%u %s]" fmt "\n", CDC_LOG_TAG, __LINE__, __func__, ##__VA_ARGS__); \
	} while (0)

#define cdc_crit(fmt, ...) \
	do { \
		if (cdc_debug_mask & CDC_DEBUG_CRIT) \
			printk("[%s]" "[CRIT] [%u %s]" fmt "\n", CDC_LOG_TAG, __LINE__, __func__, ##__VA_ARGS__); \
	} while (0)

#define cdc_alert(fmt, ...) \
	do { \
		if (cdc_debug_mask & CDC_DEBUG_ALERT) \
			printk("[%s]" "[ALERT] [%u %s]" fmt "\n", CDC_LOG_TAG, __LINE__, __func__, ##__VA_ARGS__); \
	} while (0)

#define cdc_emerg(fmt, ...) \
	do { \
		if (cdc_debug_mask & CDC_DEBUG_EMERG) \
			printk("[%s]" "[EMERG] [%u %s]" fmt "\n", CDC_LOG_TAG, __LINE__, __func__, ##__VA_ARGS__); \
	} while (0)
#endif

#define log_secure_c_failed_if(cond, ret_val) \
do { \
	if (cond) { \
		cdc_err("secure c failed, ret value[%d], %s[%d]", ret_val, __func__, __LINE__); \
		return ret_val; \
	} \
} while (0)

#define return_value_if_run_error(cond, ret_val, format, ...) \
do { \
	if (cond) { \
		cdc_err(format, ##__VA_ARGS__); \
		return ret_val; \
	} \
} while (0)

#endif /* __COMMON_LOG_H__ */
