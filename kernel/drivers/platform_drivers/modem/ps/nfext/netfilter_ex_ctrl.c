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

#include "netfilter_ex_ctrl.h"
#include "netfilter_ex.h"
#include "ips_util.h"
#include "ips_mntn_cfg.h"
#include "ips_traffic_statistic.h"
#include "mdrv_diag.h"
#include "securec.h"

#define THIS_MODU mod_nfext

uint32_t g_nfextTaskId = 0;
STATIC NfExtEntity g_exEntity = { 0 };

enum NfExtTxBytesCntEnum {
    NF_EXT_TX_BYTES_CNT_BR = 0, /* 统计类型 */
    NF_EXT_TX_BYTES_CNT_BUTT
};

/* netfilter钩子函数掩码 */
enum NfExtHoodOnMaskIdxEnum {
    NF_EXT_BR_PRE_ROUTING_HOOK_ON_MASK_IDX       = 0x00,
    NF_EXT_BR_POST_ROUTING_HOOK_ON_MASK_IDX      = 0x01,
    NF_EXT_BR_FORWARD_HOOK_ON_MASK_IDX           = 0x02,
    NF_EXT_BR_LOCAL_IN_HOOK_ON_MASK_IDX          = 0x03,
    NF_EXT_BR_LOCAL_OUT_HOOK_ON_MASK_IDX         = 0x04,
    NF_EXT_ARP_LOCAL_IN_ON_MASK_IDX              = 0x05,
    NF_EXT_ARP_LOCAL_OUT_ON_MASK_IDX             = 0x06,
    NF_EXT_IP4_PRE_ROUTING_HOOK_ON_MASK_IDX      = 0x07,
    NF_EXT_IP4_POST_ROUTING_HOOK_ON_MASK_IDX     = 0x08,
    NF_EXT_IP4_LOCAL_IN_HOOK_ON_MASK_IDX         = 0x09,
    NF_EXT_IP4_LOCAL_OUT_HOOK_ON_MASK_IDX        = 0x0A,
    NF_EXT_IP4_FORWARD_HOOK_ON_MASK_IDX          = 0x0B,
    NF_EXT_IP6_PRE_ROUTING_HOOK_ON_MASK_IDX      = 0x0C,
    NF_EXT_IP6_POST_ROUTING_HOOK_ON_MASK_IDX     = 0x0D,
    NF_EXT_IP6_LOCAL_IN_HOOK_ON_MASK_IDX         = 0x0E,
    NF_EXT_IP6_LOCAL_OUT_HOOK_ON_MASK_IDX        = 0x0F,
    NF_EXT_IP6_FORWARD_HOOK_ON_MASK_IDX          = 0x10,
    NF_EXT_BR_FORWARD_FLOW_CTRL_HOOK_ON_MASK_IDX = 0x11,
    NF_EXT_HOOK_ON_MASK_IDX_ENUM_BUTT
};
typedef uint8_t NfExtHoodOnMaskIdxEnumUint8;

/* netfilter钩子函数掩码优先级(规避MIXED_ENUMS新增) */
enum NfExtHookOnMaskPriEnum {
    NF_EXT_BR_PRI_FILTER_OTHER   = NF_BR_PRI_FILTER_OTHER,
    NF_EXT_BR_PRI_FILTER_BRIDGED = NF_BR_PRI_FILTER_BRIDGED,
    NF_EXT_IP_PRI_CONNTRACK      = NF_IP_PRI_CONNTRACK,
    NF_EXT_IP_PRI_MANGLE         = NF_IP_PRI_MANGLE,
    NF_EXT_IP_PRI_SELINUX_LAST   = NF_IP_PRI_SELINUX_LAST,

    NF_EXT_HOOK_ON_MASK_PRIORITY_ENUM_BUTT
};

typedef struct {
    uint32_t fcMask;
    uint32_t txBytesCnt[NF_EXT_TX_BYTES_CNT_BUTT];
    struct net_device *brDev;
} NfExtFlowCtrlEntity;

