/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2023. All rights reserved.
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

#ifndef BASTETINTERFACE_H
#define BASTETINTERFACE_H

#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif
#endif

#pragma pack(4)

/* Bastet模块丢包消息类型 */
#define BST_ASPEN_INFO_PKT_DROP             0xc0000001
/* 上报给A核丢包数组大小 */
#define BST_ASPEN_PKT_DROP_SIZE             (128u)

enum BST_ACORE_CORE_MSG_TYPE_ENUM
{
    BST_ACORE_CORE_MSG_TYPE_DSPP,
    BST_ACORE_CORE_MSG_TYPE_ASPEN,
    BST_ACORE_CORE_MSG_TYPE_EMCOM_SUPPORT,
    BST_ACORE_CORE_MSG_TYPE_EMCOM_KEY_PSINFO,
    BST_ACORE_CORE_MSG_TYPE_RESET_INFO,
    BST_ACORE_CORE_MSG_TYPE_BUTT
};
typedef unsigned int BST_ACORE_CORE_MSG_TYPE_ENUM_UINT32;

enum BST_ACORE_CORE_KEY_PSSTATE_ENUM
{
    BST_ACORE_CORE_KEY_PS_STOP,
    BST_ACORE_CORE_KEY_PS_START,
    BST_ACORE_CORE_KEY_PS_BUTT
};
typedef unsigned int BST_ACORE_CORE_KEY_PSSTATE_ENUM_UINT32;

enum BST_EMCOM_SUPPORT_ENUM
{
    BST_CCORE_EMCOM_NOT_SUPPORT,
    BST_CCORE_EMCOM_SUPPORT,
    BST_CCORE_EMCOM_BUTT
};
typedef unsigned char BST_EMCOM_SUPPORT_ENUM_UINT8;


/*****************************************************************************
 * 5 类/结构定义                                                             *
*****************************************************************************/
typedef struct
{
    BST_ACORE_CORE_MSG_TYPE_ENUM_UINT32 enMsgType;
    const unsigned char                    *pucData;
} BST_ACOM_DATA_STRU;

typedef struct
{
    BST_ACORE_CORE_MSG_TYPE_ENUM_UINT32 enMsgType;
    unsigned int                          ulLen;
    unsigned char                           aucValue[0];
} BST_ACOM_MSG_STRU;

typedef struct
{
    unsigned int                          ulTcpSeq;
    unsigned short                          usSrcPort;
    unsigned short                          usDropType;
} BST_ASPEN_TCP_INFO_STRU;

typedef struct
{
    BST_ACORE_CORE_MSG_TYPE_ENUM_UINT32    enMsgType;
    BST_ACORE_CORE_KEY_PSSTATE_ENUM_UINT32 enState;
} BST_KEY_PSINFO_STRU;

typedef struct
{
    BST_ACORE_CORE_MSG_TYPE_ENUM_UINT32 enMsgType;
    BST_EMCOM_SUPPORT_ENUM_UINT8        enState;
    unsigned char                           aucReverse[3];
} BST_EMCOM_SUPPORT_STRU;

typedef struct
{
    unsigned int                          ulAspenInfoType;
    unsigned int                          ulPktNum;                           /* 丢包个数 */
    BST_ASPEN_TCP_INFO_STRU             stPkt[BST_ASPEN_PKT_DROP_SIZE];     /* 丢包数组(一次最大传输128个丢包信息) */
} BST_ASPEN_PKT_DROP_STRU;

#pragma pack()

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* BASTETINTERFACE_H */
