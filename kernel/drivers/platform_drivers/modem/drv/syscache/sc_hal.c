/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2021. All rights reserved.
 * Description: gid ctrl driver source code
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <bsp_syscache.h>
#include <bsp_print.h>
#include <bsp_slice.h>
#include "sc_gid_ctrl.h"
#include "sc_hal.h"
#include "sc.h"

#define THIS_MODU mod_syscache
#define SC_FLOW_START_MASK   0x4000319
#define SC_FLOW_END_MASK     0x318
#define SC_FLOW_GID_SHIFT    10
#define SC_FLOW_GID_MASK     (0xffff << FLOW_GID_SHIFT)
#define SC_FLOW_ENABLE_MASK  BIT(26)
#define SC_CMO_IRQ_CLEAR     0x1f000

void sc_hal_alloc_way(struct sc *sc_ctx, int gid, unsigned int way)
{
    writel(way, sc_ctx->regs + GID_WAY_ALLC(gid));
}

void sc_hal_set_bypass(struct sc *sc_ctx, int gid, unsigned int bypass)
{
    unsigned int val;

    val = readl(sc_ctx->regs + GLB_ROUTE_PLC_S);
    if (bypass) {
        writel(val | BIT(gid), sc_ctx->regs + GLB_ROUTE_PLC_S);
    } else {
        writel(val & (~BIT(gid)), sc_ctx->regs + GLB_ROUTE_PLC_S);
    }
}

void sc_hal_set_gid_config(struct sc *sc_ctx, unsigned int gid, unsigned int plc,
    unsigned int way, unsigned int quota)
{
    unsigned int val;

    bsp_err("gid cfg:0x%x-0x%x-0x%x-0x%x\n", gid, plc, way, quota);

    val = readl(sc_ctx->regs + GLB_ROUTE_PLC_S); // invalid gid bypass
    writel(val & (~BIT(gid)), sc_ctx->regs + GLB_ROUTE_PLC_S);

    writel(plc, (sc_ctx->regs + GID_ALLC_PLC(gid)));
    writel(way, (sc_ctx->regs + GID_WAY_ALLC(gid)));
    writel((quota << 16) | quota, (sc_ctx->regs + GID_QUOTA(gid)));
}

unsigned int sc_hal_get_cache_size(struct sc *sc_ctx, unsigned int gid)
{
    unsigned int i;
    unsigned int size = 0;

    for (i = 0; i < sc_ctx->slice_cnt; i++) {
        size += readl(sc_ctx->regs + SLC_GID_COUNT(i, gid));
    }

    return size;
}

void sc_hal_cfg_cmo(struct sc *sc_ctx, unsigned long long value)
{
    writeq(value, (sc_ctx->regs + CMO_CMD_L));
}

void sc_hal_clear_cmo_intr(struct sc *sc_ctx, unsigned int idx, unsigned int value)
{
    writel(value, (sc_ctx->regs + CFG_CMO_CLEAR(idx)));
}

unsigned int sc_hal_get_cmo_stat(struct sc *sc_ctx, unsigned int idx)
{
    return readl(sc_ctx->regs + CFG_CMO_STATUS(idx));
}

void sc_hal_start_flow(struct sc *sc_ctx, unsigned int gid, int bypass)
{
    unsigned int reg;
    unsigned int i;

    for (i = 0; i < sc_ctx->slice_cnt; i++) {
        reg = readl(sc_ctx->regs + GLB_FLOW_STAT(i));
        if (reg & SC_FLOW_ENABLE_MASK) {
            continue;
        }

        reg = SC_FLOW_START_MASK | BIT(SC_FLOW_GID_SHIFT + gid);
        writel(reg, (sc_ctx->regs + GLB_FLOW_STAT(i)));
    }
}

void sc_hal_end_flow(struct sc *sc_ctx, unsigned int id)
{
    unsigned int i;

    for (i = 0; i < sc_ctx->slice_cnt; i++) {
        writel(SC_FLOW_END_MASK, sc_ctx->regs + GLB_FLOW_STAT(i));
    }
}

unsigned long long sc_hal_get_flow(struct sc *sc_ctx, unsigned int dir)
{
    unsigned long long flow_cnt = 0;
    unsigned int i;

    for (i = 0; i < sc_ctx->slice_cnt; i++) {
        if (dir) {
            flow_cnt += readl(sc_ctx->regs + GLB_FLOW_STATS_IN(i));
        } else {
            flow_cnt += readl(sc_ctx->regs + GLB_FLOW_STATS_OUT(i));
        }
    }

    return flow_cnt;
}

void sc_hal_irq_enable_fc(struct sc *sc_ctx)
{
    writel(SC_FC_IRQ_MASK, sc_ctx->regs + CMO_CFG_INT_EN);
}

void sc_hal_irq_enable_dfx(struct sc *sc_ctx)
{
    unsigned int i;

    writel(SC_FC_IRQ_MASK, sc_ctx->regs + CMO_CFG_INT_EN);
    for (i = 0; i < sc_ctx->slice_cnt; i++) {
        writel(SC_DFX_IRQ_MASK, sc_ctx->regs + SLC_INT_EN(i));
    }
}

void sc_hal_irq_disable_fc(struct sc *sc_ctx)
{
    writel(0, sc_ctx->regs + CMO_CFG_INT_EN);
}

void sc_hal_irq_disable_dfx(struct sc *sc_ctx)
{
    unsigned int i;

    writel(0, sc_ctx->regs + CMO_CFG_INT_EN);
    for (i = 0; i < sc_ctx->slice_cnt; i++) {
        writel(0, sc_ctx->regs + SLC_INT_EN(i));
    }
}

unsigned int sc_hal_get_dfx_cmo_irq(struct sc *sc_ctx)
{
    return readl(sc_ctx->regs + CMO_CFG_INT_INI);
}

void sc_hal_clr_dfx_cmo_irq(struct sc *sc_ctx)
{
    writel(SC_CMO_IRQ_CLEAR, sc_ctx->regs + CMO_CFG_INT_CLR);
}

unsigned int sc_hal_get_dfx_irq(struct sc *sc_ctx, unsigned int slice)
{
    return readl(sc_ctx->regs + SLC_INT_INI(slice));
}

void sc_hal_clr_dfx_irq(struct sc *sc_ctx, unsigned int slice)
{
    writel(SC_CMO_IRQ_CLEAR, sc_ctx->regs + SLC_INT_CLEAR(slice));
}