/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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
#include <bsp_print.h>
#include <bsp_reset.h>
#include <bsp_dump.h>
#include <nva_stru.h>
#include "reset_debug.h"

#undef THIS_MODU
#define THIS_MODU mod_reset
#define reset_print_err(fmt, ...) (bsp_err("[%s] " fmt, __FUNCTION__, ##__VA_ARGS__))

reset_dump_ctrl_s g_reset_dump_ctrl;

void reinit_main_stage(void)
{
    g_reset_dump_ctrl.acore_dump_info.main_stage = 0;
}

u32 get_updated_main_stage(void)
{
    return g_reset_dump_ctrl.acore_dump_info.main_stage++;
}

void reset_reinit_acore_dump(void)
{
    int ret;
    int tmp_size = RESET_ACORE_DUMP_SIZE - sizeof(acore_file_head_s);
    acore_dump_cfg_s *dump_info = &(g_reset_dump_ctrl.acore_dump_info);

    dump_info->file_head->record_num = 0;
    ret = memset_s((void *)(dump_info->base_addr + sizeof(acore_file_head_s)),
        tmp_size, 0, tmp_size);
    if (ret) {
        reset_print_err("acore dump reinit error %d\n", ret);
        return;
    }
    return;
}

static void reset_dump(reset_stage_e stage, record_type_e record_type, int time_stamp)
{
    static int flag = 0;
    acore_dump_cfg_s *dump_info = &(g_reset_dump_ctrl.acore_dump_info);
    acore_dump_record_s *dump_addr = NULL;

    if (dump_info->dump_status != (u32)MAGIC_ACORE_DUMP_HEAD) {
        flag = 1;
        if (flag != 0) {
            reset_print_err("reset dump fail, reset dump init abnormal!\n");
        }
        return;
    }

    if (dump_info->file_head->record_num >= dump_info->record_num_supported) {
        dump_info->file_head->record_num = 0;
    }
    dump_addr = dump_info->dump_addr + dump_info->file_head->record_num;
    dump_addr->record_info.val = GET_ACORE_RECORD(record_type, stage);

    dump_addr->time_stamp = time_stamp;
    dump_info->file_head->record_num++;
}

void reset_timestamp_dump(reset_stage_e stage, record_type_e record_type)
{
    reset_dump(stage, record_type, bsp_get_slice_value());
}

static void reset_acore_dump_init(acore_dump_cfg_s *dump_info)
{
    int ret;
    dump_info->base_addr = (uintptr_t)bsp_dump_register_field(DUMP_MODEMAP_CPRESET_ACORE,
        "MDM_RESET_ACORE", RESET_ACORE_DUMP_SIZE, DUMP_PARSE_SCRIPT_VERSION);
    if ((void *)dump_info->base_addr == NULL) {
        reset_print_err("get acore dump addr error, reset init continue.\n");
    } else {
        ret = memset_s((void *)dump_info->base_addr, RESET_ACORE_DUMP_SIZE, 0, RESET_ACORE_DUMP_SIZE);
        if (ret) {
            reset_print_err("acore dump memset error %d\n", ret);
            return;
        }
        dump_info->file_head = (acore_file_head_s *)(dump_info->base_addr);
        dump_info->file_head->magic = MAGIC_ACORE_DUMP_HEAD;
        dump_info->file_head->dump_size = RESET_ACORE_DUMP_SIZE;
        dump_info->file_head->version = DUMP_PARSE_SCRIPT_VERSION;
        dump_info->file_head->record_num = 0;
        dump_info->dump_addr = (acore_dump_record_s *)(dump_info->base_addr + sizeof(acore_file_head_s));
        dump_info->main_stage = 0;
        dump_info->record_num_supported =
            (RESET_ACORE_DUMP_SIZE - sizeof(acore_file_head_s)) / sizeof(acore_dump_record_s);
        dump_info->dump_status = MAGIC_ACORE_DUMP_HEAD;
    }
}

static void reset_ccore_dump_init(ccore_dump_cfg_s *dump_info)
{
    int ret;
    dump_info->base_addr = (uintptr_t)bsp_dump_register_field(DUMP_MODEMAP_CPRESET_CCORE,
        "MDM_RESET_CCORE", RESET_CCORE_DUMP_SIZE, DUMP_PARSE_SCRIPT_VERSION);
    if ((void *)dump_info->base_addr == NULL) {
        reset_print_err("get ccore dump addr error, reset init continue.\n");
    } else {
        ret = memset_s((void *)dump_info->base_addr, RESET_CCORE_DUMP_SIZE, 0, RESET_CCORE_DUMP_SIZE);
        if (ret) {
            reset_print_err("ccore dump memset error %d\n", ret);
            return;
        }
        dump_info->file_head = (ccore_file_head_s *)(dump_info->base_addr);
        dump_info->file_head->magic = MAGIC_CCORE_DUMP_HEAD;
        dump_info->file_head->dump_size = RESET_CCORE_DUMP_SIZE;
        dump_info->file_head->version = DUMP_PARSE_SCRIPT_VERSION;
    }
}

