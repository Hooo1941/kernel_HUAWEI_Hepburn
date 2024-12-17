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
#include <securec.h>
#include <bsp_syscache.h>
#include <bsp_print.h>
#include <bsp_dt.h>
#include <bsp_version.h>
#include "sc_gid_ctrl_hal.h"
#include "sc_gid_ctrl.h"
#include "sc.h"

#define THIS_MODU mod_syscache

#define SC_GID_CTRL_USED 0xffffffaa
#define SC_GID_CTRL_UNUSED 0
#define SC_U64_BITLEN 64
#define SC_ALIGN 4096
unsigned int sc_rw_policy[SC_RW_POLICY_MAX] = {
    0x8, 0x9, 0xa, 0xb, 0x8, 0x9, 0xa, 0xb
};
struct gid_ctrl g_gid_ctrl;

struct gid_ctrl *sc_get_gid_ctx(void)
{
    return &g_gid_ctrl;
}

int bsp_sc_get_sc_mode(struct sc_mode *mode, int rpolicy, int wpolicy, unsigned int type)
{
    gid_cfg cfg = g_gid_ctrl.common_cfg;
    struct gid_ctrl *gid_ctx = &g_gid_ctrl;

    UNUSED(type);
    if (mode == NULL || rpolicy >= SC_RW_POLICY_MAX || wpolicy >= SC_RW_POLICY_MAX) {
        bsp_err("para err:%d-%d\n", rpolicy, wpolicy);
        return -1;
    }

    if (gid_ctx->gid_only && gid_ctx->regs_acpu != NULL) {
        cfg.bits.gid = SC_GID_DRA_MEM;
        cfg.bits.arhint = sc_rw_policy[rpolicy];
        cfg.bits.awhint = sc_rw_policy[wpolicy];
    } else {
        cfg.bits.gid = 0;
    }

    mode->sc_idx = g_gid_ctrl.sc_mode_idx;
    mode->cfg = cfg.val;

    return 0;
}
EXPORT_SYMBOL(bsp_sc_get_sc_mode);

int sc_gid_ctrl_cfg(struct gid_ctrl *gid_ctx, unsigned long long phy_addr, unsigned int size)
{
    unsigned long flag;
    gid_cfg cfg = gid_ctx->common_cfg;
    unsigned int start_addr;
    unsigned int end_addr;
    int i;

    start_addr = phy_addr >> GID_CTRL_ADDR_MASK_BIT;
    end_addr = ((phy_addr + size) >> GID_CTRL_ADDR_MASK_BIT) - 1;

    if (gid_ctx->gid_only && gid_ctx->regs_acpu != NULL) {
        cfg.bits.gid = SC_GID_RINGBUF;
    } else {
        cfg.bits.gid = 0;
    }

    spin_lock_irqsave(&gid_ctx->lock, flag);
    for (i = 0; i < SC_GID_CTRL_MAX; i++) {
        if (gid_ctx->cfg_om[i].enable == 0) {
            break;
        }
    }

    if (i == SC_GID_CTRL_MAX) {
        bsp_err("no idle gid ctrl entry\n");
        spin_unlock_irqrestore(&gid_ctx->lock, flag);
        return -1;
    }
    bsp_err("gid ctrl addr:0x%llx-0x%x\n", phy_addr, size);

    gid_ctx->cfg_om[i].start_addr = phy_addr;
    gid_ctx->cfg_om[i].end_addr = phy_addr + size;
    gid_ctx->cfg_om[i].size = size;
    gid_ctx->cfg_om[i].cfg = cfg.val;
    gid_ctx->cfg_om[i].enable = SC_GID_CTRL_USED;
    sc_gid_ctrl_hal_addr_cfg(gid_ctx, start_addr, end_addr, cfg.val, i);
    spin_unlock_irqrestore(&gid_ctx->lock, flag);

    return 0;
}