typedef struct {
    uint32_t brArpMask; /* 网桥和ARP钩子函数对应的掩码 */
    uint32_t inMask;    /* IP层PRE_ROUTING钩子函数对应的掩码 */
    uint32_t outMask;   /* IP层POST_ROUTING钩子函数对应的掩码 */
    uint32_t fcMask;    /* 网桥流控钩子函数所对应的掩码 */
    uint32_t localMask; /* IP层LOCAL钩子函数对应的掩码  */
    uint32_t rsv;
} NfExtHookMaskNv;

/* 勾子开关掩码映射表结构体 */
typedef struct {
    u_int32_t hookMask;
    uint8_t rsv[4];
    struct nf_hook_ops nfExtOps;
} NfExtMaskOps;

#define NF_EXT_GET_MASK_FROM_INDEX(idx) (1 << (idx))

STATIC NfExtFlowCtrlEntity g_exFlowCtrlEntity;
STATIC NfExtHookMaskNv g_exHookMask;

/* 注意不同内核版本对于nf_hook_ops的定义,参考kernel\linux 4.x\include\linux\netfilter.h */
#define INIT_MASK_OPS_ITEM(mask_, func_, priv_, pf_, hooknum_, priority_)                                          \
    {                                                                                                              \
        .hookMask = (mask_), .nfExtOps.hook = (nf_hookfn*)(func_), .nfExtOps.priv = (priv_), .nfExtOps.pf = (pf_), \
        .nfExtOps.hooknum = (hooknum_), .nfExtOps.priority = (priority_)                                           \
    }

