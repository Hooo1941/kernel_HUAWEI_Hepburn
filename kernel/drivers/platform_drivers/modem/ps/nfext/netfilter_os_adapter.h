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

#ifndef NETFILTER_OS_ADAPTER_H
#define NETFILTER_OS_ADAPTER_H

#include "product_config_ps_ap.h"
#include "msg_id.h"
#include "network_interface.h"
#include "netfilter_ips_mntn.h"
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter_arp.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_bridge.h>
#include <linux/wait.h>
#include <linux/time.h>
#include <linux/in.h>
#include <linux/tcp.h>
#include <linux/ip.h>
#include <linux/version.h>
#include <linux/kthread.h>
#include <linux/spinlock.h>
#include <uapi/linux/sched/types.h>
#include <linux/types.h>
#include <linux/list.h>
#include <linux/rtnetlink.h>
#include <linux/sched.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NF_OK 0
#define NF_ERR 1
#define NF_ERROR (-1)
#define NF_NULL_PTR 0L
#define NF_CONNECT 1
#define NF_DISCONN 0
#define NF_MODEM_ID_0 0
#define NF_OS_MSG_HEADER  uint16_t msgFlag;\
                          uint16_t msgTimeStamp;\
                          uint32_t length;\
                          uint32_t senderPid;\
                          uint32_t receiverPid;\

/* VOS common message header length, sizeof(NF_OS_MSG_HEADER) */
#define NF_OS_MSG_HEAD_LENGTH              16

#pragma pack(push, 1)
struct NfMsgCB {
    NF_OS_MSG_HEADER
    uint8_t  value[2];
};
#pragma pack(pop)

typedef unsigned int NF_SIZE_T;
typedef unsigned long NF_UINT_PTR;
typedef struct NfMsgCB NfMsg_CB;
typedef struct NfMsgCB NfMsgBlock;

#define NF_OS_GET_SENDER_ID(msg) (((NfMsgBlock *)(msg))->senderPid)

#define NF_OS_SET_SENDER_ID(msg, id) (((NfMsgBlock *)(msg))->senderPid = (id))

#define NF_OS_GET_RECEIVER_ID(msg) (((NfMsgBlock *)(msg))->receiverPid)

#define NF_OS_SET_RECEIVER_ID(msg, id) (((NfMsgBlock *)(msg))->receiverPid = (id))

#define NF_OS_GET_MSG_LEN(msg) (((NfMsgBlock *)(msg))->length)

#define NF_OS_SET_MSG_LEN(msg, len) (((NfMsgBlock *)(msg))->length = (len))

NfMsgBlock *NF_OS_AllocMsg(uint32_t pid, uint32_t length);
uint32_t NF_OS_SendMsg(uint32_t pid, void **msg);
uint32_t NF_OS_EventWrite(uint32_t taskID, uint32_t events);

#ifdef __cplusplus
}
#endif

#endif
