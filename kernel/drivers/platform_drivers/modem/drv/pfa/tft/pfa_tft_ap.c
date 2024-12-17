/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: pfa_tft ap driver
 * Author: shennaiyu
 * Create: 2020-10-12
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
#include <osl_bio.h>
#include <linux/log2.h>
#include <linux/pm.h>
#include <linux/clk.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/of_platform.h>
#include <linux/dma-mapping.h>
#include <linux/module.h>
#include <securec.h>
#include <mdrv_sysboot.h>
#include <bsp_reset.h>
#include <bsp_slice.h>
#include <bsp_dra.h>
#include "pfa.h"
#include "pfa_tft.h"
#include "pfa_tft_reg.h"
#include "bsp_pfa_tft.h"
#include "bsp_dt.h"
#include <bsp_syscache.h>

#define NUM_PER_SLICE (32768)
#define BIT_NUM_PER_BYTE (8)
#define PD_CRG_SRSTEN3 (1)
#define PD_CRG_SRSTDIS3 (2)
#define PD_CRG_SRSTSTAT (3)
#define RESET_VALUE_FOR_PFA_TFT (4)
#define SHIFT_BIT_32 (32)
#define ENABLE_ADQ0_CHANNEL 1
#define ENABLE_BOTH_ADQ_CHANNEL (2)
#define ADQ_EMPTY_THRU (32)
#define PFA_TFT_RD_AMST_FULL_TH_OFFSET 14
#define PFA_TFT_AD_SIZE_MULTIPLES_OF_32(x) ((x) / 32)

struct pfa_tft_ctx_s g_pfa_tft_ap = {0};

static inline void phy_addr_write(u64 phy_addr, void *reg_h, void *reg_l)
{
    union phy_addr_u pa;

    pa.addr = phy_addr;

    writel(pa.u32_t.addr_h, reg_h);
    writel(pa.u32_t.addr_l, reg_l);
}

static inline u64 phy_addr_read(const void *reg_h, const void *reg_l)
{
    union phy_addr_u pa;

    pa.u32_t.addr_h = readl(reg_h);
    pa.u32_t.addr_l = readl(reg_l);

    return pa.addr;
}

int pfa_tft_pm_prepare(void)
{
    unsigned int val;

    val = pfa_tft_read(PFA_TFT_CH0_STATE_REG);
    if (UFIELD(U_PFA_TFT_CH0_STATE, &val).ul_bdq0_busy) {
        g_pfa_tft_ap.dl_bdq0_busy++;
        return -1;
    }

    if (UFIELD(U_PFA_TFT_CH0_STATE, &val).ul_bdq1_busy) {
        g_pfa_tft_ap.dl_bdq1_busy++;
        return -1;
    }

    val = pfa_tft_read(PFA_TFT_CH1_RDQ_DEPTH_REG);
    if (UFIELD(U_PFA_TFT_CH1_RDQ_DEPTH, &val).dl_rdq_depth) {
        g_pfa_tft_ap.dl_rdq_depth++;
        return -1;
    }

    val = pfa_tft_read(PFA_TFT_CH0_BDQ0_DEPTH_REG);
    if (UFIELD(U_PFA_TFT_CH0_BDQ0_DEPTH, &val).ul_bdq0_depth) {
        g_pfa_tft_ap.ul_bdq0_depth++;
        return -1;
    }

    g_pfa_tft_ap.prepare_done++;
    return 0;
}

void pfa_tft_pm_suspend(void)
{
    g_pfa_tft_ap.suspend_done++;

    return;
}

void pfa_tft_pm_resume(void)
{
    g_pfa_tft_ap.resume_done++;

    return;
}

