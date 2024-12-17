/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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

#ifndef _FIPC_DRIVER_H
#define _FIPC_DRIVER_H

#include "fipc_platform.h"
#include "fipc_device.h"

/* offset register_define */
#define FIPC_V200_OFFSET 0x0080
#define FIPC_V300_OFFSET 0x0400

#define FIPC_HW_VERSION_V300 0x33303061
#define FIPC_FIPC_OPIPE_STAT0 0x0050 /* FIPC状态寄存器 */
#define FIPC_FIPC_OPIPE_STAT1 0x0054 /* FIPC状态寄存器 */
#define FIPC_FIPC_IPIPE_STAT0 0x0058 /* FIPC状态寄存器 */
#define FIPC_FIPC_IPIPE_STAT1 0x005C /* FIPC状态寄存器 */

#define FIPC_CORE_OPIPE_INT_MASKSET0(cpu_id, regsoffset) (0x0000 + (regsoffset) + 0x40L * (cpu_id)) /* COREx OPIPE中断屏蔽设置寄存器 */
#define FIPC_CORE_OPIPE_INT_MASKSET1(cpu_id, regsoffset) (0x0004 + (regsoffset) + 0x40L * (cpu_id)) /* COREx OPIPE中断屏蔽设置寄存器 */
#define FIPC_CORE_IPIPE_INT_MASKSET0(cpu_id, regsoffset) (0x0008 + (regsoffset) + 0x40L * (cpu_id)) /* COREx IPIPE中断屏蔽设置寄存器 */
#define FIPC_CORE_IPIPE_INT_MASKSET1(cpu_id, regsoffset) (0x000C + (regsoffset) + 0x40L * (cpu_id)) /* COREx IPIPE中断屏蔽设置寄存器 */

#define FIPC_CORE_OPIPE_INT_MASKCLR0(cpu_id, regsoffset) (0x0010 + (regsoffset) + 0x40L * (cpu_id)) /* COREx OPIPE中断屏蔽清除寄存器 */
#define FIPC_CORE_OPIPE_INT_MASKCLR1(cpu_id, regsoffset) (0x0014 + (regsoffset) + 0x40L * (cpu_id)) /* COREx OPIPE中断屏蔽清除寄存器 */
#define FIPC_CORE_IPIPE_INT_MASKCLR0(cpu_id, regsoffset) (0x0018 + (regsoffset) + 0x40L * (cpu_id)) /* COREx IPIPE中断屏蔽清除寄存器 */
#define FIPC_CORE_IPIPE_INT_MASKCLR1(cpu_id, regsoffset) (0x001C + (regsoffset) + 0x40L * (cpu_id)) /* COREx IPIPE中断屏蔽清除寄存器 */

#define FIPC_CORE_OPIPE_INT_MASK0(cpu_id, regsoffset) (0x0020 + (regsoffset) + 0x40L * (cpu_id)) /* COREx OPIPE中断屏蔽寄存器 */
#define FIPC_CORE_OPIPE_INT_MASK1(cpu_id, regsoffset) (0x0024 + (regsoffset) + 0x40L * (cpu_id)) /* COREx OPIPE中断屏蔽寄存器 */
#define FIPC_CORE_IPIPE_INT_MASK0(cpu_id, regsoffset) (0x0028 + (regsoffset) + 0x40L * (cpu_id)) /* COREx IPIPE中断屏蔽寄存器 */
#define FIPC_CORE_IPIPE_INT_MASK1(cpu_id, regsoffset) (0x002C + (regsoffset) + 0x40L * (cpu_id)) /* COREx IPIPE中断屏蔽寄存器 */

#define FIPC_CORE_OPIPE_INT_STAT0(cpu_id, regsoffset) (0x0030 + (regsoffset) + 0x40L * (cpu_id)) /* COREx OPIPE中断状态寄存器 */
#define FIPC_CORE_OPIPE_INT_STAT1(cpu_id, regsoffset) (0x0034 + (regsoffset) + 0x40L * (cpu_id)) /* COREx OPIPE中断状态寄存器 */
#define FIPC_CORE_IPIPE_INT_STAT0(cpu_id, regsoffset) (0x0038 + (regsoffset) + 0x40L * (cpu_id)) /* COREx IPIPE中断状态寄存器 */
#define FIPC_CORE_IPIPE_INT_STAT1(cpu_id, regsoffset) (0x003C + (regsoffset) + 0x40L * (cpu_id)) /* COREx IPIPE中断状态寄存器 */

