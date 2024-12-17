/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: DMA-BUF adaptation interface
 * Author: @CompanyNameTag
 */

#ifndef OAL_LINUX_DMA_BUF_H
#define OAL_LINUX_DMA_BUF_H

#include <linux/dma-buf.h>
#include "pcie_linux.h"

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 6, 0))
static inline int32_t oal_dma_buf_begin_cpu_access(struct dma_buf *dmabuf, enum dma_data_direction dir)
{
    return dma_buf_begin_cpu_access(dmabuf, DMA_FROM_DEVICE);
}

static inline int32_t oal_dma_buf_end_cpu_access(struct dma_buf *dmabuf, enum dma_data_direction dir)
{
    return dma_buf_begin_cpu_access(dmabuf, DMA_FROM_DEVICE);
}
#else
static inline int32_t oal_dma_buf_begin_cpu_access(struct dma_buf *dmabuf, enum dma_data_direction dir)
{
    return -OAL_EFAIL;
}

static inline int32_t oal_dma_buf_end_cpu_access(struct dma_buf *dmabuf, enum dma_data_direction dir)
{
    return -OAL_EFAIL;
}
#endif

static inline struct dma_buf_attachment *oal_dma_buf_attach(struct dma_buf *dmabuf, struct device *dev)
{
    return dma_buf_attach(dmabuf, dev);
}

static inline struct sg_table *oal_dma_buf_map_attach(struct dma_buf_attachment *attach, enum dma_data_direction dir)
{
    return dma_buf_map_attachment(attach, dir);
}

static inline void oal_dma_buf_detach(struct dma_buf *dmabuf, struct dma_buf_attachment *attach)
{
    dma_buf_detach(dmabuf, attach);
}

static inline void oal_dma_buf_unmap_attachment(
    struct dma_buf_attachment *attach, struct sg_table *sg_table, enum dma_data_direction dir)
{
    dma_buf_unmap_attachment(attach, sg_table, dir);
}

#endif
