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

#include "ipcmsg_platform.h"
#include "ipcmsg_core.h"
#include "ipcmsg_device.h"
#include "ipcmsg_driver.h"
#include "ipc_interrupt.h"
#include <bsp_reset.h>
#include <bsp_sec_call.h>

u32 g_dbg_print_ipcmsg = 0;
u32 g_modem_reset_send_flag = 0;
u32 g_modem_reset_recv_flag = 0;

msg_send_mode_t g_ipcmsg_send_table[] = {
    {DATA_WITH_HEADER, NULL},      /* 非透传发送 */
    {DATA_PASS_THROUGH, NULL},         /* 透传发送 */
    {FAST_IPC_IRQ, ipcmsg_send_fastipc},                  /* 快速ipc中断 */
};

s32 bsp_ipcmsg_send(ipcmsg_channel_t *channel, void *buf, u32 length)
{
    s32 ret;
    u32 mbx_type, reset_flag;
    reset_flag = get_modem_send_reset_flag();
    if (channel == NULL || channel->mbx == NULL || channel->magic != IPC_MSG_CHN_MAGIC) {
        ipcmsg_print_err("bsp_ipcmsg_send: channel is illegal!\n");
        return IPCMSG_ERROR;
    }
    if (channel->chn_state == CHANNEL_CLOSE) {
        ipcmsg_print_err("bsp_ipcmsg_send: channel is not ready!\n");
        return IPCMSG_ERROR;
    }
    if (buf == NULL || length > IPCMSG_LEN_MAX || length == 0) {
        ipcmsg_print_err("bsp_ipcmsg_send: chn_id :0x%x, buf or length is illegal, len = 0x%x!\n", channel->chn_id,
            length);
        return IPCMSG_ERROR;
    }
    channel->msg_len = length;
    mbx_type = channel->mbx->type;
    if (reset_flag && channel->mbx->cp_reset == RESET_FLAG_ACORE2MODEM) {
        ipcmsg_print_err("modem resetting, disable send ipcmsg! mbxid = %d\n", channel->mbx->mbx_id);
        return IPC_ERR_MODEM_RESETING;
    }
    if (mbx_type >= IPCMSG_MODE_END) {
        ipcmsg_print_err("bsp_ipcmsg_send: mbx_type is illegal!\n");
        return IPCMSG_ERROR;
    }
    if (g_ipcmsg_send_table[mbx_type].send_cb == NULL) {
        ipcmsg_print_err("ipcmsg_send cb is NULL!\n");
        return IPCMSG_ERROR;
    }
    ret = g_ipcmsg_send_table[mbx_type].send_cb(channel, buf, length);
    return ret;
}

ipcmsg_channel_t *bsp_ipcmsg_open(ipcmsg_chn_id_e chn_id, ipcmsg_cb_func routine, void *context)
{
    ipcmsg_channel_t *channel = NULL;
    channel = ipcmsg_channel_get(chn_id);
    if (channel == NULL) {
        ipcmsg_print_err("[channel_id]: 0x%x: channel can't be found in DTS!\n", chn_id);
        return NULL;
    }
    channel->mbx->src_cpu = channel->chn_cfg.chn_info.src_cpu_id;
    channel->func_cb.ipcmsg_cb = routine;
    channel->func_cb.context = context;
    channel->chn_state = CHANNEL_OPEN;
    return channel;
}

void bsp_ipc_modem_reset(drv_reset_cb_moment_e stage, int userdata)
{
    int ret;
    FUNC_CMD_ID cmd = FUNC_MDRV_IPCMSG_RESET;
    if (stage == MDRV_RESET_CB_BEFORE) {
        g_modem_reset_send_flag = 1;
        ret = bsp_sec_call(cmd, IPCMSG_RESET_BEFORE_CMD);
        if (ret) {
            ipcmsg_print_err("ipcmsg seccall err!\n");
            return;
        }
    }
    if (stage == MDRV_RESET_RESETTING) {
        g_modem_reset_send_flag = 0;
        ipcmsg_resetting_handler();
        ret = bsp_sec_call(cmd, IPCMSG_RESET_ING_CMD);
        if (ret) {
            ipcmsg_print_err("ipcmsg seccall err!\n");
            return;
        }
    }
    return;
}

void bsp_ccore_ipc_disable(void)
{
    g_modem_reset_recv_flag = 1;
    return;
}

void bsp_ccore_ipc_enable(void)
{
    g_modem_reset_recv_flag = 0;
    return;
}

u32 get_modem_send_reset_flag(void)
{
    return g_modem_reset_send_flag;
}

u32 get_modem_recv_reset_flag(void)
{
    return g_modem_reset_recv_flag;
}

EXPORT_SYMBOL(bsp_ipc_modem_reset);
EXPORT_SYMBOL(bsp_ccore_ipc_enable);
EXPORT_SYMBOL(bsp_ccore_ipc_disable);