#define FIPC_OPIPE_BASE_ADDR_L(opipe_id) (0x4000 + 0x80L * (opipe_id)) /* OPIPE起始地址寄存器低位 */
#define FIPC_OPIPE_BASE_ADDR_H(opipe_id) (0x4004 + 0x80L * (opipe_id)) /* OPIPE起始地址寄存器高位 */
#define FIPC_OPIPE_DEPTH(opipe_id) (0x4008 + 0x80L * (opipe_id))       /* OPIPE深度寄存器 */
#define FIPC_OPIPE_WPTR(opipe_id) (0x400C + 0x80L * (opipe_id))        /* OPIPE写指针寄存器 */
#define FIPC_OPIPE_RPTR(opipe_id) (0x4010 + 0x80L * (opipe_id))        /* OPIPE读指针寄存器 */
#define FIPC_OPIPE_PTR_ADDR_L(opipe_id) (0x4018 + 0x80L * (opipe_id))  /* OPIPE指针更新地址寄存器低位 */
#define FIPC_OPIPE_PTR_ADDR_H(opipe_id) (0x401C + 0x80L * (opipe_id))  /* OPIPE指针更新地址寄存器高位 */
#define FIPC_OPIPE_INT_MASK(opipe_id) (0x4040 + 0x80L * (opipe_id))    /* OPIPE中断屏蔽寄存器 */
#define FIPC_OPIPE_INT_STAT(opipe_id) (0x4044 + 0x80L * (opipe_id))    /* OPIPE中断状态寄存器 */
#define FIPC_OPIPE_INT_RAW(opipe_id) (0x4048 + 0x80L * (opipe_id))     /* OPIPE中断原始状态寄存器 */
#define FIPC_OPIPE_CTRL(opipe_id) (0x404C + 0x80L * (opipe_id))        /* OPIPE控制寄存器 */
#define FIPC_OPIPE_STAT(opipe_id) (0x4050 + 0x80L * (opipe_id))        /* OPIPE状态寄存器 */

#define FIPC_IPIPE_BASE_ADDR_L(ipipe_id) (0x6000 + 0x80L * (ipipe_id)) /* IPIPE起始地址寄存器低位 */
#define FIPC_IPIPE_BASE_ADDR_H(ipipe_id) (0x6004 + 0x80L * (ipipe_id)) /* IPIPE起始地址寄存器高位 */
#define FIPC_IPIPE_DEPTH(ipipe_id) (0x6008 + 0x80L * (ipipe_id))       /* IPIPE深度寄存器 */
#define FIPC_IPIPE_WPTR(ipipe_id) (0x600C + 0x80L * (ipipe_id))        /* IPIPE写指针寄存器 */
#define FIPC_IPIPE_RPTR(ipipe_id) (0x6010 + 0x80L * (ipipe_id))        /* IPIPE读指针寄存器 */
#define FIPC_IPIPE_REMOTE_CTRL(ipipe_id) (0x6014 + 0x80L * (ipipe_id)) /* IPIPE remote控制寄存器	 */
#define FIPC_IPIPE_PTR_ADDR_L(ipipe_id) (0x6018 + 0x80L * (ipipe_id))  /* IPIPE指针更新地址寄存器低位 */
#define FIPC_IPIPE_PTR_ADDR_H(ipipe_id) (0x601C + 0x80L * (ipipe_id))  /* IPIPE指针更新地址寄存器高位 */
#define FIPC_IPIPE_CFG_ADDR_L(ipipe_id) (0x6020 + 0x80L * (ipipe_id))  /* IPIPE指针更新地址寄存器低位 */
#define FIPC_IPIPE_CFG_ADDR_H(ipipe_id) (0x6024 + 0x80L * (ipipe_id))  /* IPIPE指针更新地址寄存器高位 */
#define FIPC_IPIPE_INT_MASK(ipipe_id) (0x6040 + 0x80L * (ipipe_id))    /* IPIPE中断屏蔽寄存器 */
#define FIPC_IPIPE_INT_STAT(ipipe_id) (0x6044 + 0x80L * (ipipe_id))    /* IPIPE中断状态寄存器 */
#define FIPC_IPIPE_INT_RAW(ipipe_id) (0x6048 + 0x80L * (ipipe_id))     /* IPIPE中断原始状态寄存器 */
#define FIPC_IPIPE_CTRL(ipipe_id) (0x604C + 0x80L * (ipipe_id))        /* IPIPE控制寄存器 */
#define FIPC_IPIPE_STAT(ipipe_id) (0x6050 + 0x80L * (ipipe_id))        /* IPIPE状态寄存器 */

