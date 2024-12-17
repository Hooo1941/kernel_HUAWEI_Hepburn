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

#include <timer_driver_arm.h>
#include <bsp_sysctrl.h>
#include <osl_bio.h>
#include <bsp_dt.h>

#define ARM_TIMER_LOAD_OFFSET 0x0
#define ARM_TIMER_VALUE_OFFSET 0x4
#define ARM_TIMER_CTRL_OFFSET 0x8
#define ARM_TIMER_INTCLR_OFFSET 0xC
#define ARM_TIMER_INTMIS_OFFSET 0x14

#define TIMER_CLKCTRL_REG_NUM 2  /* sysctrl中涉及到timer时钟配置的寄存器个数 */

typedef union {
    struct {
        u32 oneshot:1;
        u32 timersize:1;
        u32 timerpre:2;
        u32 :1;
        u32 internable:1;
        u32 timermode:1;
        u32 timeren:1;
        u32 :24;
    };
    u32 ctrl_val;
} ctrl_reg_u;

typedef struct {
    void *en_reg_addr;
    u32 enable_mask;
    void *freq_reg_addr;
    u32 freq_mask;
} timer_sctrl_cfg_s;
timer_sctrl_cfg_s g_timer_sctrl_cfg;

static inline void timer_set_ctrl_value(ctrl_reg_u *reg_value, timer_mode_e mode)
{
    reg_value->ctrl_val = 0;
    reg_value->timersize = 1; /* 默认使用32bit位宽计时 */
    reg_value->internable = 1; /* 中断默认使能 */
    if (mode == TIMER_MODE_ONESHOT) {
        reg_value->oneshot = 1;
        reg_value->timermode = 0;
    } else {
        reg_value->oneshot = 0;
        reg_value->timermode = 1;
    }
    return;
}

void timer_set_ctrl_word_arm(void *ctrl_reg_value, timer_mode_e mode, uintptr_t base_addr)
{
    ctrl_reg_u *reg_value = (ctrl_reg_u *)ctrl_reg_value;
    timer_set_ctrl_value(reg_value, mode);

    writel_relaxed(reg_value->ctrl_val, (void *)(base_addr + ARM_TIMER_CTRL_OFFSET));
    return;
}

u32 timer_get_ctrl_word_arm(uintptr_t base_addr)
{
    return readl_relaxed((void *)(base_addr + ARM_TIMER_CTRL_OFFSET));
}

void timer_set_load_value_arm(u32 tick_num, uintptr_t base_addr)
{
    writel_relaxed(tick_num, (void *)(base_addr + ARM_TIMER_LOAD_OFFSET));
    return;
}

bool timer_is_int_enable_arm(uintptr_t base_addr)
{
    u32 status = readl_relaxed((void *)(base_addr + ARM_TIMER_INTMIS_OFFSET));
    return (bool)(status & 0x1U);
}

void timer_clear_int_arm(uintptr_t base_addr)
{
    writel_relaxed(0x1, (void *)(base_addr + ARM_TIMER_INTCLR_OFFSET));
    /* 清中断加同步，确保中断被清掉 */
    cache_sync();
    return;
}

s32 timer_enable_arm(void *ctrl_reg_value, uintptr_t base_addr)
{
    ctrl_reg_u *reg_value = (ctrl_reg_u *)ctrl_reg_value;
    if (reg_value->timeren == 1) {
        return BSP_ERROR;
    }

    reg_value->timeren = 1;
    writel_relaxed(reg_value->ctrl_val, (void *)(base_addr + ARM_TIMER_CTRL_OFFSET));
    return BSP_OK;
}

void timer_disable_arm(void *ctrl_reg_value, uintptr_t base_addr)
{
    ctrl_reg_u *reg_value = (ctrl_reg_u *)ctrl_reg_value;
    reg_value->timeren = 0;
    writel_relaxed(reg_value->ctrl_val, (void *)(base_addr + ARM_TIMER_CTRL_OFFSET));
    timer_clear_int_arm(base_addr);
    return;
}

bool timer_is_enable_arm(void *ctrl_reg_value)
{
    ctrl_reg_u *reg_value = (ctrl_reg_u *)ctrl_reg_value;
    if (reg_value->timeren == 1) {
        return true;
    } else {
        return false;
    }
}

void timer_start_arm(void *ctrl_reg_value, timer_mode_e mode, uintptr_t base_addr)
{
    ctrl_reg_u *reg_value = (ctrl_reg_u *)ctrl_reg_value;

    timer_set_ctrl_value(reg_value, mode);
    reg_value->timeren = 1;
    writel_relaxed(reg_value->ctrl_val, (void *)(base_addr + ARM_TIMER_CTRL_OFFSET));

    return;
}

u32 timer_get_cur_tick_arm(uintptr_t base_addr)
{
    u32 cur_value = readl_relaxed((void *)(base_addr + ARM_TIMER_VALUE_OFFSET));
    return cur_value;
}

u32 timer_get_init_tick_arm(uintptr_t base_addr)
{
    u32 init_value = readl_relaxed((void *)(base_addr + ARM_TIMER_LOAD_OFFSET));
    return init_value;
}

