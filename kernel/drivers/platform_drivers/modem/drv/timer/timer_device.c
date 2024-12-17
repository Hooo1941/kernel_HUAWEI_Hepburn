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

#include <timer_device.h>
#include <timer_dpm.h>
#include <bsp_dt.h>
#include <osl_malloc.h>
#include <osl_irq.h>
#include <securec.h>
#include <asm/io.h>
#include <linux/printk.h>

timer_device_mgr_s g_timer_device_mgr;

timer_device_mgr_s *timer_get_device_mgr(void)
{
    return (timer_device_mgr_s *)(&g_timer_device_mgr);
}

timer_device_s *timer_get_device(u32 timer_id)
{
    return (timer_device_s *)(&(g_timer_device_mgr.device[timer_id]));
}

timer_clk_ops_s *timer_get_clk_ops(void)
{
    return g_timer_device_mgr.clk_ops;
}

s32 timer_dev_alloc(u32 wake_type, u32 *timer_id)
{
    u32 i;
    unsigned long flags;
    timer_device_s *device = NULL;

    spin_lock_irqsave(&(g_timer_device_mgr.lock), flags);
    for (i = 0; i < g_timer_device_mgr.number; i++) {
        device = (timer_device_s *)(&(g_timer_device_mgr.device[i]));
        if ((wake_type != device->wake_type) || (device->used)) {
            continue;
        }
        device->used = true;
        *timer_id = i;
        break;
    }
    spin_unlock_irqrestore(&(g_timer_device_mgr.lock), flags);

    if (i == g_timer_device_mgr.number) {
        return BSP_ERROR;
    }

    return BSP_OK;
}

void timer_dev_free(timer_device_s *device)
{
    unsigned long flags;

    if (device->driver_ops->is_enable((void *)(&(device->ctrl_value_latest)))) {
        device->driver_ops->disable((void *)(&(device->ctrl_value_latest)), device->base_addr);
    }

    spin_lock_irqsave(&(g_timer_device_mgr.lock), flags);
    device->used = false;
    spin_unlock_irqrestore(&(g_timer_device_mgr.lock), flags);

    return;
}

void timer_start(timer_device_s *device, u32 mode, u32 tick_num)
{
    device->load_value_latest = (u32)tick_num;
    device->driver_ops->set_load_value(tick_num, device->base_addr);
    device->driver_ops->start((void *)(&(device->ctrl_value_latest)), mode, device->base_addr);
    return;
}

static inline s32 timer_parse_res_number(device_node_s *node)
{
    u32 number = 0;
    if (bsp_dt_property_read_u32(node, "number", &number)) {
        timer_err("[DTS]number is not found!\n");
        return BSP_ERROR;
    }
    g_timer_device_mgr.number = number;
    spin_lock_init(&(g_timer_device_mgr.lock)); /* 用于资源分配，防止两个用户申请到的是同一个timer */

    return BSP_OK;
}

static inline s32 timer_clkctrl_init(device_node_s *node, u32 *ip_type)
{
    s32 ret = bsp_dt_property_read_u32(node, "ip_type", ip_type);
    if (ret) {
        timer_err("fail to read ip_type! ret=0x%x.\n", ret);
        return BSP_ERROR;
    }

    if (timer_clk_ops_init(*ip_type, &(g_timer_device_mgr.clk_ops))) {
        return BSP_ERROR;
    }

    return BSP_OK;
}