/* -------------------STAT BITS FIELD----------------------------- */
#define PIPE_STATUS_EMPTY (1 << 0)
#define PIPE_STATUS_FULL (1 << 1)
#define PIPE_STATUS_IDLE (1 << 2)

#define FIPC_PIPE_CNT_PER_REG 32

int fipc_irqs_init(void);

int ipipe_neg_check(const void *base_addr, u32 ipipe_id);
int ipipe_status_check(const void *base_addr, u32 ipipe_id);
int opipe_status_check(const void *base_addr, u32 opipe_id);

void fipc_opipe_startup(void *base_addr, struct opipe_config *opipe);
void fipc_ipipe_startup(void *base_addr, struct ipipe_config *ipipe);

/*lint -esym(528,fipc_opipe_is_en) */
static inline u32 fipc_opipe_is_en(const void *base_addr, u32 pipe_id)
{
    u32 status;
    status = readl((void *)((char *)base_addr + FIPC_OPIPE_CTRL(pipe_id)));
    return (status & 1);
}

/*lint -esym(528,fipc_opipe_local_en) */
static inline void fipc_opipe_local_en(void *base_addr, u32 pipe_id)
{
    u32 status;

    status = readl((void *)((char *)base_addr + FIPC_OPIPE_CTRL(pipe_id)));
    writel(status | 1, (void *)((char *)base_addr + FIPC_OPIPE_CTRL(pipe_id)));
}

/*lint -esym(528,fipc_opipe_local_dis) */
static inline void fipc_opipe_local_dis(void *base_addr, u32 pipe_id)
{
    u32 status;

    status = readl((void *)((char *)base_addr + FIPC_OPIPE_CTRL(pipe_id)));
    writel(status & (~1), (void *)((char *)base_addr + FIPC_OPIPE_CTRL(pipe_id)));
}

/*lint -esym(528,fipc_opipe_abort_set) */
static inline void fipc_opipe_abort_set(void *base_addr, u32 pipe_id)
{
    u32 status;

    status = readl((void *)((char *)base_addr + FIPC_OPIPE_CTRL(pipe_id)));
    writel(status | 0x100, (void *)((char *)base_addr + FIPC_OPIPE_CTRL(pipe_id)));
}

/*lint -esym(528,fipc_opipe_abort_clr) */
static inline void fipc_opipe_abort_clr(void *base_addr, u32 pipe_id)
{
    u32 status;

    status = readl((void *)((char *)base_addr + FIPC_OPIPE_CTRL(pipe_id)));
    writel(status & (~0x100), (void *)((char *)base_addr + FIPC_OPIPE_CTRL(pipe_id)));
}

/*lint -esym(528,fipc_ipipe_is_en) */
static inline u32 fipc_ipipe_is_en(const void *base_addr, u32 pipe_id)
{
    u32 status;
    status = readl((void *)((char *)base_addr + FIPC_IPIPE_CTRL(pipe_id)));
    return (status & 1);
}

