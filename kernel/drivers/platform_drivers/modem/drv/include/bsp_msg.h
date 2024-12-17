/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2018. All rights reserved.
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
#ifndef __BSP_MSG_H__
#define __BSP_MSG_H__

#include <osl_types.h>
#include "mdrv_msg.h"
#include "mdrv_event.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @brief 唤醒msg业务通知回调 */
typedef int (*wakeup_notify)(void *arg);

int bsp_msg_init(void);
typedef struct msg_chn_hdl *msg_chn_t;
void bsp_msgchn_attr_init(struct msgchn_attr *pattr);
int bsp_msg_lite_open(msg_chn_t *pchn_hdl, struct msgchn_attr *pattr);
int bsp_msg_lite_close(msg_chn_t chn_hdl);
int bsp_msg_lite_sendto(msg_chn_t chn_hdl, const struct msg_addr *dst, void *msg, unsigned len);
void bsp_msg_act_before_rst(u32 cid);
void bsp_msg_act_after_rst(u32 cid);

/**
 * @brief 唤醒msg业务回调注册
 *
 * @par 描述:
 * msg被业务唤醒时，会调用该业务注册的唤醒回调
 *
 * @param[in]  chn_hdl, 通道成功打开后返回的句柄
 * @param[in]  wakeup_notify, 唤醒msg业务通知回调
 * @param[in]  arg, 回传给回调的参数，允许NULL
 *
 * @retval 0,注册成功
 * @retval 其他, 执行失败
 */
int bsp_msg_wakeup_register(msg_chn_t chn_hdl, wakeup_notify func, void *arg);

#ifdef __cplusplus
}
#endif

#endif