char *bsp_sc_alloc_ringbuf(struct device *dev, unsigned int size, unsigned long long *phy_addr, gfp_t gfp)
{
    struct gid_ctrl *gid_ctx = &g_gid_ctrl;
    char *base = NULL;
    int ret;

    if (gid_ctx->init == 0 || phy_addr == NULL) {
        bsp_err("sc no init:%u\n", gid_ctx->init);
        return NULL;
    }

    size = ALIGN(size, SC_GID_CTRL_ALIGN);
    if (gid_ctx->gid_only) {
        base = (char *)dma_alloc_coherent(dev, size, phy_addr, gfp);
    } else {
        base = (char *)kmalloc(size, gfp);
        *phy_addr = virt_to_phys((char *)(uintptr_t)base);
    }
    if (base == NULL) {
        bsp_err("alloc ringbuf fail\n");
        return NULL;
    }

    ret = sc_gid_ctrl_cfg(gid_ctx, *phy_addr, size);
    if (ret) {
        goto ERR;
    }
    return base;
ERR:
    if (gid_ctx->gid_only) {
        dma_free_coherent(dev, size, base, *phy_addr);
    } else {
        kfree(base);
    }

    return NULL;
}
EXPORT_SYMBOL(bsp_sc_alloc_ringbuf);

void bsp_sc_free_ringbuf(struct device *dev, size_t size, void *cpu_addr, dma_addr_t dma_handle)
{
    unsigned long flag;
    struct gid_ctrl *gid_ctx = &g_gid_ctrl;
    unsigned int start_addr;
    int i;

    if (dev == NULL || size == 0 || cpu_addr == NULL || dma_handle == 0) {
        bsp_err("param err\n");
        return;
    }

    start_addr = dma_handle >> GID_CTRL_ADDR_MASK_BIT;

    spin_lock_irqsave(&gid_ctx->lock, flag);
    for (i = 0; i < SC_GID_CTRL_MAX; i++) {
        if (gid_ctx->cfg_om[i].start_addr == start_addr) {
            break;
        }
    }

    if (i == SC_GID_CTRL_MAX) {
        bsp_err("no find gid ctrl entry\n");
        spin_unlock_irqrestore(&gid_ctx->lock, flag);
        return;
    }

    gid_ctx->cfg_om[i].enable = SC_GID_CTRL_UNUSED;
    sc_gid_ctrl_hal_enabel(gid_ctx, i, 0);
    sc_invalid_cache(dma_handle, gid_ctx->cfg_om[i].size);
    if (gid_ctx->gid_only) {
        dma_free_coherent(dev, gid_ctx->cfg_om[i].size, cpu_addr, dma_handle);
    } else {
        kfree(cpu_addr);
    }
    spin_unlock_irqrestore(&gid_ctx->lock, flag);
    return;
}
EXPORT_SYMBOL(bsp_sc_free_ringbuf);

dma_addr_t bsp_sc_dma_map_single(struct device *dev, void *addr, size_t size, enum dma_data_direction dir)
{
    if (g_gid_ctrl.gid_only) {
        return dma_map_single(dev, addr, size, dir);
    } else {
        return virt_to_phys(addr);
    }
}
EXPORT_SYMBOL(bsp_sc_dma_map_single);

void bsp_sc_dma_unmap_single(struct device *dev, dma_addr_t addr, size_t size, enum dma_data_direction dir)
{
    if (g_gid_ctrl.gid_only) {
        if (bsp_get_version_info()->plat_type != PLAT_ESL) {
            sc_invalid_cache(addr, size);
        }
        dma_unmap_single(dev, addr, size, dir);
    }
}
EXPORT_SYMBOL(bsp_sc_dma_unmap_single);

int gid_ctrl_acpu_dt_config(struct gid_ctrl *gid_ctx)
{
    device_node_s *node = NULL;

    node = bsp_dt_find_compatible_node(NULL, NULL, "gid_ctrl_acpu");
    if (node == NULL) {
        bsp_err("no support gid_ctrl_acpu\n");
        return 0;
    }

    gid_ctx->regs_acpu = bsp_dt_iomap(node, 0);
    if (gid_ctx->regs_acpu == NULL) {
        bsp_err("no gid ctrl acpu reg\n");
        return -1;
    }

    return 0;
}

