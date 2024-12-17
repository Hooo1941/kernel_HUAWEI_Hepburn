/*
 * Copyright(C) 2004-2020 Hisilicon Technologies Co., Ltd. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _MM_IOMMU_H
#define _MM_IOMMU_H

#include <linux/dma-buf.h>
#include <linux/types.h>
#include <linux/iommu.h>
#include <linux/platform_device.h>

/* Only for SMMUv2: mm_smmu_lpae */
#define IOMMU_DEVICE (1 << 7)
#define IOMMU_SEC    (1 << 8)
#define IOMMU_EXEC   (1 << 9)

/* SysCache Prot: Start from BIT(12), use 8 bits (bit 12 ~ 19) totally */
#ifdef CONFIG_MM_LB
#define IOMMU_PORT_SHIFT	(12)
#define IOMMU_PORT_MASK	(0xFF << IOMMU_PORT_SHIFT)
#endif

/* For DRM protect buf, used by io-pgtable-arm.c */
#ifdef CONFIG_MM_IO_PGTABLE_EBIT
#define IOMMU_PROTECT (1 << 20)
#endif

/* For io-pgtable outer-shareable flag */
#ifdef CONFIG_MM_LB_FULL_COHERENCY
#define IOMMU_OSH (1 << 21)
#endif

/* This bit is multiplexed with bit 31 of the IOMMU_PROT attribute. */
#define NO_SLEEP (1 << 31)

extern int of_get_iova_info(struct device_node *np, const char *name,
		unsigned long *iova_start, unsigned long *iova_size, unsigned long *iova_align);

#ifdef CONFIG_MM_IOMMU
void dmabuf_release_iommu(struct dma_buf *dmabuf);
phys_addr_t kernel_domain_get_ttbr(struct device *dev);
unsigned long mm_iommu_map_sg(struct device *dev, struct scatterlist *sgl,
				int prot, unsigned long *out_size);
int mm_iommu_unmap_sg(struct device *dev, struct scatterlist *sgl,
			unsigned long iova);
unsigned long mm_drm_iommu_map_dmabuf(struct device *dev, struct dma_buf *dmabuf,
				    int prot, unsigned long *out_size);
int mm_drm_iommu_unmap_dmabuf(struct device *dev, struct dma_buf *dmabuf,
			    unsigned long iova);
unsigned long kernel_iommu_map_dmabuf(struct device *dev, struct dma_buf *dmabuf,
				    int prot, unsigned long *out_size);
int kernel_iommu_unmap_dmabuf(struct device *dev, struct dma_buf *dmabuf,
			    unsigned long iova);
unsigned long mm_iommu_map(struct device *dev, phys_addr_t paddr,
			    size_t size, int prot);
int mm_iommu_unmap(struct device *dev, unsigned long iova,
			    size_t size);
unsigned long mm_iommu_idle_display_map(struct device *dev, u32 policy_id,
			struct dma_buf *dmabuf, size_t allsize,
			size_t l3size, size_t lbsize);
int mm_iommu_idle_display_unmap(struct device *dev, unsigned long iova,
			size_t size, u32 policy_id, struct dma_buf *dmabuf);
int mm_iommu_dev_flush_tlb(struct device *dev, unsigned int ssid);
size_t mm_iommu_unmap_fast(struct device *dev,
			unsigned long iova, size_t size);

#ifdef CONFIG_BG_DMABUF_RECLAIM
unsigned long kernel_iommu_map_bg_dmabuf(struct device *dev,
	struct dma_buf *dmabuf, struct sg_table *sgt,
	unsigned long iova, int prot, unsigned long *out_size);
int kernel_iommu_unmap_bg_dmabuf(struct device *dev,
	struct dma_buf *dmabuf, unsigned long iova);
#endif /* CONFIG_BG_DMABUF_RECLAIM */

#else
static inline void dmabuf_release_iommu(struct dma_buf *dmabuf) {}

static inline phys_addr_t kernel_domain_get_ttbr(struct device *dev)
{
	return 0;
}

