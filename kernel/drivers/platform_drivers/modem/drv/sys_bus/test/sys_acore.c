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

#include <linux/version.h>
#include <linux/of.h>
#include <linux/dma-map-ops.h>
#include <linux/dma-mapping.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/of_device.h>
#include <bsp_sysctrl.h>
#include <sys_bus_api.h>
#include "sys_acore.h"
#include <securec.h>

struct sys_acore g_sys_acore;
volatile u32 g_ap_visit_lr_nr_reg_flag = 0;
u32 g_sysctrl_mdm_addr = 0;

void lr_nr_reg_pressure_start(void)
{
    g_ap_visit_lr_nr_reg_flag = 1;
}

void lr_nr_reg_pressure(void)
{
    u32 i = 0;
    if (g_ap_visit_lr_nr_reg_flag) {
        for (i = 0; i < SYS_ACORE_TEST_CIRCLE; i++)
            (void)readl((void *)(uintptr_t)g_sysctrl_mdm_addr + i * SYS_ACORE_TEST_OFFSET);
    }
}

void sys_acore_report(void) {}

void sys_acore_start(void)
{
    sys_bus_trace("[sys_bus]acore, task start\n");
    g_sys_acore.flag = 1;
}

void sys_acore_stop(void)
{
    sys_bus_trace("[sys_bus]acore, task stop\n");
    g_sys_acore.flag = 0;
}

void sys_acore_thread(void)
{
    volatile u32 tmp_flag = 0;
    while (1) {
        tmp_flag = g_sys_acore.flag;
        if (tmp_flag) {
            lr_nr_reg_pressure();
            memset_s((void *)g_sys_acore.dst_nc, g_sys_acore.size_nc, 0x0, g_sys_acore.size_nc);
            lr_nr_reg_pressure();
            memset_s((void *)g_sys_acore.src_nc, g_sys_acore.size_nc, 0xF, g_sys_acore.size_nc);
            lr_nr_reg_pressure();
            memcpy_s((void *)g_sys_acore.dst_nc, g_sys_acore.size_nc, (const void *)g_sys_acore.src_nc,
                g_sys_acore.size_nc);
            lr_nr_reg_pressure();
            memset_s((void *)g_sys_acore.dst, g_sys_acore.size, 0x0, g_sys_acore.size);
            lr_nr_reg_pressure();
            memset_s((void *)g_sys_acore.src, g_sys_acore.size, 0xF, g_sys_acore.size);
            lr_nr_reg_pressure();
            memcpy_s((void *)g_sys_acore.dst, g_sys_acore.size, (const void *)g_sys_acore.src, g_sys_acore.size);
            lr_nr_reg_pressure();
        }
        msleep(SYS_ACORE_STOP_DELAY);
    }
}

int sys_acore_entry(sys_bus_evt_e event)
{
    sys_bus_err("[sys_bus]acore, rcv evt: %d\n", event);

    if (event == E_SYS_BUS_EVT_START) {
        sys_acore_start();
    } else if (event == E_SYS_BUS_EVT_STOP) {
        sys_acore_stop();
    } else if (event == E_SYS_BUS_EVT_REPORT) {
        sys_acore_report();
    } else {
        sys_bus_err("[sys_bus]acore receive incorrect event: %d\n", event);
        return E_SYS_BUS_RET_ERR;
    }

    return E_SYS_BUS_RET_OK;
}

void sys_acore_dev_init(void)
{
    dma_addr_t dma_handle;
#ifdef CONFIG_ARM64
    struct device dev1;
#endif
#ifdef CONFIG_ARM64
    sys_bus_trace("sys_acore_init enter.\n ");
    if (memset_s(&dev1, sizeof(dev1), 0, sizeof(dev1))) {
        sys_bus_trace("memset fail.\n ");
        return;
    }
    of_dma_configure(&dev1, NULL, true);
    arch_setup_dma_ops(&dev1, 0, 0, NULL, 0);
    dma_set_mask_and_coherent(&dev1, DMA_BIT_MASK(64)); /* 64 : Addressing capability supported by the 64-bit device */
    g_sys_acore.src_nc = (u8 *)dma_alloc_coherent(&dev1, (size_t)ACORE_DDR_SIZE, &dma_handle, GFP_KERNEL);
#else
    g_sys_acore.src_nc = (u8 *)dma_alloc_coherent(NULL, ACORE_DDR_SIZE, &dma_handle, GFP_KERNEL);
#endif
    if (g_sys_acore.src_nc == NULL) {
        sys_bus_trace("g_sys_acore.src_nc failed\n ");
        return;
    }
#ifdef CONFIG_ARM64
    if (memset_s(&dev1, sizeof(dev1), 0, sizeof(dev1))) {
        sys_bus_trace("memset fail.\n ");
        return;
    }
    of_dma_configure(&dev1, NULL, true);
    arch_setup_dma_ops(&dev1, 0, 0, NULL, 0);
    dma_set_mask_and_coherent(&dev1, DMA_BIT_MASK(64)); /* 64 : Addressing capability supported by the 64-bit device */
    g_sys_acore.dst_nc = (u8 *)dma_alloc_coherent(&dev1, (size_t)ACORE_DDR_SIZE, &dma_handle, GFP_KERNEL);
#else
    g_sys_acore.dst_nc = (u8 *)dma_alloc_coherent(NULL, ACORE_DDR_SIZE, &dma_handle, GFP_KERNEL);
#endif
}

void sys_acore_sysbus_test_init(struct device_node* np)
{
    unsigned char *addr = NULL;

    int ret;

    ret = of_property_read_u32_index(np, "sysctrl_mdm_addr", 0, &g_sysctrl_mdm_addr);
    if (ret != 0) {
        sys_bus_trace("[sysbus]acore, read sysctrl_mdm_addr err\n");
        return;
    }

    sys_acore_dev_init();
    if (g_sys_acore.dst_nc == NULL) {
        sys_bus_trace("g_sys_acore.dst_nc failed\n ");
        return;
    }
    g_sys_acore.size_nc = ACORE_DDR_SIZE;

    addr = (unsigned char*)kmalloc(ACORE_DDR_SIZE, GFP_ATOMIC);
    g_sys_acore.src = addr;
    if (g_sys_acore.src == NULL) {
        sys_bus_trace("g_sys_acore.src failed\n ");
        return;
    }
    addr = (unsigned char*)kmalloc(ACORE_DDR_SIZE, GFP_ATOMIC);
    g_sys_acore.dst = addr;
    if (g_sys_acore.src == NULL) {
        sys_bus_trace("g_sys_acore.dst failed\n ");
        return;
    }
    g_sys_acore.size = ACORE_DDR_SIZE;
    sys_bus_trace("sys_acore_init malloc end.\n ");
    g_sys_acore.tsk = kthread_run((void *)sys_acore_thread, (void *)NULL, "sys_acore");
    if (IS_ERR(g_sys_acore.tsk)) {
        g_sys_acore.tsk = NULL;
        sys_bus_trace("cannot start sys_acore thread\n");
        return;
    }

    sys_bus_register_observer(SYS_ACORE_NAME, sys_acore_entry);
    sys_bus_trace("sys bus test acore ok\n");
}