/* 扩展netfilter开关映射表 */
STATIC NfExtMaskOps g_nfExtMaskOps[NF_EXT_HOOK_ON_MASK_IDX_ENUM_BUTT] = {
    INIT_MASK_OPS_ITEM(NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_BR_PRE_ROUTING_HOOK_ON_MASK_IDX),
        NFEXT_BrPreRoutingHook, NULL, NFPROTO_BRIDGE, NF_BR_PRE_ROUTING, NF_EXT_BR_PRI_FILTER_OTHER),
    INIT_MASK_OPS_ITEM(NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_BR_POST_ROUTING_HOOK_ON_MASK_IDX),
        NFEXT_BrPostRoutingHook, NULL, NFPROTO_BRIDGE, NF_BR_POST_ROUTING, NF_EXT_BR_PRI_FILTER_OTHER),
    INIT_MASK_OPS_ITEM(NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_BR_FORWARD_HOOK_ON_MASK_IDX),
        NFEXT_BrForwardHook, NULL, NFPROTO_BRIDGE, NF_BR_FORWARD, NF_EXT_BR_PRI_FILTER_OTHER),
    INIT_MASK_OPS_ITEM(NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_BR_LOCAL_IN_HOOK_ON_MASK_IDX),
        NFEXT_BrLocalInHook, NULL, NFPROTO_BRIDGE, NF_BR_LOCAL_IN, NF_EXT_BR_PRI_FILTER_OTHER),
    INIT_MASK_OPS_ITEM(NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_BR_LOCAL_OUT_HOOK_ON_MASK_IDX),
        NFEXT_BrLocalOutHook, NULL, NFPROTO_BRIDGE, NF_BR_LOCAL_OUT, NF_EXT_BR_PRI_FILTER_OTHER),
    INIT_MASK_OPS_ITEM(NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_ARP_LOCAL_IN_ON_MASK_IDX),
        NFEXT_ArpInHook, NULL, NFPROTO_ARP, NF_ARP_IN, NF_EXT_IP_PRI_CONNTRACK),
    INIT_MASK_OPS_ITEM(NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_ARP_LOCAL_OUT_ON_MASK_IDX),
        NFEXT_ArpOutHook, NULL, NFPROTO_ARP, NF_ARP_OUT, NF_EXT_IP_PRI_CONNTRACK),
    INIT_MASK_OPS_ITEM(NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_IP4_PRE_ROUTING_HOOK_ON_MASK_IDX),
        NFEXT_Ip4PreRoutingHook, NULL, NFPROTO_IPV4, NF_INET_PRE_ROUTING, NF_EXT_IP_PRI_MANGLE),
    INIT_MASK_OPS_ITEM(NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_IP4_POST_ROUTING_HOOK_ON_MASK_IDX),
        NFEXT_Ip4PostRoutingHook, NULL, NFPROTO_IPV4, NF_INET_POST_ROUTING, NF_EXT_IP_PRI_SELINUX_LAST),
    INIT_MASK_OPS_ITEM(NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_IP4_LOCAL_IN_HOOK_ON_MASK_IDX),
        NFEXT_Ip4LocalInHook, NULL, NFPROTO_IPV4, NF_INET_LOCAL_IN, NF_EXT_IP_PRI_SELINUX_LAST),
    INIT_MASK_OPS_ITEM(NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_IP4_LOCAL_OUT_HOOK_ON_MASK_IDX),
        NFEXT_Ip4LocalOutHook, NULL, NFPROTO_IPV4, NF_INET_LOCAL_OUT, NF_EXT_IP_PRI_SELINUX_LAST),
    INIT_MASK_OPS_ITEM(NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_IP4_FORWARD_HOOK_ON_MASK_IDX),
        NFEXT_Ip4ForwardHook, NULL, NFPROTO_IPV4, NF_INET_FORWARD, NF_EXT_IP_PRI_SELINUX_LAST),
    INIT_MASK_OPS_ITEM(NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_IP6_PRE_ROUTING_HOOK_ON_MASK_IDX),
        NFEXT_Ip6PreRoutingHook, NULL, NFPROTO_IPV6, NF_INET_PRE_ROUTING, NF_EXT_IP_PRI_MANGLE),
    INIT_MASK_OPS_ITEM(NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_IP6_POST_ROUTING_HOOK_ON_MASK_IDX),
        NFEXT_Ip6PostRoutingHook, NULL, NFPROTO_IPV6, NF_INET_POST_ROUTING, NF_EXT_IP_PRI_SELINUX_LAST),
    INIT_MASK_OPS_ITEM(NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_IP6_LOCAL_IN_HOOK_ON_MASK_IDX),
        NFEXT_Ip6LocalInHook, NULL, NFPROTO_IPV6, NF_INET_LOCAL_IN, NF_EXT_IP_PRI_SELINUX_LAST),
    INIT_MASK_OPS_ITEM(NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_IP6_LOCAL_OUT_HOOK_ON_MASK_IDX),
        NFEXT_Ip6LocalOutHook, NULL, NFPROTO_IPV6, NF_INET_LOCAL_OUT, NF_EXT_IP_PRI_SELINUX_LAST),
    INIT_MASK_OPS_ITEM(NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_IP6_FORWARD_HOOK_ON_MASK_IDX),
        NFEXT_Ip6ForwardHook, NULL, NFPROTO_IPV6, NF_INET_FORWARD, NF_EXT_IP_PRI_SELINUX_LAST),
    INIT_MASK_OPS_ITEM(NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_BR_FORWARD_FLOW_CTRL_HOOK_ON_MASK_IDX),
        NFEXT_BrForwardFlowCtrlHook, NULL, NFPROTO_BRIDGE, NF_BR_FORWARD, NF_EXT_BR_PRI_FILTER_BRIDGED)
};

uint32_t NFEXT_GetTaskId(void)
{
    return g_nfextTaskId;
}

void NFEXT_SetTaskId(uint32_t taskId)
{
    g_nfextTaskId = taskId;
}

STATIC int32_t NFEXT_RegisterHook(struct nf_hook_ops *reg)
{
    struct net *net = NF_NULL_PTR;
    struct net *last = NF_NULL_PTR;
    int32_t ret = 0;
    bool rollBackFlag = false;

    rtnl_lock();
    for_each_net(net)
    {
        ret = nf_register_net_hook(net, reg);
        if ((ret != 0) && (ret != -ENOENT)) {
            rollBackFlag = true;
            break;
        }
    }

    if (rollBackFlag == true) {
        last = net;
        for_each_net(net) {
            if (net == last) {
                break;
            }
            nf_unregister_net_hook(net, reg);
        }
    }
    rtnl_unlock();

    return ret;
}

STATIC void NFEXT_UnregisterHook(struct nf_hook_ops *reg)
{
    struct net *net = NF_NULL_PTR;

    rtnl_lock();
    for_each_net(net)
    {
        nf_unregister_net_hook(net, reg);
    }
    rtnl_unlock();

    return;
}

