/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2018. All rights reserved.
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

#ifndef __NV_DEBUG_H__
#define __NV_DEBUG_H__

#include <bsp_print.h>

#define THIS_MODU mod_nv

#define CONFIG_NV_DEBUG 0
#define nv_dbg_printf(fmt, ...)          \
    do {                                 \
        if (CONFIG_NV_DEBUG) {           \
            bsp_err(fmt, ##__VA_ARGS__); \
        }                                \
    } while (0)

enum {
    NV_CRIT,
    NV_ERR,
    NV_WARN,
    NV_INFO,
    NV_DBG,
    NV_DEFT,
};

#define CONFIG_NV_DEBUG_LEVEL 3
#define nv_debug_printf(level, fmt, ...)     \
    do {                                     \
        if (level < CONFIG_NV_DEBUG_LEVEL) { \
            bsp_err(fmt, ##__VA_ARGS__);     \
        }                                    \
    } while (0)

#define nv_printf(fmt, ...) bsp_err(fmt, ##__VA_ARGS__)

#define NV_DUMP_SIZE (4 * 1024)

enum {
    NV_API_READ = 0,
    NV_FUN_READ_EX = 1,
    NV_API_AUTHREAD = 2,
    NV_API_DIRECTREAD = 3,
    NV_API_WRITE = 4,
    NV_FUN_WRITE_EX = 5,
    NV_API_AUTHWRITE = 6,
    NV_API_DIRECTWRITE = 7,
    NV_API_GETLEN = 8,
    NV_API_AUTHGETLEN = 9,
    NV_API_FLUSH = 10,
    NV_API_REVERT = 11,
    NV_API_BACKUP = 12,
    NV_API_REVERT_MANF = 13,
    NV_API_REVERT_USER = 14,
    NV_API_IMPORT = 15,
    NV_API_EXPORT = 16,
    NV_FUN_MEM_INIT = 17,
    NV_FUN_UPGRADE_PROC = 18,
    NV_FUN_KERNEL_INIT = 19,
    NV_FUN_REMAIN_INIT = 20,
    NV_FUN_NVM_INIT = 21,
    NV_FUN_XML_DECODE = 22,
    NV_FUN_SEARCH_NV = 23,
    NV_FUN_REVERT_DATA = 24,
    NV_FUN_UPDATE_DEFAULT = 25,
    NV_FUN_REVERT_DEFAULT = 26,
    NV_FUN_ERR_RECOVER = 27,
    NV_FUN_KEY_CHECK = 28,
    NV_FUN_FLUSH_SYS = 29,
    NV_API_FLUSH_LIST = 32,
    NV_CRC32_MAKE_DDR = 33,
    NV_CRC32_DATA_CHECK = 34,
    NV_CRC32_GET_FILE_LEN = 35,
    NV_CRC32_MAKE_DDR_CRC = 36,
    NV_CRC32_DLOAD_FILE_CHECK = 37,
    NV_CRC32_DDR_CRC_CHECK = 38,
    NV_CRC32_DDR_RESUME_IMG = 39,
    NV_FUN_READ_FILE = 40,
    NV_FUN_RESUME_ITEM = 41,
    NV_FUN_RESUME_IMG_ITEM = 42,
    NV_FUN_RESUME_BAK_ITEM = 43,
    NV_FUN_MAX_ID,
};

typedef struct debug_point {
    u32 callnum;
    u32 rsv1;
    u32 rsv2;
    u32 rsv3;
    u32 rsv4;
} debug_point;

enum nv_debug_state {
    NV_DEBUG_WRITEEX_START = 1,
    NV_DEBUG_WRITEEX_GET_IPC_START,
    NV_DEBUG_WRITEEX_GET_IPC_END,
    NV_DEBUG_WRITEEX_GIVE_IPC,
    NV_DEBUG_WRITEEX_MEM_START,
    NV_DEBUG_WRITEEX_FILE_START,
    NV_DEBUG_FLUSH_START,
    NV_DEBUG_FLUSH_END,
    NV_DEBUG_REQ_FLUSH_START,
    NV_DEBUG_REQ_FLUSH_END,
    NV_DEBUG_FLUSHEX_START,
    NV_DEBUG_FLUSHEX_OPEN_START,
    NV_DEBUG_FLUSHEX_OPEN_END,
    NV_DEBUG_FLUSHEX_GET_PROTECT_SEM_START,
    NV_DEBUG_FLUSHEX_GET_PROTECT_SEM_END,
    NV_DEBUG_FLUSHEX_GIVE_IPC,
    NV_DEBUG_FLUSHEX_GIVE_SEM,
    NV_DEBUG_FLUSHEX_WRITE_FILE_START,
    NV_DEBUG_FLUSHEX_WRITE_FILE_END,
    NV_DEBUG_WRITEEX_END,
    NV_DEBUG_SEND_MSG_START,
    NV_DEBUG_SEND_ICC,
    NV_DEBUG_SEND_MSG_TIMEOUT,
    NV_DEBUG_RECEIVE_ICC,
    NV_DEBUG_READ_ICC,
    NV_DEBUG_SEND_MSG_END,
    NV_DEBUG_REQ_ASYNC_WRITE,
    NV_DEBUG_REQ_REMOTE_WRITE,
    NV_DEBUG_BUTT
};

typedef struct debug_element {
    u32 state;
    u32 slice;
} debug_element;

typedef struct {
    u32 maxnum;
    u32 head;
    u32 tail;
    u32 num;
    debug_element data[0];
} debug_queue;

typedef struct {
    u32 state;
    char *info;
} debug_table;

struct nvm_debug_stru {
    u32 print;
    debug_point point[NV_FUN_MAX_ID];
    debug_queue *dump_queue;
    debug_table *table;
};

u32 nv_debug_init(void);

void nv_debug_record(u32 state);

void nv_debug(u32 type, u32 rsv1, u32 rsv2, u32 rsv3, u32 rsv4);

void nv_help(u32 type);

void nv_show_ddr_info(void);

void nv_show_ref_info(u32 startid, u32 endid);

u32 nv_show_item_info(u32 itemid);

#endif