/*lint -esym(528,fipc_ipipe_local_en) */
static inline void fipc_ipipe_local_en(void *base_addr, u32 pipe_id)
{
    u32 status;

    status = readl((void *)((char *)base_addr + FIPC_IPIPE_CTRL(pipe_id)));
    writel(status | 1, (void *)((char *)base_addr + FIPC_IPIPE_CTRL(pipe_id)));
}

/*lint -esym(528,fipc_ipipe_local_dis) */
static inline void fipc_ipipe_local_dis(void *base_addr, u32 pipe_id)
{
    u32 status;

    status = readl((void *)((char *)base_addr + FIPC_IPIPE_CTRL(pipe_id)));
    writel(status & (~1), (void *)((char *)base_addr + FIPC_IPIPE_CTRL(pipe_id)));
}

/*lint -esym(528,fipc_ipipe_remote_status) */
static inline u32 fipc_ipipe_remote_status(const void *base_addr, u32 pipe_id)
{
    u32 status;
    status = readl((void *)((char *)base_addr + FIPC_IPIPE_REMOTE_CTRL(pipe_id)));
    return status;
}

/*lint -esym(528,fipc_ipipe_remote_en) */
static inline void fipc_ipipe_remote_en(void *base_addr, u32 pipe_id)
{
    writel(1, (void *)((char *)base_addr + FIPC_IPIPE_REMOTE_CTRL(pipe_id)));
}

/*lint -esym(528,fipc_opipe_coreint_en) */
static inline void fipc_opipe_coreint_en(void *base_addr, u32 pipe_id, u32 cpu_id, u32 regsoffset)
{
    if (pipe_id < FIPC_PIPE_CNT_PER_REG) {
        writel(1UL << pipe_id, (void *)((char *)base_addr + FIPC_CORE_OPIPE_INT_MASKSET0(cpu_id, regsoffset)));
    } else {
        writel(1UL << (pipe_id - FIPC_PIPE_CNT_PER_REG),
            (void *)((char *)base_addr + FIPC_CORE_OPIPE_INT_MASKSET1(cpu_id, regsoffset)));
    }
}

/*lint -esym(528,fipc_opipe_coreint_dis) */
static inline void fipc_opipe_coreint_dis(void *base_addr, u32 pipe_id, u32 cpu_id, u32 regsoffset)
{
    if (pipe_id < FIPC_PIPE_CNT_PER_REG) {
        writel(1UL << pipe_id, (void *)((char *)base_addr + FIPC_CORE_OPIPE_INT_MASKCLR0(cpu_id, regsoffset)));
    } else {
        writel(1UL << (pipe_id - FIPC_PIPE_CNT_PER_REG),
            (void *)((char *)base_addr + FIPC_CORE_OPIPE_INT_MASKCLR1(cpu_id, regsoffset)));
    }
}

/*lint -esym(528,fipc_ipipe_coreint_en) */
static inline void fipc_ipipe_coreint_en(void *base_addr, u32 pipe_id, u32 cpu_id, u32 regsoffset)
{
    if (pipe_id < FIPC_PIPE_CNT_PER_REG) {
        writel(1UL << pipe_id, (void *)((char *)base_addr + FIPC_CORE_IPIPE_INT_MASKSET0(cpu_id, regsoffset)));
    } else {
        writel(1UL << (pipe_id - FIPC_PIPE_CNT_PER_REG),
            (void *)((char *)base_addr + FIPC_CORE_IPIPE_INT_MASKSET1(cpu_id, regsoffset)));
    }
}

/*lint -esym(528,fipc_ipipe_coreint_dis) */
static inline void fipc_ipipe_coreint_dis(void *base_addr, u32 pipe_id, u32 cpu_id, u32 regsoffset)
{
    if (pipe_id < FIPC_PIPE_CNT_PER_REG) {
        writel(1UL << pipe_id, (void *)((char *)base_addr + FIPC_CORE_IPIPE_INT_MASKCLR0(cpu_id, regsoffset)));
    } else {
        writel(1UL << (pipe_id - FIPC_PIPE_CNT_PER_REG),
            (void *)((char *)base_addr + FIPC_CORE_IPIPE_INT_MASKCLR1(cpu_id, regsoffset)));
    }
}

