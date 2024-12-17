/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2021. All rights reserved.
 * Description: gid ctrl driver source code
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/types.h>
#include <bsp_print.h>
#include "sc_gid_ctrl_hal.h"
#include "sc_gid_ctrl.h"

#define THIS_MODU mod_syscache

void sc_gid_ctrl_hal_enabel(struct gid_ctrl *gid_ctx, unsigned int idx, unsigned int enable)
{
    writel(enable, gid_ctx->regs_peri + GID_REGION_EN(idx));
    if (gid_ctx->regs_acpu != NULL) {
        writel(enable, gid_ctx->regs_acpu + GID_REGION_EN(idx));
    }
}

void sc_gid_ctrl_hal_addr_cfg(struct gid_ctrl *gid_ctx, unsigned int start_addr, unsigned int end_addr, unsigned int cfg, int idx)
{
    writel_relaxed(start_addr, gid_ctx->regs_peri + START_ADDR(idx));
    writel_relaxed(end_addr, gid_ctx->regs_peri + END_ADDR(idx));
    writel_relaxed(cfg, gid_ctx->regs_peri + GID_CFG(idx));
    if (gid_ctx->regs_acpu != NULL) {
        writel_relaxed(start_addr, gid_ctx->regs_acpu + START_ADDR(idx));
        writel_relaxed(end_addr, gid_ctx->regs_acpu + END_ADDR(idx));
        writel_relaxed(cfg, gid_ctx->regs_acpu + GID_CFG(idx));
    }
    sc_gid_ctrl_hal_enabel(gid_ctx, idx, 1);
}

void sc_gid_ctrl_show(unsigned int idx)
{
    struct gid_ctrl *gid_ctx = sc_get_gid_ctx();

    if (idx >= SC_GID_CTRL_MAX) {
        bsp_err("idx invalid:%u\n", idx);
        return;
    }

    bsp_err("start_addr:0x%llx\n", gid_ctx->cfg_om[idx].start_addr);
    bsp_err("end_addr  :0x%llx\n", gid_ctx->cfg_om[idx].end_addr);
    bsp_err("size      :0x%x\n", gid_ctx->cfg_om[idx].size);
    bsp_err("cfg       :0x%x\n", gid_ctx->cfg_om[idx].cfg);
    bsp_err("enable    :0x%x\n", gid_ctx->cfg_om[idx].enable);

    bsp_err("gid_ctrl_peri\n");
    bsp_err("start_addr:0x%x\n", readl(gid_ctx->regs_peri + START_ADDR(idx)));
    bsp_err("end_addr  :0x%x\n", readl(gid_ctx->regs_peri + END_ADDR(idx)));
    bsp_err("cfg       :0x%x\n", readl(gid_ctx->regs_peri + GID_CFG(idx)));
    bsp_err("en        :0x%x\n", readl(gid_ctx->regs_peri + GID_REGION_EN(idx)));

    if (gid_ctx->gid_only && gid_ctx->regs_acpu != NULL) {
        bsp_err("gid_ctrl_acpu\n");
        bsp_err("start_addr:0x%x\n", readl(gid_ctx->regs_acpu + START_ADDR(idx)));
        bsp_err("end_addr  :0x%x\n", readl(gid_ctx->regs_acpu + END_ADDR(idx)));
        bsp_err("cfg       :0x%x\n", readl(gid_ctx->regs_acpu + GID_CFG(idx)));
        bsp_err("en        :0x%x\n", readl(gid_ctx->regs_acpu + GID_REGION_EN(idx)));
    }
}
