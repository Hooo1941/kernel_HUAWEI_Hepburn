/** @file
 * Copyright (c) 2020-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
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

#ifndef HDMIRX_DEBUG_H
#define HDMIRX_DEBUG_H

#include <linux/types.h>
#include <linux/ktime.h>

extern uint32_t g_hdmirx_log_level;
enum {
	HDMIRX_LOG_LVL_ERR = 0,
	HDMIRX_LOG_LVL_WARNING,
	HDMIRX_LOG_LVL_INFO,
	HDMIRX_LOG_LVL_DEBUG,
	HDMIRX_LOG_LVL_MAX,
};

#define DISP_LOG_TAG       "[HDMIRX %s]"
#define disp_dbg_nop(...)  ((void)#__VA_ARGS__)

#define disp_pr_err(msg, ...)  \
	do { \
		pr_err(DISP_LOG_TAG"[%s:%d]"msg, "E", __func__, __LINE__, ## __VA_ARGS__); \
	} while (0)

#define HDMIRX_CONFIG_LOG
#ifdef HDMIRX_CONFIG_LOG
#define disp_pr_warn(msg, ...)  \
	do { \
		if (g_hdmirx_log_level > HDMIRX_LOG_LVL_WARNING) \
			pr_warn(DISP_LOG_TAG"[%s:%d]"msg, "W", __func__, __LINE__, ## __VA_ARGS__); \
	} while (0)

#define disp_pr_info(msg, ...)  \
	do { \
		if (g_hdmirx_log_level > HDMIRX_LOG_LVL_INFO) \
			pr_err(DISP_LOG_TAG"[%s:%d]"msg, "I", __func__, __LINE__, ## __VA_ARGS__); \
	} while (0)

#define disp_pr_debug(msg, ...)  \
	do { \
		if (g_hdmirx_log_level > HDMIRX_LOG_LVL_DEBUG) \
			pr_info(DISP_LOG_TAG"[%s:%d]"msg, "D", __func__, __LINE__, ## __VA_ARGS__); \
	} while (0)
#else


#define disp_pr_warn(msg, ...)  disp_dbg_nop(msg, __VA_ARGS__)
#define disp_pr_info(msg, ...)  disp_dbg_nop(msg, __VA_ARGS__)
#define disp_pr_debug(msg, ...) disp_dbg_nop(msg, __VA_ARGS__)

#endif

#define disp_check_and_return(condition, ret, level, msg, ...) \
	do { \
		if (condition) { \
			disp_pr_##level(msg, ##__VA_ARGS__);\
			return ret; \
		} \
	} while (0)

#endif