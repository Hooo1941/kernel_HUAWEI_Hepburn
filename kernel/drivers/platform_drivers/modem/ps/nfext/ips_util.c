/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2019. All rights reserved.
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

#include "ips_util.h"
#include "securec.h"
#include "mdrv_diag.h"

#define THIS_MODU mod_nfext

typedef struct {
    uint16_t              headLen;
    uint16_t              totalLen;
    IpDataProtocolEnumUint8 protocolType;
    uint8_t               rsv[3];
} NfIpHeadParseResult;

typedef uint32_t (*NfIpCheckDataLenFun)(
    const uint8_t *data, uint16_t dataLen, const NfIpHeadParseResult *ipHeadInfo);
typedef uint16_t (*NfIpGetTraceLen)(const uint8_t *data, const NfIpHeadParseResult *ipHeadInfo);

typedef struct {
    NfIpCheckDataLenFun checkLenFun;
    NfIpGetTraceLen     getTraceLenFun;
} NfIpProtocolMethod;

typedef struct {
    uint32_t          type;
    NfIpProtocolMethod method;
} TtfIpProtocolMethodMap;

STATIC uint16_t IPS_GetTcpHeadLen(const uint8_t *ipData, uint16_t headLen)
{
    /* TCP HEAD LEN占用字节高4bit，字段含义是32位字节长度，这里需要右移4位、左移2位，合并后右移2位 */
    return (ipData[headLen + TCP_LEN_POS] & TCP_LEN_MASK) >> 2;
}

STATIC uint32_t IPS_CheckTcpDataLen(const uint8_t *data, uint16_t dataLen, const NfIpHeadParseResult *ipHeadInfo)
{
    uint16_t tcpHeadLen;

    if (dataLen < (ipHeadInfo->headLen + TCP_HEAD_NORMAL_LEN)) {
        /* 数据包长度，不够容纳完整的TCP头， */
        return NF_ERR;
    }

    tcpHeadLen = IPS_GetTcpHeadLen(data, ipHeadInfo->headLen);
    if (dataLen < (tcpHeadLen + ipHeadInfo->headLen)) {
        /* 数据包长度，不够容纳完整的TCP头， */
        return NF_ERR;
    }

    return NF_OK;
}

STATIC uint32_t IPS_CheckUdpDataLen(const uint8_t *data, uint16_t dataLen, const NfIpHeadParseResult *ipHeadInfo)
{
    uint16_t udpLen;

    if (dataLen < (ipHeadInfo->headLen + UDP_HEAD_LEN)) {
        /* 数据包长度，不够容纳完整的UDP头 */
        return NF_ERR;
    }

    /* 获取 UDP的总长度 */
    udpLen = IP_GET_VAL_NTOH_U16(data, (ipHeadInfo->headLen + UDP_LEN_POS));
    if (udpLen < UDP_HEAD_LEN) {
        /* UdpLen 字段非法 */
        return NF_ERR;
    }

    return NF_OK;
}

STATIC uint32_t IPS_CheckIcmpDataLen(
    const uint8_t *data, uint16_t dataLen, const NfIpHeadParseResult *ipHeadInfo)
{
    if (dataLen < (ipHeadInfo->headLen + ICMP_HEADER_LEN)) {
        /* 数据包长度，不够容纳完整的ICMPV4头 */
        return NF_ERR;
    }

    return NF_OK;
}

STATIC uint32_t IPS_CheckDataLenDefault(
    const uint8_t *data, uint16_t dataLen, const NfIpHeadParseResult *ipHeadInfo)
{
    return NF_OK;
}

/* 解析TCP包，获取TCP 可trace长度; data 入参有效性由调用者保证，为IP数据包 */
STATIC uint16_t IPS_GetTcpTraceLen(const uint8_t *data, const NfIpHeadParseResult *ipHeadInfo)
{
    uint16_t traceLen;
    uint16_t sourcePort;
    uint16_t destPort;
    const uint16_t tcpHeadLen = IPS_GetTcpHeadLen(data, ipHeadInfo->headLen);
    /* SDU数据长度等于IP包头长度和TCP包头部长度之和，并且TCP包FLAG标志中含有ACK */
    if (ipHeadInfo->totalLen == (tcpHeadLen + ipHeadInfo->headLen)) {
        traceLen = ipHeadInfo->totalLen;
    } else {
        sourcePort = IP_GET_VAL_NTOH_U16(data, ipHeadInfo->headLen);
        destPort   = IP_GET_VAL_NTOH_U16(data, ipHeadInfo->headLen + TCP_DST_PORT_POS);
        /* FTP命令全部勾取，其它勾TCP头 */
        if ((sourcePort == FTP_DEF_SERVER_SIGNALLING_PORT) ||
            (destPort == FTP_DEF_SERVER_SIGNALLING_PORT)) {
            traceLen = ipHeadInfo->totalLen;
        } else {
            traceLen = ipHeadInfo->headLen + tcpHeadLen;
        }
    }

    return traceLen;
}

