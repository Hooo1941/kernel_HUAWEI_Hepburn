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

#ifndef IPS_OM_INTERFACE_H
#define IPS_OM_INTERFACE_H

#include <linux/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 4)
/* AP上的TCP协议栈的可维可测消息, 编号以D3开头 */
enum IPS_MntnMsgType {
    ID_OM_IPS_MNTN_TRAFFIC_CTRL_REQ = 0xD312,     /* 开启IPS LOG流控功能 */
    ID_OM_IPS_ADVANCED_TRACE_CONFIG_REQ = 0xD313, /* 捕获TCP/IP协议栈 报文配置 */
    ID_IPS_OM_ADVANCED_TRACE_CONFIG_CNF = 0xD314, /* 捕获TCP/IP协议栈 报文指示 */
    ID_OM_IPS_MNTN_INFO_CONFIG_REQ = 0xD315,      /* 周期性捕获TCP/IP协议栈 基础信息配置 */
    ID_IPS_OM_MNTN_INFO_CONFIG_CNF = 0xD316,      /* 周期性捕获TCP/IP协议栈 基础信息配置指示 */
    ID_IPS_TRACE_INPUT_DATA_INFO = 0xD317,        /* TCP/IP可维可测接收报文 */
    ID_IPS_TRACE_OUTPUT_DATA_INFO = 0xD318,       /* TCP/IP可维可测发送报文 */
    ID_IPS_TRACE_BRIDGE_DATA_INFO = 0xD319,       /* TCP/IP网桥中转报文 */
    ID_IPS_TRACE_RECV_ARP_PKT = 0xD31A,           /* TCP/IP协议栈接收的ARP控制报文 */
    ID_IPS_TRACE_SEND_ARP_PKT = 0xD31B,           /* TCP/IP协议栈发送的ARP控制报文 */
    ID_IPS_TRACE_RECV_DHCPC_PKT = 0xD31C,         /* TCP/IP协议栈接收的DHCP控制报文 */
    ID_IPS_TRACE_SEND_DHCPC_PKT = 0xD31D,         /* TCP/IP协议栈发送的DHCP控制报文 */
    ID_IPS_TRACE_RECV_DHCPS_PKT = 0xD31E,         /* TCP/IP协议栈接收的DHCP控制报文 */
    ID_IPS_TRACE_SEND_DHCPS_PKT = 0xD31F,         /* TCP/IP协议栈发送的DHCP控制报文 */
    ID_IPS_TRACE_APP_CMD = 0xD320,                /* APP控制命令信息 */
    ID_IPS_TRACE_MNTN_INFO = 0xD321,              /* TCP/IP协议栈基础信息 */
    ID_IPS_TRACE_BRIDGE_PRE_ROUTING_INFO = 0xD322,
    ID_IPS_TRACE_BRIDGE_POST_ROUTING_INFO = 0xD323,
    ID_IPS_TRACE_BRIDGE_LOCAL_IN_INFO = 0xD324,
    ID_IPS_TRACE_BRIDGE_LOCAL_OUT_INFO = 0xD325,
    ID_IPS_TRACE_IP4_FORWARD_INFO = 0xD326,
    ID_IPS_TRACE_IP4_LOCAL_IN_INFO = 0xD327,
    ID_IPS_TRACE_IP4_LOCAL_OUT_INFO = 0xD328,
    ID_IPS_TRACE_IP6_FORWARD_INFO = 0xD329,
    ID_IPS_TRACE_IP6_LOCAL_IN_INFO = 0xD32A,
    ID_IPS_TRACE_IP6_LOCAL_OUT_INFO = 0xD32B,
    ID_IPS_TRACE_BR_FORWARD_FLOW_CTRL_START = 0xD32C,
    ID_IPS_TRACE_BR_FORWARD_FLOW_CTRL_STOP = 0xD32D,
    ID_IPS_TRACE_ADS_UL = 0xD330,
    ID_IPS_TRACE_ADS_DL = 0xD331,
    ID_IPS_MSG_TYPE_BUTT = 0xFFFF
};
typedef uint16_t IPS_MntnMsgTypeUint16;

enum IPS_MntnTraceChosen {
    IPS_MNTN_TRACE_NULL_CHOSEN = 0,             /* 不捕获报文信息 */
    IPS_MNTN_TRACE_MSG_HEADER_CHOSEN = 1,       /* 捕获报文头部 */
    IPS_MNTN_TRACE_CONFIGURABLE_LEN_CHOSEN = 2, /* 按照配置捕获报文 */
    IPS_MNTN_TRACE_WHOLE_DATA_LEN_CHOSEN = 3    /* 捕获报文全部内容 */
};
typedef uint32_t IPS_MntnTraceChosenUint32;

