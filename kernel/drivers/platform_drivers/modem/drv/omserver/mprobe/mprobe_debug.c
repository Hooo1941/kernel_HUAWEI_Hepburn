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
#include <osl_types.h>
#include <osl_malloc.h>
#include "bsp_mprobe.h"
#include "mprobe_port.h"
#include "mprobe_service.h"
#include "mprobe_message.h"
#include "mprobe_channel_manager.h"
#include "mprobe_debug.h"

#undef THIS_MODU
#define THIS_MODU mod_mprobe

#define MPROBE_TEST_DATA_MAX 1000

int mprobe_test_func_reg(void *data, u32 len)
{
    mprobe_debug("enter mprobe_test_func_reg");
    mprobe_show(data, len);
    mprobe_debug("mprobe_test_func_reg end");
    return BSP_OK;
}

void mprobe_set(void)
{
    mprobe_debug("enter mprobe_set");
    bsp_mprobe_func_reg(MPROBE_COMPID_DEFAULT, mprobe_test_func_reg);
    mprobe_debug("mprobe_set ok");
}

int mprobe_send(u32 comp_id, u32 len)
{
    int i = 0;
    mprobe_msg_report_head_s report_info;
    mprobe_srv_head_s head = {0};
    u8 *data;

    data = (u8 *)osl_malloc(MPROBE_TEST_DATA_MAX);
    if (data == NULL) {
        mprobe_error("malloc data err");
        return -1;
    }
    (void)memset_s(data, MPROBE_TEST_DATA_MAX, 0 ,MPROBE_TEST_DATA_MAX);

    for(i = 0; i < MPROBE_TEST_DATA_MAX; i++) {
        data[i] = i + MPROBE_TEST_DATA_MAX;
    }

    head.header.magic = MPROBE_ODT_HEAD_MAGIC;
    head.header.data_len = len + sizeof(mprobe_srv_head_s) - MPROBE_ODT_HEAD_SIZE;

    head.frame_header.srv_head.sid = 0x8;
    head.frame_header.srv_head.ver = 0x1;
    head.frame_header.srv_head.mdmid = 0x2;
    head.frame_header.srv_head.rsv = 0x0;
    head.frame_header.srv_head.ssid = 0x1;
    head.frame_header.srv_head.msgtyp = 0x3;
    head.frame_header.srv_head.sidx = 0x2;
    head.frame_header.srv_head.esflag = 0x1;
    head.frame_header.srv_head.sflag = 0x0;
    head.frame_header.srv_head.srctransid = 0x7d;
    head.frame_header.srv_head.odttransid = 0x7e;
    head.frame_header.srv_head.timestamp[0] = 0x7;

    head.frame_header.msg_id.msgid_16b = 0x1;
    head.frame_header.msg_id.comp_8b = comp_id;
    head.frame_header.msg_id.rsv_8b = 0x3;

    head.frame_header.msg_len.msg_len = len;
    head.frame_header.msg_len.resv = 0x6;

    report_info.header = &head;
    report_info.header_size = sizeof(mprobe_srv_head_s);
    report_info.data = data;
    report_info.data_size = len;

    (void)bsp_mprobe_data_report(&report_info);

    return BSP_OK;
}

void mprobe_show(void *data, u32 len)
{
    int i;
    mprobe_frame_head_s *head = NULL;

    if ((data == NULL) || (len == 0)) {
        mprobe_error("show data null!");
    }

    head = (mprobe_frame_head_s *)data;

    mprobe_error("show data start\n");
    mprobe_error("len:0x%x\n", len);

    mprobe_error("omp_service_head:\n");
    mprobe_error("sid:0x%x\n", head->srv_head.sid);
    mprobe_error("ver:0x%x\n", head->srv_head.ver);
    mprobe_error("mdmid:0x%x\n", head->srv_head.mdmid);
    mprobe_error("rsv:0x%x\n", head->srv_head.rsv);
    mprobe_error("ssid:0x%x\n", head->srv_head.ssid);
    mprobe_error("msgtyp:0x%x\n", head->srv_head.msgtyp);
    mprobe_error("sidx:0x%x\n", head->srv_head.sidx);
    mprobe_error("esflag:0x%x\n", head->srv_head.esflag);
    mprobe_error("sflag:0x%x\n", head->srv_head.sflag);
    mprobe_error("srctransid:0x%x\n", head->srv_head.srctransid);
    mprobe_error("odttransid:0x%x\n", head->srv_head.odttransid);
    mprobe_error("timestamp:0x%x\n", head->srv_head.timestamp[0]);

    mprobe_error("mprobe_msg_id_s:\n");
    mprobe_error("msg_id:0x%x\n", head->msg_id.msgid_16b);
    mprobe_error("comp_id_id:0x%x\n", head->msg_id.comp_8b);
    mprobe_error("rsv_8b:0x%x\n", head->msg_id.rsv_8b);

    mprobe_error("mprobe_msg_len_s:\n");
    mprobe_error("msg_len:0x%x\n", head->msg_len.msg_len);
    mprobe_error("resv:0x%x\n", head->msg_len.resv);

    for (i = 0; i < head->msg_len.msg_len; i++) {
        mprobe_error("data_u8: 0x%x ", head->data[i]);
    }

    mprobe_error("show data end");
}

EXPORT_SYMBOL(mprobe_set);
EXPORT_SYMBOL(mprobe_send);