/* 解析UDP包，获取UDP 可trace的长度; data 入参有效性由调用者保证，为IP数据包 */
STATIC uint16_t IPS_GetUdpTraceLen(const uint8_t *data, const NfIpHeadParseResult *ipHeadInfo)
{
    uint16_t sourcePort = IP_GET_VAL_NTOH_U16(data, ipHeadInfo->headLen);
    uint16_t destPort   = IP_GET_VAL_NTOH_U16(data, ipHeadInfo->headLen + UDP_DST_PORT_POS);
    uint16_t traceLen;

    /* DNS全部勾取，其它勾UDP头 */
    if ((sourcePort == DNS_DEF_SERVER_PORT) || (destPort == DNS_DEF_SERVER_PORT)) {
        traceLen = ipHeadInfo->totalLen;
    } else {
        traceLen = ipHeadInfo->headLen + UDP_HEAD_LEN;
    }

    return traceLen;
}

STATIC uint16_t IPS_GetIcmpTraceLen(const uint8_t *data, const NfIpHeadParseResult *ipHeadInfo)
{
    return ipHeadInfo->headLen + ICMP_HEADER_LEN;
}

STATIC uint16_t IPS_GetTraceLenDefault(const uint8_t *data, const NfIpHeadParseResult *ipHeadInfo)
{
    return ipHeadInfo->headLen;
}

#define DECLEAR_METHOD(chkLen, getTraceLen)                                \
    {                                                                               \
        .checkLenFun = (chkLen), .getTraceLenFun = (getTraceLen) \
    }

STATIC TtfIpProtocolMethodMap g_protocolMethodMap[] = {
    { .type = IP_DATA_PROTOCOL_TCP, .method = DECLEAR_METHOD(IPS_CheckTcpDataLen, IPS_GetTcpTraceLen) },
    { .type = IP_DATA_PROTOCOL_UDP, .method = DECLEAR_METHOD(IPS_CheckUdpDataLen, IPS_GetUdpTraceLen) },
    { .type = IP_DATA_PROTOCOL_ICMPV4, .method = DECLEAR_METHOD(IPS_CheckIcmpDataLen, IPS_GetIcmpTraceLen) },
    { .type = IP_DATA_PROTOCOL_ICMPV6, .method = DECLEAR_METHOD(IPS_CheckIcmpDataLen, IPS_GetIcmpTraceLen) },
};

STATIC NfIpProtocolMethod g_protocolMethodNullObj = DECLEAR_METHOD(
    IPS_CheckDataLenDefault, IPS_GetTraceLenDefault);

STATIC NfIpProtocolMethod *IPS_GetProtocolMethod(IpDataProtocolEnumUint8 protocolType)
{
    uint32_t loop;

    for (loop = 0; loop < IPS_GET_ARRAYSIZE(g_protocolMethodMap); loop++) {
        if (protocolType == g_protocolMethodMap[loop].type) {
            return &(g_protocolMethodMap[loop].method);
        }
    }

    return &g_protocolMethodNullObj;
}

STATIC uint32_t IPS_PraseIpHead(
    uint32_t pid, const uint8_t *data, uint16_t dataLen, NfIpHeadParseResult *result)
{
    uint32_t ipVersion;

    if (dataLen <= IPV4_HEAD_NORMAL_LEN) {
        return NF_ERR;
    }

    ipVersion = data[0] & IP_VER_MASK;
    if (ipVersion == IPV4_VER_VAL) {
        result->headLen      = IP_HEADER_LEN_2_BYTE_NUM(data[0] & IP_HEADER_LEN_MASK);
        result->totalLen     = IP_GET_VAL_NTOH_U16(data, IP_IPV4_DATA_LEN_POS);
        result->protocolType = data[PROTOCOL_POS];
    } else if (ipVersion == IPV6_VER_VAL) {
        result->headLen      = IPV6_HEAD_NORMAL_LEN;
        result->totalLen     = IP_GET_VAL_NTOH_U16(data, IP_IPV6_DATA_LEN_POS) + IPV6_HEAD_NORMAL_LEN;
        result->protocolType = data[PROTOCOL_POS_V6];
    } else {
        return NF_ERR;
    }

    if ((dataLen < result->totalLen) || (result->totalLen < result->headLen)) {
        return NF_ERR;
    }

    return NF_OK;
}

