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

#ifndef __IPCMSG_DRIVER_H__
#define __IPCMSG_DRIVER_H__

#include "ipcmsg_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * (1/1) register_define
 */
/*
 * 寄存器说明：源处理器配置寄存器。
 * 位域定义UNION结构:  IPC_MBX_SOURCE_UNION
 */
#define IPC_MBX_SOURCE_ADDR(mbx_id, offset) (0x000 + (mbx_id) * (offset))

/*
 * 寄存器说明：目的处理器设置寄存器。
 * 位域定义UNION结构:  IPC_MBX_DSET_UNION
 */
#define IPC_MBX_DSET_ADDR(mbx_id, offset) (0x004 + (mbx_id) * (offset))

/*
 * 寄存器说明：目的处理器清除寄存器。
 * 位域定义UNION结构:  IPC_MBX_DCLEAR_UNION
 */
#define IPC_MBX_DCLEAR_ADDR(mbx_id, offset) (0x008 + (mbx_id) * (offset))

/*
 * 寄存器说明：目的处理器状态寄存器。
 * 位域定义UNION结构:  IPC_MBX_DSTATUS_UNION
 */
#define IPC_MBX_DSTATUS_ADDR(mbx_id, offset) (0x00C + (mbx_id) * (offset))

/*
 * 寄存器说明：邮箱通道模式状态机状态寄存器。
 * 位域定义UNION结构:  IPC_MBX_MODE_UNION
 */
#define IPC_MBX_MODE_ADDR(mbx_id, offset) (0x010 + (mbx_id) * (offset))

/*
 * 寄存器说明：处理器中断屏蔽寄存器。
 * 位域定义UNION结构:  IPC_MBX_IMASK_UNION
 */
#define IPC_MBX_IMASK_ADDR(mbx_id, offset) (0x014 + (mbx_id) * (offset))

/*
 * 寄存器说明：处理器中断状态清除寄存器。
 * 位域定义UNION结构:  IPC_MBX_ICLR_UNION
 */
#define IPC_MBX_ICLR_ADDR(mbx_id, offset) (0x018 + (mbx_id) * (offset))

/*
 * 寄存器说明：发送寄存器。
 * 位域定义UNION结构:  IPC_MBX_SEND_UNION
 */
#define IPC_MBX_SEND_ADDR(mbx_id, offset) (0x01C + (mbx_id) * (offset))

/*
 * 寄存器说明：数据寄存器。
 * 位域定义UNION结构:  IPC_MBX_DATA0_UNION
 */
#define IPC_MBX_DATA_ADDR(mbx_id, reg_idx, offset) (0x020 + (mbx_id) * (offset) + ((reg_idx) << 2))

/*
 * 寄存器说明：中断屏蔽状态寄存器。
 * 位域定义UNION结构:  IPC_CPU_IMST_UNION
 */
#define IPC_CPU_IMST_ADDR(cpu_id, region, addr) ((addr) + (cpu_id) * 8 + (region) * 256)

/*
 * 寄存器说明：中断原始状态寄存器。
 * 位域定义UNION结构:  IPC_CPU_IRST_UNION
 */
#define IPC_CPU_IRST_ADDR(cpu_id, region, addr) ((addr) + (cpu_id) * 8 + (region) * 256)

/*
 * 寄存器说明：IPC lock寄存器。
 * 位域定义UNION结构:  IPC_LOCK_UNION
 */
#define IPC_LOCK_ADDR(offset) (offset)

#define GET_IPCMBX_NUM(length) ((((length) - 1) >> 2) + 1)

#define IPCMSG_EINTRAW 0xFFFFFFFF
#define REG_TYPE_NUM 2
#define MBX_RETRY_TIMES 3000
#define MBX_DELAY_TIME 100
#define LOCK_RETRY_TIMES 3
#define IPCMSG_FAST_IPC 0xFFFFABCD
#define IPCMSG_IMASK_ALL 0xFFFFFFFF

typedef struct {
    u32 mailbox_offset;
    u32 imst_addr;
    u32 lock_addr;
} ipcmsg_reg_t;

typedef void (*recv_func_t)(ipcmsg_mailbox_t *mailbox, u32 *data_reg);

typedef struct {
    u32 type;             /* 邮箱type */
    recv_func_t recv_cb;  /* 接收回调函数 */
} msg_recv_mode_t;

void ipcmsg_ack(irq_bundle_t *irq_bdl, u32 mbx_id);
void ipcmsg_mailbox_recv(ipcmsg_mailbox_t *mailbox);
irqreturn_t ipcmsg_common_handler(int irq_no, void *arg);
irqreturn_t ipcmsg_mailbox_handler(int irq_no, void *arg);
s32 ipcmsg_send_fastipc(ipcmsg_channel_t *channel, u8 *buf, u32 length);
void ipcmsg_recv_fastipc(ipcmsg_mailbox_t *mailbox, u32 *data_reg);
s32 ipcmsg_device_unlock(void *base, u32 reg_type);
void ipcmsg_mailbox_prepare(ipcmsg_mailbox_t *mailbox, ipcmsg_regs_backup_t *ipcmsg_regs_backup);
void ipcmsg_mailbox_resume(ipcmsg_mailbox_t *mailbox, ipcmsg_regs_backup_t *ipcmsg_regs_backup);
void set_mailbox_into_reset_state(ipcmsg_mailbox_t *mailbox);

#ifdef __cplusplus
}
#endif

#endif
