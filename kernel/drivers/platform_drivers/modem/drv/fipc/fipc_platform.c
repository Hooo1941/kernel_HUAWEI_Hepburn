/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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
#include <linux/syscore_ops.h>
#include "fipc_platform.h"
#include "fipc_dts.h"

#include "fipc_device.h"
#include "fipc_driver.h"
#include "fipc_core.h"
#include "fipc_pmsr.h"
#include "fipc_dump.h"
#include "fipc_reset.h"

#define FIPC_MEM_ALIGN_DEFAULT (4)

static struct platform_device g_fipc_plat_device = {
    .name = "fipc_plat",
};

static void fipc_int_wakeup_ipipe_handler(struct fipc_device *pdev, struct irq_bundle *pbundle, u32 int_pipe_status,
    u32 pipe_id_base, u32 irq_no)
{
    int reg_bit;
    u32 pipe_id;
    struct fipc_channel *pchannel;
    u32 lipipe_id;
    struct fipc_device *pldev = NULL;
    for (reg_bit = 0; reg_bit < FIPC_PIPE_CNT_PER_REG; reg_bit++) {
        if (!((int_pipe_status >> reg_bit) & 1)) {
            continue;
        }
        pipe_id = reg_bit + pipe_id_base;
        pchannel = pdev->channels[pipe_id];
        if (pchannel == NULL) {
            if (fipc_relv_dev_chn_get(pdev, pipe_id, &pldev, &lipipe_id) != 0) {
                fipc_print_error("[Wakeup Info] irq %d from pipe_id 0x%08x, no channel found!", irq_no, pipe_id);
                continue;
            }
            pchannel = pldev->channels[lipipe_id];
            if (pchannel == NULL) {
                fipc_print_error("[Wakeup Info] irq %d from pipe_id 0x%08x, no channel found for local ipipeid 0x%08x!",
                    irq_no, pipe_id, lipipe_id);
                continue;
            }
        }
        if (pchannel->irq_wakeup_cbk != NULL) {
            pchannel->irq_wakeup_cbk(pchannel->irq_cbk_arg);
        }
        fipc_print_always("[Wakeup Info] irq %d from channel user_id 0x%08x \n", irq_no, pchannel->user_id);
    }
}

static void fipc_int_wakeup_channel_handler(struct fipc_device *pdev, struct irq_bundle *pbundle, u32 irq_no)
{
    u32 int_pipe_status;
    u32 reg_offset;

    reg_offset = pdev->config_diff->core_pipe_irq_off;
    int_pipe_status = readl((void *)((char *)(pdev->base_va) + FIPC_CORE_IPIPE_INT_STAT0(pbundle->cpu_id, reg_offset)));
    fipc_int_wakeup_ipipe_handler(pdev, pbundle, int_pipe_status, 0, irq_no);

    if(pdev->pipepair_cnt > FIPC_PIPE_CNT_PER_REG) {
        int_pipe_status = readl((void *)((char *)(pdev->base_va) + FIPC_CORE_IPIPE_INT_STAT1(pbundle->cpu_id, reg_offset)));
        fipc_int_wakeup_ipipe_handler(pdev, pbundle, int_pipe_status, FIPC_PIPE_CNT_PER_REG, irq_no);
    }
}

