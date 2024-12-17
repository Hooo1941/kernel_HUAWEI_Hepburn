/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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
#include <sys_bus_api.h>
#include <sys_sample.h>

#include <linux/kthread.h>
#include <linux/of.h>

u32 g_sys_bus_sample_task_id; // 任务ID
volatile u32 g_sys_bus_sample_run; // 任务执行标记：0 - 退出任务执行；1 - 循环执行任务

void sys_sample_task(void* data)
{
    sys_bus_err("[sys_bus]sample, entry task, running flag: %d.\n", g_sys_bus_sample_run);

    while (g_sys_bus_sample_run == 1) {
        // 执行用例。
    }

    return;
}

// 启动当前IP的总线压测。
// 建议创建线程，并绑核来执行用例。
void sys_sample_start(void)
{
    struct task_struct* tsk = NULL;
    char* task_name = "sys_bus_sample";

    g_sys_bus_sample_run = 1;

    tsk = kthread_run((void*)sys_sample_task, NULL, task_name);
    if (IS_ERR(tsk)) {
        sys_bus_trace("[sys_bus]sample, cannot start thread!\n");
        return;
    }

    sys_bus_trace("[sys_bus]sample, task start\n");
    return;
}

// 停止当前IP的总线压测。
void sys_sample_stop(void)
{
    g_sys_bus_sample_run = 0;

    // 执行其他停止的动作。
    return;
}

// 打印当前IP的总线压测上下文、调试信息等。
void sys_sample_report(void)
{
    return;
}

// 当前IP的总线压测事件入口。
int sys_sample_entry(sys_bus_evt_e event)
{
    sys_bus_err("[sys_bus]sample, rcv evt: %d\n", event);

    if (event == E_SYS_BUS_EVT_START) {
        sys_sample_start();
    } else if (event == E_SYS_BUS_EVT_STOP) {
        sys_sample_stop();
    } else if (event == E_SYS_BUS_EVT_REPORT) {
        sys_sample_report();
    } else {
        sys_bus_err("receive incorrect event: %d\n", event);
        return E_SYS_BUS_RET_ERR;
    }

    return E_SYS_BUS_RET_OK;
}

void sys_sample_init(struct device_node* np)
{
    // IP测试模块初始化。
    // 注册当前IP的总线压测事件入口函数。
    sys_bus_register_observer(SYS_SAMPLE_NAME, sys_sample_entry);

    sys_bus_trace("ip - example init ok\n");
}
