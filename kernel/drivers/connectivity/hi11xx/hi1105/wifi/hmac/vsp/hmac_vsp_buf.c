/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : vsp dma-buf管理
 * 作    者 : wifi
 * 创建日期 : 2022年3月22日
 */

#include "hmac_vsp_buf.h"
#include "oal_dma_buf.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_VSP_BUF_C

#ifdef _PRE_WLAN_FEATURE_VSP
#ifdef CONFIG_VCODEC_VSP_SUPPORT
typedef struct {
    struct dma_buf *dmabuf;
    struct dma_buf_attachment *attachment;
    struct sg_table *table;
    oal_pci_dev_stru *pci_dev;
} hmac_vsp_dma_buf_context_stru;

static uint32_t hmac_vsp_source_init_dma_buf_context(hmac_vsp_dma_buf_context_stru *context)
{
    context->pci_dev = oal_get_wifi_pcie_dev();

    if (!context->pci_dev) {
        return OAL_FAIL;
    }

    context->attachment = oal_dma_buf_attach(context->dmabuf, &context->pci_dev->dev);
    context->table = oal_dma_buf_map_attach(context->attachment, DMA_BIDIRECTIONAL);

    return OAL_SUCC;
}

static void hmac_vsp_source_deinit_dma_buf_context(hmac_vsp_dma_buf_context_stru *context)
{
    oal_dma_buf_unmap_attachment(context->attachment, context->table, DMA_BIDIRECTIONAL);
    oal_dma_buf_detach(context->dmabuf, context->attachment);
}

uint32_t hmac_vsp_source_map_dma_buf(struct dma_buf *dmabuf, uintptr_t *iova)
{
    hmac_vsp_dma_buf_context_stru context = { .dmabuf = dmabuf };

    if (oal_dma_buf_begin_cpu_access(dmabuf, DMA_FROM_DEVICE)) {
        return OAL_FAIL;
    }

    if (hmac_vsp_source_init_dma_buf_context(&context) != OAL_SUCC) {
        return OAL_FAIL;
    }

    if (!dma_map_sg(&context.pci_dev->dev, context.table->sgl, context.table->nents, DMA_BIDIRECTIONAL)) {
        return OAL_FAIL;
    }

    *iova = sg_dma_address(context.table->sgl);
    hmac_vsp_source_deinit_dma_buf_context(&context);

    return OAL_SUCC;
}

uint32_t hmac_vsp_source_unmap_dma_buf(struct dma_buf *dmabuf)
{
    hmac_vsp_dma_buf_context_stru context = { .dmabuf = dmabuf };

    if (oal_dma_buf_end_cpu_access(dmabuf, DMA_TO_DEVICE)) {
        return OAL_FAIL;
    }

    if (hmac_vsp_source_init_dma_buf_context(&context) != OAL_SUCC) {
        return OAL_FAIL;
    }

    dma_unmap_sg(&context.pci_dev->dev, context.table->sgl, context.table->nents, DMA_BIDIRECTIONAL);
    hmac_vsp_source_deinit_dma_buf_context(&context);

    return OAL_SUCC;
}

void hmac_vsp_source_dma_buf_func_register(void)
{
    vsp_source_map_dma_buf = hmac_vsp_source_map_dma_buf;
    vsp_source_unmap_dma_buf = hmac_vsp_source_unmap_dma_buf;
    oam_warning_log0(0, 0, "{hmac_vsp_source_dma_buf_func_register::dma-buf map/unmap function enabled}");
}

void hmac_vsp_source_dma_buf_func_unregister(void)
{
    vsp_source_map_dma_buf = NULL;
    vsp_source_unmap_dma_buf = NULL;
    oam_warning_log0(0, 0, "{hmac_vsp_source_dma_buf_func_unregister::dma-buf map/unmap function enabled}");
}
#else
void hmac_vsp_source_dma_buf_func_register(void)
{
    oam_warning_log0(0, 0, "{hmac_vsp_source_dma_buf_func_register::dma-buf map/unmap function not enabled}");
}

void hmac_vsp_source_dma_buf_func_unregister(void)
{
    oam_warning_log0(0, 0, "{hmac_vsp_source_dma_buf_func_unregister::dma-buf map/unmap function not enabled}");
}
#endif

#endif