static void pfa_tft_coop_dra(void)
{
    unsigned int reg;
    unsigned int offset;
    dma_addr_t push_target;
    unsigned int adq0_size_sel;
    unsigned int adq1_size_sel;

    phy_addr_write(g_pfa_tft_ap.adq0_dma, (unsigned char *)g_pfa_tft_ap.regs + PFA_TFT_CH1_ADQ0_BASE_H_REG,
                   (unsigned char *)g_pfa_tft_ap.regs + PFA_TFT_CH1_ADQ0_BASE_L_REG);

    phy_addr_write(g_pfa_tft_ap.adq1_dma, (unsigned char *)g_pfa_tft_ap.regs + PFA_TFT_CH1_ADQ1_BASE_H_REG,
                   (unsigned char *)g_pfa_tft_ap.regs + PFA_TFT_CH1_ADQ1_BASE_L_REG);

    adq0_size_sel = pfa_get_adq_size_sel(g_pfa_tft_ap.adq0_size);
    adq1_size_sel = pfa_get_adq_size_sel(g_pfa_tft_ap.adq1_size);
    UPDATE4(reg, PFA_TFT_CH1_ADQ_SIZE_CTRL, dl_adq_plen_th, DRA_OPIPE_LEVEL_0_SIZE, dl_adq_empty_th,
            g_pfa_tft_ap.adq1_size >> 1, dl_adq1_size_sel, adq1_size_sel, dl_adq0_size_sel, adq0_size_sel);

    if (g_pfa_tft_ap.soft_push) {
        push_target = g_pfa_tft_ap.adq0_wptr;
    } else {
        push_target = g_pfa_tft_ap.reg_phy + PFA_TFT_CH1_ADQ0_WPTR_REG;
    }
    offset = bsp_dra_set_adqbase(g_pfa_tft_ap.adq0_dma, push_target, g_pfa_tft_ap.adq0_size, DRA_IPIPE_FOR_PFA_TFT_512);
    if (offset != 0) {
        pfa_tft_write(offset, PFA_TFT_CH1_ADQ0_RPTR_UPDATE_ADDR_L_REG);
        pfa_tft_write(0, PFA_TFT_CH1_ADQ0_RPTR_UPDATE_ADDR_H_REG);
    } else {
        UPDATE1(reg, PFA_TFT_CH1_ADQ_SIZE_CTRL, dl_adq_plen_th, 0); // ADQ0 is disable
    }

    if (g_pfa_tft_ap.soft_push) {
        push_target = g_pfa_tft_ap.adq1_wptr;
    } else {
        push_target = g_pfa_tft_ap.reg_phy + PFA_TFT_CH1_ADQ1_WPTR_REG;
    }
    offset = bsp_dra_set_adqbase(g_pfa_tft_ap.adq1_dma, push_target, g_pfa_tft_ap.adq1_size, DRA_IPIPE_FOR_PFA_TFT_2K);
    if (offset == 0) {
        bsp_err("pfa_tft set adq1 2k fail\n");
        return;
    }
    pfa_tft_write(offset, PFA_TFT_CH1_ADQ1_RPTR_UPDATE_ADDR_L_REG);
    pfa_tft_write(0, PFA_TFT_CH1_ADQ1_RPTR_UPDATE_ADDR_H_REG);

    UPDATE1(reg, PFA_TFT_CH1_ADQ_EN_CTRL, dl_adq_en, ENABLE_BOTH_ADQ_CHANNEL);
}

void pfa_tft_pm_check_bd(void)
{
    unsigned int pfa_tft_ch0_stat;
    unsigned int pfa_tft_bd_wp;
    unsigned int pfa_tft_bd_rp;

    pfa_tft_bd_rp = pfa_tft_read(PFA_TFT_CH0_BDQ0_RPTR_REG);
    pfa_tft_ch0_stat = pfa_tft_read(PFA_TFT_CH0_STATE_REG);
    pfa_tft_bd_wp = pfa_tft_read(PFA_TFT_CH0_BDQ0_WPTR_REG);

    bsp_err("pfa_tft_ch0_stat = 0x%x pfa_tft_bd_wp = 0x%x pfa_tft_bd_rp = 0x%x bdq0_rptr_old 0x%x\n",
            pfa_tft_ch0_stat, pfa_tft_bd_wp, pfa_tft_bd_rp, g_pfa_tft_ap.bdq0_rptr_old);
}

