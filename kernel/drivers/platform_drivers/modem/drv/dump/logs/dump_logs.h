/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef __DUMP_LOGS_H__
#define __DUMP_LOGS_H__

#include <linux/version.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <mdrv_om.h>
#include <osl_types.h>
#include <bsp_dump.h>
#include <bsp_dt.h>
#include <dump_frame.h>

#define DUMP_LOG_FILE_AUTH 0640
#define DUMP_LOG_MBB_DIR_AUTH 0750
#define DUMP_LOG_PHONE_DIR_AUTH 0770

typedef struct {
    s32 init_flag;
    spinlock_t lock;
    struct list_head log_list;
} dump_logs_ctrl_s;

void dump_logs_init(void);
void dump_logs_list_init(void);
int dump_create_dir(char *path);
void dump_save_log_callback(const char *path, dump_subsys_e subsys);
int dump_create_file(const char *filename);
s32 dump_get_log_notifier_init_flag(void);
s32 dump_save_ddr_file(const char *dir_name, const char *ddr_region, const char *file_name, bool head_flag);
int dump_log_save(const char *dir, const char *file_name, const void *address, const void *phy_addr, u32 length);
int dump_append_file(const char *dir, const char *filename, const void *address, u32 length,
    u32 max_size);
s32 dump_mkdir(const char *dir);
char *dump_backup_dts_str(const char *src_str);

struct file *dump_open(const char *path, s32 flags, s32 file_mode);
s32 dump_close(struct file *fd);
s32 dump_write_sync(struct file *fd, const s8 *ptr, u32 size);
void dump_logs_reset_proc(void);
#endif
