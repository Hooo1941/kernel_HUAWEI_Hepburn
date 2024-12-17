# SPDX-License-Identifier: GPL-2.0
ccflags-y += -I$(srctree)/drivers/platform_drivers/tzdriver
ccflags-y += -I$(srctree)/platform_source/basicplatform/drivers/secmem
ccflags-y += -I$(srctree)/drivers/dma-buf/heaps
ccflags-y += -I$(srctree)/platform_source/basicplatform/drivers/dma-buf/mm
obj-y					+= mm_heap_helpers.o
obj-$(CONFIG_DMABUF_HEAPS_SYSTEM)	+= mm_system_heap.o
obj-$(CONFIG_DMABUF_HEAPS_SECSG)	+= mm_secsg_heap.o
obj-$(CONFIG_DMABUF_HEAPS_SECCG)	+= mm_seccg_heap.o mm_sec_contig.o
obj-$(CONFIG_DMABUF_HEAPS_CPA)		+= mm_cpa_heap.o
obj-$(CONFIG_DMABUF_HEAPS_CMA)		+= mm_cma_heap.o
obj-$(CONFIG_DMABUF_HEAPS_DMA_POOL)	+= mm_dma_pool_heap.o
obj-$(CONFIG_DMABUF_HEAPS_CARVEOUT)	+= mm_carveout_heap.o
