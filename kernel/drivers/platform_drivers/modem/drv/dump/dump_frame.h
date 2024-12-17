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
#ifndef __DUMP_FRAME_H__
#define __DUMP_FRAME_H__
#include "product_config.h"
#ifdef BSP_CONFIG_PHONE_TYPE
#include <adrv_om.h>
#else
#include <bsp_rdr.h>
#endif
#include <linux/workqueue.h>
#include <bsp_dump.h>
#include <bsp_print.h>

#define RDR_DUMP_FILE_PATH_LEN 128
#define RDR_DUMP_FILE_CP_PATH "cp_log/"
#define RDR_DUMP_DIR_PTH_LEN (RDR_DUMP_FILE_PATH_LEN - strlen(RDR_DUMP_FILE_CP_PATH))
#define DUMP_EXC_DESC_SIZE (48)
#define DUMP_INVALID_MODID (0xFFABBAFF)
#define DUMP_MSLEEP_60 600
#define DUMP_SLEEP_5MS_NUM 5
#define DUMP_SLEEP_30MS_NUM 30
#define DUMP_WAIT_30S_NUM 30000 /* 30000 ms */
#define DUMP_WAIT_15S_NUM 15000 /* 15000 ms */
#define DUMP_WAIT_5S_NUM 5000   /* 5000 ms */
#define DUMP_WAIT_3S_NUM 3000   /* 3000 ms */

#define DUMP_CORE(n) (1 << (n))
#define DUMP_BIT(nr) (1UL << (nr))

#define dump_debug(fmt, ...) (bsp_debug("<%s> " fmt, __FUNCTION__, ##__VA_ARGS__))
#define dump_warning(fmt, ...) (bsp_wrn("<%s> " fmt, __FUNCTION__, ##__VA_ARGS__))
#define dump_error(fmt, ...) (bsp_err("<%s> " fmt, __FUNCTION__, ##__VA_ARGS__))
#define dump_print(fmt, ...) (bsp_err(fmt, ##__VA_ARGS__))

#define DUMP_MDM_EXC_TYPE(x) \
    {                        \
        x, #x                \
    }

static inline void dump_wait_done(u32 *flag, int *time)
{
    while (*flag) {
        set_current_state(TASK_INTERRUPTIBLE);
        (void)schedule_timeout(HZ / 10); /* 10 means wait 100ms */
        if (((*time)--) <= 0) {
            break;
        }
    }
}

typedef void (*dump_done)(u32 modid, u32 sysid);
typedef s32 (*notify_hook)(u32 modid, dump_done done_cb);
typedef int (*dump_reset_hook)(void);

typedef enum {
    DUMP_RET_MODID_UNMATCH = 0xa000,
    DUMP_RET_PARA_ERR = 0xa001,
    DUMP_RET_INT_VAL = 0xa002,
    EXC_RET_TYPE_BUTT,
} dump_ret_type_e;

typedef enum {
    SUBSYS_AP = 0x0,
    SUBSYS_CP = 0x1,
    SUBSYS_NR = 0x2,
    SUBSYS_M3 = 0x3,
    SUBSYS_COMM = 0x4,
    SUBSYS_MAX = SUBSYS_COMM,
    SUBSYS_BUTT = 5,
} dump_subsys_e;

typedef enum {
    DUMP_AP = DUMP_CORE(SUBSYS_AP),
    DUMP_CP = DUMP_CORE(SUBSYS_CP),
    DUMP_NR = DUMP_CORE(SUBSYS_NR),
    DUMP_M3 = DUMP_CORE(SUBSYS_M3),
    DUMP_SUBSYS_MAX = DUMP_CORE(SUBSYS_MAX)
} dump_core_e;

typedef struct {
    u32 modid;
    u32 product_type;
    u64 coreid;
    char log_path[RDR_DUMP_FILE_PATH_LEN];
    pfn_cb_dump_done dump_done;
} rdr_exc_info_s;

typedef struct {
    u32 core;
    u32 modid;
    u32 arg1;
    u32 arg2;
    u32 slice;
} dump_exc_reentry_info_s;

typedef struct {
    u32 exc_modid;
    char exc_desc[DUMP_EXC_DESC_SIZE];
} dump_exc_type_s;

typedef struct {
    u64 e_reset_core_mask;
    u32 e_exce_type;
    u32 e_reboot_priority;
    u32 src_modid_start;
    u32 src_modid_end;
    dump_exc_type_s exc_info;
} dump_mdm_exc_table_info_s;

typedef struct {
    dump_subsys_e subsys;
    notify_hook notify_ops;
    dump_reset_hook reset_cb;
} dump_subsys_info_s;

typedef struct {
    spinlock_t spinlock;
    u32 notify_core;
    u32 cur_modid;
    int wait_max_time;
    struct workqueue_struct *done_wq;
    dump_subsys_info_s subsys_ops[SUBSYS_MAX];
} dump_frame_ctrl_s;

s32 dump_frame_init(void);
void dump_register_module_ops(dump_subsys_e subsys, notify_hook ops, dump_reset_hook dump_reset_cb);
void dump_notify_module_dump_cb(u32 modid);
void dump_notify_module_reset_start(void);
void dump_fill_exc_info(u32 mod_id, u32 arg1, u32 arg2, u32 core, u32 reason, const char *desc);
void dump_fill_exc_contex(dump_reboot_ctx_e contex, u32 task_id, u32 int_no, const char *task_name);
s32 dump_exception_handler(void);
void dump_exc_handle_done(void);
dump_exception_info_s *dump_get_cur_exc_info(void);
rdr_exc_info_s *dump_get_rdr_exc_info(void);
void dump_queue_work(struct work_struct *wq);
s32 dump_wait_save_done(void);
int dump_is_in_exception(void);
int dump_sysreboot_exception_reentry_handler(u32 ori_modid, u32 arg1, u32 arg2, u32 core);
void dump_frame_proc_reentry_exc_info(u32 modid);
dump_exc_reentry_info_s *dump_get_reentry_exc_info(void);
#endif
