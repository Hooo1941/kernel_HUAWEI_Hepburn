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
#include <bsp_slice.h>
#include <linux/string.h>
#include <linux/platform_device.h>
#include <linux/syscore_ops.h>

timer_pmdbg_list_s g_timer_pmdbg_list;

struct platform_device g_timer_platform_device = {
    .name = TIMER_PLATFORM_NAME,
    .id = -1,
    .num_resources = 0,
};

#ifdef CONFIG_PM
void timer_suspend(timer_device_s *device)
{
    /* 协议栈需求,唤醒后VOS_TIMER接着进入睡眠时的定时进度继续定时 */
    if (strcmp(device->name, "VOS_TIMER") == 0) {
        device->dpm_resume_value = device->driver_ops->get_cur_tick(device->base_addr);
    } else {
        device->dpm_resume_value = device->load_value_latest;
    }
    device->dmp_resume_work = true;

    device->driver_ops->disable((void *)(&(device->ctrl_value_latest)), device->base_addr);
    device->driver_ops->clear_int(device->base_addr);

    return;
}

s32 timer_suspend_noirq(struct device *dev)
{
    u32 i;
    u32 stamp;
    timer_device_s *device = NULL;
    timer_device_mgr_s *device_mgr = timer_get_device_mgr();
    u32 number = device_mgr->number;

    pr_info("%s +\n", __func__);

    for (i = 0; i < number; i++) {
        device = timer_get_device(i);
        /* 满足三个条件时走备份恢复流程：1.已分配；2.非唤醒源；3.当前正在运行
           不需要备份时将count_resume_value置为0，便于恢复时判断需不需要恢复 */
        if ((device->used) && (device->suspend_resume) &&
            (device->driver_ops->is_enable((void *)(&(device->ctrl_value_latest))))) {
            timer_suspend(device);
        } else {
            device->dpm_resume_value = 0;
        }
    }

    stamp = bsp_get_slice_value();
    while (get_timer_slice_delta(stamp, bsp_get_slice_value()) < 0x2) {
    }
    pr_info("%s -\n", __func__);

    return BSP_OK;
}

void timer_resume(timer_device_s *device)
{
    timer_mode_e mode = device->driver_ops->get_mode((void *)(&(device->ctrl_value_latest)));
    timer_start(device, mode, device->dpm_resume_value);

    return;
}

s32 timer_resume_noirq(struct device *dev)
{
    u32 i;
    u32 mode;
    timer_device_s *device = NULL;
    timer_device_mgr_s *device_mgr = timer_get_device_mgr();
    u32 number = device_mgr->number;

    pr_info("%s +\n", __func__);

    timer_clk_freq_init();

    for (i = 0; i < number; i++) {
        device = timer_get_device(i);
        if ((device->used) && (device->suspend_resume)) {
            /* 不论唤醒后还需不需要继续定时，都需要先恢复配置寄存器 */
            mode = device->driver_ops->get_mode((void *)(&(device->ctrl_value_latest)));
            device->driver_ops->set_ctrl_word((void *)(&(device->ctrl_value_latest)), mode, device->base_addr);

            /* 如果suspend时还在计时，唤醒后使能寄存器继续定时 */
            if (device->dmp_resume_work) {
                device->driver_ops->set_load_value(device->dpm_resume_value, device->base_addr);
                device->load_value_latest = device->dpm_resume_value;

                device->driver_ops->enable((void *)(&(device->ctrl_value_latest)), device->base_addr);
                device->dmp_resume_work = false;
            }
        }
    }

    pr_info("%s -\n", __func__);

    return BSP_OK;
}

const struct dev_pm_ops g_timer_drv_pm_ops = {
    .suspend_noirq = timer_suspend_noirq,
    .resume_noirq = timer_resume_noirq,
};

#define TIMER_DRV_PM_OPS (&g_timer_drv_pm_ops)
#else
#define TIMER_DRV_PM_OPS NULL
#endif

static int timer_drv_probe_stub(struct platform_device *dev)
{
    return BSP_OK;
}

struct platform_driver g_timer_platform_driver = {
    .probe = timer_drv_probe_stub,
    .driver = {
        .name = TIMER_PLATFORM_NAME,
        .owner = THIS_MODULE,
        .pm = TIMER_DRV_PM_OPS,
    },
};

void timer_dpm_dbg_register(timer_pmdbg_s *node)
{
    list_add_tail(&(node->entry), &(g_timer_pmdbg_list.list_head));
    return;
}

/* 恢复过程中执行，来自协议栈的需求 */
void timer_resume_debug(void)
{
    timer_pmdbg_s *node = NULL;
    u32 timer_id;
    timer_device_s *device = NULL;

    list_for_each_entry(node, &(g_timer_pmdbg_list.list_head), entry) {
        timer_id = node->timer_id;
        device = timer_get_device(timer_id);
        if (likely((node->dpm_dbg_func != NULL) && (device->driver_ops->is_int_enable(device->base_addr)))) {
            (void)node->dpm_dbg_func(node->dpm_dbg_para);
        }
    }

    return;
}

static struct syscore_ops g_timer_dpmdbg_ops = {
    .resume = timer_resume_debug,
};

s32 timer_dpm_init(void)
{
    s32 ret;

    /* 注册timer低功耗备份恢复接口 */
    INIT_LIST_HEAD(&(g_timer_pmdbg_list.list_head));

    register_syscore_ops(&g_timer_dpmdbg_ops);

    /* 注册timer低功耗钩子 */
    ret = platform_device_register(&g_timer_platform_device);
    if (ret) {
        timer_err("fail to register timer platform device!\n");
        return BSP_ERROR;
    }

    ret = platform_driver_register(&g_timer_platform_driver);
    if (ret) {
        timer_err("fail to register timer platform device driver!\n");
        platform_device_unregister(&g_timer_platform_device);
        return BSP_ERROR;
    }

    return BSP_OK;
}