enum IPS_TraceMsgFlag {
    IPS_TRACE_MSG_NO = 0,
    IPS_TRACE_MSG_YES = 1
};
typedef uint8_t IPS_TraceMsgFlagUint8;

typedef uint32_t IPS_RSLT_CODE_ENUM_UINT32; /* NF_OK表示成功，NF_ERR表示失败 */

enum IPS_MntnInfoAction {
    IPS_MNTN_INFO_REPORT_STOP = 0, /* 开启周期性捕获TCP/IP协议栈基础信息 */
    IPS_MNTN_INFO_REPORT_START = 1 /* 关闭周期性捕获TCP/IP协议栈基础信息 */
};
typedef uint16_t IPS_MntnInfoActionUint16;

typedef struct {
    uint16_t msgFlag;
    uint16_t msgTimeStamp;
    uint32_t length;
    uint32_t senderPid;
    uint32_t receiverPid;
    uint32_t msgId;
    uint16_t originalId;
    uint16_t terminalId;
    uint32_t timeStamp;
    uint32_t sn;
} IPS_TransMsgHeader;

typedef struct {
    IPS_MntnTraceChosenUint32 choice;
    uint32_t traceDataLen; /* 当选择TRACE_CONFIGURABLE_LEN_CHOSEN时，此值有效 */
} IPS_MntnTraceCfg;

typedef IPS_MntnTraceCfg IPS_MntnBridgeTraceCfg;
typedef IPS_MntnTraceCfg IPS_MntnInputTraceCfg;
typedef IPS_MntnTraceCfg IPS_MntnOutputTraceCfg;
typedef IPS_MntnTraceCfg IPS_MntnLocalTraceCfg;

/* 对应消息:  ID_OM_IPS_ADVANCED_TRACE_CONFIG_REQ */
typedef struct {
    IPS_MntnBridgeTraceCfg bridgeArpTraceCfg;    /* 配置捕获网桥消息 */
    IPS_MntnInputTraceCfg  preRoutingTraceCfg;   /* 配置捕获TCP/IP协议栈接收消息 */
    IPS_MntnOutputTraceCfg postRoutingTraceCfg;  /* 配置捕获TCP/IP协议栈发送消息 */
    IPS_MntnLocalTraceCfg  localTraceCfg;        /* 配置捕获TCP/IP协议栈发送消息 */
    IPS_TraceMsgFlagUint8  adsIpConfig;
    uint8_t                logFilterFlag;        /* 1:脱敏 0:不脱敏 */
    uint8_t                rsv[2];               /* 保留位 */
} IPS_MntnTraceCfgReq;

/* 对应消息:  OM_IpsAdvancedTracCfgReq */
typedef struct {
    IPS_TransMsgHeader diagHdr;
    IPS_MntnTraceCfgReq ipsAdvanceCfgReq;
} OM_IpsAdvancedTraceCfgReq;

/* 对应消息:  ID_IPS_OM_ADVANCED_TRACE_CONFIG_CNF */
typedef struct {
    IPS_RSLT_CODE_ENUM_UINT32 result;
} IPS_MntnTraceCfgCnf;

typedef struct {
    IPS_TransMsgHeader diagHdr;
    IPS_MntnTraceCfgCnf ipsAdvanceCfgCnf;
} IPS_OmAdvancedTraceCfgCnf;

/* 对应消息:  ID_OM_IPS_MNTN_INFO_CONFIG_REQ */
typedef struct {
    IPS_MntnInfoActionUint16 command;
    uint16_t timeLen; /* 单位：秒 */
} IPS_MntnCfgReq;

typedef struct {
    IPS_TransMsgHeader diagHdr;
    IPS_MntnCfgReq ipsMntnCfgReq;
} OM_IpsMntnCfgReq;

/* 对应消息:  ID_IPS_OM_MNTN_INFO_CONFIG_CNF */
typedef struct {
    IPS_RSLT_CODE_ENUM_UINT32 result;
    IPS_MntnInfoActionUint16 command;
    uint8_t rsv[2];
} IPS_MntnCfgCnf;

typedef struct {
    IPS_TransMsgHeader diagHdr;
    IPS_MntnCfgCnf ipsMntnCfgCnf;
} IPS_OmMntnCfgCnf;

/* 对应消息:  ID_OM_IPS_MNTN_TRAFFIC_CTRL_REQ */
typedef struct {
    IPS_TransMsgHeader diagHdr;
    uint16_t trafficCtrlOn;
    uint16_t speedKBps; /* 单位：KB/s */
} OM_IpsMntnTrafficCtrlReq;

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif
