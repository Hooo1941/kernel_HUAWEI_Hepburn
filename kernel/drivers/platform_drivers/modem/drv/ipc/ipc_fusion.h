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

#ifndef __IPC_FUSION_H__
#define __IPC_FUSION_H__

#include <osl_types.h>
#include <bsp_print.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ipc_print_err(fmt, ...) bsp_err("[%s] :" fmt, __FUNCTION__, ##__VA_ARGS__)
#define ipc_print_info(fmt, ...) bsp_info("[%s] :" fmt, __FUNCTION__, ##__VA_ARGS__)
#define ipc_print_dbg(fmt, ...) do {                                       \
        if (g_dbg_print_sw) {                  \
            ipc_print_err(fmt, ##__VA_ARGS__); \
        }                                      \
    } while (0)

#define IPC_PLAT_NAME "ipc_driver"

#define IPC_OK 0
#define IPC_ERR (-1)
#define IPC_HANDLE_ERR (-2)

#define IPC_NUM 1 /* 可访问IPC的个数 */
#define CHN_MAX_NUM 8 /* 可用中断最大数目 */
#define RES_MAX_NUM 32 /* 核上最大的通道数目 */

#define IPC_BIT_MASK(n) (1UL << (n))
#define MKU32_SHIFT 8
#define DEV_IRQ_SHIFT 4
#define IPC_MAGIC 0xabcd
#define MDM_CHN_MASK 0xE8

#define GET_DEV_IRQ_ID(handle) (((handle) >> DEV_IRQ_SHIFT) & 0xf)
#define GET_CHN_IRQ_ID(handle) ((handle)&0xf)

#define BSP_IPC_CPU_RAW_INT(i) (0x400 + ((unsigned long)(i)*0x10))
#define BSP_IPC_CPU_INT_MASK(i) (0x404 + ((unsigned long)(i)*0x10))
#define BSP_IPC_CPU_INT_STAT(i) (0x408 + ((unsigned long)(i)*0x10))
#define BSP_IPC_CPU_INT_CLR(i) (0x40C + ((unsigned long)(i)*0x10))

struct ipc_handle {
    u32 res_bit : 5;
    u32 chn_id : 5;
    u32 dev_id : 6;
    u32 magic : 16;
};

/* ipc 资源 */
struct ipc_res_t {
    u32 res_id;
    u32 res_bit; /* ipc资源对应的bit位 */
    u32 send_times; /* 发送次数 */
    u32 handle_times; /* 处理次数 */
    u32 begin_stamp; /* 开始时间戳 */
    u32 end_stamp; /* 结束时间戳 */
    struct ipc_handle handle;
    struct ipc_entry ipc_int_table; /* ipc挂的中断回调函数 */
};

/* ipc 设备上一组 */
struct ipc_channel_t {
    u32 dev_id;
    u32 chn_id;
    u32 irq_no;
    u32 used;
    u32 isrecv;
    u32 src_mdm; /* 属于mdm发送资源, 单独复位 */
    struct ipc_res_t *res[RES_MAX_NUM];
};

/* ipc 设备 */
struct ipc_device_t {
    u32 devid;
    u32 ismdm;
    void *base_addr;
    struct ipc_channel_t ipc_chn[CHN_MAX_NUM];
};

int ipc_res_show_dump(int ipc_res_id);

#ifdef __cplusplus
}
#endif /* define __cplusplus */

#endif
