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
#ifndef __MDRV_CHR_H__
#define __MDRV_CHR_H__


#define CHR_ALARM_ID_ERR_LOG_IND   (0x0A)
#define CHR_MOUDLE_ID_DRV          (0x020067)
#define CHR_MOUDLE_ID_DRV_NR       (0x020085)

typedef unsigned int (*chr_drv_ind_func_t)(void *pData, unsigned int ulLen);

typedef enum {
    CHR_DRV_WAKE_LOCK_ALARMID = 0, /* 底软低功耗上报 */
    CHR_DRV_ODT_DST_FULL_ALARMID = 1,
} chr_om_drv_alarmid_e;

typedef struct {
    unsigned int moduleid;
    unsigned short faultid; /* 主动上报时，不关注 */
    unsigned short alarmid;
    unsigned short alarm_type;
    unsigned short resv;
    unsigned int msg_type;
    unsigned int msgsn;
    unsigned int msglen;
} chr_drv_ind_s;

/*****************************************************************************
 * 函 数 名  : mdrv_chr_register_callback
 *
 * 功能描述  : 本接口用于注册CHR的回调函数
 *
 * 输入参数  : omChrFuncPtr
 * 输出参数  : 无
 *
 * 返 回 值  : NA
 *
 * 修改记录  :
 *
 *****************************************************************************/
int  mdrv_chr_drv_register_callback(chr_drv_ind_func_t omChrFuncPtr);


#endif

