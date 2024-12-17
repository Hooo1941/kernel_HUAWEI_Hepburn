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
#ifndef _FIPC_DTS_H
#define _FIPC_DTS_H
#include "fipc_platform.h"

#include <bsp_dt.h>
#include <linux/of_device.h>

/* can be defined in dts */
#define FIPC_DEVICE_NUM_MAX 8
#define FIPC_USRCHN_NUM_MAX 128

int fipc_init_meet(void);

struct fipc_rsv_meminfo {
    u32 vmem_rsv_sz;
    u32 dmamem_rsv_sz;
    u32 pushmem_rsv_sz;
    u32 dump_size;
};
int fipc_reserved_meminfo(struct fipc_rsv_meminfo *pmeminfo);

#include "fipc_device.h"

struct fipc_chn_dts {
    u32 id;
    u32 type;
    u32 ldrvchn_id;
    u32 rdrvchn_id;
    u32 const_flags;
};

struct fipc_device_dts {
    u32 dev_id;
    u32 reg_addr;
    u32 reg_size;
    u32 pipepair_cnt;
    u32 ctrl_level;
    u32 irq_type;
    struct irq_bundle *maps[FIPC_CPU_ID_MAX]; /* 大部分设备只有一组，这样节省内存 */

    u32 *push_ptr;
    struct fipc_chn_dts *child;
    u32 childcnt;
};

enum fipc_version_type {
    FIPC_VERSION_V200 = 0x0,
    FIPC_VERSION_V210,
    FIPC_VERSION_V300,
    FIPC_VERSION_V301,  /* (后续同步配置m、p clk_en后删除) */
    FIPC_VERSION_V410,
    FIPC_VERSION_MAXNUM,
};

#endif
