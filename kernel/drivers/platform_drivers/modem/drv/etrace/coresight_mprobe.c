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
#include <linux/module.h>
#include <linux/workqueue.h>
#include <osl_types.h>
#include <osl_malloc.h>
#include <bsp_mprobe.h>
#include <bsp_print.h>
#include "coresight_trace_mgr.h"
#include "coresight_mprobe.h"
#undef THIS_MODU
#define THIS_MODU mod_coresight
struct work_struct g_cs_mprobe_work;
void coresight_mprobe_send(struct work_struct *work)
{
    int i, cnt;
    u32 left;
    mprobe_msg_report_head_s report_info;
    mprobe_srv_head_s head = { 0 };
    struct coresight_trace_s trace_info = coresight_get_trace_info();
    u8 *data = (u8 *)(uintptr_t)trace_info.virt_ddr_buffer;
    u32 size = trace_info.ddr_buffer_len;
    if (data == NULL || size == 0) {
        return;
    }
    cnt = size / CORESIGHT_MPROBE_MAX_PACKAGE;
    left = size % CORESIGHT_MPROBE_MAX_PACKAGE;
    bsp_mprobe_fill_srv_head(&head);
    head.header.magic = MPROBE_ODT_HEAD_MAGIC;
    head.frame_header.msg_id.comp_8b = MPROBE_COMPID_TRACE;
    report_info.header = &head;
    report_info.header_size = sizeof(mprobe_srv_head_s);
    head.header.data_len = CORESIGHT_MPROBE_MAX_PACKAGE + sizeof(mprobe_srv_head_s) - MPROBE_ODT_HEAD_SIZE;
    head.frame_header.msg_len.msg_len = CORESIGHT_MPROBE_MAX_PACKAGE;
    report_info.data_size = CORESIGHT_MPROBE_MAX_PACKAGE;
    head.frame_header.msg_id.msgid_16b = 0x1;
    for (i = 0; i < cnt; i++) {
        report_info.data = data;
        data = data + CORESIGHT_MPROBE_MAX_PACKAGE;
        (void)bsp_mprobe_data_report(&report_info);
    }
    head.header.data_len = left + sizeof(mprobe_srv_head_s) - MPROBE_ODT_HEAD_SIZE;
    head.frame_header.msg_len.msg_len = left;
    report_info.data = data;
    report_info.data_size = left;
    head.frame_header.msg_id.msgid_16b = 0x2;
    (void)bsp_mprobe_data_report(&report_info);
    bsp_err("save complete\n");
}
static int coresight_func_reg(void *data, u32 len)
{
    u32 ddr_size = 0;
    int i;
    mprobe_frame_head_s *head = (mprobe_frame_head_s *)data;
    if (head == NULL) {
        return BSP_ERROR;
    }
    if (head->msg_id.msgid_16b == 0) {
        if (head->msg_len.msg_len == sizeof(ddr_size)) {
            for (i = 0; i < head->msg_len.msg_len; i++) {
                ddr_size = ddr_size * 0x100 + head->data[i];
            }
            return coresight_mprob_set(ddr_size);
        }
    } else {
        schedule_work(&g_cs_mprobe_work);
    }
    return BSP_OK;
}
void coresight_mprobe_init(void)
{
    INIT_WORK(&g_cs_mprobe_work, coresight_mprobe_send);
    bsp_mprobe_func_reg(MPROBE_COMPID_TRACE, coresight_func_reg);
}