static int pfa_tft_attr_init(struct pfa_tft_ctx_s *pfa_tft_ctx)
{
    if (memset_s(&pfa_tft_ctx->attr, sizeof(pfa_tft_ctx->attr), 0, sizeof(struct pfa_port_comm_attr))) {
        bsp_err("memset_s failed\n");
    }

    pfa_tft_ctx->attr.enc_type = PFA_ENC_PFA_TFT;
    if (bsp_dt_property_read_u32_array(pfa_tft_ctx->np, "pfa_tft_dl_rd_size", &pfa_tft_ctx->attr.td_depth, 1)) {
        pfa_tft_ctx->attr.td_depth = PFA_TFT_MAX_RD_SIZE;
    } else if (pfa_tft_ctx->attr.td_depth == 0 || pfa_tft_ctx->attr.td_depth > PFA_TFT_MAX_RD_SIZE) {
        bsp_err("invalid dl rd queue size. \n");
        return -1;
    }

    if (bsp_dt_property_read_u32_array(pfa_tft_ctx->np, "pfa_tft_ul_bd_size", &pfa_tft_ctx->attr.rd_depth, 1)) {
        pfa_tft_ctx->attr.rd_depth = PFA_TFT_MAX_BD_SIZE;
    } else if (pfa_tft_ctx->attr.rd_depth == 0 || pfa_tft_ctx->attr.rd_depth > PFA_TFT_MAX_BD_SIZE) {
        bsp_err("invalid ul bd queue size. \n");
        return -1;
    }

    pfa_tft_ctx->pfa_tft_attr.check_pfa_tft_bd = pfa_tft_pm_check_bd;
    pfa_tft_ctx->pfa_tft_attr.pfa_tft_addr.dltd_rptr_vir_addr =
        (dma_addr_t)(uintptr_t)((unsigned char *)pfa_tft_ctx->regs + PFA_TFT_CH1_RDQ_RPTR_REG);
    pfa_tft_ctx->pfa_tft_attr.pfa_tft_addr.dltd_rptr_phy_addr = pfa_tft_ctx->reg_phy + PFA_TFT_CH1_RDQ_RPTR_REG;
    pfa_tft_ctx->pfa_tft_attr.pfa_tft_addr.ulrd_wptr_addr = pfa_tft_ctx->reg_phy + PFA_TFT_CH0_BDQ0_WPTR_REG;

    return 0;
}

void pfa_tft_config_bd_rd_queue(void)
{
    unsigned int rd_size;

    rd_size = (g_pfa_tft_ap.rdq_size - 1) | ((g_pfa_tft_ap.rdq_size - 1) << PFA_TFT_RD_AMST_FULL_TH_OFFSET);
    pfa_tft_write(rd_size, PFA_TFT_CH1_RDQ_SIZE_REG);
    pfa_tft_write(g_pfa_tft_ap.bdq0_size - 1, PFA_TFT_CH0_BDQ0_SIZE_REG);

    phy_addr_write(g_pfa_tft_ap.pfa_tft_attr.pfa_addr.ulrd_base_addr, (unsigned char *)g_pfa_tft_ap.regs + PFA_TFT_CH0_BDQ0_BADDR_H_REG,
                   (unsigned char *)g_pfa_tft_ap.regs + PFA_TFT_CH0_BDQ0_BADDR_L_REG);

    phy_addr_write(g_pfa_tft_ap.pfa_tft_attr.pfa_addr.ulrd_rptr_addr,
                   (unsigned char *)g_pfa_tft_ap.regs + PFA_TFT_CH0_BDQ0_RPTR_UPDATE_ADDR_H_REG,
                   (unsigned char *)g_pfa_tft_ap.regs + PFA_TFT_CH0_BDQ0_RPTR_UPDATE_ADDR_L_REG);

    phy_addr_write(g_pfa_tft_ap.pfa_tft_attr.pfa_addr.dltd_base_addr, (unsigned char *)g_pfa_tft_ap.regs + PFA_TFT_CH1_RDQ_BADDR_H_REG,
                   (unsigned char *)g_pfa_tft_ap.regs + PFA_TFT_CH1_RDQ_BADDR_L_REG);

    phy_addr_write(g_pfa_tft_ap.pfa_tft_attr.pfa_addr.dltd_wptr_addr,
                   (unsigned char *)g_pfa_tft_ap.regs + PFA_TFT_CH1_RDQ_WPTR_UPDATE_ADDR_H_REG,
                   (unsigned char *)g_pfa_tft_ap.regs + PFA_TFT_CH1_RDQ_WPTR_UPDATE_ADDR_L_REG);
}