timer_mode_e timer_get_mode_arm(void *ctrl_reg_value)
{
    ctrl_reg_u *reg_value = (ctrl_reg_u *)ctrl_reg_value;
    return (reg_value->timermode == 0 ? TIMER_MODE_ONESHOT : TIMER_MODE_PERIOD);
}

static inline s32 timer_parse_clk_sctrl(u32 *ap_sctrl, u32 number)
{
    s32 ret;
    device_node_s *node = bsp_dt_find_compatible_node(NULL, NULL, "modem,timer_sysctrl");
    if (node == NULL) {
        timer_err("[DTS]timer_sysctrl node not found!\n");
        return BSP_ERROR;
    }
    ret = bsp_dt_property_read_u32_array(node, "ap_reg", ap_sctrl, number);
    if (ret) {
        timer_err("[DTS]fail to read ap_reg!\n");
        return BSP_ERROR;
    }

    return BSP_OK;
}

s32 timer_store_clk_sctrl_arm(void)
{
    u32 ap_clk_sctrl[TIMER_CLKCTRL_REG_NUM] = { 0 };
    void *ap_sctrl_base = NULL;
    void *ap_sctrl_en_reg = NULL;
    void *ap_sctrl_freq_reg = NULL;

    if (timer_parse_clk_sctrl(ap_clk_sctrl, TIMER_CLKCTRL_REG_NUM)) {
        timer_err("timer_parse_clk_sctrl fail.\n");
        return BSP_ERROR;
    }

    ap_sctrl_base = bsp_sysctrl_addr_byindex(SYSCTRL_AO);
    ap_sctrl_en_reg = ap_sctrl_base + ap_clk_sctrl[0];
    ap_sctrl_freq_reg = ap_sctrl_base + ap_clk_sctrl[1];

    timer_err("ap_sctrl_base=%llu,ap_sctrl_en_reg=0x%llx, ap_sctrl_freq_reg=0x%llx.\n",
        (u64)ap_sctrl_base, (u64)ap_sctrl_en_reg, (u64)ap_sctrl_freq_reg);

    g_timer_sctrl_cfg.freq_reg_addr = ap_sctrl_freq_reg;
    g_timer_sctrl_cfg.freq_mask = readl(ap_sctrl_freq_reg);

    return BSP_OK;
}

void timer_set_freq_mask(u32 *freq_mask, u32 *enable_mask, u32 *timer_cnt, u32 hardware_id, u32 freq)
{
    u32 hw_id = hardware_id & 0xFFFFU;

    u32 mask = (1 << hw_id);
    (*enable_mask) |= mask;
    (*timer_cnt)++;

    if (freq == TIMER_CLK_FREQ_32K) {
        (*freq_mask) &= ~mask;
    } else if (freq == TIMER_CLK_FREQ_19D2M) {
        (*freq_mask) |= mask;
    }
    return;
}

void timer_set_clk_sctrl(void *sctrl_en_reg, void *sctrl_freq_reg, u32 enable_mask, u32 freq_mask)
{
    writel(freq_mask, sctrl_freq_reg);
    return;
}

void timer_set_clk_freq_arm(u32 *hw_id, uintptr_t *base_addr, u32 *freq, u32 num)
{
    u32 timer_cnt = 0;
    u32 i;
    for (i = 0; i < num; i++) {
        timer_set_freq_mask(
            &g_timer_sctrl_cfg.freq_mask, &g_timer_sctrl_cfg.enable_mask, &timer_cnt, hw_id[i], freq[i]);
    }

    if (timer_cnt != 0) {
        timer_set_clk_sctrl(g_timer_sctrl_cfg.en_reg_addr, g_timer_sctrl_cfg.freq_reg_addr,
            g_timer_sctrl_cfg.enable_mask, g_timer_sctrl_cfg.freq_mask);
    }

    return;
}

timer_clk_ops_s g_timer_clk_ops_arm = {
    .ip_type = TIMER_IP_ARM,
    .store_clk_sctrl = NULL, /* 目前sysctrl中使能和配置AO区timer的寄存器都被设置成了安全属性，非安全态不可访问。 */
    .set_clk_freq = NULL,
};

timer_driver_ops_s g_timer_driver_ops_arm = {
    .enable = timer_enable_arm,
    .disable = timer_disable_arm,
    .is_enable = timer_is_enable_arm,
    .start = timer_start_arm,
    .set_ctrl_word = timer_set_ctrl_word_arm,
    .get_ctrl_word = timer_get_ctrl_word_arm,
    .set_load_value = timer_set_load_value_arm,
    .clear_int = timer_clear_int_arm,
    .get_cur_tick = timer_get_cur_tick_arm,
    .get_init_tick = timer_get_init_tick_arm,
    .is_int_enable = timer_is_int_enable_arm,
    .get_mode = timer_get_mode_arm,
};

timer_driver_ops_s *timer_get_drv_ops_arm(void)
{
    return &g_timer_driver_ops_arm;
}

timer_clk_ops_s *timer_get_clk_ops_arm(void)
{
    return &g_timer_clk_ops_arm;
}
