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

#include "odt_core.h"
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/vmalloc.h>
#include <linux/clk.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/of_device.h>
#include <linux/of_reserved_mem.h>
#include <linux/of_fdt.h>
#include <osl_thread.h>
#include <securec.h>
#include "bsp_version.h"
#include "bsp_dt.h"
#include "bsp_print.h"
#include "bsp_slice.h"
#include "product_config.h"
#ifdef BSP_CONFIG_PHONE_TYPE
#include <adrv_om.h>
#else
#include "bsp_bbpds.h"
#endif


struct odt_enc_dst_log_cfg g_enc_dst_buf_log_cfg;
odt_rsv_mem_s g_odt_rsv_mem_info;
odt_early_cfg_s g_odt_early_cfg;

struct platform_device *g_odt_pdev;
static struct platform_device odt_pdev = {
    .name = "modem_odt",
    .id = -1,
};

static const struct of_device_id odt_dev_of_match[] = {
    {
        .compatible = "oam,odt_app",
        .data = NULL,
    },
    {},
};
static int odt_driver_probe(struct platform_device *pdev);
static struct platform_driver g_odt_driver = {
    .driver = {
        .name = "modem_odt",
        .owner = THIS_MODULE,
        .of_match_table = odt_dev_of_match,
        .probe_type = PROBE_FORCE_SYNCHRONOUS,
        },
    .probe = odt_driver_probe,
};

static int odt_driver_probe(struct platform_device *pdev)
{
    of_dma_configure(&pdev->dev, NULL, true);
    dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64)); //lint !e598 !e648
    g_odt_pdev = pdev;
    return odt_reserved_mem_init();
}

u8 *bsp_odt_get_pdev_addr(void)
{
    return (u8 *)g_odt_pdev;
}

s32 odt_clk_enable(void)
{
#ifndef BSP_CONFIG_PHONE_TYPE
    struct clk *clock;
    int ret;

    clock = clk_get(NULL, "odt_clk");
    ret = clk_prepare(clock);
    if (ret) {
        odt_error("[init]clk prepare failed,ret=0x%x\n", ret);
        return ret;
    }
    ret = clk_enable(clock);
    if (ret) {
        odt_error("[init]clk open failed,ret=0x%x\n", ret);
        return ret;
    }
#endif
    return BSP_OK;
}