static int pfa_tft_coop_pfa(void)
{
    int ret;

    ret = pfa_tft_attr_init(&g_pfa_tft_ap);
    if (ret != 0) {
        g_pfa_tft_ap.invalid_queue_len++;
        return -1;
    }

    g_pfa_tft_ap.pfa_portno = bsp_pfa_alloc_port(&g_pfa_tft_ap.attr);
    if (g_pfa_tft_ap.pfa_portno < 0) {
        bsp_err("bsp_pfa_alloc_port error\n");
        return -1;
    }

    if (bsp_pfa_set_pfa_tftproperty(&g_pfa_tft_ap.pfa_tft_attr)) {
        bsp_err("[%s]wan_pfa_tftport_reg_get failed@line:%x!\n", __func__, __LINE__);
        return -1;
    }

    g_pfa_tft_ap.rdq_size = g_pfa_tft_ap.attr.td_depth;
    g_pfa_tft_ap.bdq0_size = g_pfa_tft_ap.attr.rd_depth;
    g_pfa_tft_ap.rdq_base = g_pfa_tft_ap.pfa_tft_attr.pfa_addr.dltd_base_addr_v;
    g_pfa_tft_ap.bdq0_base = g_pfa_tft_ap.pfa_tft_attr.pfa_addr.ulrd_base_addr_v;
    g_pfa_tft_ap.ulrd_rptr_virt = g_pfa_tft_ap.pfa_tft_attr.pfa_addr.ulrd_rptr_addr_v;

    g_pfa.hal->tft_config_rd_ip_head(g_pfa_tft_ap.pfa_tft_attr.ext_desc_en);
    pfa_tft_config_bd_rd_queue();

    return bsp_pfa_enable_port(g_pfa_tft_ap.pfa_portno);
}

static int pfa_tft_alloc_ad_mem(void)
{
    g_pfa_tft_ap.adq0_base = 0;
    g_pfa_tft_ap.adq1_base = 0;
    g_pfa_tft_ap.adq0_base = dma_alloc_coherent(g_pfa_tft_ap.dev, (g_pfa_tft_ap.adq0_size * sizeof(struct pfa_tft64_ad_s)),
        &g_pfa_tft_ap.adq0_dma, GFP_KERNEL);
    if (g_pfa_tft_ap.adq0_base == NULL) {
        bsp_err("[init]dma_alloc_coherent failed, adq0_base invalid\n");
        return -ENOMEM;
    }

    if (!dma_alloc_coherent(g_pfa_tft_ap.dev, sizeof(unsigned int), &g_pfa_tft_ap.adq0_wptr, GFP_KERNEL)) {
        bsp_err("[init]dma_alloc_coherent failed, adq0_wptr invalid\n");
        return -ENOMEM;
    }
    g_pfa_tft_ap.adq1_base = dma_alloc_coherent(g_pfa_tft_ap.dev, (g_pfa_tft_ap.adq1_size * sizeof(struct pfa_tft64_ad_s)),
        &g_pfa_tft_ap.adq1_dma, GFP_KERNEL);
    if (g_pfa_tft_ap.adq1_base == NULL) {
        bsp_err("[init]dma_alloc_coherent failed, adq1_base invalid\n");
        return -ENOMEM;
    }
    if (!dma_alloc_coherent(g_pfa_tft_ap.dev, sizeof(unsigned int), &g_pfa_tft_ap.adq1_wptr, GFP_KERNEL)) {
        bsp_err("[init]dma_alloc_coherent failed, adq1_wptr invalid\n");
        return -ENOMEM;
    }
    return 0;
}

static int pfa_tft_resource_alloc(void)
{
    if (pfa_tft_alloc_ad_mem()) {
        return -ENOMEM;
    }

    return 0;
}

static void pfa_tft_resource_free(void)
{
    if (g_pfa_tft_ap.adq0_base != NULL) {
        dma_free_coherent(g_pfa_tft_ap.dev, (g_pfa_tft_ap.adq0_size * sizeof(struct pfa_tft64_ad_s)), g_pfa_tft_ap.adq0_base,
                          g_pfa_tft_ap.adq0_dma);
        g_pfa_tft_ap.adq0_base = NULL;
    }

    if (g_pfa_tft_ap.adq1_base != NULL) {
        dma_free_coherent(g_pfa_tft_ap.dev, (g_pfa_tft_ap.adq1_size * sizeof(struct pfa_tft64_ad_s)), g_pfa_tft_ap.adq1_base,
                          g_pfa_tft_ap.adq1_dma);
        g_pfa_tft_ap.adq1_base = NULL;
    }
}

void pfa_tft_reset(void)
{
    if (g_pfa_tft_ap.need_reset) {
        writel_relaxed(g_pfa_tft_ap.reset_peri_crg[RESET_VALUE_FOR_PFA_TFT],
                   (void *)(g_pfa_tft_ap.reset_reg + g_pfa_tft_ap.reset_peri_crg[PD_CRG_SRSTEN3]));
        writel_relaxed(g_pfa_tft_ap.reset_peri_crg[RESET_VALUE_FOR_PFA_TFT],
                   (void *)(g_pfa_tft_ap.reset_reg + g_pfa_tft_ap.reset_peri_crg[PD_CRG_SRSTDIS3]));
    }
}