int gid_ctrl_dt_config(struct gid_ctrl *gid_ctx)
{
    device_node_s *node = NULL;
    unsigned int cfg_arr[5];
    int ret;

    node = bsp_dt_find_compatible_node(NULL, NULL, "gid_ctrl_peri");
    if (node == NULL) {
        bsp_err("no gid_ctrl_peri\n");
        return -1;
    }

    gid_ctx->regs_peri = bsp_dt_iomap(node, 0);
    if (gid_ctx->regs_peri == NULL) {
        bsp_err("no gid ctrl peri reg\n");
        return -1;
    }

    ret = bsp_dt_property_read_u32(node, "gid-only", &gid_ctx->gid_only);
    if (ret) {
        bsp_err("no gid ctrl gid_only\n");
        return -1;
    }

    ret = bsp_dt_property_read_u32(node, "gid-ctrl-sc-mode-idx", &gid_ctx->sc_mode_idx);
    if (ret) {
        bsp_err("no gid ctrl sc_mdoe\n");
        return -1;
    }

    ret = bsp_dt_property_read_u32(node, "gid-ctrl-max-entry-num", &gid_ctx->entry_max_num);
    if (ret || gid_ctx->entry_max_num > SC_GID_CTRL_MAX) {
        bsp_err("no gid ctrl entry_max_num:%u\n", gid_ctx->entry_max_num);
        return -1;
    }

    ret = bsp_dt_property_read_u32_array(node, "sc-mode-cfg", cfg_arr, 5);
    if (ret) {
        bsp_err("no gid ctrl policy\n");
        return -1;
    }

    gid_ctx->common_cfg.bits.arhint = cfg_arr[0];
    gid_ctx->common_cfg.bits.awhint = cfg_arr[1];
    gid_ctx->common_cfg.bits.arcache = cfg_arr[2];
    gid_ctx->common_cfg.bits.awcache = cfg_arr[3];
    gid_ctx->common_cfg.bits.domain = cfg_arr[4];
    gid_ctx->common_cfg.bits.override_en = 0x3f;

    return gid_ctrl_acpu_dt_config(gid_ctx);
}

static int gid_ctrl_probe(struct platform_device *pdev)
{
    struct gid_ctrl *gid_ctx = &g_gid_ctrl;

    bsp_err("[init] start\n");
    // sc初始化在前，调用sc初始化接口
    if (sc_init(pdev)) {
        bsp_err("sc init fail\n");
        return -1;
    }

    spin_lock_init(&gid_ctx->lock);
    if (gid_ctrl_dt_config(gid_ctx)) {
        return -1;
    }

    gid_ctx->init = 1;
    bsp_err("[init] succ\n");

    return 0;
}

static int gid_ctrl_remove(struct platform_device *pdev)
{
    return 0;
}

static struct platform_device g_gid_ctrl_device = {
    .name = "gid_ctrl",
    .id = -1,
    .num_resources = 0,
};

static struct platform_driver g_gid_ctrl_pltfm_driver = {
    .probe = gid_ctrl_probe,
    .remove = gid_ctrl_remove,
    .driver = {
        .name = "gid_ctrl",
        .probe_type = PROBE_FORCE_SYNCHRONOUS,
    },
};

int gid_ctrl_init(void)
{
    bsp_err("gid_ctrl_init\n");
    if (platform_device_register(&g_gid_ctrl_device)) {
        bsp_err("dev register fail.\n");
        return -1;
    }

    if (platform_driver_register(&g_gid_ctrl_pltfm_driver)) {
        bsp_err("driver register fail.\n");
        return -1;
    }

    return 0;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
core_initcall(gid_ctrl_init);
#endif

void gid_ctrl_exit(void)
{
    /*
     * don't need kfree platform_data in exit process,
     * platform_driver_unregister will do it.
     */
    platform_driver_unregister(&g_gid_ctrl_pltfm_driver);
    return;
}
MODULE_DESCRIPTION("Specific gid_ctrl Driver");
MODULE_LICENSE("GPL v2");
