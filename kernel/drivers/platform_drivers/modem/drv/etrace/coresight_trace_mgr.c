/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
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
#include <product_config.h>
#include <securec.h>
#include <mdrv_memory_layout.h>
#include <bsp_print.h>
#include <bsp_rfile.h>
#include <bsp_nva.h>
#include <bsp_dt.h>
#include <nva_id.h>
#include <nva_stru.h>
#include <linux/of_address.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/dma-mapping.h>
#include "coresight_errno.h"
#include "coresight_tmc.h"
#include "coresight_replicator.h"
#include "coresight_funnel.h"
#include "coresight_trace_mgr.h"
#include <linux/version.h>
#include <bsp_file_ops.h>

#undef THIS_MODU
#define THIS_MODU mod_coresight

static struct platform_device g_modem_trace_device = {
    .name = "modem_trace",
    .id = 0,
    .dev = {
        .init_name = "modem_trace",
    },
};
struct coresight_trace_s g_coresight_base;

void coresight_channel_init(void)
{
    coresight_tmc_etf_opt(ETF_DISABLE);
    coresight_disable_etr();
    coresight_funnel_enable();
    coresight_tmc_etf_opt(ETF_ENABLE);
    coresight_replicator_sel();
    coresight_tmc_enable_etr(g_coresight_base.ddr_buffer_addr, g_coresight_base.ddr_buffer_len);
}
static u32 coresight_trace_get_size(void)
{
    u32 size = 0;
    s32 ret = bsp_nv_read(NVA_ID_DRV_TRACE, (u8 *)&size, sizeof(u32));
    if (ret < 0) {
        bsp_err("err nv:%d size:%u\n", ret, size);
        return 0;
    }
    return size;
}
static int coresight_trace_init_dev(void)
{
    struct device *pdev = NULL;
    if (platform_device_register(&g_modem_trace_device)) {
        return CORESIGHT_DEV_ERR;
    }
    pdev = &g_modem_trace_device.dev;
    of_dma_configure(pdev, NULL, true);
    dma_set_mask_and_coherent(pdev, DMA_BIT_MASK(0x20));
    return BSP_OK;
}
static void coresight_ddr_clean(void)
{
    switch (g_coresight_base.ddr_flag) {
        case CORESIGHT_DDR_BUF:
            iounmap(g_coresight_base.virt_ddr_buffer);
            break;
        case CORESIGHT_DMA_BUF:
            dma_free_coherent(&g_modem_trace_device.dev, g_coresight_base.ddr_size, g_coresight_base.virt_ddr_buffer,
                g_coresight_base.ddr_buffer_addr);
            break;
        default:
            break;
    }
    g_coresight_base.virt_ddr_buffer = NULL;
    g_coresight_base.ddr_buffer_len = 0;
    g_coresight_base.ddr_size = 0;
    g_coresight_base.ddr_flag = 0;
}
static s32 coresight_trace_set_ddrinfo(u32 ddr_size)
{
    u32 trace_size = 0;
    unsigned long trace_addr = mdrv_mem_region_get("trace_ddr", &trace_size);
    dma_addr_t phy_addr = 0;
    if (ddr_size == 0 || (ddr_size % 0x4 != 0)) {
        return BSP_ERROR;
    }
    if (ddr_size <= g_coresight_base.ddr_size) {
        bsp_err("enough ddr size\n");
        g_coresight_base.ddr_buffer_len = ddr_size;
        (void)memset_s(g_coresight_base.virt_ddr_buffer, g_coresight_base.ddr_buffer_len, 0,
            g_coresight_base.ddr_buffer_len);
        return BSP_OK;
    }
    coresight_ddr_clean();
    if (ddr_size <= trace_size) {
        g_coresight_base.ddr_buffer_addr = trace_addr;
        g_coresight_base.virt_ddr_buffer = (void *)ioremap_wc((phys_addr_t)(uintptr_t)trace_addr, trace_size);
        if (g_coresight_base.virt_ddr_buffer == NULL) {
            g_coresight_base.ddr_flag = 0;
            return CORESIGHT_VIRT_ERR;
        }
        g_coresight_base.ddr_size = trace_size;
        g_coresight_base.ddr_flag = CORESIGHT_DDR_BUF;
        bsp_err("ddr size ok\n");
    } else if (ddr_size <= CORESIGHT_TRACE_MAX_SIZE) {
        ddr_size = PAGE_ALIGN(ddr_size);
        g_coresight_base.virt_ddr_buffer =
            dma_alloc_coherent(&g_modem_trace_device.dev, ddr_size, &phy_addr, GFP_KERNEL);
        if (IS_ERR_OR_NULL(g_coresight_base.virt_ddr_buffer)) {
            g_coresight_base.ddr_flag = 0;
            return CORESIGHT_VIRT_ERR;
        }
        g_coresight_base.ddr_buffer_addr = phy_addr;
        g_coresight_base.ddr_size = ddr_size;
        g_coresight_base.ddr_flag = CORESIGHT_DMA_BUF;
        bsp_err("dma size ok\n");
    } else {
        return BSP_ERROR;
    }
    g_coresight_base.ddr_buffer_len = ddr_size;
    (void)memset_s(g_coresight_base.virt_ddr_buffer, g_coresight_base.ddr_buffer_len, 0,
        g_coresight_base.ddr_buffer_len);
    return BSP_OK;
}
int coresight_trace_info_init(u32 ddr_size)
{
    int ret;
    ret = coresight_tmc_etf_init();
    if (ret) {
        bsp_err("tmc_etf_init err:%d\n", ret);
        return ret;
    }
    ret = coresight_top_funnel_init();
    if (ret) {
        bsp_err("top_funnel_init err:%d\n", ret);
        return ret;
    }
    ret = coresight_replicator_init();
    if (ret) {
        bsp_err("replicator_init err:%d\n", ret);
        return ret;
    }
    ret = coresight_tmc_etr_init();
    if (ret) {
        bsp_err("tmc_etr_init err:%d\n", ret);
        return ret;
    }
    if (ddr_size > 0) {
        coresight_channel_init();
    }
    return BSP_OK;
}
int coresight_mprob_set(u32 ddr_size)
{
    int ret;
    if (g_coresight_base.init_flag != CORESIGHT_INIT_FLAG) {
        return coresight_trace_info_init(ddr_size);
    }
    ret = coresight_trace_set_ddrinfo(ddr_size);
    if (ret) {
        bsp_err("set ddr_size err:%d\n", ret);
        return ret;
    }
    coresight_channel_init();
    return BSP_OK;
}
int coresight_trace_init(void)
{
    u32 ddr_size;
    int ret;
    device_node_s *node = bsp_dt_find_node_by_path("/coresight-trace");
    if (node == NULL) {
        return BSP_OK;
    }
    bsp_err("trace init\n");
    coresight_mprobe_init();
    if (coresight_trace_init_dev()) {
        return CORESIGHT_DEV_ERR;
    }
    ddr_size = coresight_trace_get_size();
    if (ddr_size != 0) {
        ret = coresight_trace_set_ddrinfo(ddr_size);
        if (ret) {
            bsp_err("errno:%d\n", ret);
            return ret;
        }
    }
    if (coresight_trace_info_init(ddr_size)) {
        return BSP_ERROR;
    }
    bsp_err("trace init ok\n");
    g_coresight_base.init_flag = CORESIGHT_INIT_FLAG;
    return BSP_OK;
}
static void coresight_mkdir_recursive(void)
{
    s32 ret;
    char point;
    unsigned long old_fs;
    u32 i = 0;
    char path[CORESIGHT_FILE_PATH_LEN] = {0};
    u32 pathlen = sizeof(path);
    if (strncpy_s(path, CORESIGHT_FILE_PATH_LEN, MODEM_LOG_ROOT, sizeof(MODEM_LOG_ROOT))) {
        bsp_err("path cpy err\n");
        return;
    }
    do {
        point = '\0';
        while ((path[i] != '\0') && (i + 1 < pathlen)) {
            if (path[i] == '/') {
                i++;
                point = path[i];
                path[i] = '\0';
                break;
            }
            i++;
        }
        old_fs = get_fs();
        set_fs((mm_segment_t)KERNEL_DS);
        ret = bsp_file_mkdir(path, CORESIGHT_DIR_CHMOD);
        set_fs(old_fs);
        if (ret && ret != -EEXIST) {
            bsp_err("sys_mkdir fail,ret=%d\n",ret);
            return;
        }
        path[i] = point;
    } while (point != '\0');
    return;
}

