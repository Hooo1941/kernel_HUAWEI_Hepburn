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

#include "ipcmsg_platform.h"
#include "ipcmsg_core.h"
#include "ipcmsg_device.h"
#include "ipcmsg_driver.h"
#include "ipc_interrupt.h"
#include "ipc_resource.h"

static ipcmsg_reg_t g_ipcmsg_reg_offset[REG_TYPE_NUM] = {
    {0x40, 0x800, 0xA00},
    {0x80, 0x8000, 0xA000},
};

msg_recv_mode_t g_ipcmsg_recv_table[] = {
    {DATA_WITH_HEADER, NULL},  /* 非透传接收 */
    {DATA_PASS_THROUGH, NULL},     /* 透传接收 */
    {FAST_IPC_IRQ, ipcmsg_recv_fastipc},              /* 快速ipc中断接收 */
    {SENSOR_MODEM_EXCL, NULL}, /* sensor-modem 专用邮箱 */
};

/*
 * 功能描述:  配置邮箱通道占用源处理器，配置后写相同值释放该通道邮箱
 */
static __inline__ void __ipc_set_src(void *base, u32 source, u32 mbx_id, u32 reg_type)
{
    writel(source,
        (void *)((uintptr_t)base + IPC_MBX_SOURCE_ADDR(mbx_id, g_ipcmsg_reg_offset[reg_type].mailbox_offset)));
}

/*
 * 功能描述:  读邮箱通道占用源处理器
 */
static __inline__ u32 __ipc_read_src(const void *base, u32 mbx_id, u32 reg_type)
{
    return readl((void *)((uintptr_t)base + IPC_MBX_SOURCE_ADDR(mbx_id, g_ipcmsg_reg_offset[reg_type].mailbox_offset)));
}

/*
 * 功能描述:  邮箱通道目的处理器配置
 */
static __inline__ void __ipc_set_des(void *base, u32 source, u32 mbx_id, u32 reg_type)
{
    writel(source, (void *)((uintptr_t)base + IPC_MBX_DSET_ADDR(mbx_id, g_ipcmsg_reg_offset[reg_type].mailbox_offset)));
}

/*
 * 功能描述:  邮箱通道目的处理器状态，每一bit对应一个处理器。
 */
static __inline__ u32 __ipc_des_status(const void *base, u32 mbx_id, u32 reg_type)
{
    return readl(
        (void *)((uintptr_t)base + IPC_MBX_DSTATUS_ADDR(mbx_id, g_ipcmsg_reg_offset[reg_type].mailbox_offset)));
}

/*
 * 功能描述:  中断发送
 */
static __inline__ void __ipc_send(void *base, u32 source, u32 mbx_id, u32 reg_type)
{
    writel(source, (void *)((uintptr_t)base + IPC_MBX_SEND_ADDR(mbx_id, g_ipcmsg_reg_offset[reg_type].mailbox_offset)));
}

/*
 * 功能描述:  读取数据
 */
static __inline__ u32 __ipc_read(const void *base, u32 mbx_id, u32 cpu, u32 reg_type)
{
    return readl(
        (void *)((uintptr_t)base + IPC_MBX_DATA_ADDR(mbx_id, cpu, g_ipcmsg_reg_offset[reg_type].mailbox_offset)));
}

/*
 * 功能描述:  写数据
 */
static __inline__ void __ipc_write(void *base, u32 data, u32 mbx_id, u32 reg_idx, u32 reg_type)
{
    writel(data,
        (void *)((uintptr_t)base + IPC_MBX_DATA_ADDR(mbx_id, reg_idx, g_ipcmsg_reg_offset[reg_type].mailbox_offset)));
}

/*
 * 功能描述:  获取中断屏蔽状态
 */
static __inline__ u32 __ipc_cpu_imask_get(const void *base, u32 mbx_id, u32 reg_type)
{
    return readl((void *)((uintptr_t)base + IPC_MBX_IMASK_ADDR(mbx_id, g_ipcmsg_reg_offset[reg_type].mailbox_offset)));
}

/*
 * 功能描述:  清中断屏蔽位
 */
static __inline__ void __ipc_cpu_imask_clr(void *base, u32 toclr, u32 mbx_id, u32 reg_type)
{
    unsigned int reg;

    reg = readl((void *)((uintptr_t)base + IPC_MBX_IMASK_ADDR(mbx_id, g_ipcmsg_reg_offset[reg_type].mailbox_offset)));
    reg = reg & (~(toclr));

    writel(reg, (void *)((uintptr_t)base + IPC_MBX_IMASK_ADDR(mbx_id, g_ipcmsg_reg_offset[reg_type].mailbox_offset)));
}

