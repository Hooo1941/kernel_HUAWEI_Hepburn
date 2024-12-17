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

#ifndef __BSP_TIMER_DEVICE_H__
#define __BSP_TIMER_DEVICE_H__

#include <mdrv_timer.h>
#include <timer_hal.h>
#include <linux/spinlock.h>
#include <bsp_slice.h>

#define TIMER_TASK_PRIORITY 92
#define TIMER_TASK_STACK_SIZE 0x1000
#define TIMER_IRQ_PRIORITY 15
#define TIMER_SEC_TO_USEC 1000000       /* 1秒=1000000微秒 */
#define TIMER_SEC_TO_MSEC 1000          /* 1秒=1000毫秒 */
#define TIMER_SEC_TO_USEC_ROUND 500000  /* 用于除法计算时的四舍五入 */
#define TIMER_MAX_TICK_NUM 0xFFFFFFFF
#define TIMER_DEBUG_INFO_NUMBER 4
#define TIMER_PLATFORM_NAME "b_timer"

/* DTS中属性配置数组每个元素对应的含义 */
enum {
    TIMER_DTS_HW_ID,
    TIMER_DTS_BASE_ADDR,
    TIMER_DTS_TYPE,
    TIMER_DTS_SR,
    TIMER_DTS_CELL_SZ
};

enum {
    TIMER_TYPE_N_WKSRC, /* 非唤醒源 */
    TIMER_TYPE_WKSRC,   /* 唤醒源 */
    TIMER_TYPE_MAX
};

/* 维测 */
typedef struct {
    u32 slice_input;
    u32 slice_timeout;
    u32 slice_cb_cost;
} timer_debug_info_s;

typedef struct {
    char name[TIMER_NAME_MAX_LEN];
    bool used;                  /* 是否被占用 */
    u32 hw_id;                  /* 硬件id，用于设置时钟频率 */
    uintptr_t base_addr;        /* 基地址 */
    u16 ip_type;                /* TIMER_IP_ARM:arm timer；TIMER_IP_MODEMmodem timer */
    u16 irq;                    /* 中断号 */
    u32 wake_irq;               /* 唤醒中断号，当前mbb生效 */
    u16 wake_type;              /* TIMER_TYPE_N_WKSRC:非唤醒源；TIMER_TYPE_WKSRC:唤醒源 */
    u8 suspend_resume;          /* 低功耗备份恢复标志 */
    u8 time_unit;               /* 指示时间单位, 0：微秒；1：时钟tick */
    u32 clk_freq;               /* 时钟频率 */
    u32 ctrl_value_latest;      /* 最后一次配进控制寄存器的值 */
    u32 load_value_latest;      /* 最后一次配置的计数初值 */
    u32 dpm_resume_value;       /* 待恢复的计数初值，用于低功耗备份恢复流程 */
    bool dmp_resume_work;       /* 指示resume时是否enable定时器 */
    timer_handle_cb callback;   /* 中断回调函数 */
    s32 cb_para;                /* 中断回调传参 */
    timer_driver_ops_s *driver_ops;    /* 驱动接口 */

    u32 timeout_count;
    u32 slice_pre_irqrcvd;
    timer_debug_info_s debug_info[TIMER_DEBUG_INFO_NUMBER];
} timer_device_s;

typedef struct {
    u64 tick_base;
    u64 tick_previous;
    u64 tick_later;
    u64 tick_delta;
    u64 slice_base;
    u64 slice_wake;
    u64 slice_delta;
} timer_systick_comp_s; /* 系统tick低功耗补偿 */

typedef struct {
    u32 number;
    spinlock_t lock;
    timer_device_s *device;             /* 设备信息和控制 */
    timer_clk_ops_s *clk_ops;           /* 时钟频率设置接口 */
} timer_device_mgr_s;

/* 获取device控制数据块 */
timer_device_mgr_s *timer_get_device_mgr(void);
timer_device_s *timer_get_device(u32 timer_id);
timer_clk_ops_s *timer_get_clk_ops(void);

s32 timer_dev_alloc(u32 wake_type, u32 *timer_id);
void timer_dev_free(timer_device_s *device);
void timer_start(timer_device_s *device, u32 mode, u32 tick_num);
s32 timer_instances_init(void);
s32 timer_clk_freq_init(void);

/* 计算计time_us微秒需要多少个tick */
static inline u64 timer_calc_tick_num(u32 time_us, u32 clk_freq)
{
    u64 numerator = ((u64)clk_freq) * ((u64)time_us);
    u64 tick_num = ((numerator + TIMER_SEC_TO_USEC_ROUND) / TIMER_SEC_TO_USEC);
    return (tick_num < 1 ? 1 : tick_num);
}

static inline void timer_set_clk_freq(timer_device_s *device, u32 clk_freq)
{
    u32 hw_id = device->hw_id;
    uintptr_t base_addr = device->base_addr;
    u32 freq = clk_freq;
    timer_clk_ops_s *clk_ops = timer_get_clk_ops();
    clk_ops->set_clk_freq(&hw_id, &base_addr, &freq, 1);
    device->clk_freq = clk_freq;
    return;
}

#endif
