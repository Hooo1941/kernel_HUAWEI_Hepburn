/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
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
#include "netfilter_os_adapter.h"
#include "ips_util.h"
#include "securec.h"
#include "mdrv_public.h"
#include "mdrv_print.h"
#include "mdrv_event.h"
#include "mdrv_msg.h"

#define THIS_MODU mod_nfext

NfMsgBlock *NF_OS_AllocMsg(uint32_t pid, uint32_t length)
{
    NfMsgBlock        *msgBlockPointer = NF_NULL_PTR;

    msgBlockPointer = (NfMsgBlock *)mdrv_msg_alloc(pid, length, MSG_DEFAULT);
    if (msgBlockPointer == NF_NULL_PTR) {
        return ((NfMsgBlock *)NF_NULL_PTR);
    }

    NF_OS_SET_SENDER_ID(msgBlockPointer, pid);
    NF_OS_SET_MSG_LEN(msgBlockPointer, length);

    return msgBlockPointer;
}

uint32_t NF_OS_SendMsg(uint32_t pid, void **msg)
{
    int32_t             ret;

    ret = mdrv_msg_vsend(*msg, MSG_DEFAULT);
    if (ret != MDRV_OK) {
        mdrv_msg_free(pid, *msg);

        mdrv_err("<V_SendMsg> Msg Send errno %d.\n", ret);

        return NF_ERR;
    }

    return NF_OK;
}

uint32_t NF_OS_EventWrite(uint32_t taskID, uint32_t events)
{
    int32_t ret;

    ret = mdrv_event_vsend(taskID, events);
    if (ret != MDRV_OK) {
        mdrv_err("<NF_OS_EventWrite> error. ulTaskID=%d return=%d\n", taskID, ret);
        return NF_ERR;
    }

    return NF_OK;
}