/*
 * 功能描述:  屏蔽所有中断
 */
static __inline__ void __ipc_cpu_imask_all(void *base, u32 mbx_id, u32 reg_type)
{
    writel((~0), (void *)((uintptr_t)base + IPC_MBX_IMASK_ADDR(mbx_id, g_ipcmsg_reg_offset[reg_type].mailbox_offset)));
}

/*
 * 功能描述:  写中断屏蔽寄存器
 */
static __inline__ void __ipc_write_imask(void *base, u32 mask, u32 mbx_id, u32 reg_type)
{
    writel(mask, (void *)((uintptr_t)base + IPC_MBX_IMASK_ADDR(mbx_id, g_ipcmsg_reg_offset[reg_type].mailbox_offset)));
}

/*
 * 功能描述:  清中断状态
 */
static __inline__ void __ipc_cpu_iclr(void *base, u32 toclr, u32 mbx_id, u32 reg_type)
{
    writel(toclr, (void *)((uintptr_t)base + IPC_MBX_ICLR_ADDR(mbx_id, g_ipcmsg_reg_offset[reg_type].mailbox_offset)));
}

/*
 * 功能描述:  获取中断状态
 */
static __inline__ u32 __ipc_cpu_istatus(const void *base, u32 mbx_id, u32 reg_type)
{
    return readl((void *)((uintptr_t)base + IPC_MBX_ICLR_ADDR(mbx_id, g_ipcmsg_reg_offset[reg_type].mailbox_offset)));
}

/*
 * 功能描述:  处理器屏蔽后的中断状态
 */
static __inline__ u32 __ipc_mbox_istatus(const void *base, u32 cpu, u32 region, u32 reg_type)
{
    return readl((void *)((uintptr_t)base + IPC_CPU_IMST_ADDR(cpu, region, g_ipcmsg_reg_offset[reg_type].imst_addr)));
}

/*
 * 功能描述:  获取IPC的状态
 */
static __inline__ u32 __ipc_status(const void *base, u32 mbx_id, u32 reg_type)
{
    return readl((void *)((uintptr_t)base + IPC_MBX_MODE_ADDR(mbx_id, g_ipcmsg_reg_offset[reg_type].mailbox_offset)));
}

/*
 * 功能描述:  ipc模式设置，如自动应答
 */
static __inline__ void __ipc_mode(void *base, u32 mode, u32 mbx_id, u32 reg_type)
{
    writel(mode, (void *)((uintptr_t)base + IPC_MBX_MODE_ADDR(mbx_id, g_ipcmsg_reg_offset[reg_type].mailbox_offset)));
}

/*
 * 功能描述:  解锁寄存器
 */
s32 ipcmsg_device_unlock(void *base, u32 reg_type)
{
    int retry = LOCK_RETRY_TIMES;

    do {
        writel(REG_UNLOCK_KEY, (void *)((uintptr_t)base + IPC_LOCK_ADDR(g_ipcmsg_reg_offset[reg_type].lock_addr)));
        if (0x0 == readl((void *)((uintptr_t)base + IPC_LOCK_ADDR(g_ipcmsg_reg_offset[reg_type].lock_addr)))) {
            break;
        }

        retry--;
    } while (retry);

    if (!retry) {
        return IPCMSG_ERROR;
    }

    return IPCMSG_OK;
}

s32 ipcmsg_send_fastipc(ipcmsg_channel_t *channel, u8 *buf, u32 length)
{
    u32 reg_type, reg_num, mbx_id, timestamp;
    u32 *reg_idx = NULL;
    void *base_addr = NULL;
    ipcmsg_mailbox_t *mailbox = NULL;
    ipcmsg_device_t *device = NULL;
    mailbox = channel->mbx;
    device = (ipcmsg_device_t *)mailbox->dev;
    base_addr = device->base_addr;
    reg_type = device->reg_type;
    reg_num = device->data_reg_num;
    mbx_id = mailbox->mbx_id;
    reg_idx = (u32 *)buf;
    if (*reg_idx >= reg_num) {
        return IPCMSG_ERROR;
    }
    timestamp = bsp_get_slice_value();
    timestamp = timestamp == 0 ? IPCMSG_FAST_IPC : timestamp;
    __ipc_write(base_addr, timestamp, mailbox->mbx_id, *reg_idx, reg_type);
    __ipc_cpu_iclr(base_addr, IPC_BIT_MASK(mailbox->src_cpu), mbx_id, reg_type);
    __ipc_send(base_addr, IPC_BIT_MASK(mailbox->src_cpu), mbx_id, reg_type);
    return IPCMSG_OK;
}

