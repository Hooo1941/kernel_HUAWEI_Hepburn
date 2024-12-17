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

#include <timer_driver_modem.h>
#include <osl_bio.h>

#define MODEM_TIMER_CLK_CTRL_OFFSET 0
#define MODEM_TIMER_LOAD_OFFSET 0x4
#define MODEM_TIMER_CTRL_OFFSET 0x8
#define MODEM_TIMER_INTR_OFFSET 0xC
#define MODEM_TIMER_INTR_CLR_OFFSET 0x10
#define MODEM_TIMER_VALUE_OFFSET 0x14
#define TIMER_CTRL_MASK 0xFFFF0000

typedef union {
    struct {
        u32 timer_en:1;
        u32 timer_size:1;
        u32 timer_stop:1;
        u32 timer_reload_sel:1;
        u32 timer_int_mask:1;
        u32 reserved:27;
    };
    u32 ctrl_val;
} ctrl_reg_u;


static inline void timer_set_ctrl_value(ctrl_reg_u *ctrl_reg_value, timer_mode_e mode)
{
    ctrl_reg_u *reg_value = (ctrl_reg_u *)ctrl_reg_value;
    reg_value->ctrl_val = 0;
    reg_value->timer_size = 1; /* 默认使用32bit位宽计时 */
    reg_value->timer_int_mask = 0; /* 中断默认使能 */
    reg_value->timer_reload_sel = 1;
    if (mode == TIMER_MODE_ONESHOT) {
        reg_value->timer_stop = 1;
    } else {
        reg_value->timer_stop = 0;
    }
    /* V510版本开始 ctrl增加高16bit bitmasken */
    reg_value->ctrl_val = (reg_value->ctrl_val) | TIMER_CTRL_MASK;
    return;
}

void timer_set_ctrl_word_modem(void *ctrl_reg_value, timer_mode_e mode, uintptr_t base_addr)
{
    ctrl_reg_u *reg_value = (ctrl_reg_u *)ctrl_reg_value;
    timer_set_ctrl_value(reg_value, mode);
    writel_relaxed(reg_value->ctrl_val, (void *)(base_addr + MODEM_TIMER_CTRL_OFFSET));

    return;
}

u32 timer_get_ctrl_word_modem(uintptr_t base_addr)
{
    return (readl_relaxed((void *)(base_addr + MODEM_TIMER_CTRL_OFFSET)) | TIMER_CTRL_MASK);
}

void timer_set_load_value_modem(u32 tick_num, uintptr_t base_addr)
{
    writel_relaxed(tick_num, (void *)(base_addr + MODEM_TIMER_LOAD_OFFSET));
    return;
}

bool timer_is_int_enable_modem(uintptr_t base_addr)
{
    u32 status = readl_relaxed((void *)(base_addr + MODEM_TIMER_INTR_OFFSET));
    return (bool)(status & 0x1);
}

void timer_clear_int_modem(uintptr_t base_addr)
{
    writel_relaxed(0x1, (void *)(base_addr + MODEM_TIMER_INTR_CLR_OFFSET));
    /* 清中断加同步，确保中断被清掉 */
    cache_sync();
    return;
}

s32 timer_enable_modem(void *ctrl_reg_value, uintptr_t base_addr)
{
    ctrl_reg_u *reg_value = (ctrl_reg_u *)ctrl_reg_value;
    if (reg_value->timer_en == 1) {
        return BSP_ERROR;
    }

    reg_value->timer_en = 1;
    writel_relaxed(reg_value->ctrl_val, (void *)(base_addr + MODEM_TIMER_CTRL_OFFSET));
    return BSP_OK;
}

void timer_disable_modem(void *ctrl_reg_value, uintptr_t base_addr)
{
    ctrl_reg_u *reg_value = (ctrl_reg_u *)ctrl_reg_value;
    reg_value->timer_en = 0;
    writel_relaxed(reg_value->ctrl_val, (void *)(base_addr + MODEM_TIMER_CTRL_OFFSET));
    timer_clear_int_modem(base_addr);

    return;
}

bool timer_is_enable_modem(void *ctrl_reg_value)
{
    ctrl_reg_u *reg_value = (ctrl_reg_u *)ctrl_reg_value;
    if (reg_value->timer_en == 1) {
        return true;
    } else {
        return false;
    }
}

void timer_start_modem(void *ctrl_reg_value, timer_mode_e mode, uintptr_t base_addr)
{
    ctrl_reg_u *reg_value = (ctrl_reg_u *)ctrl_reg_value;

    timer_set_ctrl_value(reg_value, mode);
    reg_value->timer_en = 1;
    writel_relaxed(reg_value->ctrl_val, (void *)(base_addr + MODEM_TIMER_CTRL_OFFSET));

    return;
}

u32 timer_get_cur_tick_modem(uintptr_t base_addr)
{
    u32 cur_value = readl_relaxed((void *)(base_addr + MODEM_TIMER_VALUE_OFFSET));
    return cur_value;
}

u32 timer_get_init_tick_modem(uintptr_t base_addr)
{
    u32 init_value = readl_relaxed((void *)(base_addr + MODEM_TIMER_LOAD_OFFSET));
    return init_value;
}

timer_mode_e timer_get_mode_modem(void *ctrl_reg_value)
{
    ctrl_reg_u *reg_value = (ctrl_reg_u *)ctrl_reg_value;
    return (reg_value->timer_stop == 1 ? TIMER_MODE_ONESHOT : TIMER_MODE_PERIOD);
}

void timer_set_clk_freq_modem(u32 *hw_id, uintptr_t *base_addr, u32 *freq, u32 num)
{
    u32 i;
    for (i = 0; i < num; i++) {
        if (freq[i] == TIMER_CLK_FREQ_32K) {
            /* V510版本开始clk ctrl增加高16bit bitmasken */
            writel(0x70004, (void *)(base_addr[i] + MODEM_TIMER_CLK_CTRL_OFFSET));
        } else {
            writel(0x70006, (void *)(base_addr[i] + MODEM_TIMER_CLK_CTRL_OFFSET));
        }
    }

    return;
}

timer_clk_ops_s g_timer_clk_ops_modem = {
    .ip_type = TIMER_IP_MODEM,
    .store_clk_sctrl = NULL, /* modem timer在ip内部配置时钟频率，无须保存sysctrl的偏移地址 */
    .set_clk_freq = timer_set_clk_freq_modem,
};

timer_driver_ops_s g_timer_driver_ops_modem = {
    .enable = timer_enable_modem,
    .disable = timer_disable_modem,
    .is_enable = timer_is_enable_modem,
    .start = timer_start_modem,
    .set_ctrl_word = timer_set_ctrl_word_modem,
    .get_ctrl_word = timer_get_ctrl_word_modem,
    .set_load_value = timer_set_load_value_modem,
    .clear_int = timer_clear_int_modem,
    .get_cur_tick = timer_get_cur_tick_modem,
    .get_init_tick = timer_get_init_tick_modem,
    .is_int_enable = timer_is_int_enable_modem,
    .get_mode = timer_get_mode_modem,
};

timer_driver_ops_s *timer_get_drv_ops_modem(void)
{
    return (timer_driver_ops_s *)(&g_timer_driver_ops_modem);
}

timer_clk_ops_s *timer_get_clk_ops_modem(void)
{
    return (timer_clk_ops_s *)(&g_timer_clk_ops_modem);
}
