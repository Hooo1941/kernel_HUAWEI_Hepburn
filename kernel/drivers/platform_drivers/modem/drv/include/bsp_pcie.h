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

#ifndef __BSP_PCIE_H__
#define __BSP_PCIE_H__

#include "bsp_pcie_ab.h"
#include <osl_irq.h>
#include <osl_bio.h>
#include <product_config.h>
#include <osl_types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PCIE_CFG_MAGIC  0xA5A55A5A

/* ****** work mode********* */
/* ep mode */
#ifndef PCIE_WORK_MODE_EP
#define PCIE_WORK_MODE_EP   0x0
#endif

/* legacy ep mode */
#ifndef PCIE_WORK_MODE_LEP
#define PCIE_WORK_MODE_LEP  0x1
#endif

/* rc mode */
#ifndef PCIE_WORK_MODE_RC
#define PCIE_WORK_MODE_RC   0x4
#endif

/* pcie_ports */
#ifndef PCIE_PORT_MODE_AUTO
#define PCIE_PORT_MODE_AUTO     0x0
#endif

#ifndef PCIE_PORT_MODE_NATIVE
#define PCIE_PORT_MODE_NATIVE   0x1
#endif

#ifndef PCIE_PORT_MODE_COMPAT
#define PCIE_PORT_MODE_COMPAT   0x2
#endif

#ifndef PCIE_PORT_MODE_DPC_NATIVE
#define PCIE_PORT_MODE_DPC_NATIVE   0x3
#endif

/* speed mode */
#ifndef PCIE_SPEED_MODE_DEFAULT
#define PCIE_SPEED_MODE_DEFAULT 0x0
#endif

/* clock mode */
#ifndef PCIE_CLOCK_MODE_OUTER_PHY
#define PCIE_CLOCK_MODE_OUTER_PHY   0x0
#endif

#ifndef PCIE_CLOCK_MODE_INNER
#define PCIE_CLOCK_MODE_INNER       0x1
#endif

#ifndef PCIE_CLOCK_MODE_OUTER_IO
#define PCIE_CLOCK_MODE_OUTER_IO    0x2
#endif

#ifndef PCIE_CLOCK_MODE_DEFAULT
#define PCIE_CLOCK_MODE_DEFAULT (PCIE_CLOCK_MODE_INNER)
#endif

/* common clock mode */
#ifndef PCIE_COMMON_CLOCK_MODE_ASYNC
#define PCIE_COMMON_CLOCK_MODE_ASYNC   0x0
#endif

#ifndef PCIE_COMMON_CLOCK_MODE_SYNC
#define PCIE_COMMON_CLOCK_MODE_SYNC   0x1
#endif

#ifndef PCIE_COMMON_CLOCK_MODE_DEFAULT
#define PCIE_COMMON_CLOCK_MODE_DEFAULT (PCIE_COMMON_CLOCK_MODE_SYNC)
#endif

#ifndef PCIE_ENABLE
#define PCIE_ENABLE (0x1)
#endif
#ifndef PCIE_DISABLE
#define PCIE_DISABLE (0x0)
#endif

#ifndef PCIE_CFG_U8
#define PCIE_CFG_U8(cfg) ((cfg) | 0x80U)
#endif
#ifndef PCIE_CFG_U32
#define PCIE_CFG_U32(cfg) ((cfg) | 0x80000000U)
#endif

#ifndef PCIE_CFG_U8_VALID
#define PCIE_CFG_U8_VALID(cfg) ((cfg) & 0x80U)
#endif
#ifndef PCIE_CFG_U32_VALID
#define PCIE_CFG_U32_VALID(cfg) ((cfg) & 0x80000000U)
#endif

#ifndef PCIE_CFG_U8_VALUE
#define PCIE_CFG_U8_VALUE(cfg) ((cfg) & 0x7FU)
#endif
#ifndef PCIE_CFG_U32_VALUE
#define PCIE_CFG_U32_VALUE(cfg) ((cfg) & 0x7FFFFFFFU)
#endif

/* ******func, bar, bar_index max****** */
#define PCIE_FUNC_NUM   0x8
#define PCIE_BAR_NUM     0x6
#define PCIE_IATU_REG_NUM 0x9
#define PCIE_BAR_ADDR_OFFSET 0X4
#define PCIE_CFG_REG_NUM 0x10
#define PCIE_BAR_INDEX_NUM  (PCIE_FUNC_NUM*PCIE_BAR_NUM)

/* ******rc<->ep user id lookup start addr****** */
#define IPC_INT_BAR_OFFSET   0x400
#define IPC_INT_ENABLE_OFFSET 0x600

