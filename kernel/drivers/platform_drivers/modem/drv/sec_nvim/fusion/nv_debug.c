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

#include <securec.h>
#include <bsp_slice.h>
#include <bsp_dump.h>
#include "nv_comm.h"
#include "nv_debug.h"

static debug_table g_debug_table[NV_DEBUG_BUTT - 1] = {
    { NV_DEBUG_WRITEEX_START, "write nv start" },
    { NV_DEBUG_WRITEEX_GET_IPC_START, "for check crc,start to get ipc sem" },
    { NV_DEBUG_WRITEEX_GET_IPC_END, "get ipc sem end" },
    { NV_DEBUG_WRITEEX_GIVE_IPC, "check crc end, release ipc sem" },
    { NV_DEBUG_WRITEEX_MEM_START, "write to mem start" },
    { NV_DEBUG_WRITEEX_FILE_START, "write to file start" },
    { NV_DEBUG_FLUSH_START, "flush nv list start" },
    { NV_DEBUG_FLUSH_END, "flush nv list end" },
    { NV_DEBUG_REQ_FLUSH_START, "req flush nv list start" },
    { NV_DEBUG_REQ_FLUSH_END, "req flush nv list end" },
    { NV_DEBUG_FLUSHEX_START, "flush nv to file start" },
    { NV_DEBUG_FLUSHEX_OPEN_START, "open nv file start" },
    { NV_DEBUG_FLUSHEX_OPEN_END, "open nv file end" },
    { NV_DEBUG_FLUSHEX_GET_PROTECT_SEM_START, "before write to file get ipc and sem start" },
    { NV_DEBUG_FLUSHEX_GET_PROTECT_SEM_END, "before write to file get ipc and sem end" },
    { NV_DEBUG_FLUSHEX_GIVE_IPC, "write to file release ipc" },
    { NV_DEBUG_FLUSHEX_GIVE_SEM, "release sem" },
    { NV_DEBUG_FLUSHEX_WRITE_FILE_START, "write to nv.bin start" },
    { NV_DEBUG_FLUSHEX_WRITE_FILE_END, "write to nv.bin end" },
    { NV_DEBUG_WRITEEX_END, "write nv end" },
    { NV_DEBUG_RECEIVE_ICC, "receive icc msg from acore" },
    { NV_DEBUG_SEND_ICC, "send icc msg to acore" },
    { NV_DEBUG_READ_ICC, "read icc from ccore" }

};

struct nvm_debug_stru *nv_get_debug_info(void)
{
    struct nvm_info *nvminfo = nv_get_nvminfo();

    return &nvminfo->debug_info;
}

u32 nv_debug_print(void)
{
    struct nvm_debug_stru *debug_info = nv_get_debug_info();

    return debug_info->print;
}

void nv_debug_queuein(debug_queue *queue, debug_element element)
{
    if (queue->num >= queue->maxnum) {
        return;
    }

    queue->data[queue->tail].state = element.state;
    queue->data[queue->tail].slice = element.slice;
    queue->tail = (queue->tail + 1) % (queue->maxnum);
    queue->num++;
}

void nv_debug_queueloopin(debug_queue *queue, debug_element element)
{
    if (queue->num < queue->maxnum) {
        nv_debug_queuein(queue, element);
    } else {
        queue->data[queue->tail].state = element.state;
        queue->data[queue->tail].slice = element.slice;
        queue->tail = (queue->tail + 1) % (queue->maxnum);
        queue->head = (queue->head + 1) % (queue->maxnum);
    }
}

void nv_debug_record(u32 state)
{
    debug_element new_element = { 0 };
    u32 table_index = 0;
    struct nvm_debug_stru *debug_info = nv_get_debug_info();

    if (nv_debug_print()) {
        for (table_index = 0; table_index < NV_DEBUG_BUTT; table_index++) {
            if (debug_info->table[table_index].state == (state & 0xFFFF)) {
                nv_printf("%s\n", debug_info->table[table_index].info);
            }
        }

        switch (state & 0xFFFF) {
            case NV_DEBUG_WRITEEX_START:
            case NV_DEBUG_WRITEEX_END:
                nv_printf("nv id:0x%x\n", (state >> 0x10));
                break;
            case NV_DEBUG_SEND_ICC:
            case NV_DEBUG_READ_ICC:
                nv_printf("msg type:0x%x\n", (state >> 0x10));
                break;
            default:
                break;
        }
    }

    if (debug_info->dump_queue == NULL) {
        return;
    }

    new_element.slice = bsp_get_slice_value();
    new_element.state = state;

    nv_debug_queueloopin(debug_info->dump_queue, new_element);
}

void nv_debug_queueinit(debug_queue *queue, u32 element_num)
{
    queue->maxnum = element_num;
    queue->head = 0;
    queue->tail = 0;
    queue->num = 0;

    (void)memset_s((void *)queue->data, (element_num * sizeof(debug_element)), 0,
        (element_num * sizeof(debug_element)));
}

u32 nv_debug_init(void)
{
    struct nvm_debug_stru *debug_info = nv_get_debug_info();
    debug_info->table = g_debug_table;

    debug_info->dump_queue = (debug_queue *)bsp_dump_register_field(DUMP_MODEMAP_NV, "NVA", NV_DUMP_SIZE, 0x0000);
    if (debug_info->dump_queue == NULL) {
        return NV_OK;
    }

    nv_debug_queueinit(debug_info->dump_queue, (NV_DUMP_SIZE - sizeof(debug_queue)) / sizeof(debug_element));
    nv_printf("nv debug init success\n");
    return NV_OK;
}

void nv_debug(u32 type, u32 reseverd1, u32 reserved2, u32 reserved3, u32 reserved4)
{
    struct nvm_debug_stru *debug_info = nv_get_debug_info();

    if (reseverd1 == 0) {
        debug_info->point[type].callnum++;
    }

    debug_info->point[type].rsv1 = reseverd1;
    debug_info->point[type].rsv2 = reserved2;
    debug_info->point[type].rsv3 = reserved3;
    debug_info->point[type].rsv4 = reserved4;
}

void nv_help(u32 type)
{
    u32 i = type;
    struct nvm_debug_stru *debug_info = nv_get_debug_info();

    nv_printf("fun id %d start:\n", i);
    nv_printf("call num             : 0x%x\n", debug_info->point[i].callnum);
    nv_printf("out branch (reseved1): 0x%x\n", debug_info->point[i].rsv1);
    nv_printf("reseved2             : 0x%x\n", debug_info->point[i].rsv2);
    nv_printf("reseved3             : 0x%x\n", debug_info->point[i].rsv3);
    nv_printf("reseved4             : 0x%x\n", debug_info->point[i].rsv4);
    nv_printf("fun id %d end\n", i);
}
