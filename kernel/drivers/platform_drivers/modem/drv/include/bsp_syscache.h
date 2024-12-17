/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2023. All rights reserved.
 * Description: gid ctrl driver source code
 */

#ifndef __BSP_SYSCACHE_H_
#define __BSP_SYSCACHE_H_

#include <product_config.h>
#include <linux/types.h>
#include <linux/dma-mapping.h>

struct sc_mode {
    unsigned int sc_idx;
    unsigned int cfg;
};

enum sc_gid {
    SC_GID_DRA_MEM = 1,
    SC_GID_RINGBUF = 2,
    SC_GID_RINGBUF_TSP = 3,
    SC_GID_MAX
};

enum sc_rw_alloc_policy{
    SC_READ_NOT_ALLOC,
    SC_READ_ALLOC,
    SC_READ_DROP,
    SC_READ_CLEAN,
    SC_WRITE_NOT_ALLOC,
    SC_WRITE_ALLOC,
    SC_WRITE_NOT_REFILL,
    SC_WRITE_CLEAN,
    SC_RW_POLICY_MAX
};

#ifdef CONFIG_SC_GID_CTRL
// 获取sc reg配置
int bsp_sc_get_sc_mode(struct sc_mode *mode, int rpolicy, int wpolicy, unsigned int type);
// 申请ringbuf内存,映射gid_ctrl,policy可选,各IP统一申请后切分
char *bsp_sc_alloc_ringbuf(struct device *dev, unsigned int size, unsigned long long *phy_addr, gfp_t gfp);
void bsp_sc_free_ringbuf(struct device *dev, size_t size, void *cpu_addr, dma_addr_t dma_handle);
dma_addr_t bsp_sc_dma_map_single(struct device *dev, void *addr, size_t size, enum dma_data_direction dir);
void bsp_sc_dma_unmap_single(struct device *dev, dma_addr_t addr, size_t size, enum dma_data_direction dir);
#else
static inline int bsp_sc_get_sc_mode(struct sc_mode *mode, int rpolicy, int wpolicy, unsigned int type)
{
    if (mode != NULL) {
        mode->sc_idx = 0;
        mode->cfg = 0;
    }
    return 0;
}

static inline char *bsp_sc_alloc_ringbuf(struct device *dev, unsigned int size, unsigned long long *phy_addr, gfp_t gfp)
{
    return dma_alloc_coherent(dev, size, (dma_addr_t *)phy_addr, gfp);
}

static inline void bsp_sc_free_ringbuf(struct device *dev, size_t size, void *cpu_addr, dma_addr_t dma_handle)
{
    dma_free_coherent(dev, size, cpu_addr, dma_handle);
}

static inline dma_addr_t bsp_sc_dma_map_single(struct device *dev, void *addr, size_t size, enum dma_data_direction dir)
{
    return dma_map_single(dev, addr, size, dir);
}

static inline void bsp_sc_dma_unmap_single(struct device *dev, dma_addr_t addr, size_t size, enum dma_data_direction dir)
{
    dma_unmap_single(dev, addr, size, dir);
}
#endif

#endif