static void reset_mcore_dump_init(mcore_dump_cfg_s *dump_info)
{
    int ret;
    dump_info->base_addr = (uintptr_t)bsp_dump_register_field(DUMP_MODEMAP_CPRESET_MCORE,
        "MDM_RESET_MCORE", RESET_MCORE_DUMP_SIZE, DUMP_PARSE_SCRIPT_VERSION);
    if ((void *)dump_info->base_addr == NULL) {
        reset_print_err("get mcore dump addr error, reset init continue.\n");
    } else {
        ret = memset_s((void *)dump_info->base_addr, RESET_MCORE_DUMP_SIZE, 0, RESET_MCORE_DUMP_SIZE);
        if (ret) {
            reset_print_err("mcore dump memset error %d\n", ret);
            return;
        }
        dump_info->file_head = (mcore_file_head_s *)(dump_info->base_addr);
        dump_info->file_head->magic = MAGIC_MCORE_DUMP_HEAD;
        dump_info->file_head->dump_size = RESET_MCORE_DUMP_SIZE;
        dump_info->file_head->version = DUMP_PARSE_SCRIPT_VERSION;
        dump_info->file_head->record_num = 0;
    }
}

int reset_dump_ctrl_init(void)
{
    int ret;

    // 全局变量初始化
    reset_dump_ctrl_s *reset_dump_ctrl = &g_reset_dump_ctrl;
    ret = memset_s(reset_dump_ctrl, sizeof(reset_dump_ctrl_s), 0, sizeof(reset_dump_ctrl_s));
    if (ret) {
        reset_print_err("memset error %d\n", ret);
        return RESET_ERROR;
    }

    reset_acore_dump_init(&(reset_dump_ctrl->acore_dump_info));
    reset_ccore_dump_init(&(reset_dump_ctrl->ccore_dump_info));
    reset_mcore_dump_init(&(reset_dump_ctrl->mcore_dump_info));
    return RESET_OK;
}

void ccore_head_parse(ccore_dump_cfg_s *dump_info, u32 *dump_size_used)
{
    *dump_size_used += sizeof(ccore_file_head_s);
    if (*dump_size_used > dump_info->file_head->dump_size) {
        bsp_err("ccore file head addr flows out!\n");
        return;
    }
    bsp_err("[ccore head]\n");
    bsp_err("magic :0x%x\n", dump_info->file_head->magic);
    bsp_err("host_num :%u\n", dump_info->file_head->host_num);
    bsp_err("dump_size :%u\n", dump_info->file_head->dump_size);
    bsp_err("parse version :%u\n", dump_info->file_head->version);

}

void ccore_frame_data_parse(ccore_dump_cfg_s *dump_info, u32 *dump_size_used)
{

    reset_data_head_s *data_head = NULL;
    reset_frame_record_s *frame_record = NULL;
    int len = 0;
    int i;
    int cnt = 0;

    data_head = (reset_data_head_s *)(dump_info->base_addr + *dump_size_used);
    (*dump_size_used) = (*dump_size_used) + data_head->data_len;
    if (*dump_size_used > dump_info->file_head->dump_size) {
        bsp_err("ccore frame data addr flows out!\n");
        return;
    }
    if (data_head->data_type != MAGIC_32_FRAME_DUMP_TYPE) {
        bsp_err("ccore frame data head type not match!\n");
        return;
    }

    frame_record = (reset_frame_record_s *)((uintptr_t)data_head + sizeof(reset_data_head_s));

    len = (data_head->data_len - sizeof(reset_data_head_s)) / sizeof(reset_frame_record_s);
    bsp_err("[Frame dump begin]!\n");
    bsp_err("data_type: 0x%x data_len: %d len: %d!\n", data_head->data_type, data_head->data_len, len);

    for (i = 0; i < len; i++) {
        if ((u32)(frame_record->record_info.bit_data.magic_num) == MAGIC_16_FRAME_GENERAL_THREAD) {
            bsp_err("%d thread %d %s: 0x%x\n", cnt, (u32)(frame_record->record_info.bit_data.thread_id),
                frame_record->name, frame_record->timestamp);
            cnt++;
        }
        frame_record += 1;
    }
    bsp_err("[Frame dump end]!\n");
    return;
}

