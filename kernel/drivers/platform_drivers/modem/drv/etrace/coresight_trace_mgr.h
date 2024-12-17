/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
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
#ifndef __CORESIGHT_TRACE_MGR_H__
#define __CORESIGHT_TRACE_MGR_H__
#include <of.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/of_device.h>
#include <securec.h>
#include <drv_comm.h>
#include "bsp_dt.h"
#define CORESIGHT_FUNNEL_FUNCTL 0x000
#define CORESIGHT_FUNNEL_HOLDTIME_MASK 0xF00
#define CORESIGHT_FUNNEL_HOLDTIME_SHFT 0x8
#define CORESIGHT_FUNNEL_HOLDTIME (0x7 << CORESIGHT_FUNNEL_HOLDTIME_SHFT)
#define MODEM_LOG_ROOT "/data/dfx_logs/modem_log"
#define CORESIGHT_LOG_PATH MODEM_LOG_ROOT "/trace.tmc"
#define CORESIGHT_FILE_PATH_LEN 128
#define CORESIGHT_REPLICATOR_FLT0 0x0
#define CORESIGHT_REPLICATOR_FLT1 0x4
#define CORESIGHT_DIR_CHMOD 0660
#define CORESIGHT_TRACE_MAX_SIZE 0x400000
#define CORESIGHT_LAR 0xFB0
#define CORESIGHT_INIT_FLAG 1
#define CORESIGHT_DDR_BUF 1
#define CORESIGHT_DMA_BUF 2

#define CORESIGHT_READL(base, off) readl((void *)((uintptr_t)(base) + (off)))
#define CORESIGHT_WRITEL(base, val, off) writel((val), (void *)((uintptr_t)(base) + (off)))
#define CORESIGHT_TMC_LOCK(base)                    \
    do {                                            \
        mb();                                       \
        CORESIGHT_WRITEL(base, 0x0, CORESIGHT_LAR); \
    } while (0)
#define CORESIGHT_TMC_UNLOCK(base)                         \
    do {                                                   \
        /* write a non zero data to unlock */              \
        CORESIGHT_WRITEL(base, 0xC5ACCE55, CORESIGHT_LAR); \
        mb();                                              \
    } while (0)

/* TMC register */
#define CORESIGHT_TMC_RSZ (0x004)
#define CORESIGHT_TMC_STS (0x00C)
#define CORESIGHT_TMC_RRD (0x010)
#define CORESIGHT_TMC_TRG (0x01C)
#define CORESIGHT_TMC_CTL (0x020)
#define CORESIGHT_TMC_MODE (0x028)
#define CORESIGHT_TMC_BUFWM (0x034)
#define CORESIGHT_TMC_FFCR (0x304)
#define CORESIGHT_TMC_RRP (0x014)
#define CORESIGHT_TMC_RWP (0x018)
#define CORESIGHT_TMC_AXICTL (0x110)
#define CORESIGHT_TMC_DBALO (0x118)
#define CORESIGHT_TMC_DBAHI (0x11C)

#define BYTES_PER_WORD 4

/* CORESIGHT_TMC_STS */
#define TMCSTS_TMCREADY_MASK CSBIT(2)
#define TMCSTS_TMCREADY TMCSTS_TMCREADY_MASK
#define TMCSTS_TMCIDLE (0u)

#define CSBIT(nr) (1UL << (u32)(nr))
/* CORESIGHT_TMC_CTL */
#define TMC_CTL_TRACECAPEN CSBIT(0)
#define TMC_CTL_TRACECAPDIS (0u)

/* CORESIGHT_TMC_FFCR */
#define TMC_FFCR_STOPONFLUSH CSBIT(12)
#define TMC_FFCR_TRIGONTRIGIN CSBIT(8)
#define TMC_FFCR_FLUSHMAN_MASK CSBIT(6)
#define TMC_FFCR_FLUSHMAN TMC_FFCR_FLUSHMAN_MASK
#define TMC_FFCR_FONTRIGEVT CSBIT(5)
#define TMC_FFCR_FONFLIN CSBIT(4)
#define TMC_FFCR_ENTI CSBIT(1)
#define TMC_FFCR_ENFT CSBIT(0)

#define TMC_FFCR_CIRCULAR_CFG \
    (TMC_FFCR_TRIGONTRIGIN | TMC_FFCR_FONTRIGEVT | TMC_FFCR_FONFLIN | TMC_FFCR_ENTI | TMC_FFCR_ENFT)

enum coresight_tmc_mode {
    CORESIGHT_TMC_MODE_CIRCULAR_BUFFER,
    CORESIGHT_TMC_MODE_SOFTWARE_FIFO,
    CORESIGHT_TMC_MODE_HARDWARE_FIFO,
};

struct coresight_trace_s {
    void *virt_ddr_buffer;
    char *path;
    unsigned long ddr_buffer_addr;
    u32 ddr_buffer_len;
    u32 ddr_flag;
    u32 ddr_size;
    u32 init_flag;
};
int coresight_trace_init(void);
int coresight_trace_info_init(u32 ddr_size);
void coresight_mprobe_init(void);
int coresight_mprob_set(u32 ddr_size);
struct coresight_trace_s coresight_get_trace_info(void);
#endif