/* 获取各个timer硬件资源的基地址、中断号、唤醒类型和低功耗备份恢复标识 */
static inline s32 timer_parse_each_res(device_node_s *node, device_node_s *wakenode, u32 number, u32 ip_type)
{
    s32 ret;
    u32 i, offset, cell_size;
    u32 *dev_desc = NULL;
    timer_device_s *device = NULL;

    ret = bsp_dt_property_read_u32(node, "cell_size", &cell_size);
    if (ret || (cell_size < TIMER_DTS_CELL_SZ)) {
        timer_err("fail to read cell_size! ret=0x%x, cell_size=%d.\n", ret, cell_size);
        return BSP_ERROR;
    }

    /* dev_desc用于存储dts中timer的设备描述信息 */
    dev_desc = (u32 *)osl_malloc(number * cell_size * sizeof(u32));
    if (dev_desc == NULL) {
        timer_err("fail to malloc mem for dev_desc!\n");
        return BSP_ERROR;
    }

    ret = bsp_dt_property_read_u32_array(node, "devices", dev_desc, number * cell_size);
    if (ret) {
        osl_free((void *)dev_desc);
        timer_err("fail to read devices! ret=0x%x.\n", ret);
        return BSP_ERROR;
    }

    for (i = 0; i < number; i++) {
        offset = i * cell_size;
        device = (timer_device_s *)(&(g_timer_device_mgr.device[i]));
        device->hw_id = dev_desc[offset];
        device->base_addr = (uintptr_t)ioremap(dev_desc[offset + TIMER_DTS_BASE_ADDR], 0x1000);
        /* 当前按照同一个平台只支持一种类型的ip来设计，后续如果某个平台同时有多个类型的ip，
           在设备描述字段中增加一项配置即可 */
        device->ip_type = ip_type;
        device->wake_irq = 0xff;
        device->irq = bsp_dt_irq_parse_and_map(node, i);
        if (wakenode != NULL) {
            if (bsp_dt_property_read_u32_index(wakenode, "interrupts", i, &device->wake_irq)) {
                timer_err("get %s interrupts fail\n", wakenode->name);
                device->wake_irq = 0xff;
            }
        }
        if (device->wake_irq != 0xff) {
            device->wake_irq = bsp_dt_irq_parse_and_map(wakenode, i);
        }
        device->wake_type = dev_desc[offset + TIMER_DTS_TYPE];
        device->suspend_resume = dev_desc[offset + TIMER_DTS_SR];
        device->time_unit = 0;
        device->clk_freq = TIMER_CLK_FREQ_32K; /* AP timer默认都使用32K时钟频率 */
        if (timer_set_driver_ops(ip_type, &(device->driver_ops))) {
            osl_free((void *)dev_desc);
            return BSP_ERROR;
        }
    }

    osl_free((void *)dev_desc);

    return BSP_OK;
}

/* 完成设备信息的收集 */
s32 timer_dev_res_init(device_node_s *node, device_node_s *wakenode)
{
    u32 ip_type = 0;
    u32 number = g_timer_device_mgr.number;
    timer_device_s *device = (timer_device_s *)osl_malloc(number * sizeof(timer_device_s));
    if (device == NULL) {
        timer_err("fail to malloc mem for device.\n");
        return BSP_ERROR;
    }
    (void)memset_s(device, number * sizeof(timer_device_s), 0, number * sizeof(timer_device_s));
    g_timer_device_mgr.device = device;

    if (timer_clkctrl_init(node, &ip_type)) {
        return BSP_ERROR;
    }

    if (timer_parse_each_res(node, wakenode, number, ip_type)) {
        return BSP_ERROR;
    }

    return BSP_OK;
}

s32 timer_parse_dts(void)
{
    device_node_s *node = NULL;
    device_node_s *wakenode = NULL;
    node = bsp_dt_find_node_by_path("/timer_device");
    if (node == NULL) {
        timer_err("[DTS]timer_device node not found!\n");
        return BSP_ERROR;
    }
    wakenode = bsp_dt_find_node_by_path("/timer_wake");
    if (wakenode == NULL) {
        timer_err("[DTS]timer_wake node not found!\n");
    }

    if (timer_parse_res_number(node)) {
        return BSP_ERROR;
    }

    if (timer_dev_res_init(node, wakenode)) {
        return BSP_ERROR;
    }

    return BSP_OK;
}

s32 timer_clk_freq_init(void)
{
    u32 i;
    timer_device_s *device = NULL;
    u32 *hw_id = NULL;
    u32 *freq = NULL;
    uintptr_t *base_addr = NULL;
    u32 timer_num = g_timer_device_mgr.number;

    /* 如果set_clk_freq为NULL，说明无须设置时钟 */
    if (g_timer_device_mgr.clk_ops->set_clk_freq == NULL) {
        return BSP_OK;
    }

    hw_id = (u32 *)osl_malloc(timer_num * sizeof(u32));
    base_addr = (uintptr_t *)osl_malloc(timer_num * sizeof(uintptr_t));
    freq = (u32 *)osl_malloc(timer_num * sizeof(u32));
    if ((hw_id == NULL) || (base_addr == NULL) || (freq == NULL)) {
        timer_err("fail to malloc mem.\n");
        return BSP_ERROR;
    }

    for (i = 0; i < timer_num; i++) {
        device = (timer_device_s *)(&g_timer_device_mgr.device[i]);
        hw_id[i] = device->hw_id;
        base_addr[i] = device->base_addr;
        freq[i] = device->clk_freq;
    }

    g_timer_device_mgr.clk_ops->set_clk_freq(hw_id, base_addr, freq, timer_num);

    osl_free(hw_id);
    osl_free(base_addr);
    osl_free(freq);

    return BSP_OK;
}

