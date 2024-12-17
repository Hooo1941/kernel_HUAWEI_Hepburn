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

#ifndef IPS_UTIL_H
#define IPS_UTIL_H

#include "mdrv_print.h"
#include "netfilter_os_adapter.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 4)

#ifndef STATIC
#if defined(LLT_OS_VER)
#define STATIC
#else
#define STATIC static
#endif
#endif

#define mod_nfext "nfext"

#define IPS_PRINT_ERR(fmt, ...) (mdrv_err(fmt, ##__VA_ARGS__))
#define IPS_PRINT_WARNING(fmt, ...) (mdrv_wrn(fmt, ##__VA_ARGS__))

#define IPS_GET_ARRAYSIZE(array) (sizeof(array) / sizeof((array)[0])) /* 获取数组的大小 */

#define IPS_MIN(x, y) (((x) < (y)) ? (x) : (y))

/* IP头按照协议解析出来的格式 */
enum IpDataProtocalEnum {
    IP_DATA_PROTOCOL_ICMPV4 = 0x01,        /* ICMPV4协议标识的值 */
    IP_DATA_PROTOCOL_IGMP = 0x02,          /* IGMP协议标识的值 */
    IP_DATA_PROTOCOL_IPINIP = 0x04,        /* IP in IP (encapsulation)协议标识的值 */
    IP_DATA_PROTOCOL_TCP = 0x06,           /* TCP协议标识的值 */
    IP_DATA_PROTOCOL_UDP = 0x11,           /* UDP协议标识的值 */
    IP_DATA_PROTOCOL_IPV6_ROUTE = 0x2b,    /* Routing Header for IPv6协议标识的值 */
    IP_DATA_PROTOCOL_IPV6_FRAGMENT = 0x2c, /* Fragment Header for IPv6协议标识的值 */
    IP_DATA_PROTOCOL_ESP = 0x32,           /* Encapsulating Security Payload协议标识的值 */
    IP_DATA_PROTOCOL_AH = 0x33,            /* Authentication Header协议标识的值 */
    IP_DATA_PROTOCOL_ICMPV6 = 0x3a,        /* ICMPV6协议标识的值 */
    IP_DATA_PROTOCOL_IPV6_NONEXT = 0x3b,   /* No Next Header for IPv6协议标识的值 */
    IP_DATA_PROTOCOL_IPV6_OPTS = 0x3c,     /* Destination Options for IPv6协议标识的值 */
    IP_DATA_PROTOCOL_IPV6_IPIP = 0x5e,     /* IP-within-IP Encapsulation Protocol协议标识的值 */
    IP_DATA_PROTOCOL_IPCOMP = 0x6c,        /* IP Payload Compression Protocol协议标识的值 */

    IP_DATA_PROTOCOL_BUTT
};
typedef uint8_t IpDataProtocolEnumUint8;

/* IP数据承载协议 */
enum IpDataTypeEnum {
    IP_DATA_TYPE_NULL = 0,           /* NULL */
    IP_DATA_TYPE_ICMP = 1,           /* ICMP */
    IP_DATA_TYPE_IGMP = 2,           /* IGMP */
    IP_DATA_TYPE_TCP = 3,            /* TCP */
    IP_DATA_TYPE_TCP_SYN = 4,        /* TCP ACK */
    IP_DATA_TYPE_TCP_ACK = 5,        /* TCP ACK */
    IP_DATA_TYPE_FTP_SIGNALLING = 6, /* FTP SIGNALLING */
    IP_DATA_TYPE_UDP = 7,            /* UDP */
    IP_DATA_TYPE_UDP_DNS = 8,        /* UDP DNS */
    IP_DATA_TYPE_UDP_MIP = 9,        /* UDP MIP AGENT */
    IP_DATA_TYPE_USER_HIGH = 10,     /* user high data */

    IP_DATA_TYPE_BUTT
};
typedef uint8_t IpDataTypeEnumUint8;