uint32_t NFEXT_GetEntityOmIp(void)
{
    return g_exEntity.omIp;
}

void NFEXT_TxBytesInc(uint32_t incValue)
{
    g_exFlowCtrlEntity.txBytesCnt[NF_EXT_TX_BYTES_CNT_BR] += incValue;
}

uint32_t NFEXT_GetFcMask(void)
{
    return g_exFlowCtrlEntity.fcMask;
}

int32_t NFEXT_CreateRingBuffer(void)
{
    g_exEntity.ringBuf = NFEXT_BufferCreate();
    if (g_exEntity.ringBuf == NF_NULL_PTR) {
        IPS_PRINT_ERR("[init]<NFEXT_RingBufferInit>: ERROR: Create ring buffer Failed!\n");
        return NF_ERR;
    }
    return NF_OK;
}

/* NFExt_Pid初始化 */
STATIC int32_t NFEXT_RingBufferInit(void)
{
#ifdef CONFIG_IPS_NFEXT
    if (NFEXT_CreateRingBuffer() != NF_OK) {
        return NF_ERR;
    }
#endif
    return NF_OK;
}

/* 清空ring buffer里数据 */
STATIC void NFEXT_FlushRingBuffer(NFEXT_Buff *ringBuff)
{
    NFEXT_DataRingBuf data = { 0 };
    int32_t flushCnt = 0;

    while (NFEXT_RingBufferDequeue(ringBuff, &data) == NF_OK) {
        NF_EXT_MEM_FREE(ACPU_PID_NFEXT, data.data);
        flushCnt++;
    }
    IPS_PRINT_WARNING("Flush ring buffer=%d", flushCnt);
}

/* 根据需要停止抓包模块的掩码，将抓包的钩子函数解除内核注册 */
STATIC void NFEXT_UnregHooks(uint32_t mask)
{
    uint32_t idx;

    for (idx = 0; idx < ARRAY_SIZE(g_nfExtMaskOps); idx++) {
        if (g_nfExtMaskOps[idx].hookMask == (mask & g_nfExtMaskOps[idx].hookMask)) {
            /* 卸载钩子函数 */
            NFEXT_UnregisterHook(&(g_nfExtMaskOps[idx].nfExtOps));
            /* 重置相应的掩码位 */
            g_exEntity.curHookOnMask &= (~g_nfExtMaskOps[idx].hookMask);
        }
    }
}

/* 根据需要开启抓包模块的掩码，将抓包的钩子函数注册到内核 */
STATIC int32_t NFEXT_RegHooks(uint32_t mask)
{
    int32_t ret;
    uint32_t idx;

    for (idx = 0; idx < ARRAY_SIZE(g_nfExtMaskOps); idx++) {
        if (g_nfExtMaskOps[idx].hookMask != (mask & g_nfExtMaskOps[idx].hookMask)) {
            continue;
        }

        /* 注册相应的钩子函数 */
        ret = NFEXT_RegisterHook(&(g_nfExtMaskOps[idx].nfExtOps));
        if (ret != 0) {
            IPS_PRINT_WARNING("register_hook error!\n");

            /* 若有一个注册失败则卸载当前所有已经注册上的钩子函数 */
            NFEXT_UnregHooks(g_exEntity.curHookOnMask);
            return ret;
        }

        g_exEntity.curHookOnMask |= g_nfExtMaskOps[idx].hookMask;
    }

    return 0;
}

/* 重新注册钩子函数 */
STATIC int32_t NFEXT_ReRegHooks(uint32_t mask)
{
    int32_t ret;

    /* 重新注册前先卸载当前所有的钩子函数 */
    if (g_exEntity.curHookOnMask != 0) {
        NFEXT_UnregHooks(g_exEntity.curHookOnMask);
    }

    ret = NFEXT_RegHooks(mask);
    return ret;
}

