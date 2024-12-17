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


#ifndef MDMPM_DEBUG_H
#define MDMPM_DEBUG_H

#include <bsp_ipc_fusion.h>
#include <bsp_print.h>

#define pm_err(fmt, ...) (bsp_err("<%s> <%d> " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__))

#define PM_WL_NAME_MAXLEN 32
#define PM_WU_MAX_DSS_NUM 4
#define CORE_MASK 0xF0
#define CORE_SHIFT 4
#define WAKEUP_TYPE_MASK 0xF
#define PM_WAKEUP_MODE 0644
#define MAX_U64_STR_SIZE 16

typedef enum {
    PM_WAKEUP_THEN_LOCK = 0,
    PM_WAKEUP_THEN_UNLOCK,
    PM_WAKEUP_TYPE_MAX,
} wakeup_type_e;

typedef enum {
    DSS0 = 0,
    DSS1,
    DSS2,
    DSS3,
    CCORE_MAX,
} ccore_coreid_e;

typedef struct {
    ipc_res_id_e ipc_id;
    ipc_handle_t ipc_handle;
} mdmpm_debug_ipc;

typedef struct {
    unsigned int dss_id;
    unsigned int ipc_cnt;
    unsigned int cur_wakeup_type;
} core_wakeup;

typedef struct {
    unsigned int cmd;
    core_wakeup wakeup[CCORE_MAX];
} core_wakeup_ctrl;

#endif