/*lint -esym(528,fipc_opipe_stat_isbusy) */
static inline int fipc_opipe_stat_isbusy(const void *base_addr, u32 pipe_id)
{
    u32 status;

    if (pipe_id < FIPC_PIPE_CNT_PER_REG) {
        status = readl((void *)((char *)base_addr + FIPC_FIPC_OPIPE_STAT0));
        status &= (1UL << pipe_id);
        if (!status) {
            return 1;
        }
    } else {
        status = readl((void *)((char *)base_addr + FIPC_FIPC_OPIPE_STAT1));
        status &= (1UL << (pipe_id - FIPC_PIPE_CNT_PER_REG));
        if (!status) {
            return 1;
        }
    }
    return 0;
}

/*lint -esym(528,fipc_ipipe_stat_isbusy) */
static inline int fipc_ipipe_stat_isbusy(const void *base_addr, u32 pipe_id)
{
    u32 status;

    if (pipe_id < FIPC_PIPE_CNT_PER_REG) {
        status = readl((void *)((char *)base_addr + FIPC_FIPC_IPIPE_STAT0));
        status &= (1UL << pipe_id);
        if (!status) {
            return 1;
        }
    } else {
        status = readl((void *)((char *)base_addr + FIPC_FIPC_IPIPE_STAT1));
        status &= (1UL << (pipe_id - FIPC_PIPE_CNT_PER_REG));
        if (!status) {
            return 1;
        }
    }
    return 0;
}

/*lint -esym(528,fipc_opipe_devint_en) */
static inline u32 fipc_opipe_devint_en(void *base_addr, u32 pipe_id, u32 level)
{
    u32 reg;

    reg = readl((void *)((char *)base_addr + FIPC_OPIPE_INT_MASK(pipe_id)));
    writel(reg | level, (void *)((char *)base_addr + FIPC_OPIPE_INT_MASK(pipe_id)));

    return (reg | level);
}

/*lint -esym(528,fipc_opipe_devint_dis) */
static inline u32 fipc_opipe_devint_dis(void *base_addr, u32 pipe_id, u32 level)
{
    u32 reg;

    reg = readl((void *)((char *)base_addr + FIPC_OPIPE_INT_MASK(pipe_id)));
    reg &= (~level);
    writel(reg, (void *)((char *)base_addr + FIPC_OPIPE_INT_MASK(pipe_id)));

    return reg;
}

/*lint -esym(528,fipc_opipe_devint_stat) */
static inline u32 fipc_opipe_devint_stat(const void *base_addr, u32 pipe_id)
{
    return readl((void *)((char *)base_addr + FIPC_OPIPE_INT_STAT(pipe_id)));
}

/*lint -esym(528,fipc_opipe_devint_raw) */
static inline u32 fipc_opipe_devint_raw(const void *base_addr, u32 pipe_id)
{
    return readl((void *)((char *)base_addr + FIPC_OPIPE_INT_RAW(pipe_id)));
}

/*lint -esym(528,fipc_opipe_devint_clr) */
static inline void fipc_opipe_devint_clr(void *base_addr, u32 pipe_id, u32 level)
{
    writel(level, (void *)((char *)base_addr + FIPC_OPIPE_INT_RAW(pipe_id)));
}

/*lint -esym(528,fipc_ipipe_devint_en) */
static inline u32 fipc_ipipe_devint_en(void *base_addr, u32 pipe_id, u32 level)
{
    u32 reg;

    reg = readl((void *)((char *)base_addr + FIPC_IPIPE_INT_MASK(pipe_id)));
    writel(reg | level, (void *)((char *)base_addr + FIPC_IPIPE_INT_MASK(pipe_id)));

    return (reg | level);
}

/*lint -esym(528,fipc_ipipe_devint_dis) */
static inline u32 fipc_ipipe_devint_dis(void *base_addr, u32 pipe_id, u32 level)
{
    u32 reg;

    reg = readl((void *)((char *)base_addr + FIPC_IPIPE_INT_MASK(pipe_id)));
    reg &= (~level);
    writel(reg, (void *)((char *)base_addr + FIPC_IPIPE_INT_MASK(pipe_id)));

    return reg;
}

