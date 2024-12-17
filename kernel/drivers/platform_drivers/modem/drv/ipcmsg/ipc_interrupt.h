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

#ifndef __IPC_INTERRUPT_H__
#define __IPC_INTERRUPT_H__

#include <bsp_ipc_fusion.h>
#include "ipcmsg_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IPC_HANDLE_ERR (-2)
#define IPC_BIT_MASK(n) (1UL << (n))
#define IPC_MAGIC (0xabcd4321)
#define IPC_HANDLE_NULL (0xABCDEF00)

#define IPCMSG_RESET_BEFORE_CMD 0x1
#define IPCMSG_RESET_ING_CMD 0x2

struct ipcmsg_res_info {
    u32 func_id : 8;
    u32 chn_id : 8;
    u32 dev_id : 4;
    u32 res_id : 12;
};

/* ipc 资源 */
struct ipc_handle {
    u32 res_id;                         /* ipc资源逻辑ID */
    u32 reg_idx;                        /* ipc资源对应的邮箱寄存器索引 */
    u32 send_times;                     /* 发送次数 */
    u32 handle_times;                   /* 处理次数 */
    u32 magic;                          /* ipc资源magic */
    void *channel;                     /* ipcmsg channel指针 */
    void *channel_dts;                 /* ipc resource里channel dts指针 */
    struct ipc_entry ipc_int_table;     /* ipc挂的中断回调函数 */
};

s32 ipc_handle_init(void);
struct ipc_handle *__ipc_handle_get(ipc_res_id_e ipc_res_id);
int __ipc_res_show_dump(int ipc_res_id);

s32 bsp_ipc_send(struct ipc_handle *phandle);
s32 bsp_ipc_bind(ipc_handle_t phandle, u32 core_mask);
s32 bsp_ipc_open(ipc_res_id_e ipc_res_id, ipc_handle_t *pphandle);
s32 bsp_ipc_connect(ipc_handle_t phandle, ipc_cbk routine, void *parameter);
s32 bsp_ipc_disconnect(ipc_handle_t phandle);
u32 get_modem_recv_reset_flag(void);
u32 get_modem_send_reset_flag(void);

#ifdef __cplusplus
}
#endif /* define __cplusplus */

#endif
