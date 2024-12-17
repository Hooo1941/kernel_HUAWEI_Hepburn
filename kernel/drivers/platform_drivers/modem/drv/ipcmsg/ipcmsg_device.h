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

#ifndef __IPCMSG_DEVICE_H__
#define __IPCMSG_DEVICE_H__

#include "ipcmsg_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IPCMSG_FIFO_LEN 0x800
#define IPCMSG_MBX_ID_MAX 70
#define IPCMSG_CPU_MAX 16
#define IPCMSG_DEVICE_MAX 4
#define IPCMSG_PKT_MAX 8
#define IPCMSG_FUNC_MAX 256
#define IPCMSG_LEN_MAX 352
#define IPCMSG_BUF_MAX 384
#define IPCMSG_REGION_NUM 3
#define IPCMSG_REGION_MBX 32
#define IPCMSG_REG_MAX 12        /* 邮箱寄存器最大个数 */
#define IPCMSG_SEND_MBX_NUM 10
#define IPCMSG_DEVICE_TYPE_NUM 2
#define IPCMSG_DATA_REG_SIZE 4

#define IDLE_STATUS (1UL << 4)
#define SOURCE_STATUS (1UL << 5)
#define DESTINATION_STATUS (1UL << 6)
#define ACK_STATUS (1UL << 7)

#define CHANNEL_OPEN 0
#define CHANNEL_CLOSE 1
#define CHANNEL_BUSY 2

#define IPC_BIT_MASK(n) (1UL << (n))
#define REG_UNLOCK_KEY 0x1ACCE551
#define IPC_MIN(x, y) ((x) < (y) ? (x) : (y))

#define GET_FUNC_ID(chn_id) ((chn_id) & 0xFF)

#define MBX_MODE_SEND(mode) ((mode) & 0x1)
#define MBX_MODE_RECV(mode) ((mode) & 0x2)
#define MBX_MODE_COMM(mode) ((mode) & 0x4)
#define MBX_MODE_BIND(mode) ((mode) & 0x8)

#define RESET_FLAG_MODEM2ACORE 1
#define RESET_FLAG_ACORE2MODEM 2

typedef enum {
    DATA_WITH_HEADER = 0,   /* 数据类非透传发送邮箱 */
    DATA_PASS_THROUGH,      /* 数据类透传发送邮箱 */
    FAST_IPC_IRQ,           /* 快速IPC中断 */
    SENSOR_MODEM_EXCL,      /* sensor-modem 专用邮箱 */
    IPCMSG_MODE_END
} mailbox_mode;

typedef enum {
    MANUAL_ACK = 0,
    AUTO_ACK
} mbox_ack_type;

typedef struct {
    u32 type;
    u32 mode;
    u32 dev_id;
    u32 mbx_id;
    u32 src_cpu;
    u32 dst_cpu;
    u32 irq_no;
    u32 cp_reset;
    void *dev;
    ipcmsg_channel_t *channel;
    spinlock_t mbx_lock;
    u32 *data;
} ipcmsg_mailbox_t;

typedef struct ipcmsg_device ipcmsg_device_t;
typedef struct irq_bundle irq_bundle_t;
struct ipcmsg_device {
    void *base_addr;
    u32 dev_id;
    u32 mbx_num;
    u32 mbx_ptr;
    u32 reg_type;
    u32 data_reg_num;
    u32 data_reg_size;
    irq_bundle_t **irq_bdl;
    ipcmsg_mailbox_t **ipcmsg_mailbox;
    void *of_node;
};

struct irq_bundle {
    ipcmsg_device_t *dev;
    u32 ack_cpu;
};

typedef struct {
    ipcmsg_cb_func ipcmsg_cb;
    void *context;
} ipcmsg_cb_t;

struct ipcmsg_channel {
    u32 chn_id;    /* 业务逻辑号 */
    u32 magic;     /* channel 魔数 */
    u32 chn_state; /* channel 状态 */
    u32 msg_len;   /* 消息长度 */
    u8 *chn_rptr;  /* channel buf指针 */
    union {
        u32 value; /* 通道信息，包含业务id,邮箱号,src_cpu_id,消息优先级等 */
        struct {
            u32 func_id : 8;    /* bit[0-7]  	: 业务通道逻辑号 */
            u32 mbx_id : 8;     /* bit[8-15]  	: 邮箱号 */
            u32 dev_id : 4;     /* bit[16-19]  	: IPCMSG设备号 */
            u32 src_cpu_id : 4; /* bit[20-23]  	: 源核号(区分DSS) */
            u32 version : 4;    /* bit[24-27]  	: 消息头版本 */
            u32 type : 1;       /* bit[28]  	: 0:send 1:recv */
            u32 priority : 1;   /* bit[29]  	: 消息优先级 */
            u32 reserved : 2;   /* bit[30-31]  	: 保留 */
        } chn_info;
    } chn_cfg;
    ipcmsg_cb_t func_cb;
    ipcmsg_mailbox_t *mbx; /* 邮箱基地址 */
    u8 *chn_buf;           /* 通道数据buffer，包含数据头 */
};

typedef struct {
    u32 src_reg;
    u32 dst_reg;
    u32 mask_reg;
    u32 mode_reg;
} ipcmsg_reg_backup_t;

typedef struct {
    u32 reg_idx;
    ipcmsg_reg_backup_t *reg[IPCMSG_SEND_MBX_NUM];   /* 发送邮箱数量增加后要确认此处最大数量是否满足要求 */
} ipcmsg_regs_backup_t;

ipcmsg_device_t *ipcmsg_get_device(u32 dev_id);
ipcmsg_channel_t *ipcmsg_get_channel(u32 func_id);
ipcmsg_channel_t *ipcmsg_channel_get(u32 chn_id);
ipcmsg_mailbox_t *ipcmsg_mailbox_get(u32 dev_id, u32 mbx_id);
void *get_ipcmsg_ns_base_vaddr(void);
void ipcmsg_resetting_handler(void);

#ifdef __cplusplus
}
#endif

#endif
