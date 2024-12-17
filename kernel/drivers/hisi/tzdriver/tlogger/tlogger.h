/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: TEE Logging Subsystem, read the tee os log from rdr memory
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
#ifndef TLOGGER_H
#define TLOGGER_H

#include <linux/types.h>

#define OPEN_FILE_MODE          0640U
#define ROOT_UID                0
#define ROOT_GID                0
#define SYSTEM_GID              1000
#ifdef LAST_TEE_MSG_ROOT_GID
#define FILE_CHOWN_GID                0
#else
/* system gid for last_teemsg file sys chown */
#define FILE_CHOWN_GID                1000
#endif

#define UINT64_MAX (uint64_t)(~((uint64_t)0)) /* 0xFFFFFFFFFFFFFFFF */

#ifdef CONFIG_TEELOG
void tz_log_write(void);
int tlogger_store_msg(const char *file_path, uint32_t file_path_len);
int register_mem_to_teeos(uint64_t mem_addr, uint32_t mem_len, bool is_cache_mem);

#ifdef CONFIG_TZDRIVER_MODULE
int init_tlogger_service(void);
void free_tlogger_service(void);
int register_tloger_mem(void);
#endif

#else
static inline void tz_log_write(void)
{
	return;
}

static inline int tlogger_store_msg(const char *file_path, uint32_t file_path_len)
{
	(void)file_path;
	(void)file_path_len;
	return 0;
}
static inline int register_mem_to_teeos(uint64_t mem_addr, uint32_t mem_len,
	bool is_cache_mem)
{
	(void)mem_addr;
	(void)mem_len;
	return 0;
}
static inline int init_tlogger_service(void)
{
	return 0;
}
static inline void free_tlogger_service(void)
{
}
#endif
#endif
