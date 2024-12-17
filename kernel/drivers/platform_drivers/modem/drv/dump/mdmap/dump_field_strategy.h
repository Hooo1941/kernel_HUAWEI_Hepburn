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

#ifndef __DUMP_FIELD_STRATEGY_H__
#define __DUMP_FIELD_STRATEGY_H__

#include <osl_types.h>
#include <dump_frame.h>

typedef struct {
    u32 field_id;
    u32 paddr;
    u8 *vaddr;
    u32 max_length;
    u16 status;
    u16 version;
    /* u64 associated_cmpnts,预留给smart dump */
    char name[DUMP_NAME_LEN];
} dump_field_info_s;

/* ===============================dump field alloc policy================================================ */
#define DUMP_MEM_ALLOC_EXCEPTION (DUMP_BIT(0))
#define DUMP_MEM_ALLOC_FULLDUMP (DUMP_BIT(1))
/* DUMP_MEM_ALLOC_ALWAYS = 0  not exception, not fulldump; 表示总是分配内存 */
/* 系统初始化后，就固定下来的特征，策略检查时，不满足以下特征，应返回DUMP_FIELD_STRATEGY_ALLOC_REJECT
 * 虽然支持FULLDUMP动态加载, 但smart dump二阶段才支持ALLOC_LATER处理，因此先将该特性放在FIX_FEATURE里 */
#define DUMP_MEMPOLICY_DECODE_FIX_FEATURE(attr) ((attr) & DUMP_MEM_ALLOC_FULLDUMP)
/* 系统初始化后，可能会动态修改的特征，策略检查时，不满足以下特征，应返回DUMP_FIELD_STRATEGY_ALLOC_LATER */
#define DUMP_MEMPOLICY_DECODE_DYN_FEATURE(attr) ((attr) & DUMP_MEM_ALLOC_EXCEPTION)
/* ==============================dump field alloc policy end============================================== */

/* dump_field_check_strategy的返回值*/
#define DUMP_FIELD_STRATEGY_ALLOC_ALLOW 0
#define DUMP_FIELD_STRATEGY_ALLOC_LATER 1
#define DUMP_FIELD_STRATEGY_ALLOC_REJECT (-1)

/* dump field strategy mode */
typedef enum {
    DUMP_FIELD_STRATEGY_DISABLE = 0,
    DUMP_FIELD_STRATEGY_PASS,
    DUMP_FIELD_STRATEGY_BLOCK,
    DUMP_FIELD_STRATEGY_BUTT
} dump_field_strategy_mode_e;

/* 对外返回的结构体 */
typedef struct {
    u32 field_id;
    u32 max_len;
    u32 alloc_policy;
} dump_field_strategy_desc_s;


int dump_field_strategy_init(void);
int dump_field_check_strategy(u32 field_id, u32 alloc_len);

#endif /* __DUMP_FIELD_H__ */
