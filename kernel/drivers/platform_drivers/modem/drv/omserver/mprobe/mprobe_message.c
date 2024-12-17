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

#include "securec.h"
#include "osl_thread.h"
#include "osl_malloc.h"
#include "bsp_slice.h"
#include "mprobe_message.h"
#include "mprobe_debug.h"

#undef THIS_MODU
#define THIS_MODU mod_mprobe

mprobe_message_state_s g_mprobe_message_state;

int mprobe_msg_send(void *data, u32 len)
{
    s32 ret;
    struct msg_addr dst;

    dst.core = MSG_CORE_TSP;
    dst.chnid = MSG_CHN_MPROBE;

    ret = bsp_msg_lite_sendto(g_mprobe_message_state.msghdl, &dst, data, len);
    if (ret) {
        mprobe_error("msg send fail,ret=0x%x\n", ret);
        return ret;
    }

    return ret;
}

void mprobe_tsp_msg_proc(void *msg_data)
{
    return;
}

int mprobe_message_proc(const struct msg_addr *src, void *buf, u32 len)
{
    if (src == NULL || buf == NULL) {
        return BSP_ERROR;
    }

    switch (src->core) {
        case MSG_CORE_TSP:
            mprobe_tsp_msg_proc(buf);
            break;

        default:
            mprobe_error("unknown msg, src core = 0x%x\n", src->core);
            break;
    }

    return BSP_OK;
}

int mprobe_message_task_init(void)
{
    s32 ret;
    struct msgchn_attr lite_attr = { 0 };

    bsp_msgchn_attr_init(&lite_attr);

    lite_attr.chnid = MSG_CHN_MPROBE;
    lite_attr.coremask = MSG_CORE_MASK(MSG_CORE_TSP);
    lite_attr.lite_notify = mprobe_message_proc;

    ret = bsp_msg_lite_open(&g_mprobe_message_state.msghdl, &lite_attr);
    if (ret) {
        mprobe_error("register msg mid fail, ret=%x\n", ret);
        return ret;
    }

    return ret;
}

void mprobe_message_init(void)
{
    s32 ret;

    spin_lock_init(&g_mprobe_message_state.spin_lock);

    ret = mprobe_message_task_init();
    if (ret) {
        return;
    }

    mprobe_crit("mprobe message init ok\n");
}