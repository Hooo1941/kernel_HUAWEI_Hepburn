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

#include "dra.h"
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/clk.h>
#include <linux/workqueue.h>
#include <linux/version.h>
#include <bsp_dt.h>
#include <bsp_dra.h>
#include <securec.h>

#define DRA_U64_BITLEN 64
#define DRA_OPIPE_NAME_MAX 64
#define DRA_MEM_LINE_DEAULT 100
#define DRA_OPIPE_CHECK_MAX 10
#define DRA_IPIPE_CHECK_MAX 64
#define DRA_QUEUE_LEN_CHECK_MAX 0x20000

struct dra_para_s g_dra_para = { { { 0 } } };

struct dra_dt_field_u32_s {
    char *name;
    unsigned int *value;
    unsigned int *size;
};

static struct dra_dt_field_u32_s g_dra_dt_u32[] = {
    {
        "dra-ipipe-max-count",
        &g_dra_para.iparamter.cnt,
    },
    {
        "dra-release-poll-depth",
        &g_dra_para.release_depth,
    },
    {
        "dra-mem-protect-threshold",
        &g_dra_para.protect_thresh,
    },
    {
        "dra_rls_opipes",
        &g_dra_para.rls_chn_idx,
    },
    {
        "dra_rls_opipes_depth",
        &g_dra_para.rls_chn_depth,
    },
    {
        "dra-rls-dbgcnt-idx",
        &g_dra_para.rls_chn_dbg_idx,
    },
    {
        "dra-sc-mode",
        &g_dra_para.sc_mode,
    },
};

static struct dra_dt_field_u32_s g_dra_dt_array[] = {
    {
        "dra-opipe-level-used",
        g_dra_para.opipes_used,
        &g_dra_para.opipe_cnt,
    },
    {
        "dra-opipe-tab-used",
        g_dra_para.oparamter.use_tab,
    },
    {
        "dra-opipe-levels-depth",
        g_dra_para.oparamter.depth,
    },
    {
        "dra-opipe-from",
        g_dra_para.oparamter.from,
    },
    {
        "dra-opipe-tab-size",
        g_dra_para.oparamter.tab_size,
    },
    {
        "dra-opipe-disable_irq",
        g_dra_para.oparamter.disable_irq,
    },
    {
        "dra-ipipe-levels-idx",
        g_dra_para.iparamter.ipipeidx,
    },
    {
        "dra-ipipe-levels-depth",
        g_dra_para.iparamter.depth,
    },
    {
        "dra-ddr-buf-depth",
        g_dra_para.ddr_buf_depth,
    },
    {
        "dra-opipe-dbgcnt-idx",
        g_dra_para.oparamter.dbg_idx,
    },
    {
        "dra-ddrbuf-dbgcnt-idx",
        g_dra_para.ddr_buf_dbg_idx,
    },
    {
        "dra-ipipe-dbgcnt-idx",
        g_dra_para.iparamter.dbg_idx,
    },
};

static struct dra_allc_para g_dra_alloc_para_set[] = {
    {
        .name = "dra-ipipe-for-pfa-2k",
        .ipipe_id = DRA_IPIPE_FOR_PFA_2K,
    },
    {
        .name = "dra-ipipe-for-pfa-tft-2k",
        .ipipe_id = DRA_IPIPE_FOR_PFA_TFT_2K,
    },
    {
        .name = "dra-ipipe-for-himac0-2k",
        .ipipe_id = DRA_IPIPE_FOR_HIMAC0_2K,
    },
    {
        .name = "dra-ipipe-for-himac1-2k",
        .ipipe_id = DRA_IPIPE_FOR_HIMAC1_2K,
     },
};

static struct dra_rls_para g_dra_rls_para_set[] = {
    {
        .name = "dra-rls-for-himac0-2k",
        .rls_id = DRA_RLS_FOR_HIMAC0_2K,
    },
    {
        .name = "dra-rls-for-himac1-2k",
        .rls_id = DRA_RLS_FOR_HIMAC1_2K,
    },
};

static struct dra_dt_field_u32_s g_dra_cust_u32[] = {
    {
        "dra-release-poll-depth",
        &g_dra_para.release_depth,
    },
};

