/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 * This file is the dvfs implementation of the PM module.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_LOG_H
#define HVGR_LOG_H
#include <linux/device.h>
#include <linux/types.h>

struct hvgr_log_dev {
	unsigned long log_flag;
	bool dumpstack_flag;
	bool timing_flag;
	bool systrace_flag;
};

/*
 * enum HVGR_LOG_LEVEL - The types of log level
 */
enum HVGR_LOG_LEVEL {
	HVGR_LOG_INFO = 0,
	HVGR_LOG_DEBUG,
	HVGR_LOG_ERROR,
	HVGR_LOG_MAX,
};

bool hvgr_log_is_enable(enum HVGR_LOG_LEVEL level, uint32_t module);
char *hvgr_log_get_module_name(uint32_t module);
char *hvgr_log_get_level_name(uint32_t level);

void hvgr_systrace_write(const char *fmt, ...);

#define hvgr_systrace_begin(fmt, ...) \
	hvgr_systrace_write("B|%d|"fmt"\n", current->tgid, ##__VA_ARGS__)
#define hvgr_systrace_end() hvgr_systrace_write("E|%d\n", current->tgid)

#define hvgr_log(gdev, module, level, fmt, ...) do { \
	if (hvgr_log_is_enable(level, module)) { \
		hvgr_systrace_begin("%s %s: "fmt, hvgr_log_get_module_name(module), \
			hvgr_log_get_level_name(level), ##__VA_ARGS__); \
		hvgr_systrace_end(); \
		dev_err(gdev->dev, "[%s][%s] " fmt, hvgr_log_get_module_name(module), \
			hvgr_log_get_level_name(level), ##__VA_ARGS__); \
	} \
} while (0)
#endif