s32 odt_init(void)
{
    s32 ret;
    device_node_s *dev = NULL;

    odt_crit("[init]start\n");

    ret = odt_clk_enable();
    if (ret) {
        return ret;
    }

    odt_enc_dst_spinlock_init();

    odt_hal_init();

    dev = bsp_dt_find_compatible_node(NULL, NULL, "oam,odt_app");
    if (dev == NULL) {
        odt_error("get odt dtsi node err\n");
        return BSP_ERROR;
    }

    if (bsp_dt_device_is_available(dev) == false) {
        ret = platform_device_register(&odt_pdev);
        if (ret) {
            odt_error("register odt platform device err\n");
            return ret;
        }
    }

    ret = platform_driver_register(&g_odt_driver);
    if (ret) {
        odt_error("register odt driver fail, ret=0x%x\n", ret);
        return ret;
    }

    odt_mode_swt_init();

    ret = odt_int_manager_init();
    if (ret) {
        return ret;
    }

    bsp_odt_set_timeout(ODT_TIMEOUT_TFR_SHORT, ODT_TIMEOUT_TFR_SHORT_VAL);

    odt_crit("[init]ok\n");

    return BSP_OK;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(odt_init);
#endif

/*
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: 初始化成功的标识码
 */
s32 odt_reserved_mem_init(void)
{
    device_node_s *dev = NULL;
    s32 ret;

    /* get rsv mem info from kernel */
    odt_get_rsv_mem_info();

    /* 获取cmdline参数 */
    odt_get_cmdline_info();

    /* 对rsv mem进行map */
    ret = odt_rsv_mem_mmap();
    if (ret) {
        odt_error("rsv mem mmap fail\n");
        return ret;
    }

    dev = odt_hal_get_dev_info();
    if (dev == NULL) {
        return BSP_ERROR;
    }

    ret = odt_buffer_init(dev);
    if (ret) {
        odt_error("odt dst logbuffer init faield!\n");
        return ret;
    }

    odt_crit("reserved mem init sucess\n");

    return BSP_OK;
}

void odt_get_rsv_mem_info(void)
{
#ifdef BSP_CONFIG_PHONE_TYPE
    membuf_attr_s *mem_attr = NULL;

    if (g_odt_rsv_mem_info.init_flag != BSP_TRUE) {
        mem_attr = get_mdm_dfx_mem_info();
        if (mem_attr != NULL) {
            g_odt_rsv_mem_info.virt_addr = NULL;
            g_odt_rsv_mem_info.phy_addr = mem_attr->meminfo.base_addr;
            g_odt_rsv_mem_info.buff_size = mem_attr->meminfo.buffer_size;
            g_odt_rsv_mem_info.buff_useable = mem_attr->meminfo.rsv_mem_usable;

            g_odt_rsv_mem_info.init_flag = BSP_TRUE;
        }
    } else {
        g_odt_rsv_mem_info.virt_addr = NULL;
    }
#else
    g_odt_rsv_mem_info.virt_addr = NULL;
#endif

    return;
}


static s32 odt_get_rsv_mem_addr(void)
{
    /* 物理地址是32位的实地址并且是8字节对齐的 */
    if ((g_odt_rsv_mem_info.phy_addr % 0x8) || (g_odt_rsv_mem_info.phy_addr == 0)) {
        odt_crit("rsv mem addr invalid, 0x%lx\n", g_odt_rsv_mem_info.phy_addr);
        return -1;
    }

    g_odt_early_cfg.phy_addr = g_odt_rsv_mem_info.phy_addr;

    return 0;
}

static s32 odt_get_rsv_mem_size(void)
{
    u32 buff_size;

    buff_size = g_odt_rsv_mem_info.buff_size;

    if (buff_size % 0x8) {
        odt_error("BuffSize no 8 byte allignment,BuffSize: 0x%x\n", buff_size);
        return BSP_ERROR;
    }
    g_odt_early_cfg.buff_size = buff_size;
    odt_crit("buff_size 0x%x\n", g_odt_early_cfg.buff_size);

    return BSP_OK;
}

void odt_get_cmdline_info(void)
{
    s32 ret;

    ret = odt_get_rsv_mem_addr();
    if (ret) {
        g_odt_early_cfg.buff_useable = BSP_FALSE;
        return;
    }

    ret = odt_get_rsv_mem_size();
    if (ret) {
        g_odt_early_cfg.buff_useable = BSP_FALSE;
        return;
    }

    g_odt_early_cfg.buff_useable = BSP_TRUE;
    return;
}

static void *odt_mem_remap(unsigned long phys_addr, size_t size)
{
    unsigned long i;
    u8 *vaddr = NULL;
    unsigned long npages = ((unsigned long)PAGE_ALIGN((phys_addr & (PAGE_SIZE - 1)) + size) >> PAGE_SHIFT);
    unsigned long offset = phys_addr & (PAGE_SIZE - 1);
    struct page **pages = NULL;

    pages = vmalloc(sizeof(struct page *) * npages);
    if (pages == NULL) {
        odt_error("vmalloc fail, len=0x%lx\n", sizeof(struct page *) * npages);
        return NULL;
    }

    pages[0] = phys_to_page((uintptr_t)phys_addr);

    for (i = 0; i < npages - 1; i++) {
        pages[i + 1] = pages[i] + 1;
    }

    vaddr = (u8 *)vmap(pages, (unsigned int)npages, (unsigned long)VM_MAP, pgprot_writecombine(PAGE_KERNEL));
    if (vaddr != NULL) {
        vaddr += offset;
    } else {
        odt_error("vmap fail, npages=0x%lx, VM_MAP=0x%lx\n", npages, (unsigned long)VM_MAP);
    }

    vfree(pages);
    return (void *)vaddr;
}

s32 odt_rsv_mem_mmap(void)
{
    /* step1: if logcfg is on, mmap kernel reserved memory */
    if (g_odt_early_cfg.buff_useable == BSP_TRUE) {
        g_odt_rsv_mem_info.virt_addr =
            odt_mem_remap(g_odt_rsv_mem_info.phy_addr, (size_t)g_odt_rsv_mem_info.buff_size);
        if (g_odt_rsv_mem_info.virt_addr == NULL) {
            odt_error("remap odt buffer fail\n");
            g_odt_rsv_mem_info.buff_useable = BSP_FALSE;
            return BSP_ERROR;
        }

        odt_crit("kernel reserved buffer mmap success\n");
    } else {
        odt_crit("do not use kernel rsv mem\n");
    }

    return BSP_OK;
}

s32 odt_buffer_init(device_node_s *dev)
{
    s32 ret;

    /* step1: if mdmlog rsv mem is useable, use kernel reserved buffer */
    if (g_odt_early_cfg.buff_useable == BSP_TRUE) {
        g_enc_dst_buf_log_cfg.virt_addr = g_odt_rsv_mem_info.virt_addr;
        g_enc_dst_buf_log_cfg.phy_addr = g_odt_rsv_mem_info.phy_addr;
        g_enc_dst_buf_log_cfg.buff_size = g_odt_rsv_mem_info.buff_size;
        g_enc_dst_buf_log_cfg.over_time = g_odt_rsv_mem_info.timeout;
        g_enc_dst_buf_log_cfg.log_on_flag = ODT_DST_CHAN_DELAY;
    } else { /* step2: if mdmlog rsv mem is not useable, update logOnFlag state */
        g_enc_dst_buf_log_cfg.log_on_flag = ODT_DST_CHAN_NOT_CFG;
    }

    /* step3: if mdmlog rsv mem is not useable, alloc dma mem for dst channel */
    if (g_enc_dst_buf_log_cfg.log_on_flag == ODT_DST_CHAN_NOT_CFG) {
        ret = odt_uncacheable_mem_alloc(dev);
        if (ret) {
            odt_error("of_property_read_u32_array failed!\n");
            return BSP_ERROR;
        }
    }

    return BSP_OK;
}

/*
 * 函 数 名: odt_uncacheable_mem_alloc
 * 功能描述: 动态申请一致性内存
 * 输入参数: 设备节点
 * 输出参数: 无
 * 返 回 值: 设置成功与否标志
 */
s32 odt_uncacheable_mem_alloc(device_node_s *dev)
{
    dma_addr_t address = 0;
    u8 *buffer = NULL;
    int ret;
    u32 dst_chan[ODT_DST_CHAN_CFG_BUTT] = {0};
    u32 size;

    ret = bsp_dt_property_read_u32_array(dev, "dst_chan_cfg", dst_chan, (size_t)ODT_DST_CHAN_CFG_BUTT);
    if (ret == 0) {
        odt_crit("of_property_read_u32_array get size 0x%x!\n", dst_chan[ODT_DST_CHAN_CFG_SIZE]);
        size = dst_chan[ODT_DST_CHAN_CFG_SIZE];

        buffer = (u8 *)dma_alloc_coherent(&g_odt_pdev->dev, (size_t)size, &address, GFP_KERNEL);
        if (buffer != NULL) {
            g_enc_dst_buf_log_cfg.phy_addr = (unsigned long)address;
            g_enc_dst_buf_log_cfg.virt_addr = buffer;
            g_enc_dst_buf_log_cfg.buff_size = size;
            g_enc_dst_buf_log_cfg.log_on_flag = ODT_DST_CHAN_DTS;

            odt_crit("logbuffer_dmalloc success\n");
            return BSP_OK;
        }
    }

    buffer = (u8 *)dma_alloc_coherent(&g_odt_pdev->dev, (size_t)ODT_BUFFER_MIN, &address, GFP_KERNEL);
    if (buffer == NULL) {
        odt_error("alloc min buffer size fail\n");
        return BSP_ERROR;
    }

    g_enc_dst_buf_log_cfg.phy_addr = (unsigned long)address;
    g_enc_dst_buf_log_cfg.virt_addr = buffer;
    g_enc_dst_buf_log_cfg.buff_size = ODT_BUFFER_MIN;
    g_enc_dst_buf_log_cfg.log_on_flag = ODT_DST_CHAN_MIN;

    return BSP_OK;
}

static int __init odt_reserve_area(struct reserved_mem *rmem)
{
    char *status;

    g_odt_rsv_mem_info.init_flag = BSP_TRUE;

    status = (char *)of_get_flat_dt_prop(rmem->fdt_node, "status", NULL);
    odt_error("status is %s\n", status);
    if (status && (strncmp(status, "ok", strlen("ok")) != 0)) {
        return 0;
    }

    g_odt_rsv_mem_info.buff_size = (unsigned int)rmem->size;
    g_odt_rsv_mem_info.phy_addr = rmem->base;

    /* if reserved buffer is too small, set kernel reserved buffer is not usable */
    if ((g_odt_rsv_mem_info.phy_addr == 0) || (g_odt_rsv_mem_info.phy_addr % 8)) {
        odt_error("kernel reserved addr is null or not 8bits align!\n");
        g_odt_rsv_mem_info.buff_useable = BSP_FALSE;
        return -1;
    }

    if (g_odt_rsv_mem_info.buff_size % 8) {
        odt_error("kernel reserved buffer size is not 8bits align\n");
        g_odt_rsv_mem_info.buff_useable = BSP_FALSE;
        return -1;
    }

    g_odt_rsv_mem_info.buff_useable = BSP_TRUE;
    odt_crit("kernel reserved buffer is useful\n");

    return 0;
}

RESERVEDMEM_OF_DECLARE(modem_odt, "platform_mdmlog", (reservedmem_of_init_fn)odt_reserve_area);


void bsp_odt_get_log_cfg(odt_encdst_buf_log_cfg_s *cfg)
{
    struct odt_enc_dst_log_cfg *log_cfg = &g_enc_dst_buf_log_cfg;

    if (cfg == NULL) {
        return;
    }

    cfg->log_on_flag = log_cfg->log_on_flag;
    cfg->buffer_size = log_cfg->buff_size;
    cfg->cur_time_out = odt_get_ind_dst_cur_timouet();
    cfg->vir_buffer = (unsigned long long)(uintptr_t)log_cfg->virt_addr;
    cfg->phy_buffer_addr = (unsigned long long)log_cfg->phy_addr;
}

struct platform_device *bsp_get_odt_pdev(void)
{
    return g_odt_pdev;
}
EXPORT_SYMBOL(bsp_get_odt_pdev);

void bsp_odt_set_timeout(odt_timeout_en_e time_out_en, u32 time_out)
{
    if (odt_hal_get_timeout_mode() == 0) {
        odt_hal_set_timeout_mode(1);
    }

    odt_hal_set_timeout_threshold(time_out_en, time_out);

    return;
}

u32 odt_get_mem_log_on_flag(void)
{
    return g_enc_dst_buf_log_cfg.log_on_flag;
}

