/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2022-2022. All rights reserved.
 * This file is the dvfs implementation of the PM module.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */
#include "hvgr_log.h"

#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include "hvgr_defs.h"
#include "hvgr_log_api.h"
#include "hvgr_debugfs_api.h"

static struct hvgr_log_dev log_dev;

static char *module_name[] = {
	"PM",
	"DM",
	"MEM",
	"CQ",
	"DATAN",
	"SCH",
	"SYNC",
#if defined(HVGR_SUPPORT_SPEC_LOW) && HVGR_SUPPORT_SPEC_LOW != 0
	"SPEC_LOW",
#endif
	"FENCE"
};

static char *level_name[] = {
	"HVGR_INFO",
	"HVGR_DEBUG",
	"HVGR_ERROR"
};

bool hvgr_log_is_enable(enum HVGR_LOG_LEVEL level, uint32_t module)
{
	if ((level == HVGR_LOG_INFO) && ((module & log_dev.log_flag) != module))
		return false;
	return true;
}

char *hvgr_log_get_module_name(uint32_t module)
{
	uint32_t index;

	index = (uint32_t)ffs((int)module) - 1;
	if (index >= ARRAY_SIZE(module_name))
		return "UNKNOWN";
	return module_name[index];
}

char *hvgr_log_get_level_name(uint32_t level)
{
	if (level >= ARRAY_SIZE(level_name))
		return "UNKNOWN";

	return level_name[level];
}

void hvgr_dump_stack(void)
{
	if (!log_dev.dumpstack_flag)
		return;
	dump_stack();
}

uint64_t hvgr_start_timing(void)
{
	if (!log_dev.timing_flag)
		return 0;
	return (uint64_t)ktime_to_ns(ktime_get());
}

void hvgr_end_timing_and_log(struct hvgr_device *gdev, uint32_t module,
	uint64_t start_time, const char *msg)
{
	if (!log_dev.timing_flag)
		return;
	hvgr_log(gdev, module, HVGR_LOG_ERROR, "%s time = %d",
		msg, ktime_to_ns(ktime_get()) - start_time);
}

#ifdef CONFIG_TRACING
static int tracing_mark_write(void)
{
	return 0;
}

void hvgr_systrace_write(const char *fmt, ...)
{
	va_list ap;
	if (!log_dev.systrace_flag)
		return;
	va_start(ap, fmt);
	(void)__ftrace_vbprintk((unsigned long)&tracing_mark_write, fmt, ap);
	va_end(ap);
}
#else
void hvgr_systrace_write(const char *fmt, ...) {}
#endif

static ssize_t hvgr_show_log_status(char * const buf)
{
	ssize_t ret = 0;
	uint32_t index;

	ret += scnprintf(buf + ret, PAGE_SIZE - ret, "Info Log status:\n");

	for (index = 0; index < ARRAY_SIZE(module_name); index++)
		ret += scnprintf(buf + ret, PAGE_SIZE - ret,
			"    %s Log status: %s\n", module_name[index],
			test_bit((int)index, &log_dev.log_flag) ? "Open" : "Close");

	return ret;
}

ssize_t hvgr_show_log_switch(void *data, char * const buf)
{
	ssize_t ret = 0;

	ret += hvgr_show_log_status(buf);
	ret += scnprintf(buf + ret, PAGE_SIZE - ret,
		"Dump stack status: %s\n", log_dev.dumpstack_flag ? "Open" : "Close");
	ret += scnprintf(buf + ret, PAGE_SIZE - ret,
		"Timer status: %s\n", log_dev.timing_flag ? "Open" : "Close");
	ret += scnprintf(buf + ret, PAGE_SIZE - ret,
		"Systrace status: %s\n", log_dev.systrace_flag ? "Open" : "Close");

	return ret;
}

static bool hvgr_switch_log_status(const char *buf)
{
	uint32_t index;
	char tmp_str[20] = {0};

	if (sysfs_streq("log open", buf)) {
		log_dev.log_flag = HVGR_LOG_OPEN_ALL;
		return true;
	}

	if (sysfs_streq("log close", buf)) {
		log_dev.log_flag = HVGR_LOG_CLOSE_ALL;
		return true;
	}

	for (index = 0; index < ARRAY_SIZE(module_name); index++) {
		(void)scnprintf(tmp_str, 20, "log open %s", module_name[index]);
		if (strnstr(buf, tmp_str, strlen(tmp_str)))
			break;
	}

	if (index < ARRAY_SIZE(module_name)) {
		set_bit(index, &log_dev.log_flag);
		return true;
	}

	for (index = 0; index < ARRAY_SIZE(module_name); index++) {
		(void)scnprintf(tmp_str, 20, "log close %s", module_name[index]);
		if (strnstr(buf, tmp_str, strlen(tmp_str)))
			break;
	}

	if (index < ARRAY_SIZE(module_name)) {
		clear_bit(index, &log_dev.log_flag);
		return true;
	}
	return false;
}

ssize_t hvgr_set_log_switch(void *data, const char *buf, size_t count)
{
	if (hvgr_switch_log_status(buf))
		return (ssize_t)count;

	if (sysfs_streq("dump open", buf)) {
		log_dev.dumpstack_flag = true;
		return (ssize_t)count;
	}

	if (sysfs_streq("dump close", buf)) {
		log_dev.dumpstack_flag = false;
		return (ssize_t)count;
	}

	if (sysfs_streq("timing open", buf)) {
		log_dev.timing_flag = true;
		return (ssize_t)count;
	}

	if (sysfs_streq("timing close", buf)) {
		log_dev.timing_flag = false;
		return (ssize_t)count;
	}

	if (sysfs_streq("systrace open", buf)) {
		log_dev.systrace_flag = true;
		return (ssize_t)count;
	}

	if (sysfs_streq("systrace close", buf)) {
		log_dev.systrace_flag = false;
		return (ssize_t)count;
	}

	return (ssize_t)count;
}

hvgr_debugfs_read_write_declare(fops_log_switch, hvgr_show_log_switch, hvgr_set_log_switch);

void hvgr_log_init(struct hvgr_device * const gdev)
{
	log_dev.log_flag = 0;
	log_dev.dumpstack_flag = false;
	log_dev.timing_flag = false;
	log_dev.systrace_flag = false;
#ifdef CONFIG_DFX_DEBUG_FS
	hvgr_debugfs_register(gdev, "log_switch", gdev->root_debugfs_dir, &fops_log_switch);
#endif
}
