/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2023. All rights reserved.
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
#include <linux/module.h>
#include <msg_id.h>
#include <bsp_diag_cmdid.h>
#include <bsp_print.h>
#include "diag_agent.h"
#undef THIS_MODU
#define THIS_MODU mod_coresight
struct diag_dpa_onoff g_diag_agent_onoff;
static int diag_dpa_msg_send(u32 type, u32 onoff)
{
    struct msg_addr dst;
    struct diag_msg_s data;
    dst.core = MSG_CORE_DPA;
    dst.chnid = MSG_CHN_DIAG_DPA;
    data.type = type;
    data.onoff = onoff;
    return bsp_msg_lite_sendto(g_diag_agent_onoff.msghdl, &dst, &data, sizeof(struct diag_msg_s));
}
void bsp_diag_dpa_set_translog_onoff(u32 transonoff)
{
    int ret;
    if (g_diag_agent_onoff.init_flag != DIAG_DPA_INIT_OK) {
        return;
    }
    ret = diag_dpa_msg_send(DIAG_DPA_TRANSONOFF, transonoff);
    g_diag_agent_onoff.transonoff = transonoff;
    if (ret) {
        bsp_err("set translog err %d\n", ret);
    }
}

void bsp_diag_dpa_set_onoff(u32 onoff)
{
    int ret;
    if (g_diag_agent_onoff.init_flag != DIAG_DPA_INIT_OK) {
        return;
    }
    ret = diag_dpa_msg_send(DIAG_DPA_CONNECT, onoff);
    g_diag_agent_onoff.connect = onoff;
    if (ret) {
        bsp_err("set onoff err %d\n", ret);
    }
}
int bsp_diag_dpa_msg_init(void)
{
    s32 ret;
    struct msgchn_attr lite_attr = { 0 };
    bsp_msgchn_attr_init(&lite_attr);
    lite_attr.chnid = MSG_CHN_DIAG_DPA;
    lite_attr.coremask = MSG_CORE_MASK(MSG_CORE_DPA);
    ret = bsp_msg_lite_open(&g_diag_agent_onoff.msghdl, &lite_attr);
    if (ret) {
        g_diag_agent_onoff.init_flag = BSP_ERROR;
        return BSP_ERROR;
    }
    g_diag_agent_onoff.init_flag = DIAG_DPA_INIT_OK;
    return 0;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
late_initcall(bsp_diag_dpa_msg_init);
#endif