static void fipc_int_wakeup_src_irq_bundle_handler(struct fipc_device *pdev, struct irq_bundle *pbundle)
{
    bool irq_state = 0;
    int ret = -1;
    int irq_idx = 0;
    if (pbundle == NULL) {
        return;
    }

    // 存在pdev->ctrl_level无效的可能，不要把irq_get_irqchip_state提取到if-else外执行
    if (pdev->ctrl_level == FIPC_DEV_CONTROL_LEVEL_HOST || pdev->ctrl_level == FIPC_DEV_CONTROL_LEVEL_GUEST ||
        pdev->ctrl_level == FIPC_DEV_CONTROL_LEVEL_PART) {
        ret = irq_get_irqchip_state(pbundle->irq[0x1], IRQCHIP_STATE_PENDING, &irq_state);
        irq_idx = 0x1;
    } else if (pdev->ctrl_level == FIPC_DEV_CONTROL_LEVEL_IRQCLR) {
        ret = irq_get_irqchip_state(pbundle->irq[0x0], IRQCHIP_STATE_PENDING, &irq_state);
        irq_idx = 0x0;
    }

    if (ret == 0 && irq_state) {
        fipc_int_wakeup_channel_handler(pdev, pbundle, pbundle->irq[irq_idx]);
    }
}

static void fipc_int_wakeup_src_device_handler(struct fipc_device *pdev) {
    u32 mapid;
    if( pdev == NULL) {
        return ;
    }
    for(mapid = 0; mapid < FIPC_CPU_ID_MAX; ++mapid) {
        struct irq_bundle *pbundle = pdev->maps[mapid];
        fipc_int_wakeup_src_irq_bundle_handler(pdev, pbundle);
    }
}
static void fipc_int_wakeup_src_handler(void)
{
    u32 devid;
    struct fipc_device *pdev;
    for(devid = 0; devid < FIPC_DEVICE_NUM_MAX; ++devid) {
        pdev = fipc_device_get_fast(devid);
        fipc_int_wakeup_src_device_handler(pdev);
    }
    return ;
}

// 依赖GIC，已经确认在执行此回调函数时GIC已经ready
static struct syscore_ops fipc_dpm_ops = {
    .resume = fipc_int_wakeup_src_handler,
};

void *fipc_dma_alloc(u32 size, fipcsoc_ptr_t *pa, u32 align)
{
    void *mem_addr = NULL;
    u32 align_sz;

    if (align < FIPC_MEM_ALIGN_DEFAULT) {
        align = FIPC_MEM_ALIGN_DEFAULT;
    }
    align_sz = fipc_roundup(size, align);
    mem_addr = dma_alloc_coherent(&g_fipc_plat_device.dev, align_sz, pa, GFP_KERNEL);
    if (mem_addr == NULL) {
        fipc_print_error("fipc_dma_alloc failed\n");
        return NULL;
    }
    return mem_addr;
}

void *fipc_pushmem_alloc(u32 size, fipcsoc_ptr_t *pa, u32 align)
{
    void *mem_addr = NULL;

    mem_addr = fipc_dma_alloc(size, pa, align);
    if (mem_addr == NULL) {
        fipc_print_error("pushmem dma_alloc_coherent failed\n");
        return NULL;
    }
    return mem_addr;
}

void *fipc_ioremap(u32 pa, u32 size)
{
    return ioremap(pa, size);
}

/* ******************************************************************************
 * 低功耗相关处理
 * ***************************************************************************** */
int bsp_fipc_suspend(void)
{
    int ret;
    fipc_pm_suspend_start();
    ret = fipc_chn_suspend();
    if (ret) {
        fipc_pm_suspend_failed();
        return ret;
    }
    ret = fipc_dev_suspend();
    if (ret) {
        fipc_chn_resume();
        fipc_pm_suspend_failed();
        return ret;
    }
    fipc_pm_suspend_end();
    return ret;
}
int bsp_fipc_resume(void)
{
    fipc_pm_resume_start();
    fipc_dev_resume();
    fipc_chn_resume();
    fipc_pm_resume_end();
    return 0;
}

static int fipc_suspend_wrapper(struct device *dev)
{
    return bsp_fipc_suspend();
}
static int fipc_resume_wrapper(struct device *dev)
{
    return bsp_fipc_resume();
}

int bsp_fipc_chn_before_resetting(void)
{
    return fipc_chn_before_resetting();
}

int bsp_fipc_chn_after_resetting(void)
{
    return fipc_chn_after_resetting();
}