void ipcmsg_recv_fastipc(ipcmsg_mailbox_t *mailbox, u32 *data_reg)
{
    u32 i, reg_num, reset_flag;
    struct ipc_handle *phandle = NULL;
    ipcmsg_channel_dts_t *chn_dts = NULL;
    ipcmsg_channel_t *channel = NULL;
    ipcmsg_device_t *device = NULL;
    if (mailbox == NULL || mailbox->channel == NULL) {
        ipcmsg_print_err("mailbox or channel is NULL!\n");
        return;
    }
    device = (ipcmsg_device_t *)(mailbox->dev);
    channel = mailbox->channel;
    chn_dts = (ipcmsg_channel_dts_t *)(channel->func_cb.context);
    if (data_reg == NULL) {
        ipcmsg_print_err("data register is NULL!\n");
        return;
    }
    if (chn_dts == NULL) {
        ipcmsg_print_err("chn_dts is NULL!\n");
        return;
    }
    reset_flag = get_modem_recv_reset_flag();
    reg_num = device->data_reg_num;
    for (i = 0; i < reg_num; i++) {
        if (data_reg[i] != 0) {
            phandle = &chn_dts->ipc_res[i];
            if (reset_flag && mailbox->cp_reset == RESET_FLAG_MODEM2ACORE && phandle->res_id != IPC_ACPU_INT_SRC_TSP_RESET_DSS0_IDLE) {
                ipcmsg_print_err("modem resetting, not process interrupts! ipc_res_id:0x%x\n", phandle->res_id);
                return;
            }
            if (phandle->magic != IPC_MAGIC) {
                ipcmsg_print_err("phandle magic is wrong! ipc_res_id:0x%x\n", phandle->res_id);
                return;
            }
            if (phandle->ipc_int_table.routine != NULL) {
                phandle->ipc_int_table.routine(phandle->ipc_int_table.arg);
            } else {
                ipcmsg_print_err("IpcIntHandler:No IntConnect,MDRV_ERROR! ipc_res_id =%d, reg_idx %d\n",
                    phandle->res_id, phandle->reg_idx);
            }
            phandle->handle_times++;
        }
    }
}

u32 match_mailbox_no(u32 irq_status, u32 region)
{
    u32 mbx_id = region * IPCMSG_REGION_MBX;
    if (irq_status == 0) {
        ipcmsg_print_err("irq_status err,not match!\n");
        return IPCMSG_EINTRAW;
    }
    if ((irq_status & 0xFFFF) == 0) {
        mbx_id += 0x10;
        irq_status >>= 0x10;
    }
    if ((irq_status & 0xFF) == 0) {
        mbx_id += 0x8;
        irq_status >>= 0x8;
    }
    if ((irq_status & 0xF) == 0) {
        mbx_id += 0x4;
        irq_status >>= 0x4;
    }
    if ((irq_status & 0x3) == 0) {
        mbx_id += 0x2;
        irq_status >>= 0x2;
    }
    if ((irq_status & 0x1) == 0) {
        mbx_id += 0x1;
    }
    return mbx_id;
}

void ipcmsg_mailbox_resume(ipcmsg_mailbox_t *mailbox, ipcmsg_regs_backup_t *ipcmsg_regs_backup)
{
    u32 mbx_id, reg_type, reg_idx, src_status;
    ipcmsg_device_t *device = NULL;
    void *base_addr = NULL;
    if (mailbox == NULL || ipcmsg_regs_backup == NULL) {
        return;
    }
    device = (ipcmsg_device_t *)mailbox->dev;
    base_addr = device->base_addr;
    reg_type = device->reg_type;
    mbx_id = mailbox->mbx_id;
    reg_idx = ipcmsg_regs_backup->reg_idx;
    /* 如果读源处理器的值不为0，说明已在其他核配置过，此处跳过 */
    src_status = __ipc_read_src(base_addr, mbx_id, reg_type);
    if (src_status != 0) {
        return;
    }
    __ipc_set_src(base_addr, ipcmsg_regs_backup->reg[reg_idx]->src_reg, mbx_id, reg_type);
    __ipc_write_imask(base_addr, ipcmsg_regs_backup->reg[reg_idx]->mask_reg, mbx_id, reg_type);
    __ipc_set_des(base_addr, ipcmsg_regs_backup->reg[reg_idx]->dst_reg, mbx_id, reg_type);
    __ipc_mode(base_addr, ipcmsg_regs_backup->reg[reg_idx]->mode_reg, mbx_id, reg_type);
    return;
}

