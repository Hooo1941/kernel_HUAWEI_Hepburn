/*
 * mm_dma_heap.h
 *
 * Copyright(C) 2022 Hisilicon Technologies Co., Ltd. All rights reserved.
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

#ifndef _LINUX_MM_DMA_HEAP_H
#define _LINUX_MM_DMA_HEAP_H

#include <linux/dma-buf.h>
#include <linux/dma-heap.h>
#include <linux/sizes.h>
#include <uapi/linux/dma_heap_ext.h>

#define dma_buf_set_cache(__cache)	((__cache) | DMA_HEAP_FLAG_CACHED)
#define dma_buf_set_uncache(__cache)	((__cache) & ~DMA_HEAP_FLAG_CACHED)

#define dma_buf_is_cached(__flags)	((__flags) & DMA_HEAP_FLAG_CACHED)
#define dma_buf_is_uncached(__flags)	(!((__flags) & DMA_HEAP_FLAG_CACHED))
#define dma_buf_is_secure(__flags)	((__flags) & DMA_HEAP_FLAG_SECURE_BUFFER)
#define dma_buf_is_unsecure(__flags)	(!((__flags) & DMA_HEAP_FLAG_SECURE_BUFFER))

#define MM_DMA_HEAP_NAME_LEN	32

#ifdef CONFIG_MM_LB
#include <linux/mm_lb/mm_lb.h>
enum ion_lb_pid_ids {
	INVALID_PID = -1,
	ION_LB_BYPASS = PID_BY_PASS,
	ION_LB_CAMAIP = PID_CAMAIP,
	ION_LB_GPUFBO = PID_GPUFBO,
	ION_LB_GPUTXT = PID_GPUTXT,
	ION_LB_IDISPLAY = PID_IDISPLAY,
	ION_LB_NPU = PID_NPU,
	ION_LB_VIDEO = PID_VIDEO,
	ION_LB_CAMTOF = PID_CAMTOF,
	ION_LB_TINY = PID_TINY,
	ION_LB_AUDIO = PID_AUDIO,
	ION_LB_VOICE = PID_VOICE,
	ION_LB_ISPNN = PID_ISPNN,
	ION_LB_HIFI = PID_HIFI,
	ION_LB_ISPPIPE = PID_ISPPIPE,
	ION_LB_VENC = PID_VENC,

	ION_LB_MAX = PID_MAX,
};
#endif
/* user command add for additional use */
enum DMA_HEAP_MM_CUSTOM_CMD {
	DMA_HEAP_MM_CLEAN_CACHES,
	DMA_HEAP_MM_INV_CACHES,
	DMA_HEAP_MM_CLEAN_INV_CACHES,
};

enum dma_heap_ta_tag {
	DMA_HEAP_SEC_CMD_PGATBLE_INIT = 0,
	DMA_HEAP_SEC_CMD_ALLOC,
	DMA_HEAP_SEC_CMD_FREE,
	DMA_HEAP_SEC_CMD_MAP_IOMMU,
	DMA_HEAP_SEC_CMD_UNMAP_IOMMU,
	DMA_HEAP_SEC_CMD_MAP_USER,
	DMA_HEAP_SEC_CMD_UNMAP_USER,
	DMA_HEAP_SEC_CMD_TABLE_SET,
	DMA_HEAP_SEC_CMD_TABLE_CLEAN,
	DMA_HEAP_SEC_CMD_VLTMM,
#ifdef CONFIG_MM_SECMEM_TEST
	DMA_HEAP_SEC_CMD_TEST,
#endif
	DMA_HEAP_SEC_CMD_MAX,
};

enum SEC_SVC {
	SEC_TUI = 0,
	SEC_EID = 1,
	SEC_TINY = 2,
	SEC_FACE_ID = 3,
	SEC_FACE_ID_3D = 4,
	SEC_DRM_TEE = 5,
	SEC_AI_ION = 6,
	ISPNN_NORMAL = 7,
	SEC_SVC_MAX,
};

