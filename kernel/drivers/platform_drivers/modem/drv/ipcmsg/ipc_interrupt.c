/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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
#include "ipc_interrupt.h"
#include "ipc_resource.h"
#include "ipcmsg_device.h"
#include "ipcmsg_core.h"

struct ipc_handle *__ipc_handle_get(ipc_res_id_e ipc_res_id)
{
    u32 i, j, k;
    u8 func_id;
    struct ipc_handle *phandle = NULL;
    ipcmsg_root_dts_t *ipcmsg_root = ipcmsg_of_find_root_node();
    ipcmsg_device_dts_t *ipcmsg_device = ipcmsg_root->ipcmsg_device_dts;
    for (i = 0; i < ipcmsg_root->device_cnt; i++) {
        for (j = 0; j < ipcmsg_device[i].channel_cnt; j++) {
            for (k = 0; k < ipcmsg_device[i].ipcmsg_channel_dts[j].ipc_res_num; k++) {
                phandle = &ipcmsg_device[i].ipcmsg_channel_dts[j].ipc_res[k];
                if (ipc_res_id == phandle->res_id) {
                    func_id = GET_FUNC_ID(ipcmsg_device[i].ipcmsg_channel_dts[j].chn_cfg);
                    phandle->channel = (void *)ipcmsg_get_channel(func_id);
                    phandle->channel_dts = (void *)(&ipcmsg_device[i].ipcmsg_channel_dts[j]);
                    phandle->magic = IPC_MAGIC;
                    return phandle;
                }
            }
        }
    }
    return NULL;
}

/* ****************************************************************************
 * 函 数 名: bsp_ipc_send
 *
 * 功能描述  :   IPC通道发送
 *
 * 返 回 值: IPC通道发送
 * *************************************************************************** */
s32 bsp_ipc_send(struct ipc_handle *phandle)
{
    s32 ret;
    if (phandle == NULL || phandle->magic != IPC_MAGIC) {
        ipcmsg_print_err("ipc connect err!\n");
        return IPCMSG_ERROR;
    }
    /* ipcmsg send */
    ret = bsp_ipcmsg_send((ipcmsg_channel_t *)(phandle->channel), (void *)(&phandle->reg_idx), sizeof(phandle->reg_idx));
    if (ret) {
        ipcmsg_print_err("bsp_ipc_send failed! ipc_res_id:0x%x\n", phandle->res_id);
        return ret;
    }
    phandle->send_times++;
    return IPCMSG_OK;
}

/* ****************************************************************************
 * 函 数 名: bsp_ipc_send
 *
 * 功能描述  :   IPC通道发送,兼容之前老接口
 *
 * 返 回 值: IPC通道发送
 * *************************************************************************** */
s32 bsp_ipc_bind(ipc_handle_t phandle, u32 core_mask)
{
    return IPCMSG_OK;
}

s32 bsp_ipc_init(void)
{
    return IPCMSG_OK;
}

/* ****************************************************************************
 * 函 数 名: bsp_ipc_open
 *
 * 功能描述  :   打开IPC
 *
 * 返 回 值: 打开是否成功
 * *************************************************************************** */
s32 bsp_ipc_open(ipc_res_id_e ipc_res_id, struct ipc_handle **pphandle)
{
    struct ipc_handle *phandle = NULL;
    ipcmsg_channel_t *channel = NULL;
    if (pphandle == NULL) {
        return IPCMSG_ERROR;
    }
    phandle = __ipc_handle_get(ipc_res_id);
    if (phandle == NULL || phandle->channel == NULL) {
        ipcmsg_print_err("open ipc res[0x%x] failed handle NULL!\n", ipc_res_id);
        return IPCMSG_ERROR;
    }
    channel = (ipcmsg_channel_t *)phandle->channel;
    channel->mbx->src_cpu = channel->chn_cfg.chn_info.src_cpu_id;
    channel->func_cb.ipcmsg_cb = NULL;
    channel->func_cb.context = phandle->channel_dts;
    channel->chn_state = CHANNEL_OPEN;
    *pphandle = phandle;
    return IPCMSG_OK;
}

/* ****************************************************************************
 * 函 数 名: bsp_ipc_connect
 *
 * 功能描述  :   打开IPC
 *
 * 返 回 值: 打开是否成功
 * *************************************************************************** */
s32 bsp_ipc_connect(ipc_handle_t phandle, ipc_cbk routine, void *parameter)
{
    if (phandle == NULL || phandle->magic != IPC_MAGIC) {
        ipcmsg_print_err("ipc connect err!\n");
        return IPCMSG_ERROR;
    }
    phandle->ipc_int_table.routine = routine;
    phandle->ipc_int_table.arg = parameter;
    return IPCMSG_OK;
}

/* ****************************************************************************
 * 函 数 名: bsp_ipc_disconnect
 *
 * 功能描述  :   打开IPC
 *
 * 返 回 值: 打开是否成功
 * *************************************************************************** */
s32 bsp_ipc_disconnect(ipc_handle_t phandle)
{
    if (phandle == NULL || phandle->magic != IPC_MAGIC) {
        ipcmsg_print_err("ipc connect err!\n");
        return IPCMSG_ERROR;
    }
    phandle->ipc_int_table.routine = NULL;
    phandle->ipc_int_table.arg = NULL;
    return IPCMSG_OK;
}

EXPORT_SYMBOL(bsp_ipc_open);
EXPORT_SYMBOL(bsp_ipc_connect);
EXPORT_SYMBOL(bsp_ipc_disconnect);
EXPORT_SYMBOL(bsp_ipc_send);
EXPORT_SYMBOL(bsp_ipc_init);
EXPORT_SYMBOL(bsp_ipc_bind);