void pfa_tft_deinit(void)
{
    unsigned int reg;

    if (g_pfa_tft_ap.need_reset) {
        phy_addr_write(0, (unsigned char *)g_pfa_tft_ap.regs + PFA_TFT_CH0_BDQ0_RPTR_UPDATE_ADDR_H_REG,
                   (unsigned char *)g_pfa_tft_ap.regs + PFA_TFT_CH0_BDQ0_RPTR_UPDATE_ADDR_L_REG);
        phy_addr_write(0, (unsigned char *)g_pfa_tft_ap.regs + PFA_TFT_CH1_RDQ_WPTR_UPDATE_ADDR_H_REG,
                   (unsigned char *)g_pfa_tft_ap.regs + PFA_TFT_CH1_RDQ_WPTR_UPDATE_ADDR_L_REG);
    }

    UPDATE1(reg, PFA_TFT_CH1_ADQ_EN_CTRL, dl_adq_en, 0);
}

void bsp_pfa_tft_reinit(void)
{
    bsp_err("pfa_tft_reset++\n");
    pfa_tft_reset();
    bsp_err("pfa_tft_reset--\n");
    g_pfa_tft_ap.bdq0_rptr_old = 0;

    pfa_tft_coop_dra();

    g_pfa.hal->tft_config_rd_ip_head(g_pfa_tft_ap.pfa_tft_attr.ext_desc_en);
    pfa_tft_config_bd_rd_queue();
}

int pfa_tft_mdmreset_cb(drv_reset_cb_moment_e eparam, int userdata)
{
    if (eparam == MDRV_RESET_CB_BEFORE) {
        pfa_tft_deinit();
    }

    return 0;
}

void pfa_tft_dts_init(device_node_s *np)
{
    unsigned int adq_size;

    g_pfa_tft_ap.need_reset = 0;
    if (bsp_dt_property_read_u32_array(np, "pfa_tft_need_reset", &g_pfa_tft_ap.need_reset, 1)) {
        bsp_err("pfa tft no need reset alone, need_reset:%d\n", g_pfa_tft_ap.need_reset);
    }

    if (bsp_dt_property_read_u32_array(np, "pfa_tft_rst_crg", g_pfa_tft_ap.reset_peri_crg, PFA_TFT_RESET_DTS_ARRAY)) {
        bsp_err("rst_crg dts parsed failed \n");
        return;
    }

    if (bsp_dt_property_read_u32_array(np, "pfa_tft_soft_push", &g_pfa_tft_ap.soft_push, 1)) {
        g_pfa_tft_ap.soft_push = 0;
        bsp_err("PFA_TFT-DRA ADQ pushed by hardware.\n");
    }
    if (bsp_dt_property_read_u32_array(np, "pfa_tft_adq_size", &adq_size, 1)) {
        g_pfa_tft_ap.adq0_size = PFA_TFT_DL_AD_MEM_SIZE_DEFAULT;
        g_pfa_tft_ap.adq1_size = PFA_TFT_DL_AD_MEM_SIZE_DEFAULT;
        bsp_err("PFA_TFT-DRA ADQ size use default.\n");
    } else {
        if (adq_size > PFA_ADQ_SIZE_MAX) {
            adq_size = PFA_ADQ_SIZE_MAX;
            bsp_err("adq size is too large!!\n");
        }
        g_pfa_tft_ap.adq0_size = adq_size;
        g_pfa_tft_ap.adq1_size = adq_size;
    }
}

int pfa_tft_global_init(void)
{
    device_node_s *np = g_pfa_tft_ap.np;
    const __be32 *reg_addr_p = NULL;
    int na, ns;
    void __iomem *reg_vir;
    u64 reg_phy;
    int cells_size;

    reg_addr_p = bsp_dt_get_property(np, "reg", NULL);
    if (reg_addr_p == NULL) {
        bsp_err("get_address failed.\r\n");
        return -1;
    }
    na = bsp_dt_n_addr_cells(np);
    ns = bsp_dt_n_size_cells(np);
    cells_size = na + ns;

    reg_phy = bsp_dt_read_number(reg_addr_p + 1 * cells_size, na); /*lint !e679*/

    reg_vir = bsp_dt_iomap(np, 1);
    if (reg_vir == NULL) {
        bsp_err("iomap failed.\r\n");
        return -1;
    }
    g_pfa_tft_ap.reg_phy = reg_phy;
    g_pfa_tft_ap.regs = reg_vir;

    return 0;
}