int ccore_single_host_dump_parse(ccore_dump_cfg_s *dump_info, u32 *dump_size_used)
{
    reset_data_head_s *data_head = NULL;
    reset_host_data_head_s *record_head = NULL;
    reset_host_record_s *record_data = NULL;
    u32 dump_len;
    int record_num = 0;
    int i;

    data_head = (reset_data_head_s *)(dump_info->base_addr + *dump_size_used);
    (*dump_size_used) = (*dump_size_used) + data_head->data_len;
    if (*dump_size_used > dump_info->file_head->dump_size) {
        bsp_err("host data size flows out!\n");
        return RESET_ERROR;
    }

    if (data_head->data_type != MAGIC_32_HOST_DUMP_TYPE) {
        bsp_err("host data type error!\n");
        return RESET_ERROR;
    }

    record_head = (reset_host_data_head_s *)((uintptr_t)data_head + sizeof(reset_data_head_s));
    if (record_head->is_registered != CP_HOST_IS_REGISTERED) {
        bsp_err("%d %s not registered!\n", record_head->prior, record_head->name);
        return RESET_OK;
    }
    record_num = record_head->record_data_len / sizeof(reset_host_record_s);
    bsp_err("%d %s registered, record_num: %d!\n", record_head->prior, record_head->name, record_num);
    record_data = (reset_host_record_s *)((uintptr_t)record_head
         + sizeof(reset_host_data_head_s));
    dump_len = data_head->data_len - sizeof(reset_data_head_s)
        - sizeof(reset_host_data_head_s) - record_head->record_data_len;
    if (dump_len != 0) {
        dump_len = ((uintptr_t)record_data + record_head->record_data_len) - dump_info->base_addr;
        bsp_err("dump offset: 0x%x\n", dump_len);
    }

    for (i = 0; i < record_num; i++) {
        if ((u32)((record_data->record_info >> 16) & 0xffff) == (u32)MAGIC_16_HOST_RECORD_START) {
            bsp_err("host idle in: 0x%x\n", record_data->timestamp);
        }

        if ((u32)((record_data->record_info >> 16) & 0xffff) == (u32)MAGIC_16_HOST_RECORD) {
            bsp_err("step[%d]: 0x%x, val: 0x%x\n", (u32)(record_data->record_info & 0xff),
                record_data->timestamp, record_data->val);
        }

        if ((u32)((record_data->record_info >> 16) & 0xffff) == (u32)MAGIC_16_HOST_RECORD_END) {
            bsp_err("host idle out: 0x%x\n", record_data->timestamp);
        }
        record_data += 1;
    }
    return RESET_OK;
}

void ccore_host_data_parse(ccore_dump_cfg_s *dump_info, u32 *dump_size_used)
{
    int host_num = dump_info->file_head->host_num;
    int i = 0;

    bsp_err("[cp host dump begin]\n");
    for (i = 0; i < host_num; i++) {
        if (ccore_single_host_dump_parse(dump_info, dump_size_used) != RESET_OK) {
            bsp_err("cp host %d dump parse fail!\n", i);
            break;
        }
    }
    bsp_err("[cp host dump end]\n");
}

// C核dump解析
void reset_cp_dump_parse(void)
{
    ccore_dump_cfg_s *dump_info = &(g_reset_dump_ctrl.ccore_dump_info);
    u32 dump_size_used = 0;

    if ((void *)dump_info->base_addr == NULL) {
        reset_print_err("cp dump parse base addr is NULL!\n");
        return;
    }

    reset_print_err("reset cp dump parse start!\n");
    ccore_head_parse(dump_info, &dump_size_used);
    ccore_frame_data_parse(dump_info, &dump_size_used);
    ccore_host_data_parse(dump_info, &dump_size_used);
    reset_print_err("cp mem left 0x%x!\n", dump_info->file_head->dump_size - dump_size_used);
    reset_print_err("reset cp dump parse end!\n");
}

void reset_dump_clear_ccore_mem(void)
{
    ccore_dump_cfg_s *dump_info = &(g_reset_dump_ctrl.ccore_dump_info);
    int ret;
    void *dest_addr = NULL;
    unsigned int dest_size;

    if ((void *)dump_info->base_addr == NULL) {
        reset_print_err("cp dump parse base addr is NULL!\n");
        return;
    }

    dest_addr = (void *)(dump_info->base_addr
        + sizeof(ccore_file_head_s) + sizeof(reset_data_head_s));
    dest_size = dump_info->file_head->dump_size
        - sizeof(ccore_file_head_s) - sizeof(reset_data_head_s);
    ret = memset_s(dest_addr, dest_size, 0, dest_size);
    if (ret) {
        reset_print_err("memset fail ret:%d!\n", ret);
    }
}