OSL_IRQ_FUNC(static irqreturn_t, timer_irq_callback, irq_id, arg)
{
    u32 idx;
    u32 slice_start_cb;
    u32 slice_end_cb;
    timer_device_s *device = (timer_device_s *)arg;

    if (device->driver_ops->is_int_enable(device->base_addr)) {
        device->driver_ops->clear_int(device->base_addr);
        if (device->driver_ops->get_mode((void *)(&(device->ctrl_value_latest))) == TIMER_MODE_ONESHOT) {
            device->driver_ops->disable((void *)(&(device->ctrl_value_latest)), device->base_addr);
        }

        idx = device->timeout_count & 0x3;
        slice_start_cb = bsp_get_slice_value_hrt();
        device->debug_info[idx].slice_timeout = slice_start_cb - device->slice_pre_irqrcvd;
        device->debug_info[idx].slice_input = device->load_value_latest;
        if (device->callback != NULL) {
            device->callback(device->cb_para);
        }
        slice_end_cb = bsp_get_slice_value_hrt();
        device->debug_info[idx].slice_cb_cost = slice_end_cb - slice_start_cb;
        device->slice_pre_irqrcvd = slice_start_cb;
        device->timeout_count++;
    }

    return IRQ_HANDLED;
}

s32 timer_irq_init(void)
{
    u32 i;
    s32 ret;
    unsigned long irq_flags;
    timer_device_s *device = NULL;
    for (i = 0; i < g_timer_device_mgr.number; i++) {
        device = (timer_device_s *)(&(g_timer_device_mgr.device[i]));
        irq_flags = (device->wake_type == TIMER_TYPE_WKSRC ? IRQF_NO_SUSPEND : 0);
        if (device->wake_irq != 0xff) {
            ret = request_irq(device->wake_irq, (irq_handler_t)timer_irq_callback, irq_flags, "timer", NULL);
            if (ret) {
                timer_err("fail to register wake_irq, timer_id=%d, irq=%d, ret=0x%x.\n", i, device->wake_irq, ret);
                return BSP_ERROR;
            }
            enable_irq_wake(device->wake_irq);
        } else if (device->wake_type == TIMER_TYPE_WKSRC) {
            /* 针对phone注册便于唤醒统计 */
            enable_irq_wake(device->irq);
        }
        /* 此阶段普通timer和systimer已注册中断，此处不再二次注册 */
        if (device->used) {
            continue;
        }

        ret = request_irq(device->irq, (irq_handler_t)timer_irq_callback, irq_flags, "timer",
            (void *)((uintptr_t)device));
        if (ret) {
            timer_err("fail to register irq, timer_id=%d, irq=%d, ret=0x%x.\n", i, device->irq, ret);
            return BSP_ERROR;
        }
    }

    return BSP_OK;
}

s32 bsp_timer_init(void)
{
    /* 解析dts */
    if (timer_parse_dts()) {
        return BSP_ERROR;
    }

    /* 设置timer的频率 */
    if (timer_clk_freq_init()) {
        return BSP_ERROR;
    }

    /* 初始化普通timer */
    if (timer_instances_init()) {
        return BSP_ERROR;
    }

    /* 注册中断，唤醒源中断也在此处注册 */
    if (timer_irq_init()) {
        return BSP_ERROR;
    }

    /* 低功耗相关处理初始化 */
    if (timer_dpm_init()) {
        return BSP_ERROR;
    }

    timer_err("timer init OK.");

    return BSP_OK;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
arch_initcall(bsp_timer_init);
#endif

EXPORT_SYMBOL(timer_get_device_mgr);