void ipcmsg_mailbox_prepare(ipcmsg_mailbox_t *mailbox, ipcmsg_regs_backup_t *ipcmsg_regs_backup)
{
    u32 mask, mbx_id, reg_type, reg_idx, src_status;
    ipcmsg_device_t *device = NULL;
    void *base_addr = NULL;
    device = (ipcmsg_device_t *)mailbox->dev;
    base_addr = device->base_addr;
    reg_type = device->reg_type;
    mbx_id = mailbox->mbx_id;
    reg_idx = ipcmsg_regs_backup->reg_idx;
    /* 对各个发送邮箱进行初始化配置，并将配置值写备份 */
    /* 如果读源处理器的值不为0，说明已在其他核配置过，此处跳过 */
    src_status = __ipc_read_src(base_addr, mbx_id, reg_type);
    if (src_status != 0) {
        return;
    }
    /* 配置及备份源处理器 */
    __ipc_set_src(base_addr, IPC_BIT_MASK(mailbox->src_cpu), mbx_id, reg_type);
    ipcmsg_regs_backup->reg[reg_idx]->src_reg = IPC_BIT_MASK(mailbox->src_cpu);
    /* 配置及备份中断屏蔽器 */
    mask = IPCMSG_IMASK_ALL & (~(IPC_BIT_MASK(mailbox->dst_cpu)));
    __ipc_write_imask(base_addr, mask, mbx_id, reg_type);
    ipcmsg_regs_backup->reg[reg_idx]->mask_reg = mask;
    /* 配置及备份目的处理器, 仅普通邮箱需要 */
    if (MBX_MODE_COMM(mailbox->mode) != 0) {
        __ipc_set_des(base_addr, IPC_BIT_MASK(mailbox->dst_cpu), mbx_id, reg_type);
        ipcmsg_regs_backup->reg[reg_idx]->dst_reg = IPC_BIT_MASK(mailbox->dst_cpu);
    }
    /* 配置及备份模式寄存器 */
    __ipc_mode(base_addr, AUTO_ACK, mbx_id, reg_type);
    ipcmsg_regs_backup->reg[reg_idx]->mode_reg = AUTO_ACK;
    ipcmsg_regs_backup->reg_idx++;
    return;
}

/*
 * 功能描述:  ipcmsg 普通中断处理函数
 */
irqreturn_t ipcmsg_common_handler(int irq_no, void *arg)
{
    u32 irq_status, ipc_status;
    u32 core_id, mbx_id, region, reg_type;
    ipcmsg_mailbox_t *mbx = NULL;
    irq_bundle_t *irq_bdl = (irq_bundle_t *)arg;
    ipcmsg_device_t *dev = irq_bdl->dev;
    reg_type = dev->reg_type;
    core_id = irq_bdl->ack_cpu;
    for (region = 0; region < IPCMSG_REGION_NUM; region++) {
        irq_status = __ipc_mbox_istatus(dev->base_addr, core_id, region, reg_type);
        if (irq_status != 0) {
            break;
        }
    }
    mbx_id = match_mailbox_no(irq_status, region);
    if (mbx_id == IPCMSG_EINTRAW) {
        return IRQ_NONE;
    }
    ipc_status = __ipc_status(dev->base_addr, mbx_id, reg_type);
    if ((ipc_status & ACK_STATUS) != 0) {
        /* receive other mailbox ack irq */
        ipcmsg_print_err("Ack irq\n");
        /* clear irq */
        ipcmsg_ack(irq_bdl, mbx_id);
    } else {
        /* receive normal mailbox data */
        if ((ipc_status & DESTINATION_STATUS) != 0) {
            ipcmsg_print_err("RecvNormal\n");
            mbx = ipcmsg_mailbox_get(dev->dev_id, mbx_id);
            if (mbx != NULL) {
                ipcmsg_mailbox_recv(mbx);
            }
        }
    }
    return IRQ_HANDLED;
}

