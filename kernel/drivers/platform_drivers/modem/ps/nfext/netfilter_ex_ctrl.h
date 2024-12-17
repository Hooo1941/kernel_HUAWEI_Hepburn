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

#ifndef NETFILTEREXCTRL_H
#define NETFILTEREXCTRL_H

#include "ips_om_interface.h"
#include "netfilter_buffer.h"
#include "netfilter_os_adapter.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NFEXT_DATA_PROC_NOTIFY 0x0001
#define NFEXT_DEBUG_TOOL_DISCONN_NOTIFY 0x0002
#define NF_EXT_RING_BUF_SIZE (8 * 1024 - 1) /* 环形buff的大小 */
#define NF_ONCE_DEAL_MAX_CNT 200

enum NfExtFlagBlockEnum {
    NF_EXT_FLAG_BLOCKED = 0,   /* 阻塞标志 */
    NF_EXT_FLAG_UNBLOCKED = 1, /* 非阻塞标志 */

    NF_EXT_FLAG_BUTT
};

uint32_t NFEXT_GetTaskId(void);
void NFEXT_SetTaskId(uint32_t taskId);
int32_t NFEXT_CreateRingBuffer(void);
uint32_t NFEXT_GetEntityOmIp(void);
uint32_t NFEXT_AddDataToRingBuf(const NFEXT_DataRingBuf *ringBuf);
void NFEXT_RcvNfExtInfoCfgReq(const NfMsgBlock *msg);
void NFEXT_TxBytesInc(uint32_t incValue);
uint32_t NFEXT_GetFcMask(void);
bool NFEXT_ConfigEffective(const IPS_MntnTraceCfgReq *msg);
int32_t NFEXT_Init(void);
void NFEXT_BindToCpu(void);
void NFEXT_EventProc(uint32_t event);
#ifdef __cplusplus
}
#endif

#endif