static inline unsigned long mm_iommu_map_sg(struct device *dev,
					      struct scatterlist *sgl,
					      int prot,
					      unsigned long *out_size)
{
	return 0;
}

static inline int mm_iommu_unmap_sg(struct device *dev,
				      struct scatterlist *sgl,
				      unsigned long iova)
{
	return -EINVAL;
}

static inline unsigned long mm_drm_iommu_map_dmabuf(struct device *dev, struct dma_buf *dmabuf,
				    int prot, unsigned long *out_size)
{
	return 0;
}

static inline int mm_drm_iommu_unmap_dmabuf(struct device *dev, struct dma_buf *dmabuf,
			    unsigned long iova)
{
	return -EINVAL;
}

static inline unsigned long kernel_iommu_map_dmabuf(struct device *dev,
						  struct dma_buf *dmabuf,
						  int prot,
						  unsigned long *out_size)
{
	return 0;
}

static inline int kernel_iommu_unmap_dmabuf(struct device *dev,
					  struct dma_buf *dmabuf,
					  unsigned long iova)
{
	return -EINVAL;
}

static inline unsigned long mm_iommu_map(struct device *dev,
					   phys_addr_t paddr,
					   size_t size, int prot)
{
	return 0;
}

static inline int mm_iommu_unmap(struct device *dev, unsigned long iova,
				   size_t size)
{
	return -EINVAL;
}

static inline unsigned long mm_iommu_idle_display_map(struct device *dev,
					u32 policy_id,
					struct dma_buf *dmabuf,
					size_t allsize, size_t l3size,
					size_t lbsize)
{
	return 0;
}

static inline int mm_iommu_idle_display_unmap(struct device *dev,
					unsigned long iova, size_t size,
					u32 policy_id, struct dma_buf *dmabuf)
{
	return 0;
}

static inline int mm_iommu_dev_flush_tlb(struct device *dev,
				unsigned int ssid)
{
	return 0;
}

static inline size_t mm_iommu_unmap_fast(struct device *dev,
			     unsigned long iova, size_t size)
{
	return 0;
}

#ifdef CONFIG_BG_DMABUF_RECLAIM
static inline unsigned long kernel_iommu_map_bg_dmabuf(struct device *dev,
	struct dma_buf *dmabuf, struct sg_table *sgt,
	unsigned long iova, int prot, unsigned long *out_size)
{
	return 0;
}
static inline int kernel_iommu_unmap_bg_dmabuf(struct device *dev,
	struct dma_buf *dmabuf, unsigned long iova)
{
	return -EINVAL;
}
#endif /* CONFIG_BG_DMABUF_RECLAIM */

#endif /* CONFIG_MM_IOMMU */

#ifdef CONFIG_MM_IOMMU_LAST_PAGE
unsigned long kernel_iommu_map_padding_dmabuf(struct device *dev,
		struct dma_buf *dmabuf, unsigned long padding_len,
		int prot, unsigned long *out_size);
int kernel_iommu_unmap_padding_dmabuf(struct device *dev, struct dma_buf *dmabuf,
		unsigned long padding_len, unsigned long iova);
#else
static inline unsigned long kernel_iommu_map_padding_dmabuf(struct device *dev,
		struct dma_buf *dmabuf, unsigned long padding_len,
		int prot, unsigned long *out_size)
{
	return 0;
}

static inline int kernel_iommu_unmap_padding_dmabuf(struct device *dev,
	struct dma_buf *dmabuf, unsigned long padding_len, unsigned long iova)
{
	return 0;
}
#endif /* CONFIG_MM_IOMMU_LAST_PAGE */

#ifdef CONFIG_MM_IOMMU_TEST
void mm_smmu_show_pte(struct device *dev, unsigned long iova,
			unsigned long size);
void mm_print_iova_dom(struct device *dev);
#else
static inline void mm_smmu_show_pte(struct device *dev __attribute__((__unused__)),
			unsigned long iova __attribute__((__unused__)),
			unsigned long size __attribute__((__unused__)))
{
}

static inline void mm_print_iova_dom(struct device *dev __attribute__((__unused__)))
{
}
#endif

#endif
