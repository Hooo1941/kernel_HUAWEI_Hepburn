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

#include <bsp_slice.h>
#include <bsp_ipc_fusion.h>
#include "ipc_fusion.h"

#define THIS_MODU mod_ipc

ipc_handle_t g_enable_handle;
ipc_handle_t g_send_handle;

static int ipc_test_report(int ret)
{
    if (ret) {
        ipc_print_err("ok!\n");
        return IPC_OK;
    }
    ipc_print_err("fail\n");
    return IPC_ERR;
}

void ipc_print(void *arg)
{
    ipc_handle_t handle = *(ipc_handle_t *)arg;
    if (NULL == handle) {
        ipc_print_err("ok!\n");
        return;
    }
    ipc_print_err("dev_id:%d, chn_id:%d, res_id:%d irq function ok!\n", handle->dev_id, handle->chn_id, handle->res_bit);
}

int ipc_open_enable_testcase(ipc_res_id_e ipc_res_id)
{
    s32 ret;

    ret = bsp_ipc_open(ipc_res_id, &g_enable_handle);
    if (ret) {
        ipc_print_err("ipc_open_enable_testcase fail!\n");
        return IPC_ERR;
    }
    ipc_print_err("enable_handle: %d, %d, %d\n", g_enable_handle->dev_id, g_enable_handle->chn_id, g_enable_handle->res_bit);

    return (ipc_test_report(ret == 0));
}

int ipc_open_send_testcase(ipc_res_id_e ipc_res_id)
{
    s32 ret;

    ret = bsp_ipc_open(ipc_res_id, &g_send_handle);
    if (ret) {
        ipc_print_err("ipc_open_send_testcase fail!\n");
        return IPC_ERR;
    }
    ipc_print_err("send_handle: %d, %d, %d\n", g_send_handle->dev_id, g_send_handle->chn_id, g_send_handle->res_bit);

    return (ipc_test_report(ret == 0));
}

int ipc_connect_testcase(void)
{
    s32 ret;
    u32 begin_stamp, end_stamp;

    begin_stamp = bsp_get_slice_value_hrt();
    ret = bsp_ipc_connect(g_enable_handle, ipc_print, (void *)&g_enable_handle);
    end_stamp = bsp_get_slice_value_hrt();
    ipc_print_err("ipc_connect_testcase send_times begin_stamp %d, end_stamp:%d time:%d!\n", begin_stamp, end_stamp,
        end_stamp - begin_stamp);

    return (ipc_test_report(ret == 0));
}

int ipc_disconnect_testcase(void)
{
    s32 ret;
    ret = bsp_ipc_disconnect(g_enable_handle);
    return (ipc_test_report(ret == 0));
}

int ipc_send_testcase(void)
{
    s32 ret;
    u32 begin_stamp, end_stamp;

    begin_stamp = bsp_get_slice_value_hrt();
    ret = bsp_ipc_send(g_send_handle);
    end_stamp = bsp_get_slice_value_hrt();
    ipc_print_err("ipc_send_testcase begin_stamp %d, end_stamp:%d time:%d!\n", begin_stamp, end_stamp, end_stamp - begin_stamp);
    return (ipc_test_report(ret == 0));
}

void ipc_testcase(ipc_res_id_e ipc_res_id)
{
    ipc_open_enable_testcase(ipc_res_id);
    ipc_connect_testcase();
    ipc_open_send_testcase(ipc_res_id);
    ipc_send_testcase();
    return;
}