static int pfa_tft_queue_init(void)
{
    if (pfa_tft_resource_alloc()) {
        pfa_tft_resource_free();
        return -ENOMEM;
    }

    pfa_tft_coop_dra();

    if (pfa_tft_coop_pfa()) {
        bsp_err("pfa_tft_coop_pfa failed!\n");
    }

    return 0;
}

void pfa_tft_config_sc(void)
{
    struct sc_mode mode;
    int ret;

    ret = bsp_sc_get_sc_mode(&mode, SC_READ_ALLOC, SC_WRITE_ALLOC, 0);
    if (ret || mode.sc_idx >= PFA_TFT_SC_IDX_MAX) {
        bsp_err("tft get sc mode fail\n");
        return;
    }
    pfa_tft_write(mode.cfg, PFA_TFT_SC_REG(mode.sc_idx));
}

int pfa_tft_init(void *pfa_ptr)
{
    struct pfa *pfa = NULL;

    bsp_err("[init]pfa_tft_probe:\n");
    pfa = (struct pfa*)pfa_ptr;
    g_pfa_tft_ap.dev = pfa->dev;
    g_pfa_tft_ap.np = pfa->np;

    if (pfa_tft_global_init()) {
        return -ENXIO;
    }
    pfa_tft_config_sc();

    spin_lock_init(&g_pfa_tft_ap.bd_lock);

    pfa_tft_dts_init(g_pfa_tft_ap.np);

    g_pfa_tft_ap.reset_reg = (unsigned char *)ioremap(g_pfa_tft_ap.reset_peri_crg[0], 0x100);
    if (g_pfa_tft_ap.reset_reg == NULL) {
        return -ENXIO;
    }

    if (pfa_tft_queue_init()) {
        return -ENXIO;
    }

    if (bsp_reset_cb_func_register("PFA_TFT", pfa_tft_mdmreset_cb, 0, DRV_RESET_CB_PIOR_PFA_TFT)) {
        bsp_err("set modem reset call back func failed\n");
    }

    bsp_err("pfa_tft_probe done\n");
    return 0;
}

#define PFA_TFT_BYTE_TO_MB(cnt) ((cnt) / 1024 / 1024)
void pfa_tft_dl_cnt(void)
{
    unsigned int reg;
    unsigned int time;

    static unsigned int pfa_tft_last_timestamp = 0;
    static unsigned int ul0_cnt = 0;
    static unsigned int ul1_cnt = 0;
    static unsigned int dl_cnt = 0;

    time = bsp_get_slice_value();

    reg = pfa_tft_read(PFA_TFT_CH1_PKT_CNT_REG);
    bsp_err("dl total cnt:%u Mbyte\n", reg);
    bsp_err("dl speed during last sample:%u Mb/s\n",
            PFA_TFT_BYTE_TO_MB(reg - dl_cnt) * BIT_NUM_PER_BYTE * NUM_PER_SLICE / (time - pfa_tft_last_timestamp));
    dl_cnt = reg;

    reg = pfa_tft_read(PFA_TFT_CH0_PKT_CNT0_REG);
    bsp_err("ul_lr total cnt:%u Mbyte\n", reg);
    bsp_err("dl lr speed during last sample:%u Mb/s\n",
            (reg - ul0_cnt) * BIT_NUM_PER_BYTE * NUM_PER_SLICE / (time - pfa_tft_last_timestamp));
    ul0_cnt = reg;

    reg = pfa_tft_read(PFA_TFT_CH0_PKT_CNT1_REG);
    bsp_err("ul_nr total cnt:%u Mbyte\n", reg);
    bsp_err("dl nr speed during last sample:%u Mb/s\n",
            (reg - ul1_cnt) * BIT_NUM_PER_BYTE * NUM_PER_SLICE / (time - pfa_tft_last_timestamp));
    ul1_cnt = reg;

    pfa_tft_last_timestamp = time;
}

EXPORT_SYMBOL(g_pfa_tft_ap);
EXPORT_SYMBOL(pfa_tft_dl_cnt);