void ipcmsg_mailbox_recv(ipcmsg_mailbox_t *mailbox)
{
    u32 mailbox_reg[IPCMSG_REG_MAX] = {0};
    u32 i;
    ipcmsg_device_t *device = NULL;
    device = (ipcmsg_device_t *)(mailbox->dev);
    if (mailbox->type ==
        FAST_IPC_IRQ) { /* 快速中断,业务回调与数据寄存器编号绑定,处理时要先清中断,保证对端核写中断发送寄存器能生效 */
        /* clear irq */
        __ipc_cpu_iclr(device->base_addr, IPC_BIT_MASK(mailbox->dst_cpu), mailbox->mbx_id, device->reg_type);
        /* read data from ipc reg */
        for (i = 0; i < device->data_reg_num; i++) {
            mailbox_reg[i] = __ipc_read(device->base_addr, mailbox->mbx_id, i, device->reg_type);
            if (mailbox_reg[i] != 0) { /* 避免清除发送端正在写入的数据寄存器 */
                                       /* clear ipc data reg */
                __ipc_write(device->base_addr, 0, mailbox->mbx_id, i, device->reg_type);
            }
        }
    } else if (mailbox->type < IPCMSG_MODE_END) { /* 消息接收中断,处理时要先把数据从数据寄存器取走再清中断,避免其他核判断邮箱可用后覆盖写数据寄存器
                                                   */
        /* read data from ipc reg */
        for (i = 0; i < device->data_reg_num; i++) {
            mailbox_reg[i] = __ipc_read(device->base_addr, mailbox->mbx_id, i, device->reg_type);
            /* clear ipc data reg */
            __ipc_write(device->base_addr, 0, mailbox->mbx_id, i, device->reg_type);
        }
        /* clear irq */
        __ipc_cpu_iclr(device->base_addr, IPC_BIT_MASK(mailbox->dst_cpu), mailbox->mbx_id, device->reg_type);
    } else { /* 异常分支，邮箱类型大于枚举最大值 */
        return;
    }
    if (g_ipcmsg_recv_table[mailbox->type].recv_cb == NULL) {
        ipcmsg_print_err("recv_cb is NULL.\n");
        return;
    }
    g_ipcmsg_recv_table[mailbox->type].recv_cb(mailbox, mailbox_reg);
    return;
}

/*
 * 功能描述:  ipcmsg邮箱中断处理函数
 */
irqreturn_t ipcmsg_mailbox_handler(int irq_no, void *arg)
{
    u32 ipc_status, reg_type;
    ipcmsg_device_t *device = NULL;
    ipcmsg_mailbox_t *mbx = (ipcmsg_mailbox_t *)arg;
    device = (ipcmsg_device_t *)(mbx->dev);
    reg_type = device->reg_type;
    ipc_status = __ipc_status(device->base_addr, mbx->mbx_id, reg_type);
    if ((ipc_status & DESTINATION_STATUS) != 0) {
        ipcmsg_print_dbg("DESTINATION_STATUS\n");
        ipcmsg_mailbox_recv(mbx);
    }
    return IRQ_HANDLED;
}

/*
 * 功能描述:  回ack模式清中断、释放邮箱
 */
void ipcmsg_ack(irq_bundle_t *irq_bdl, u32 mbx_id)
{
    u32 core_id, dev_id;
    ipcmsg_device_t *device = NULL;
    core_id = irq_bdl->ack_cpu;
    dev_id = irq_bdl->dev->dev_id;
    device = ipcmsg_get_device(dev_id);
    if (device == NULL) {
        return;
    }
    /* clear irq */
    __ipc_cpu_iclr(device->base_addr, IPC_BIT_MASK(core_id), mbx_id, device->reg_type);

    /* release mailbox */
    __ipc_set_src(device->base_addr, IPC_BIT_MASK(core_id), mbx_id, device->reg_type);
}

/*
 * 功能描述:  单独复位中，清除邮箱中断
 */
void set_mailbox_into_reset_state(ipcmsg_mailbox_t *mailbox)
{
    ipcmsg_device_t *device = NULL;
    device = (ipcmsg_device_t *)(mailbox->dev);
    __ipc_cpu_iclr(device->base_addr, IPC_BIT_MASK(mailbox->dst_cpu), mailbox->mbx_id, device->reg_type);
}