uint16_t IPS_GetIpDataTraceLen(uint32_t pid, const uint8_t *data, uint16_t dataLen)
{
    NfIpProtocolMethod *method     = NF_NULL_PTR;
    NfIpHeadParseResult ipHeadInfo = {0};

    if (IPS_PraseIpHead(pid, data, dataLen, &ipHeadInfo) != NF_OK) {
        return 0;
    }

    method = IPS_GetProtocolMethod(ipHeadInfo.protocolType);
    /* 安全检查: 检查数据包大小是否能够容纳对应协议包头，不能容纳的异常包，就不用继续解析了 */
    if (method->checkLenFun(data, dataLen, &ipHeadInfo) != NF_OK) {
        return 0;
    }

    return method->getTraceLenFun(data, &ipHeadInfo);
}

#define IPS_IPV4_MASK_IP_ADDR_SENSITIVE_POS 3       /* IPV4 地址脱敏的位置 */
#define IPS_IPV4_MASK_IP_ADDR_SENSITIVE_BYTE_NUM 1  /* IPV4 地址脱敏的字节数 */
#define IPS_IPV6_MASK_IP_ADDR_SENSITIVE_POS 5       /* IPV6 地址脱敏的位置 */
#define IPS_IPV6_MASK_IP_ADDR_SENSITIVE_BYTE_NUM 11 /* IPV6 地址脱敏的字节数 */

STATIC void IPS_FilterIpv4AddrSensitiveInfo(uint8_t *ipAddr)
{
    *(ipAddr + IPS_IPV4_MASK_IP_ADDR_SENSITIVE_POS) = 0;
}

STATIC void IPS_FilterIpv6AddrSensitiveInfo(uint8_t *ipAddr)
{
    uint8_t *ipSensitiveAddr = ipAddr + IPS_IPV6_MASK_IP_ADDR_SENSITIVE_POS;
    (void)memset_s(
        ipSensitiveAddr, IPS_IPV6_MASK_IP_ADDR_SENSITIVE_BYTE_NUM, 0, IPS_IPV6_MASK_IP_ADDR_SENSITIVE_BYTE_NUM);
}

void IPS_MaskIpAddrTraces(uint32_t pid, uint8_t *ipData, uint16_t dataLen)
{
    const uint32_t ipVersion = ipData[0] & IP_VER_MASK;

    if (ipVersion == IPV4_VER_VAL) {
        if (dataLen < IPV4_HEAD_NORMAL_LEN) {
            return;
        }

        IPS_FilterIpv4AddrSensitiveInfo(ipData + IPV4_SRC_IP_ADDR_OFFSET_POS);
        IPS_FilterIpv4AddrSensitiveInfo(ipData + IPV4_DST_IP_ADDR_OFFSET_POS);
    } else if (ipVersion == IPV6_VER_VAL) {
        if (dataLen < IPV6_HEAD_NORMAL_LEN) {
            return;
        }

        IPS_FilterIpv6AddrSensitiveInfo(ipData + IPV6_SRC_IP_ADDR_OFFSET_POS);
        IPS_FilterIpv6AddrSensitiveInfo(ipData + IPV6_DST_IP_ADDR_OFFSET_POS);
    } else {
    }
}

void IPS_RecordSafeFuncFailLog(bool result, uint32_t lineNo)
{
    if (result) {
        IPS_PRINT_WARNING("SafeFunc return=%d line=%d!", result, lineNo);
    }
}

void IPS_TransReport(void *msg, uint32_t contentLen, uint32_t msgId)
{
    mdrv_diag_trace_s diagTrace = {0};
    diagTrace.module = MDRV_DIAG_GEN_MODULE_EX(0, DIAG_MODE_COMM, DIAG_MSG_TYPE_PS);
    diagTrace.sendpid = NF_OS_GET_SENDER_ID(msg);
    diagTrace.recvpid = NF_OS_GET_RECEIVER_ID(msg);
    diagTrace.msgid = msgId;
    diagTrace.length = contentLen;
    diagTrace.data = msg;

    (void)mdrv_diag_trace_report(&diagTrace);
}