static struct dra_dt_field_u32_s g_dra_cust_array[] = {
    {
        "dra-opipe-tab-size",
        g_dra_para.oparamter.tab_size,
    },
    {
        "dra-ddr-buf-depth",
        g_dra_para.ddr_buf_depth,
    },
    {
        "dra-opipe-levels-depth",
        g_dra_para.oparamter.depth,
    },
};

static unsigned int dra_dt_get_regphy(device_node_s *np)
{
    const __be32 *reg_addr_p = NULL;
    int na;

    reg_addr_p = bsp_dt_get_property(np, "reg", NULL);
    if (reg_addr_p == NULL) {
        dra_err("get_address failed.\r\n");
        return 0;
    }
    na = bsp_dt_n_addr_cells(np);
    return bsp_dt_read_number(reg_addr_p, na);
}

static int dra_check_para_cnt(void)
{
    if (g_dra_para.max_opipes > DRA_OPIPE_CHECK_MAX) {
        return -1;
    }

    if (g_dra_para.iparamter.cnt > DRA_IPIPE_CHECK_MAX) {
        return -1;
    }
    return 0;
}

static int dra_check_para_len(void)
{
    unsigned int i;

    for (i = 0; i < DRA_OPIPE_ALLOC_LEVELS; i++) {
        if (g_dra_para.oparamter.size[i] > DRA_QUEUE_LEN_CHECK_MAX) {
            return -1;
        }
        if (g_dra_para.iparamter.size[i] > DRA_QUEUE_LEN_CHECK_MAX) {
            return -1;
        }
        if (g_dra_para.ddr_buf_depth[i] > DRA_QUEUE_LEN_CHECK_MAX) {
            return -1;
        }
    }

    return 0;
}

static void dra_parse_dt(struct dra *dra, device_node_s *np)
{
    unsigned int i;
    int ret;
    int size;

    for (i = 0; i < sizeof(g_dra_dt_u32) / sizeof(g_dra_dt_u32[0]); i++) {
        ret = bsp_dt_property_read_u32(np, g_dra_dt_u32[i].name, g_dra_dt_u32[i].value);
        if (ret) {
            dra_err("%s not found\n", g_dra_dt_u32[i].name);
        }
    }

    for (i = 0; i < sizeof(g_dra_dt_array) / sizeof(g_dra_dt_array[0]); i++) {
        bsp_dt_get_property(np, g_dra_dt_array[i].name, &size);
        if (size <= 0) {
            continue;
        }
        ret = bsp_dt_property_read_u32_array(np, g_dra_dt_array[i].name, g_dra_dt_array[i].value,
            size / sizeof(unsigned int));
        if (ret) {
            dra_err("%s not found\n", g_dra_dt_array[i].name);
        }

        if (g_dra_dt_array[i].size != NULL) {
            *g_dra_dt_array[i].size = size / sizeof(unsigned int);
        }
    }

    for (i = 0; i < sizeof(g_dra_alloc_para_set) / sizeof(g_dra_alloc_para_set[0]); i++) {
        ret = bsp_dt_property_read_u32_array(np, g_dra_alloc_para_set[i].name, &g_dra_alloc_para_set[i].idx, 2); // get 2 words
        if (ret) {
            dra_err("%s not found\n", g_dra_alloc_para_set[i].name);
            g_dra_alloc_para_set[i].idx = DRA_ALLOC_PARA_MAGIC;
            g_dra_alloc_para_set[i].level = DRA_ALLOC_PARA_MAGIC;
        }
        dra_err("dra alloc idx:%u, dra.level:%u", g_dra_alloc_para_set[i].idx, g_dra_alloc_para_set[i].level);
    }

    for (i = 0; i < sizeof(g_dra_rls_para_set) / sizeof(g_dra_rls_para_set[0]); i++) {
        ret = bsp_dt_property_read_u32_array(np, g_dra_rls_para_set[i].name, &g_dra_rls_para_set[i].idx, 3); // get 2 words
        if (ret) {
            dra_err("%s not found\n", g_dra_rls_para_set[i].name);
            g_dra_rls_para_set[i].idx = DRA_ALLOC_PARA_MAGIC;
            g_dra_rls_para_set[i].level = DRA_ALLOC_PARA_MAGIC;
        }
        dra_err("dra rls idx:%u, dra.level:%u", g_dra_rls_para_set[i].idx, g_dra_rls_para_set[i].level);
    }
}