/* ******************************************************************************
 * 异常dump处理
 * ***************************************************************************** */
struct fipc_dump_info {
    int dumped;
    void *dump_addr;
    u32 dump_len;
};
struct fipc_dump_info g_fipc_dump;

int bsp_fipc_dump(void)
{
    if (g_fipc_dump.dump_addr == NULL || g_fipc_dump.dump_len == 0) {
        fipc_print_error("fipc no dump space\n");
        return -1;
    }
    fipc_do_dump((u8 *)g_fipc_dump.dump_addr, g_fipc_dump.dump_len);
    fipc_print_error("fipc dump completed\n");
    return 0;
}

void fipc_dump_wrapper(void)
{
    bsp_fipc_dump();
}

static void fipc_dump_init(void)
{
    void *dump_base = NULL;
    struct fipc_rsv_meminfo meminfo;

    if (fipc_reserved_meminfo(&meminfo)) {
        return;
    }
    if (meminfo.dump_size == 0) {
        return;
    }
    dump_base = bsp_dump_register_field(DUMP_MODEMAP_FIPC, "FIPC", meminfo.dump_size, 0x1);
    if (dump_base == NULL) {
        return;
    }
    if (memset_s(dump_base, meminfo.dump_size, 0, meminfo.dump_size)) {
        fipc_print_error("dump area clear fail\n");
        return;
    }
    g_fipc_dump.dumped = 0;
    g_fipc_dump.dump_addr = dump_base;
    g_fipc_dump.dump_len = meminfo.dump_size;
    if (-1 == bsp_dump_register_hook("FIPC", fipc_dump_wrapper)) {
        fipc_print_error("dump hook fail\n");
        return;
    }
    fipc_ringbuf_dump_init();
    return;
}
static int fipc_plat_probe(struct platform_device *pdev)
{
    int ret;
    of_dma_configure(&pdev->dev, NULL, true);
    dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64)); /* 64 bit */

    fipc_dump_init();

    ret = fipc_devices_init();
    if (ret) {
        fipc_print_error("fipc_devices_init failed\n");
        return -1;
    }
    ret = fipc_irqs_init();
    if (ret) {
        fipc_print_error("fipc_irqs_init failed\n");
        return -1;
    }

    ret = fipc_reset_hifi_init();
    if (ret) {
        fipc_print_error("fipc_reset_hifi_init failed\n");
        return -1;
    }

    return 0;
}

static const struct dev_pm_ops g_fipc_pmops = {
    .suspend_noirq = fipc_suspend_wrapper,
    .resume_noirq = fipc_resume_wrapper,
    .prepare = NULL,
    .complete = NULL,
};
static struct platform_driver g_fipc_plat_driver = {
    .probe = fipc_plat_probe,
    .driver =
        {
            .name = "fipc_plat",
            .owner = THIS_MODULE,
            .pm = &g_fipc_pmops,
        },
};

/* FIPC虽然有多个设备，但鉴于其关联特性，以及低功耗时间优化， 因此创建一个虚拟设备来统一处理所有fipc事务 */
int bsp_fipc_init(void)
{
    int ret;

    static int inited = 0;
    if (inited) {
        return 0;
    }
    if (!fipc_init_meet()) {
        fipc_print_always("fipc init skiped\n");
        return 0;
    }
    ret = platform_device_register(&g_fipc_plat_device);
    if (ret) {
        return ret;
    }
    ret = platform_driver_register(&g_fipc_plat_driver);
    if (ret) {
        platform_device_unregister(&g_fipc_plat_device);
        return ret;
    }
    register_syscore_ops(&fipc_dpm_ops);
    inited = 1;
    fipc_print_always("fipc init ok\n");
    return ret;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
arch_initcall(bsp_fipc_init);
#endif


MODULE_DESCRIPTION("FIPC Driver");
MODULE_LICENSE("GPL");
EXPORT_SYMBOL(fipc_dma_alloc);