/* 根据配置注册对应的钩子函数 */
bool NFEXT_ConfigEffective(const IPS_MntnTraceCfgReq *msg)
{
    /* 流控hook，默认挂上 */
    uint32_t mask = 0;

    mask |= g_exHookMask.fcMask;
    if ((msg->bridgeArpTraceCfg.choice > IPS_MNTN_TRACE_NULL_CHOSEN) &&
        (msg->bridgeArpTraceCfg.choice <= IPS_MNTN_TRACE_WHOLE_DATA_LEN_CHOSEN)) {
        mask |= g_exHookMask.brArpMask;
    }

    if ((msg->preRoutingTraceCfg.choice > IPS_MNTN_TRACE_NULL_CHOSEN) &&
        (msg->preRoutingTraceCfg.choice <= IPS_MNTN_TRACE_WHOLE_DATA_LEN_CHOSEN)) {
        mask |= g_exHookMask.inMask;
    }

    if ((msg->postRoutingTraceCfg.choice > IPS_MNTN_TRACE_NULL_CHOSEN) &&
        (msg->postRoutingTraceCfg.choice <= IPS_MNTN_TRACE_WHOLE_DATA_LEN_CHOSEN)) {
        mask |= g_exHookMask.outMask;
    }

    if ((msg->localTraceCfg.choice > IPS_MNTN_TRACE_NULL_CHOSEN) &&
        (msg->localTraceCfg.choice <= IPS_MNTN_TRACE_WHOLE_DATA_LEN_CHOSEN)) {
        mask |= g_exHookMask.localMask;
    }

    if (NFEXT_ReRegHooks(mask) < 0) {
        IPS_PRINT_WARNING("NFEXT_ReRegHooks failed!\n");
        return false;
    }

    return true;
}

/* 设置默认NV配置 */
STATIC void NFEXT_SetDefaultHookCfg(void)
{
    g_exHookMask.brArpMask = (NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_ARP_LOCAL_IN_ON_MASK_IDX) |
        NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_ARP_LOCAL_OUT_ON_MASK_IDX)); /* 0X60 */
    g_exHookMask.inMask = (NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_IP4_PRE_ROUTING_HOOK_ON_MASK_IDX) |
        NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_IP6_PRE_ROUTING_HOOK_ON_MASK_IDX)); /* 0x1080 */
    g_exHookMask.outMask = (NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_IP4_POST_ROUTING_HOOK_ON_MASK_IDX) |
        NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_IP6_POST_ROUTING_HOOK_ON_MASK_IDX)); /* 0x2100 */
    g_exHookMask.fcMask = 0;
    g_exHookMask.localMask = (NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_IP4_LOCAL_IN_HOOK_ON_MASK_IDX) |
        NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_IP4_LOCAL_OUT_HOOK_ON_MASK_IDX) |
        NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_IP6_LOCAL_IN_HOOK_ON_MASK_IDX) |
        NF_EXT_GET_MASK_FROM_INDEX(NF_EXT_IP6_LOCAL_OUT_HOOK_ON_MASK_IDX)); /* 0xc600 */
}

/* NFExt模块实体全局变量初始化 */
STATIC void NFEXT_EntityInit(void)
{
    g_exEntity.curHookOnMask = 0;
    g_exEntity.isBlkflag = NF_EXT_FLAG_BLOCKED;
    g_exEntity.isDeviceOpen = 0;
    g_exEntity.omIp = 0;
}

/* 流控实体初始化 */
STATIC void NFEXT_FlowCtrlInit(void)
{
    if (NFEXT_RegHooks(g_exHookMask.fcMask)) {
        IPS_PRINT_ERR("[init]<NFEXT_FlowCtrlInit>: NFEXT_RegHooks fail!\n");
        return;
    }

    g_exFlowCtrlEntity.fcMask = 0;
    g_exFlowCtrlEntity.brDev = NULL;

    (void)memset_s(g_exFlowCtrlEntity.txBytesCnt, sizeof(g_exFlowCtrlEntity.txBytesCnt), 0,
        sizeof(g_exFlowCtrlEntity.txBytesCnt));

    IPSMNTN_TrafficCtrlInit();
}

/* 注册回调函数，用于diag模块diag连接上和断开时通知diag连接状态 */
STATIC void NFEXT_DiagConnStateNotifyCb(mdrv_diag_state_e state)
{
    uint32_t taskId = NFEXT_GetTaskId();
    if (state == DIAG_STATE_DISCONN) {
        (void)NF_OS_EventWrite(taskId, NFEXT_DEBUG_TOOL_DISCONN_NOTIFY);
    }
}