#define IPC_DATA_VALID_OFFSET 0x800
#define EP_MSI_VALID_OFFSET 0x804

/* ******PCIE IPC Reg Definition****** */
#define IPC_REG_BASE_ADDR  0xE5010000
#define IPC_ADDR_OFFSET  0xE400
#define IPC_ACPU_INT_SRC_PCIE_EP  21

#define PCIE_DMA_TRANS_RUNNING           0
#define PCIE_DMA_TRANS_COMPLETE         1
#define PCIE_DMA_TRANS_ABORT             (-1)
#define PCIE_DMA_INVALID_INPUT           (-2)

#define PCIE_DMA_ADDR_SHIFT 32

/* ******DMA Direction*************** */
enum pcie_dma_direction {
    PCIE_DMA_DIRECTION_READ = 0,
    PCIE_DMA_DIRECTION_WRITE = 1,
    PCIE_DMA_DIRECTION_MAX
};

enum pcie_link_mode {
    PCIE_LINK_L0 = 0,
    PCIE_LINK_L1SS = 1,
    PCIE_LINK_SLOW_L1SS = 2,
    PCIE_LINK_L3 = 3,
};

enum pcie_bar_size {
    BAR_SIZE_256B   =  0xFF,
    BAR_SIZE_512B   =  0x1FF,
    BAR_SIZE_1K       =  0x3FF,
    BAR_SIZE_2K       =  0x7FF,
    BAR_SIZE_4K       =  0xFFF,
    BAR_SIZE_8K       =  0x1FFF,
    BAR_SIZE_16K     =  0x3FFF,
    BAR_SIZE_32K     =  0x7FFF,
    BAR_SIZE_64K     =  0xFFFF,
    BAR_SIZE_128K   =  0x1FFFF,
    BAR_SIZE_256K   =   0x3FFFF,
    BAR_SIZE_512K   =   0x7FFFF,
    BAR_SIZE_1M       =  0xFFFFF,
    BAR_SIZE_2M       =  0x1FFFFF,
    BAR_SIZE_4M       =  0x3FFFFF,
    BAR_SIZE_8M       =  0x7FFFFF,
    BAR_SIZE_16M     =  0xFFFFFF,
    BAR_SIZE_32M     =  0x1FFFFFF,
    BAR_SIZE_64M     =  0x3FFFFFF,
    BAR_SIZE_128M   =  0x7FFFFFF,
    BAR_SIZE_256M   =  0xFFFFFFF,
};

/* ***********************************End*************************************/

struct pcie_dma_data_element {
    u32 channel_ctrl;
    u32 transfer_size;
    u32 sar_low;
    u32 sar_high;
    u32 dar_low;
    u32 dar_high;
};

struct pcie_dma_transfer_info {
    struct pci_dev* dev;
    u32 id;
    enum pcie_dma_chn_id channel;
    u32 direction;  /* 0 for read, 1 for write */
    pcie_callback callback;
    void *callback_args;
    struct pcie_dma_data_element element;
    void *dma_lli_addr; /* phy addr */
    enum pcie_dma_transfer_type transfer_type;  /* NORMAL MODE, LINK_MODE */
    int remote_int_enable;
    int element_cnt;
};

#ifdef __KERNEL__

/*
* Atention Please !!!!!
* addr is Vir addr
* sar is Phy addr
* dar is Phy addr
*/
static inline void bsp_pcie_set_dar_element(void* addr, u64 dar)
{
    writel_relaxed(0x0U, addr);
    writel_relaxed(dar & 0xFFFFFFFFUL, addr + 0x10U); /* dar Low */
    writel_relaxed(dar >> PCIE_DMA_ADDR_SHIFT, addr + 0x14U); /* dar High */
}

static inline void bsp_pcie_set_sar_element(void* addr, u32 transfer_size, u64 sar)
{
    writel_relaxed(0x0U, addr);
    writel_relaxed(transfer_size, addr + 0x04U); /* transfer_size */
    writel_relaxed(sar & 0xFFFFFFFFUL, addr + 0x08U); /* sar Low */
    writel_relaxed(sar >> PCIE_DMA_ADDR_SHIFT, addr + 0x0CU); /* sar High */
}

static inline void bsp_pcie_set_last_sar_element(void* addr, u32 transfer_size, u64 sar)
{
    writel_relaxed(0x08U, addr);  // enable done LIE interrupt
    writel_relaxed(transfer_size, addr + 0x04U); /* transfer_size */
    writel_relaxed(sar & 0xFFFFFFFFUL, addr + 0x08U); /* sar Low */
    writel_relaxed(sar >> PCIE_DMA_ADDR_SHIFT, addr + 0xCU); /* sar High */
}

