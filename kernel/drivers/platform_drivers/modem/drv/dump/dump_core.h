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

#ifndef __DUMP_CORE_H__
#define __DUMP_CORE_H__
#include "product_config.h"

#ifdef BSP_CONFIG_PHONE_TYPE
#include <adrv_om.h>
#else
#include <bsp_rdr.h>
#endif

#define DUMP_WAIT_TASK_PRI (96)
#define DUMP_MAX_MODULE_LEN (16)
#define DUMP_MAX_DESC_LEN (48)
#define RDR_MODEM_CP_RESET_TIMEOUT (30000)
#define DUMP_CP_RESET_EARLY_LOG_WAIT 0
#define DUMP_SAVE_FLAG "CP_DONE"
#define DUMP_CP_REST_TIME_COUNT 8
#define DUMP_CP_REST_TIME_SLICE (30 * 0x8000)
#define RDR_MODEM_CP_RESET_TIMEOUT (30000)
#define DUMP_LOG_YEAR_START 1900


typedef enum {
    DUMP_RESET_NOT_SUPPORT = 0,
    DUMP_RESET_NOT_SUCCES = 1,
    DUMP_RESET_SUCCES = 2,
} dump_reset_result_e;

typedef struct {
    u64 reset_time[DUMP_CP_REST_TIME_COUNT];
    u32 count;
} dump_cp_reset_s;

typedef struct {
    struct semaphore sem_dump_task;
    bool init_done;
    dump_cp_reset_s reset_info;
    u32 modem_off;
} dump_core_ctrl_s;

void dump_set_ap_exc(void);
void dump_get_ap_exc_node(struct rdr_exception_info_s *exc_node, u32 modid);
void dump_cp_agent_init(void);
void dump_create_save_done_file(const char *dir);
#ifdef ENABLE_BUILD_DUMP_MDM_LPM3
void dump_m3_agent_init(void);
#else
static inline s32 dump_m3_agent_init(void)
{
    return BSP_OK;
}
#endif


#endif