static int dra_dt_cust_init(struct dra *dra)
{
    device_node_s *np = NULL;
    int i, ret, size;

    np = bsp_dt_find_compatible_node(NULL, NULL, "dra_cust");
    if (np == NULL) {
        dra_err("dra dont support cust\n");
        return 0;
    }

    for (i = 0; i < sizeof(g_dra_cust_u32) / sizeof(g_dra_cust_u32[0]); i++) {
        ret = bsp_dt_property_read_u32(np, g_dra_cust_u32[i].name, g_dra_cust_u32[i].value);
        if (ret) {
            dra_err("%s not found\n", g_dra_cust_u32[i].name);
            return -1;
        }
    }

    for (i = 0; i < sizeof(g_dra_cust_array) / sizeof(g_dra_cust_array[0]); i++) {
        bsp_dt_get_property(np, g_dra_cust_array[i].name, &size);
        if (size <= 0) {
            dra_err("%s size err\n", g_dra_cust_array[i].name);
            return -1;
        }
        ret = bsp_dt_property_read_u32_array(np, g_dra_cust_array[i].name, g_dra_cust_array[i].value,
            size / sizeof(unsigned int));
        if (ret) {
            dra_err("%s not found\n", g_dra_cust_array[i].name);
            return -1;
        }

        if (g_dra_cust_array[i].size != NULL) {
            *g_dra_cust_array[i].size = size / sizeof(unsigned int);
        }
    }

    dra->cust_np = np;
    return 0;
}

static int dra_dt_init(struct dra *dra)
{
    device_node_s *np = NULL;

    np = bsp_dt_find_compatible_node(NULL, NULL, "dra31303061");
    if (np == NULL) {
        return -1;
    }

    dra->regs = bsp_dt_iomap(np, 0);
    if (dra->regs == NULL) {
        dra_err("iomap failed.\r\n");
        return -1;
    }

    dra->irq = bsp_dt_irq_parse_and_map(np, 0);
    if (dra->irq < 0) {
        dra_err("bsp_dt_irq_parse_and_map failed.\r\n");
        return -1;
    }

    dra->phy_regs = dra_dt_get_regphy(np);
    if (dra->phy_regs == 0) {
        return -1;
    }

    dra_parse_dt(dra, np);
    if (dra_dt_cust_init(dra)) {
        return -1;
    }

    if (dra_check_para_cnt()) {
        return -1;
    }

    if (dra_check_para_len()) {
        return -1;
    }

    dra->np = np;
    return 0;
}

static int dra_clk_init(struct dra *dra)
{
    unsigned int clk_num = 0;
    struct property *prop = NULL;
    struct clk *clk = NULL;
    const char *name = NULL;

    bsp_dt_property_for_each_string(dra->np, "clock-names", prop, name) {
        clk = devm_clk_get(dra->dev, name);
        if (IS_ERR_OR_NULL(clk)) {
            dra_err("dra %u clk not available\n", clk_num);
        } else {
            if (clk_prepare_enable(clk)) {
                dra_err("failed to enable dra %u clk\n", clk_num);
                return -EINVAL;
            }
        }
        clk_num++;
    }

    if (clk_num == 0 || clk_num > DRA_CLKS_MAX_NUM) {
        return -EINVAL;
    }

    return 0;
}

static void dra_init_reclaim_timer(struct dra *dra)
{
    dra->timeout_jiffies = msecs_to_jiffies(1000); // timeout expires 1000 ms
    timer_setup(&dra->timer, dra_bigpool_timer, 0);
    dra->timer.expires = jiffies + msecs_to_jiffies(1000); // timeout expires 1000 ms
    add_timer(&dra->timer);
}

