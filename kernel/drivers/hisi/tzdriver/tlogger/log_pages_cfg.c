/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: for pages log cfg api define
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include "log_cfg_api.h"

#include <linux/sizes.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sysfs.h>
#include <linux/semaphore.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/stat.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>
#include <linux/slab.h>

#include <securec.h>
#include "tc_ns_log.h"
#include "tlogger.h"
#include "shared_mem.h"

void unregister_log_exception(void)
{
}

int register_log_exception(void)
{
	return 0;
}

struct pages_module_result {
	uint64_t log_addr;
	uint32_t log_len;
};

struct pages_module_result g_mem_info = {0};

static int tee_pages_register_core(void)
{
	if (g_mem_info.log_addr != 0 || g_mem_info.log_len != 0) {
		if (memset_s((void *)g_mem_info.log_addr,  g_mem_info.log_len, 0,  g_mem_info.log_len) != 0) {
			tloge("clean log memory failed\n");
			return -EFAULT;
		}
		return 0;
	}

	g_mem_info.log_addr = get_log_mem_vaddr();
	if (IS_ERR_OR_NULL((void *)(uintptr_t)g_mem_info.log_addr)) {
		tloge("get log mem error\n");
		return -1;
	}
	g_mem_info.log_len = PAGES_LOG_MEM_LEN;
	return 0;
}

/* Register log memory */
int register_log_mem(uint64_t *addr, uint32_t *len)
{
	int ret;
	uint64_t mem_addr;
	uint32_t mem_len;

	if (!addr || !len) {
		tloge("addr or len is invalid\n");
		return -1;
	}

	ret = tee_pages_register_core();
	if (ret != 0)
		return ret;

	mem_addr = get_log_mem_paddr(g_mem_info.log_addr);
	mem_len = g_mem_info.log_len;

	ret = register_mem_to_teeos(mem_addr, mem_len, true);
	if (ret != 0)
		return ret;

	*addr = g_mem_info.log_addr;
	*len = g_mem_info.log_len;
	return ret;
}

void report_log_system_error(void)
{
}

void report_log_system_panic(void)
{
/* default support trigger ap reset */
#ifndef NOT_TRIGGER_AP_RESET
	panic("TEEOS panic\n");
#endif
}

void ta_crash_report_log(void)
{
}

int *map_log_mem(uint64_t mem_addr, uint32_t mem_len)
{
	(void)mem_len;
	return (int *)(uintptr_t)mem_addr;
}

void unmap_log_mem(int *log_buffer)
{
	free_log_mem((uint64_t)(uintptr_t)log_buffer);
}

void get_log_chown(uid_t *user, gid_t *group)
{
	if (!user || !group) {
		tloge("user or group buffer is null\n");
		return;
	}

	*user = ROOT_UID;
	*group = FILE_CHOWN_GID;
}