/* 模块初始化 */
int32_t NFEXT_Init(void)
{
    int32_t ret;

    NFEXT_SetDefaultHookCfg();

    NFEXT_EntityInit();

    NFEXT_FlowCtrlInit();

    (void)mdrv_diag_conn_state_notify_fun_reg(NFEXT_DiagConnStateNotifyCb);

    ret = NFEXT_RingBufferInit();

    return ret;
}

/* 发送数据处理指示NFEXT_DATA_PROC_NOTIFY */
STATIC void NFEXT_SndDataNotify(void)
{
    uint32_t taskId = NFEXT_GetTaskId();
    (void)NF_OS_EventWrite(taskId, NFEXT_DATA_PROC_NOTIFY);

    return;
}

/* 将需要通过OAM发送的数据放到自处理任务的RingBuf里面，由自处理任务发送出去 */
uint32_t NFEXT_AddDataToRingBuf(const NFEXT_DataRingBuf *buffer)
{
    unsigned long flag = 0UL;
    uint32_t isNeedWakeUp;
    uint32_t ret;
    if (g_exEntity.ringBuf == NF_NULL_PTR) {
        return NF_ERR;
    }

    /* 空到非空，唤醒任务处理勾包  */
    spin_lock_irqsave(&(g_exEntity.ringBuf->lockTxTask), flag);
    isNeedWakeUp = NFEXT_RingBufferIsEmpty(g_exEntity.ringBuf);
    ret = NFEXT_RingBufferEnqueue(g_exEntity.ringBuf, buffer);
    spin_unlock_irqrestore(&(g_exEntity.ringBuf->lockTxTask), flag);

    if (ret == NF_OK) {
        if (isNeedWakeUp == true) {
            NFEXT_SndDataNotify();
        }
        return NF_OK;
    } else {
        NF_EXT_STATS_INC(1, NF_EXT_STATS_BUF_FULL_DROP);
        /* 队列满，唤醒任务处理勾包 */
        NFEXT_SndDataNotify();
        return NF_ERR;
    }
}

STATIC void NFEXT_ProcDebugToolDisconnNotify(void)
{
    uint32_t mask;

    mask = g_exHookMask.fcMask ^ g_exEntity.curHookOnMask;
    /* 只有fc流控打开的场景，在NFExt_ProcDataNotify中继续释放 */
    if (mask != 0) {
        /* step1.ringbuffer清0 */
        NFEXT_FlushRingBuffer(g_exEntity.ringBuf);
        /* step2.卸载当前除流控外钩子函数 */
        NFEXT_UnregHooks(mask);
    }
}

/* 接收到OM配置可维可测信息捕获配置请求 */
void NFEXT_RcvNfExtInfoCfgReq(const NfMsgBlock *msg)
{
    const OM_IpsMntnCfgReq *nfExtCfgReq = (OM_IpsMntnCfgReq *)msg;
    IPS_OmMntnCfgCnf nfExtCfgCnf = {{0}};

    /* 构建回复消息 */
    /* Fill DIAG trans msg header */
    NF_OS_SET_SENDER_ID(&(nfExtCfgCnf.diagHdr), ACPU_PID_NFEXT);
    /* 把应答消息发送给DIAG，由DIAG把透传命令的处理结果发送给DEBUG工具 */
    NF_OS_SET_RECEIVER_ID(&(nfExtCfgCnf.diagHdr), MSP_PID_DIAG_APP_AGENT);
    NF_OS_SET_MSG_LEN(&(nfExtCfgCnf.diagHdr), sizeof(IPS_OmMntnCfgCnf) - NF_OS_MSG_HEAD_LENGTH);
    nfExtCfgCnf.diagHdr.msgId = ID_IPS_OM_MNTN_INFO_CONFIG_CNF;

    /* DIAG透传命令中的特定信息 */
    nfExtCfgCnf.diagHdr.originalId = nfExtCfgReq->diagHdr.originalId;
    nfExtCfgCnf.diagHdr.terminalId = nfExtCfgReq->diagHdr.terminalId;
    nfExtCfgCnf.diagHdr.timeStamp = nfExtCfgReq->diagHdr.timeStamp;
    nfExtCfgCnf.diagHdr.sn = nfExtCfgReq->diagHdr.sn;

    /* 填充回复OM申请的确认信息 */
    nfExtCfgCnf.ipsMntnCfgCnf.command = nfExtCfgReq->ipsMntnCfgReq.command;
    nfExtCfgCnf.ipsMntnCfgCnf.result = NF_OK;

    /* 发送OM透明消息 */
    IPSMNTN_SndCfgCnf2Om(sizeof(IPS_OmMntnCfgCnf), &nfExtCfgCnf, ID_IPS_OM_MNTN_INFO_CONFIG_CNF);
}