static void dra_para_update(struct dra *dra)
{
    unsigned int i;
    struct dra_para_s *para = &g_dra_para;

    dra->para = para;
    dra->allc_para = g_dra_alloc_para_set;
    dra->allc_para_size = sizeof(g_dra_alloc_para_set) / sizeof(g_dra_alloc_para_set[0]);
    dra->rls_para = g_dra_rls_para_set;
    dra->rls_para_size = sizeof(g_dra_rls_para_set) / sizeof(g_dra_rls_para_set[0]);
    dra->max_opipes = para->opipes_used[para->opipe_cnt - 1];
    dra->opipe_use_aximem = para->opipe_use_aximem;
    dra->protect_thresh = para->protect_thresh;
    dra->release.fifo.depth = para->release_depth;
    dra->dbg = (DRA_DEBUG_ALLOC_FREE_TIMESTAMP | DRA_DEBUG_TIMER_EN);
    for (i = 0; i < para->opipe_cnt; i++) {
        dra->opipes[para->opipes_used[i]].used = DRA_OPIPE_USED_MAGIC;
        dra->opipes[para->opipes_used[i]].use_tab = para->oparamter.use_tab[i];
    }
}

irqreturn_t dra_wake_irq_handler(int irq, void *dev_id)
{
    UNUSED(irq);
    UNUSED(dev_id);
    return IRQ_HANDLED;
}

static int dra_wakeirq_init(struct dra *dra)
{
    device_node_s *node = NULL;

    node = bsp_dt_find_node_by_path("/dra_wake_irq");
    if (node == NULL) {
        dra_err("dra no dra_wake_irq\n");
        return 0;
    }

    dra->wake_irq = bsp_dt_irq_parse_and_map(node, 0);
    if (request_irq(dra->wake_irq, dra_wake_irq_handler, 0, "dra_wake_irq", NULL)) {
        dra_err("dra_wake_irq register fail\n");
        return -1;
    }

    enable_irq_wake(dra->wake_irq);
    return 0;
}

static int dra_probe(struct platform_device *pdev)
{
    struct dra *dra = NULL;
    int ret;

    dra_err("[init] start\n");
    dra = devm_kzalloc(&pdev->dev, sizeof(*dra), GFP_KERNEL);
    if (dra == NULL) {
        dra_err("no map tab alloc\n");
        return -ENOMEM;
    }

    of_dma_configure(&pdev->dev, NULL, true);
    dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(DRA_U64_BITLEN));
    dra->dev = &pdev->dev;
    dra->irq_flags = 0;

    if (dra_dt_init(dra)) {
        return -1;
    }

    ret = dra_clk_init(dra);
    if (ret) {
        return ret;
    }
    dra_para_update(dra);

    if (dra_core_probe(dra)) {
        return -1;
    }

    dra->wake_lock = wakeup_source_register(NULL, "modem_dra_wake");
    if (dra->wake_lock == NULL) {
        dra_err("dra wakeup_source_register fail\n");
    }

    dra_init_reclaim_timer(dra);
    if (dra_wakeirq_init(dra)) {
        return -1;
    }

    if (devm_request_irq(dra->dev, dra->irq, dra_interrupt, dra->irq_flags, "dra", dra)) {
        dra_err("irq=%u  request fail\n", dra->irq);
        return -1;
    }

    if (dra->wake_irq == 0) {
        enable_irq_wake(dra->irq);
    }

    return 0;
}

static int dra_remove(struct platform_device *pdev)
{
    return 0;
}

static const struct dev_pm_ops g_dra_pmops = {
    .suspend = dra_suspend,
    .resume = dra_resume,
};

static struct platform_driver g_dra_pltfm_driver = {
    .probe = dra_probe,
    .remove = dra_remove,
    .driver = {
        .name = "dra",
        .pm = &g_dra_pmops,
        .probe_type = PROBE_FORCE_SYNCHRONOUS,
    },
};

static struct platform_device g_dra_device = {
    .name = "dra",
    .id = -1,
    .num_resources = 0,
};

int dra_init(void)
{
    dra_err("dra_init\n");
    if (platform_device_register(&g_dra_device)) {
        dra_err("dev register fail.\n");
        return -1;
    }

    if (platform_driver_register(&g_dra_pltfm_driver)) {
        dra_err("drv register fail.\n");
        return -1;
    }

    return 0;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
subsys_initcall(dra_init);
#endif
void dra_exit(void)
{
    /*
     * don't need kfree platform_data in exit process,
     * platform_driver_unregister will do it.
     */
    platform_driver_unregister(&g_dra_pltfm_driver);

    return;
}

MODULE_DESCRIPTION("Specific DRA Driver");
MODULE_LICENSE("GPL v2");