/*lint -esym(528,fipc_ipipe_devint_stat) */
static inline u32 fipc_ipipe_devint_stat(const void *base_addr, u32 pipe_id)
{
    return readl((void *)((char *)base_addr + FIPC_IPIPE_INT_STAT(pipe_id)));
}

/*lint -esym(528,fipc_ipipe_devint_raw) */
static inline u32 fipc_ipipe_devint_raw(const void *base_addr, u32 pipe_id)
{
    return readl((void *)((char *)base_addr + FIPC_IPIPE_INT_RAW(pipe_id)));
}

/*lint -esym(528,fipc_ipipe_devint_clr) */
static inline void fipc_ipipe_devint_clr(void *base_addr, u32 pipe_id, u32 level)
{
    writel(level, (void *)((char *)base_addr + FIPC_IPIPE_INT_RAW(pipe_id)));
}

/*lint -esym(528,fipc_opipe_wptr_update) */
static inline void fipc_opipe_wptr_update(void *base_addr, u32 pipe_id, u32 wptr)
{
    writel(wptr, (void *)((char *)base_addr + FIPC_OPIPE_WPTR(pipe_id)));
}

/*lint -esym(528,fipc_opipe_rptr_update) */
static inline void fipc_opipe_rptr_update(void *base_addr, u32 pipe_id, u32 rptr)
{
    writel(rptr, (void *)((char *)base_addr + FIPC_OPIPE_RPTR(pipe_id)));
}

/*lint -esym(528,fipc_opipe_wptr_get) */
static inline u32 fipc_opipe_wptr_get(const void *base_addr, u32 pipe_id)
{
    return readl((void *)((char *)base_addr + FIPC_OPIPE_WPTR(pipe_id)));
}

/*lint -esym(528,fipc_opipe_rptr_get) */
static inline u32 fipc_opipe_rptr_get(const void *base_addr, u32 pipe_id)
{
    return readl((void *)((char *)base_addr + FIPC_OPIPE_RPTR(pipe_id)));
}

/*lint -esym(528,fipc_ipipe_wptr_update) */
static inline void fipc_ipipe_wptr_update(void *base_addr, u32 pipe_id, u32 wptr)
{
    writel(wptr & 0xFFFF, (void *)((char *)base_addr + FIPC_IPIPE_WPTR(pipe_id)));
}

/*lint -esym(528,fipc_ipipe_rptr_update) */
static inline void fipc_ipipe_rptr_update(void *base_addr, u32 pipe_id, u32 rptr)
{
    writel(rptr, (void *)((char *)base_addr + FIPC_IPIPE_RPTR(pipe_id)));
}

/*lint -esym(528,fipc_ipipe_wptr_get) */
static inline u32 fipc_ipipe_wptr_get(const void *base_addr, u32 pipe_id)
{
    return readl((void *)((char *)base_addr + FIPC_IPIPE_WPTR(pipe_id))) & 0xFFFF;
}

static inline u32 fipc_ipipe_rptr_get(const void *base_addr, u32 pipe_id)
{
    return readl((void *)((char *)base_addr + FIPC_IPIPE_RPTR(pipe_id)));
}

/*lint -esym(528,fipc_opipe_local_status) */
static inline u32 fipc_opipe_local_status(const void *base_addr, u32 pipe_id)
{
    return readl((void *)((char *)base_addr + FIPC_OPIPE_STAT(pipe_id)));
}

/*lint -esym(528,fipc_ipipe_local_status) */
static inline u32 fipc_ipipe_local_status(const void *base_addr, u32 pipe_id)
{
    return readl((void *)((char *)base_addr + FIPC_IPIPE_STAT(pipe_id)));
}

int fipc_relv_dev_chn_get(struct fipc_device *prdev, u32 ripipe_id, struct fipc_device **ppdev, u32 *plipie_id);
#endif /* FIPC_DRIVER_H */
