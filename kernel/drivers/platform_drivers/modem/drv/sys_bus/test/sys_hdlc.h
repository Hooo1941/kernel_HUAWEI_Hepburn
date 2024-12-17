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
 
#ifndef __SYS_HDLC_H__
#define __SYS_HDLC_H__
#include <product_config.h>
#include <linux/of.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/dma-map-ops.h>
#include <linux/dma-mapping.h>
#include <linux/device.h>
#include <linux/of_device.h>
#include <linux/interrupt.h>
#include <securec.h>

typedef struct {
    struct task_struct* tsk;
    volatile u32 run_flag;
    void *peri_reg_base;
    void *hdlc_reg_base;
    unsigned int *hdlc_buf_ptr;
    unsigned int *hdlc_out_ptr;
    unsigned int *hdlc_node_ptr[20];
    unsigned int hdlc_buf_addr_pa;
    unsigned int hdlc_out_addr_pa;
    unsigned int hdlc_node_addr_pa[20];
} sys_hdlc_ctrl;

#define SYS_HDLC_NAME "hdlc"
#define SYS_HDLC_ERR -1
#define SYS_HDLC_OK 0
#define HDLC_BUF_PTR_LEN 2000
#define HDLC_OUT_PTR_LEN 8000
#define HDLC_NODE_PTR_LEN 1536
#define FRM_IN_LLI_1DOR2D_VALUE 0
#define DEF_IN_PKT_LEN_MAX_VALUE 1536
#define UL_FRM_DATA_LEN 500
#define SYS_HDLC_STOP_DELAY 10

#define FRM_IN_LLI_OFFSET 0
#define FRM_RPT_OFFSET 240
#define FRM_OUT_LLI_OFFSET 400
#define DEF_IN_LLI_OFFSET 880
#define DEF_RPT_OFFSET 1120

#define FRM_DEF_REPORT_DEPTH 160
#define DEF_OUT_SPC_DEPTH 8000

#define HDLC_BASE_ADDR  0xE0C2B000
//framer crg
#define PRIOR_TIMEOUT_CTRL  0x04
#define HDLC_FRM_EN         0x10
#define HDLC_FRM_INT_CLR    0x1c
#define HDLC_FRM_CFG        0x20
#define HDLC_FRM_ACCM       0x24
#define HDLC_FRM_STATUS	    0x28
#define FRM_IN_LLI_ADDR     0x30
#define FRM_OUT_LLI_ADDR    0x44
#define FRM_RPT_ADDR        0x50
#define FRM_RPT_DEP         0x54

//deframer crg
#define  HDLC_DEF_EN             0x60
#define  HDLC_DEF_RAW_INT        0x64
#define  HDLC_DEF_INT_STATUS     0x68
#define  HDLC_DEF_INT_CLR        0x6c
#define  HDLC_DEF_CFG            0x70
#define  DEF_UNCOMPLETED_LEN     0x74
#define  DEF_UNCOMPLETED_PRO     0x78
#define  DEF_UNCOMPLETED_ADDR    0x7c
#define  DEF_UNCOMPLETED_ST_AGO  0x80
#define  HDLC_DEF_GO_ON          0x84
#define  HDLC_DEF_STATUS         0x88
#define  DEF_UNCOMPLETED_ST_NOW  0x8c
#define  DEF_IN_LLI_ADDR         0x90
#define  DEF_IN_PKT_ADDR         0x94
#define  DEF_IN_PKT_LEN          0x98
#define  DEF_IN_PKT_LEN_MAX      0x9c
#define  DEF_OUT_SPC_ADDR        0xa0
#define  DEF_OUT_SPC_DEP         0xa4
#define  DEF_RPT_ADDR            0xa8
#define  DEF_RPT_DEP             0xac
#define  HDLC_DEF_ERR_INFO_0     0xb0
#define  HDLC_DEF_ERR_INFO_1     0xb4
#define  HDLC_DEF_ERR_INFO_2     0xb8
#define  HDLC_DEF_ERR_INFO_3     0xbc
#define  DEF_INFO_FRL_CNT_AGO    0xc0
#define  DEF_INFO_FRL_CNT_NOW    0xc4

void sys_hdlc_sysbus_test_init(void);

#endif /* __SYS_HDLC_H__ */