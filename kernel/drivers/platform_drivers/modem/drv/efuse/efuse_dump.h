/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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
#ifndef _EFUSE_DEBUG_H_
#define _EFUSE_DEBUG_H_

#include <osl_types.h>

#define EFUSE_DUMP_SIZE (256)

/* efuse teeos dump记录的时间ID */
/*---acore dump ID:0x1000开始--- */
typedef enum {
    EFUSE_ACORE_INIT_SUCCESS = 0x1000,
    EFUSE_ACORE_MSG_PROC_DONE,
    EFUSE_ACORE_SEC_CALL_START,
    EFUSE_ACORE_SEC_CALL_DONE,
    EFUSE_ACORE_HANDLE_WORK_START,
    EFUSE_ACORE_HANDLE_WORK_DONE,

    EFUSE_EVENT_MAX,
} efuse_debug_event_type_e;

typedef enum {
    EFUSE_RECORD_START = 0xAABBCCDD,
    EFUSE_RECORD_STOP = 0xCCDDAABB,
} efuse_record_flag_e;

typedef struct {
    u16 index;                            /* dump记录索引 */
    u16 event;                             /* 当前记录事件 */
    u32 info;                             /* 当前记录info */
} efuse_event_dump_t;

typedef struct {
    efuse_event_dump_t *base_addr;             /* efuse dump空间基地址 */
    u32 cur;                                   /* 当前偏移 */
    u32 dump_index;                            /* dump记录索引 */
    efuse_record_flag_e efuse_record_flag;     /* dump记录状态 */
} efuse_dump_info;

int efuse_debug_init(void);
void efuse_dump_rec(efuse_debug_event_type_e event);

#define EFUSE_RECORD_EVENT(event)                         \
    do {                                                  \
        efuse_dump_rec(event);                            \
    } while (0)
#endif