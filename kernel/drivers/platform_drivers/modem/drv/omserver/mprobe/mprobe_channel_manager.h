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

#ifndef _MPROBE_CHANNEL_MANAGER_H_
#define _MPROBE_CHANNEL_MANAGER_H_

#include <linux/of_platform.h>
#include <osl_types.h>
#include <osl_spinlock.h>
#include <bsp_dump.h>
#include <bsp_version.h>
#include "bsp_mprobe.h"


#define MPROBE_SRC_BUFF_LEN (1024 * 64)
#define MPROBE_DST_BUFF_LEN (1024 * 128)
#define MPROBE_DST_THRESHOLD (8 * 1024)                    /* low waterline */
#define MPROBE_CODER_DST_GTHRESHOLD ((8 * 1024) + 8 + 128) /* 编码源通道仲裁阈值 + DMA一次最小搬运字节 high waterline*/

#define MPROBE_WAIT_ODT_IDLE_SINGLE_TIMEMS 1
#define MPROBE_WAIT_ODT_IDLE_MAX_DELAY 20

typedef enum {
    MPROBE_CHANNEL_UNINIT = 0, /* 未初始化 */
    MPROBE_CHANNEL_INIT_SUCC,  /* 初始化成功 */
    MPROBE_CHANNEL_MEM_FAIL,   /* 初始化申请内存错误 */
    MPROBE_CHANNEL_CFG_FAIL,   /* 初始化通道配置 */
    MPROBE_CHANNEL_START_FAIL, /* 通道开启错误 */
    MPROBE_CHANNEL_INIT_BUTT
} mprobe_chan_init_state_e;

typedef struct {
    unsigned int init_state;
    unsigned int chan_id;
    u32 buff_len;
    u32 threshold;
    u32 timeout_mode;
    u8 *dst_virt_buff;
    u8 *dst_phy_buff;
} dst_chan_cfg_s;

typedef struct {
    unsigned int init_state;  /* 通道初始化状态，初始化后自动修改 */
    unsigned int chan_id;     /* 编码源通道ID，固定配置 */
    unsigned int dst_chan_id; /* 编码目的通道ID */
    unsigned int data_type;   /* 数据来源类型 */
    unsigned int chan_mode;   /* 通道类型 */
    unsigned int chan_prio;   /* 通道优先级 */
    unsigned int trans_id_en;
    u32 buff_len;             /* 编码源通道数据空间大小 */
    u8 *src_virt_buff;        /* 编码源通道数据空间内存指针 */
    u8 *src_phy_buff;
} src_chan_cfg_s;

extern struct platform_device *g_odt_pdev;

void mprobe_enable_dst_chan(void);

int mprobe_rls_dst_buff(u32 read_size);

int mprobe_srv_pack_data(mprobe_msg_report_head_s *msg);

int mprobe_channel_start(void);

void mprobe_channel_stop(void);


#endif