s32 coresight_trace_save_to_file(void)
{
    struct file *fp = NULL;
    unsigned long old_fs;
    loff_t pos = 0;
    s32 ret;
    unsigned long rwp;
    unsigned long ddr_buffer_addr;
    unsigned long ddr_buffer_len;
    unsigned long left;
    if (g_coresight_base.virt_ddr_buffer == NULL) {
        bsp_err("no trace\n");
        return BSP_ERROR;
    }
    bsp_err("trace path %s\n", CORESIGHT_LOG_PATH);
    coresight_mkdir_recursive();
    do_unlinkat(AT_FDCWD, getname(CORESIGHT_LOG_PATH));
    fp = filp_open(CORESIGHT_LOG_PATH, RFILE_RDWR | RFILE_CREAT, CORESIGHT_DIR_CHMOD); /* 0660:open mode */
    if (IS_ERR_OR_NULL(fp)) {
        bsp_err("fail to open file err, ret = %ld\n", PTR_ERR(fp));
        return BSP_ERROR;
    }
    old_fs = get_fs();
    set_fs((mm_segment_t)KERNEL_DS);
    (void)vfs_llseek(fp, 0, SEEK_SET);
    set_fs(old_fs);
    rwp = coresight_get_etr_rwp();
    ddr_buffer_addr = g_coresight_base.ddr_buffer_addr;
    ddr_buffer_len = g_coresight_base.ddr_buffer_len;
    left = rwp - ddr_buffer_addr;
    if (rwp == ETR_ERROR_ADDR || rwp < ddr_buffer_addr || left > ddr_buffer_len) {
        bsp_err("trace rwp err:%lx\n", rwp);
        return BSP_ERROR;
    }
    ret = kernel_write(fp, g_coresight_base.virt_ddr_buffer + left, ddr_buffer_len - left, &pos);
    (void)vfs_fsync(fp, 0);
    if (ret != (ddr_buffer_len - left)) {
        bsp_err("fail to write file:%d\n", ret);
        (void)filp_close(fp, NULL);
        return ret;
    }
    ret = kernel_write(fp, g_coresight_base.virt_ddr_buffer, left, &pos);
    if (ret != left) {
        bsp_err("fail to write file2:%d\n", ret);
        (void)filp_close(fp, NULL);
        return ret;
    }
    (void)filp_close(fp, NULL);
    return BSP_OK;
}
struct coresight_trace_s coresight_get_trace_info(void)
{
    return g_coresight_base;
}
#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
/*lint --e{528}*/
late_initcall_sync(coresight_trace_init); /*lint !e528*/
#endif
