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

#ifndef _ODT_H_
#define _ODT_H_

#include <linux/platform_device.h>
#include <linux/semaphore.h>
#include <linux/of_device.h>
#include <osl_thread.h>
#include <osl_types.h>
#include "bsp_dt.h"
#include "bsp_odt.h"
#include "bsp_print.h"
#include "hal_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ODT_CHN_ALLOCATED 1              /* 通道已经分配 */
#define ODT_CHN_ENABLE 1                 /* 通道打开 */
#define ODT_CHN_DISABLE 0                /* 通道关闭 */
#define ODT_CHN_SET 1                    /* 通道已经配置 */
#define ODT_TIMEOUT_TFR_LONG_MAX 0x927c0 /* 10min */
#define ODT_TIMEOUT_TFR_LONG_MIN 0xc8    /* 200ms */
#define ODT_TIMEOUT_TFR_SHORT_VAL 0x0a   /* 10ms */
#define ODT_ENCDST_TASK_PRO 81           /* 编码目的中断处理任务优先级定义 */
#define ODT_ENCSRC_TASK_PRO 79           /* 编码源中断处理任务优先级定义 */
#define ODT_RATE_PERIOD 1000             /* 源通道限流周期 */
#define ODT_BUFFER_MIN (128 * 1024)

typedef enum {
    ODT_RD_MASK_SET,
    ODT_RD_MASK_CLEAR,
    ODT_RD_MASK_BUTT
} odt_rd_mask_e;

typedef enum {
    ODT_DST_CHAN_CFG_ADDR = 0,
    ODT_DST_CHAN_CFG_SIZE,
    ODT_DST_CHAN_CFG_TIME,
    ODT_DST_CHAN_CFG_RSV,
    ODT_DST_CHAN_CFG_BUTT
} odt_dst_chan_cfg_e;

/* 通道状态结构体，共四类 */
typedef struct {
    unsigned long start;
    unsigned long end;
    u32 write;
    u32 read;
    u32 length;
    u32 idle_size;
} odt_ring_buff_s;

typedef struct {
    u32 chan_id;
    u32 chan_en;
    u32 trans_id_en;
    u32 ptr_img_en;
    u32 dst_chan_id;
    u32 bypass_en;
    u32 alloc_state; /* 通道已经或没有分配的标识 */
    u32 last_rd_size;
    u32 rd_threshold;
    u32 rd_irq_mask;
    odt_encsrc_chnmode_e chan_mode; /* 数据结构类型 */
    odt_chan_priority_e priority;
    odt_data_type_e data_type;
    odt_data_type_en_e data_type_en;
    odt_enc_debug_en_e debug_en;
    unsigned long read_ptr_img_phy_addr;
    unsigned long read_ptr_img_vir_addr;
    odt_ring_buff_s enc_src_buff;
    odt_ring_buff_s rd_buff;
    odt_rd_cb rd_cb;
} odt_enc_src_chan_s;

typedef struct {
    u32 dst_send_req;
    u32 dst_int_state;
} odt_dst_int_info_s;

typedef struct {
    struct semaphore enc_src_sem_id;
    struct semaphore enc_dst_sem_id;
    unsigned long enc_src_task_id;
    unsigned long enc_dst_task_id;
    u64 int_enc_src_header;
    u32 int_enc_dst_tfr;
    u32 int_enc_dst_thrh_ovf;
    u32 thrh_ovf_flag;
    spinlock_t lock;
    odt_dst_int_info_s dst_int_info[ODT_MAX_ENCDST_CHN];
} odt_int_manager_info_s;

typedef struct {
    spinlock_t lock;
    u32 odt_cur_mode;
    u32 cur_timeout;
} odt_mode_swt_info_s;

/* 编码源通道信息结构 */
typedef struct {
    int init_flag;
    odt_enc_src_chan_s enc_src_chan[ODT_MAX_ENCSRC_CHN];
} odt_enc_src_info_s;

typedef struct {
    u32 chan_id;
    u32 set_state; /* 通道已经或没有配置的标识 */
    u32 threshold; /* 阈值 */
    odt_ring_buff_s enc_dst_buff;
    odt_read_cb read_cb;
    u32 buf_thrhold;
    u32 thrhold_high;
    u32 thrhold_low;
    u32 send_req;
    u32 enable_state;
} odt_enc_dst_info_s;

typedef struct {
    void *virt_addr; /* fastboot预留的buffer虚拟BUFFER、在32位系统上是4字节，在64位系统上是8字节 */
    unsigned long phy_addr; /* fastboot预留的buffer物理地址 */
    unsigned int buff_size; /* fastboot预留的buffer大小 */
    unsigned int timeout; /* fastboot指定的数据传输超时时间, 只有在fastboot预留内存可用时才生效 */
    unsigned int buff_useable; /* fastboot预留的buffer是否可用的标志 */
} odt_early_cfg_s;

s32 odt_reserved_mem_init(void);
void odt_get_rsv_mem_info(void);
void odt_get_cmdline_info(void);
s32 odt_rsv_mem_mmap(void);
s32 odt_buffer_init(device_node_s *dev);
s32 odt_uncacheable_mem_alloc(device_node_s *dev);
u32 odt_get_mem_log_on_flag(void);
s32 odt_int_manager_init(void);
s32 odt_int_proc_task_init(void);
s32 odt_int_handler_init(void);
void odt_rd_int_mask_proc(u32 chan_id, odt_rd_mask_e rd_mask_state);
void odt_set_dst_int_open(u32 chan_id);
void odt_mode_swt_init(void);
s32 odt_set_enc_dst_mode(u32 dst_chan, u32 mode);
void odt_set_mode_direct(void);
void odt_set_mode_delay(void);
void odt_set_mode_cycle(void);
u32 odt_get_ind_dst_cur_timouet(void);
void odt_set_enc_dst_timeout(u32 chan_id, u32 timeout_mode, u32 timeout);
void odt_get_data_buffer(odt_ring_buff_s *ring_buffer, odt_buffer_rw_s *rw_buffer);
void odt_enc_dst_read_cb(u32 chan_id);
void odt_enc_dst_spinlock_init(void);
odt_enc_dst_info_s *odt_get_dst_chan_info(u32 chan_id);
void odt_clear_enc_src_int_state(u32 chan_id, u32 mode, u32 rd_irq_mask);
void bsp_odt_data_send_continue(u32 enc_dst_chan_id);
int odt_encsrc_task(void *data);
int odt_encdst_task(void *data);
irqreturn_t odt_enc_handler(int irq, void *dev_info);

#ifdef __cplusplus
}
#endif

#endif