static inline void bsp_pcie_set_data_element(void* addr, u32 transfer_size, u64 sar, u64 dar)
{
    writel_relaxed(0x0U, addr);   // fix me: writel_relax
    writel_relaxed(transfer_size, addr + 0x04U); /* transfer_size */
    writel_relaxed(sar & 0xFFFFFFFFUL, addr + 0x08U); /* sar Low */
    writel_relaxed(sar >> PCIE_DMA_ADDR_SHIFT, addr + 0x0CU); /* sar High */
    writel_relaxed(dar & 0xFFFFFFFFUL, addr + 0x10U); /* dar Low */
    writel_relaxed(dar >> PCIE_DMA_ADDR_SHIFT, addr + 0x14U); /* dar High */
}

static inline void bsp_pcie_set_last_data_element(void* addr, u32 transfer_size, u64 sar, u64 dar)
{
    writel_relaxed(0x08U, addr);  // enable done LIE interrupt
    writel_relaxed(transfer_size, addr + 0x04U); /* transfer_size */
    writel_relaxed(sar & 0xFFFFFFFFUL, addr + 0x08U); /* sar Low */
    writel_relaxed(sar >> PCIE_DMA_ADDR_SHIFT, addr + 0xCU); /* sar High */
    writel_relaxed(dar & 0xFFFFFFFFUL, addr + 0x10U); /* dar Low */
    writel_relaxed(dar >> PCIE_DMA_ADDR_SHIFT, addr + 0x14U); /* dar High */
}

static inline void bsp_pcie_set_last_remote_element(void* addr, u32 transfer_size, u64 sar, u64 dar)
{
    writel(0x10U, addr);  // enable done RIE interrupt
    writel(transfer_size, addr + 0x04U); /* transfer_size */
    writel(sar & 0xFFFFFFFFUL, addr + 0x08U); /* sar Low */
    writel(sar >> PCIE_DMA_ADDR_SHIFT, addr + 0x0CU); /* sar High */
    writel(dar & 0xFFFFFFFFUL, addr + 0x10U); /* dar Low */
    writel(dar >> PCIE_DMA_ADDR_SHIFT, addr + 0x14U); /* dar High */
}
static inline void bsp_pcie_set_link_element(void *addr, u32 is_last, u64 link_addr)
{
    u32 read_push;
    /* Linked List Element Initial Setup: the (n-1)th element */
    /* LLP = 1, last:CB=1, not last:CB=0, Channel contrl register */
    writel_relaxed((0x1U << 0x2) | (is_last ? 0x1 : 0x0), addr);
    writel_relaxed(0x0U, addr + 0x04U); /* reserved */
    writel_relaxed((is_last ? 0x0 : link_addr), addr + 0x08U); /* Linked List Element Pointer Low */
    writel_relaxed((is_last ? 0x0 : link_addr), addr + 0x0CU); /* Linked List Element Pointer High */
    read_push = readl_relaxed(addr + 0x0CU);
}

/* ******definition in pcie kernel stage****** */
int bsp_pcie_dma_transfer(struct pcie_dma_transfer_info *transfer_info);
int bsp_pcie_rc_dma_transfer(struct pcie_dma_transfer_info *transfer_info);
int bsp_pcie_ep_dma_transfer(u32 id, struct pcie_dma_transfer_info *transfer_info);
int bsp_pcie_is_dma_transfer_complete(u32 id, struct pcie_dma_transfer_info *transfer_info);
int bsp_pcie_ep_dma_isr_register(u32 id, enum pcie_dma_chn_id chn, u32 direction, pcie_callback call_back, void* arg);
int bsp_pcie_ep_bar_config(enum pcie_bar_id_e bar_index, enum pcie_bar_size bar_size, u64 addr);
int bsp_pcie_ep_send_msi(enum pcie_ep_to_rc_msi_id id);
int bsp_iot_pcie_init(void);
u32 bsp_pcie_ep_port_available(void);

#endif

void pcie_hiboot_init(void);
void pcie_hiboot_console(const char *cmd);

#if defined(CONFIG_IOT_PCIE)
int bsp_pcie_send_msi(u32 id, u32 msi_data);
#else
static inline __attribute__((unused)) int bsp_pcie_send_msi(u32 id __attribute__((unused)),
    u32 msi_data __attribute__((unused)))
{
    return 0;
}
#endif

#if defined(CONFIG_IOT_PCIE_SLT)
int bsp_pcie_slt_case(void);
#else
static inline __attribute__((unused)) int bsp_pcie_slt_case(void)
{
    return 0;
}
#endif

#ifdef __cplusplus
}
#endif

#endif

