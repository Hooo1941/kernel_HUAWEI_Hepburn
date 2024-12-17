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

#ifndef __BSP_TIMER_HAL_H__
#define __BSP_TIMER_HAL_H__

#include <mdrv_timer.h>
#include <bsp_print.h>
#include <osl_types.h>

#undef THIS_MODU 
#define THIS_MODU mod_aptimer
#define timer_err(fmt, ...) (bsp_err(fmt, ##__VA_ARGS__))

#define TIMER_CLK_FREQ_32K 32764         /* 32K时钟频率 */
#define TIMER_CLK_FREQ_19D2M 19200000    /* 19.2M时钟频率 */

/* 硬件IP的类型 */
typedef enum {
    TIMER_IP_ARM,
    TIMER_IP_MODEM,
    TIMER_IP_TYPE_MAX
} timer_ip_type_e;

typedef struct {
    s32 (*enable)(void *ctrl_reg_value, uintptr_t base_addr);
    void (*disable)(void *ctrl_reg_value, uintptr_t base_addr);
    bool (*is_enable)(void *ctrl_reg_value);
    void (*start)(void *ctrl_reg_value, timer_mode_e mode, uintptr_t base_addr); /* 配置定时模式的同时启动定时器 */
    void (*set_ctrl_word)(void *ctrl_reg_value, timer_mode_e mode, uintptr_t base_addr);
    u32 (*get_ctrl_word)(uintptr_t base_addr);
    void (*set_load_value)(u32 tick_num, uintptr_t base_addr);
    void (*clear_int)(uintptr_t base_addr);
    u32 (*get_cur_tick)(uintptr_t base_addr);
    u32 (*get_init_tick)(uintptr_t base_addr);
    bool (*is_int_enable)(uintptr_t base_addr); /* false:中断未使能；true:中断已使能 */
    timer_mode_e (*get_mode)(void *ctrl_reg_value);
} timer_driver_ops_s;

typedef struct {
    timer_ip_type_e ip_type;
    s32 (*store_clk_sctrl)(void);
    void (*set_clk_freq)(u32 *hw_id, uintptr_t *base_addr, u32 *freq, u32 num);
} timer_clk_ops_s;

s32 timer_set_driver_ops(u32 ip_type, timer_driver_ops_s **driver_ops);
s32 timer_clk_ops_init(u32 ip_type, timer_clk_ops_s **clk_ops);

#endif