#ifdef CONFIG_DMABUF_MM
struct mm_heap_helper_buffer;

unsigned long mm_dmaheap_total(void);
int mm_dmaheap_memory_info(bool verbose);
int mm_dmaheap_proecss_info(void);
void mm_dmaheap_process_summary_info(void);
struct dma_buf *file_to_dma_buf(struct file *file);
int dma_heap_alloc(const char *name, size_t len, unsigned int heap_flags);
void dma_heap_free(int fd);
int mm_dma_heap_sglist_zero(struct scatterlist *sgl, unsigned int nents,
				pgprot_t pgprot);
int mm_dma_heap_buffer_zero(struct mm_heap_helper_buffer *buffer);
int mm_dma_heap_pages_zero(struct page *page, size_t size, pgprot_t pgprot);
struct cma *get_sec_cma(void);
struct sg_table *mm_secmem_alloc(int id, unsigned long size);
void mm_secmem_free(int id, struct sg_table *table);
int mm_dma_heap_secmem_get_phys(struct dma_buf *dmabuf,
			phys_addr_t *addr, size_t *len);
int mm_dma_heap_secmem_get_buffer(int fd,
			struct sg_table **table, u64 *id, enum SEC_SVC *type);
int mm_dma_heap_cache_operate(int fd, unsigned long uaddr,
			unsigned long offset, unsigned long length, unsigned int cmd);
bool is_dma_heap_dmabuf(struct dma_buf *dmabuf);
struct device *dmabuf_heap_device_get(struct dma_buf *dmabuf);
#else
static inline unsigned long mm_dmaheap_total(void)
{
	return -EINVAL;
}

static inline int mm_dmaheap_memory_info(bool verbose)
{
	return -EINVAL;
}

static inline int mm_dmaheap_proecss_info(void)
{
	return -EINVAL;
}

static inline void mm_dmaheap_process_summary_info(void)
{
}

static inline struct dma_buf *file_to_dma_buf(struct file *file)
{
	return NULL;
}

static inline int dma_heap_alloc(const char *name, size_t len, unsigned int heap_flags)
{
	return -EINVAL;
}

static inline void dma_heap_free(int fd)
{
}

static inline int mm_dma_heap_sglist_zero(struct scatterlist *sgl,
				unsigned int nents, pgprot_t pgprot)
{
	return -EINVAL;
}

static inline int mm_dma_heap_buffer_zero(struct mm_heap_helper_buffer *buffer)
{
	return -EINVAL;
}

static inline int mm_dma_heap_pages_zero(struct page *page, size_t size, pgprot_t pgprot)
{
	return -EINVAL;
}

static inline struct cma *get_sec_cma(void)
{
	return NULL;
}

static inline struct sg_table *mm_secmem_alloc(int id, unsigned long size)
{
	return NULL;
}

static inline void mm_secmem_free(int id, struct sg_table *table)
{
}

static inline int mm_dma_heap_secmem_get_phys(int fd, struct sg_table **table,
			u64 *id, enum SEC_SVC *type)
{
	return -EINVAL;
}

static inline int mm_dma_heap_secmem_get_buffer(int fd,
			struct sg_table **table, u64 *id, enum SEC_SVC *type)
{
	return -EINVAL;
}

static inline int mm_dma_heap_cache_operate(int fd, unsigned long uaddr,
			unsigned long offset, unsigned long length, unsigned int cmd);
{
	return -EINVAL;
}

static inline bool is_dma_heap_dmabuf(struct dma_buf *dmabuf)
{
	return false;
}

static inline struct device *dmabuf_heap_device_get(struct dma_buf *dmabuf)
{
	return NULL;
}
#endif /* CONFIG_DMABUF_MM */

#ifdef CONFIG_MEMORY_VLTMM
int vltmm_agent_register(void);
#endif /* CONFIG_MEMORY_VLTMM */

#endif
