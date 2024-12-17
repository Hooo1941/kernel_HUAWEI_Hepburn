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
#include "efuse_dump.h"

#include <bsp_slice.h>
#include <bsp_dump.h>
#include <bsp_print.h>
#include <securec.h>

#define THIS_MODU mod_efuse

efuse_dump_info g_efuse_debug_info = {0};

int efuse_debug_init(void)
{
    g_efuse_debug_info.base_addr = (efuse_event_dump_t *)bsp_dump_register_field(DUMP_MDMAP_EFUSE,
        "efuse_acore", EFUSE_DUMP_SIZE, 0);
    if (g_efuse_debug_info.base_addr == NULL) {
            bsp_err("get efuse dump fail!\n");
            return -1;
    }

    (void)memset_s(g_efuse_debug_info.base_addr, EFUSE_DUMP_SIZE, 0, EFUSE_DUMP_SIZE);

    g_efuse_debug_info.efuse_record_flag = EFUSE_RECORD_START;
    return 0;
}

void efuse_dump_rec(efuse_debug_event_type_e event)
{
    u32 dump_cur = 0;
    if (g_efuse_debug_info.base_addr == NULL || g_efuse_debug_info.efuse_record_flag != EFUSE_RECORD_START ||
        event >= EFUSE_EVENT_MAX) {
        return;
    }

    if (g_efuse_debug_info.cur >= (EFUSE_DUMP_SIZE / sizeof(efuse_event_dump_t))) {
        g_efuse_debug_info.cur = 0;
    }
    dump_cur = g_efuse_debug_info.cur;

    g_efuse_debug_info.base_addr[dump_cur].index = g_efuse_debug_info.dump_index;
    g_efuse_debug_info.base_addr[dump_cur].event = event;
    g_efuse_debug_info.base_addr[dump_cur].info = bsp_get_slice_value();

    g_efuse_debug_info.cur = (dump_cur + 1) % (EFUSE_DUMP_SIZE / sizeof(efuse_event_dump_t));
    g_efuse_debug_info.dump_index++;

    return;
}