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
 */
#include <msg_id.h>
#include <bsp_print.h>
#include "sysview_comm.h"
#include "sysview_msg.h"
#undef THIS_MODU
#define THIS_MODU mod_coresight
struct sysview_msg_ctrl g_sysview_msg_ctrl;
static int sysview_msg_proc(const struct msg_addr *src, void *buf, u32 len)
{
    if (src == NULL || buf == NULL) {
        return BSP_ERROR;
    }
    switch (src->core) {
        case MSG_CORE_TSP:
            return sysview_hids_deal_tsp(buf, len);
        case MSG_CORE_LPM:
            return sysview_mprobe_deal_lpm(buf, len);
        default:
            break;
    }
    return BSP_ERROR;
}
int sysview_msg_sendtolpm(void *data, u32 size)
{
    struct msg_addr dst;
    if (data == NULL || size > SYSVIEW_MSG_MAX_SIZE || g_sysview_msg_ctrl.init_flag != SYSVIEW_INIT_OK) {
        return BSP_ERROR;
    }
    dst.core = MSG_CORE_LPM;
    dst.chnid = MSG_CHN_SYSVIEW;
    return bsp_msg_lite_sendto(g_sysview_msg_ctrl.msghdl, &dst, data, size);
}
void sysview_msg_init(void)
{
    s32 ret;
    struct msgchn_attr lite_attr = { 0 };
    /* to apss use msg_lite */
    bsp_msgchn_attr_init(&lite_attr);
    lite_attr.chnid = MSG_CHN_SYSVIEW;
    lite_attr.coremask = MSG_CORE_MASK(MSG_CORE_TSP) | MSG_CORE_MASK(MSG_CORE_LPM);
    lite_attr.lite_notify = sysview_msg_proc;
    ret = bsp_msg_lite_open(&g_sysview_msg_ctrl.msghdl, &lite_attr);
    if (ret) {
        bsp_err("msg open err, ret = %d\n", ret);
        return;
    }
    g_sysview_msg_ctrl.init_flag = SYSVIEW_INIT_OK;
    return;
}