#define TCP_LEN_POS 12    /* TCP头部长度字段相对TCP头部首字节的偏移 */
#define TCP_LEN_MASK 0xF0 /* 用于取TCP头部长度字段值的掩码 */
#define UDP_HEAD_LEN 8    /* UDP数据包头部长度 */
#define ICMP_HEADER_LEN 8 /* ICMP 头长度 */
#define TCP_FLAG_POS 13   /* TCP头部长度字段相对TCP头部首字节的偏移 */
#define TCP_FLAG_MASK 0x3F
#define TCP_SYN_MASK 0x02                     /* 用于取TCP头部长度字段值的掩码 */
#define TCP_ACK_MASK 0x10                     /* 用于取TCP头部长度字段值的掩码 */
#define TCP_DST_PORT_POS 2                    /* TCP头部目的端口号相对TCP头部首字节的偏移 */
#define FTP_DEF_SERVER_SIGNALLING_PORT 21     /* FTP熟知的服务器侧信令端口号 */
#define UDP_DST_PORT_POS 2                    /* UDP目的端口相对于UDP头部基地址的偏移 */
#define DNS_DEF_SERVER_PORT 53                /* DNS熟知的服务器侧端口号 */
#define MIP_AGENT_PORT 434                    /* Mobile IP代理端口号 */
#define IPV4_HEAD_FRAGMENT_OFFSET_POS 6       /* IPV4 Fragment offset相对于IPV4首字节的偏移 */
#define IPV4_HEAD_FRAGMENT_OFFSET_MASK 0x3FFF /* IPV4 Fragment offset掩码 */
#define ICMP_TYPE_REQUEST 8                   /* ICMP请求回显报文类型 */
#define ICMP_TYPE_REPLY 0                     /* ICMP回显应答报文类型 */
#define ICMPV6_TYPE_REQUEST 128               /* ICMP请求回显报文类型 */
#define ICMPV6_TYPE_REPLY 129                 /* ICMP回显应答报文类型 */

#define IPV4_HEAD_NORMAL_LEN 20 /* IPV4头长典型值 */
#define IP_VER_MASK 0xF0        /* 用于取IP协议版本的掩码 */
#define IPV4_VER_VAL 0x40       /* IP v4 */
#define IP_HEADER_LEN_MASK 0x0F /* 用于取IP首部长度的掩码 */
#define IP_IPV4_DATA_LEN_POS 2  /* IPV4的IP数据包长度字段偏移 */
#define PROTOCOL_POS 9          /* 协议标识相对IP头部首字节Protocal 的偏移 */
#define IPV6_VER_VAL 0x60       /* IP v6 */
#define IPV6_HEAD_NORMAL_LEN 40 /* IPV6头长典型值 */
#define IP_IPV6_DATA_LEN_POS 4  /* IPV6的IP数据包长度字段偏移 */
#define PROTOCOL_POS_V6 6       /* 协议标识相对IP头部首字节的偏移 */

#define IPV4_SRC_IP_ADDR_OFFSET_POS 12 /* IPV4 Src Ip Adrr offset相对于IPV4首字节的偏移 */
#define IPV4_DST_IP_ADDR_OFFSET_POS 16 /* IPV4 Dst Ip Adrr offset相对于IPV4首字节的偏移 */
#define IPV6_SRC_IP_ADDR_OFFSET_POS 8  /* IPV6 Src Ip Adrr offset相对于IPV6首字节的偏移 */
#define IPV6_DST_IP_ADDR_OFFSET_POS 24 /* IPV6 Dst Ip Adrr offset相对于IPV6首字节的偏移 */

#define IP_GET_VAL_NTOH_U16(data, pos) (((data)[(pos)] << 8) | ((data)[(pos) + 1]))
#define IP_HEADER_LEN_2_BYTE_NUM(len) ((len) << 2) /* IPV4 IHL表示32位个数 */
#define UDP_LEN_POS 4
#define TCP_HEAD_NORMAL_LEN 20

#define IP_PROTOCOL_TCP 0x06  /* IP承载协议是TCP  */

/* 获取结构体成员变量offset的办法 */
#define IPS_OFFSET_OF(s, m) ((NF_SIZE_T)(NF_UINT_PTR)(&(((s *)0)->m)))

#define IPS_SF_CHK(result) \
    IPS_RecordSafeFuncFailLog(((result) != EOK) ? true : false, __LINE__)

#pragma pack(pop)

/*
 * 用来获得可维可测勾取IP报的安全长度，以免泄露用户隐私
 * *data 入参有效性由调用者保证，为IP数据包
 */
uint16_t IPS_GetIpDataTraceLen(uint32_t pid, const uint8_t *data, uint16_t sduLen);

/*
 * TTF勾取非敏感的IP地址 (因为IPV4在某些国际是否是个人数据还存在争议,
 * 因此modem建议掩掉IP地址的后几位, IPv4掩掉后8bit, IPV6掩掉后88bit)
 * ipData指向ip协议起始的位置，必须是连续的内存; dataLen为缓冲区实际长度，函数会对dataLen进行校验
 */
void IPS_MaskIpAddrTraces(uint32_t pid, uint8_t *ipData, uint16_t dataLen);

void IPS_RecordSafeFuncFailLog(bool result,uint32_t lineNo);

void IPS_TransReport(void *msg, uint32_t contentLen, uint32_t msgId);

#ifdef __cplusplus
}
#endif

#endif
