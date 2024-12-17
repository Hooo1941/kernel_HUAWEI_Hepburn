/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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
#ifndef __BSP_IPCMSG_H__
#define __BSP_IPCMSG_H__

#include <osl_types.h>

#ifdef __cplusplus /* __cplusplus */
extern "C" {
#endif /* __cplusplus */

/* 通道枚举分配 */
typedef enum {
    IPCMSG_CHN_ACPU2MODEM0_IPC  =   0x1F,
    IPCMSG_CHN_ACPU2MODEM1_IPC  =   0x1E,
    IPCMSG_CHN_ACPU2MODEM2_IPC  =   0x1D,
    IPCMSG_CHN_ACPU2MODEM3_IPC  =   0x1C,
    IPCMSG_CHN_LPMCU2MODEM0_IPC =   0x2F,
    IPCMSG_CHN_LPMCU2MODEM1_IPC =   0x2E,
    IPCMSG_CHN_LPMCU2MODEM2_IPC =   0x2D,
    IPCMSG_CHN_LPMCU2MODEM3_IPC =   0x2C,
    IPCMSG_CHN_DPA2ACPU_IPC     =   0x4E,
    IPCMSG_CHN_MODEM2ACPU_IPC   =   0x4F,
    IPCMSG_CHN_LPMCU2ACPU_IPC   =   0x6F,
    IPCMSG_CHN_ACPU2LPMCU_IPC   =   0x9F,
    IPCMSG_INT_BUTTOM
} ipcmsg_chn_id_e;

typedef struct ipcmsg_channel ipcmsg_channel_t;

typedef int (*ipcmsg_cb_func)(unsigned char *buf, unsigned int len, void *context);

/* 对外接口声明start */
/* ****************************************************************************
 * 函 数 名     : bsp_ipcmsg_init
 *
 * 功能描述  : IPCMSG模块初始化
 *
 * *************************************************************************** */
int bsp_ipcmsg_init(void);

/* ****************************************************************************
* 函 数 名      : bsp_ipcmsg_open
*
* 功能描述  : ipcmsg_chn_id_e逻辑资源号获取句柄函数
*
* 输入参数  : ipc_res_id ipc逻辑资源号
            phandle 句柄
* 输出参数  : 无
*
* 返 回 值      :  OK&ERROR
**************************************************************************** */
ipcmsg_channel_t *bsp_ipcmsg_open(ipcmsg_chn_id_e chn_id, ipcmsg_cb_func routine, void *context);

/* ****************************************************************************
 * 函 数 名      : bsp_ipcmsg_send
 *
 * 功能描述  : ipcmsg向目的核发送中断或数据函数
 *
 * *************************************************************************** */
s32 bsp_ipcmsg_send(ipcmsg_channel_t *channel, void *buf, unsigned int length);

#ifdef __cplusplus /* __cplusplus */
}
#endif /* __cplusplus */

#endif /*  __BSP_IPCMSG_H__ */
