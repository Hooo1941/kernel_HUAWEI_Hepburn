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

#ifndef __SYS_ODT_H__
#define __SYS_ODT_H__


#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <bsp_odt.h>

#define SYS_BUS_ODT_NAME "odt"
#define ODT_STRESS_FLAG 0x5a5a5a5a
#define SYS_BUS_ODT_CNT_1K 0x400

#pragma pack(push)
#pragma pack(4)
struct odt_press_case {
    u32 dst_set;
    u32 src_chan_no;
    u32 dst_chan_no;
    u32 bypass_en;
    odt_data_type_e data_type;
    odt_encsrc_chnmode_e mode;
    odt_chan_priority_e priority;
    u8 *input_start;
    u32 input_size;
    u8 *rd_start;
    u32 rd_size;
    u32 rd_threshold;
    u8 *output_start;
    u32 output_size;
    u32 threshold;
    odt_event_cb event_cb;
    odt_read_cb read_cb;
    odt_rd_cb rd_cb;
    u32 task_id;
    u32 data_len;
    u32 data_cnt;
    odt_data_type_en_e data_type_en;
    odt_enc_debug_en_e enc_debug_en;
    u32 encdst_thrh;
};

struct odt_buffer {
    unsigned long src_vir_addr;
    dma_addr_t src_phy_addr;
    unsigned long rd_vir_addr;
    dma_addr_t rd_phy_addr;
    unsigned long dst_vir_addr;
    dma_addr_t dst_phy_addr;
};

void sys_odt_init(struct device_node *np);
void sys_odt_stop(void);
void sys_odt_start(void);
void sys_odt_report(void);

#pragma pack(pop)

#endif