STATIC void NFEXT_ProcDataNotify(void)
{
    NFEXT_DataRingBuf data;
    uint32_t result = NF_ERR;
    uint32_t transResult;
    uint32_t dealCntOnce = 0;
    bool rptErrFlag = false;

    if (g_exEntity.ringBuf == NF_NULL_PTR) {
        return;
    }

    do {
        /* 一次任务调度，最多处理200个勾包 */
        if (dealCntOnce >= NF_ONCE_DEAL_MAX_CNT) {
            NFEXT_SndDataNotify();
            break;
        }
        result = NFEXT_RingBufferDequeue(g_exEntity.ringBuf, &data);
        if (result == NF_OK) {
            dealCntOnce++;
            if (mdrv_diag_get_conn_state() != NF_CONNECT) {
                /* 为防止只有fc流控打开的情况下， NFEXT_ProcDebugToolDisconnNotify中走不到清除分支，这里continue继续释放 */
                NF_EXT_MEM_FREE(ACPU_PID_NFEXT, data.data);
                rptErrFlag = true;
                continue;
            }

            transResult = mdrv_diag_trans_report((mdrv_diag_trans_ind_s *)(data.data));
            if ((transResult != NF_OK) && (rptErrFlag == false)) {
                rptErrFlag = true;
            }
            NF_EXT_MEM_FREE(ACPU_PID_NFEXT, data.data);
        }
    } while (result == NF_OK);

    if (rptErrFlag == true) {
        NF_EXT_STATS_INC(1, NF_EXT_STATS_MDRV_RPT_FAIL);
    }
}

/* NFExt事件处理函数 */
void NFEXT_EventProc(uint32_t event)
{
    if (event & NFEXT_DATA_PROC_NOTIFY) {
        NFEXT_ProcDataNotify();
    }
    if (event & NFEXT_DEBUG_TOOL_DISCONN_NOTIFY) {
        NFEXT_ProcDebugToolDisconnNotify();
    }
}

/* 绑定Task到指定CPU上面 */
void NFEXT_BindToCpu(void)
{
    long ret;
    int32_t cpu;
    pid_t targetPid;

    /* 获取当前线程的Pid */
    targetPid = current->pid;
    /* 获取当前线程的affinity */
    ret = sched_getaffinity(targetPid, &(g_exEntity.origMask));
    if (ret < 0) {
        IPS_PRINT_ERR("[init]<NFEXT_BindToCpu>: unable to get cpu affinity\n");
        return;
    }

    (void)memset_s(&(g_exEntity.currMask), cpumask_size(), 0, cpumask_size());
    /* 设置当前线程的affinity */
    for_each_cpu(cpu, &(g_exEntity.origMask))
    {
        /* 绑定CPU0 */
        if ((cpu > 0) && (cpumask_test_cpu(cpu, &(g_exEntity.origMask)))) {
            cpumask_set_cpu((unsigned int)cpu, &(g_exEntity.currMask));
        }
    }

    if (cpumask_weight(&(g_exEntity.currMask)) == 0) {
        cpumask_set_cpu(0, &(g_exEntity.currMask));
        return;
    }

    ret = sched_setaffinity(targetPid, &(g_exEntity.currMask));
    if (ret < 0) {
        IPS_PRINT_WARNING("[init]<NFEXT_BindToCpu>: unable to set cpu affinity\n");
        return;
    }
}
