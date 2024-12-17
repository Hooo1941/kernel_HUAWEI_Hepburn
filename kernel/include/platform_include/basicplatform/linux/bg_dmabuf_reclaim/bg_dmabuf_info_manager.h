
/*
 * bg_dmabuf_info_manager.h
 *
 * Copyright (C) 2022 Hisilicon Technologies Co., Ltd.
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
#ifndef _BG_DMABUF_INFO_MANAGER_H
#define _BG_DMABUF_INFO_MANAGER_H

#include <linux/dma-buf.h>
#include <linux/mutex.h>
#include <linux/rwsem.h>
#include <linux/types.h>

enum bg_dmabuf_operation {
	BG_BUFFER_OPERATION_NONE,
	BG_BUFFER_RECLAIM_BUFFER,
	BG_BUFFER_REALLOC_BUFFER,
};

enum bg_dmabuf_map_state {
	BG_BUFFER_NONE,
	BG_BUFFER_MAP,
	BG_BUFFER_UNMAP,
};

enum bg_dmabuf_pin_state {
	BG_BUFFER_INIT,
	BG_BUFFER_RECLAIM_PINNED,
	BG_BUFFER_IN_RECLAIM,
	BG_BUFFER_OUT_RECLAIM,
};

/**
 * struct bg_dmabuf_iova_info - bg_reclaim iommu map va record struct
 *
 * @list: Used by the info_manager to keep a list of record iova
 * @state: this va info has been mapped or not
 * @dev: struct device for iommu map handling
 * @iova: VA by iommu map
 * @length: Length corresponding to IOVA
 * @prot: Map-Prot corresponding to IOVA
 */
struct bg_dmabuf_iova_info {
	struct list_head list;
	enum bg_dmabuf_map_state state;
	struct device *dev;
	unsigned long iova;
	unsigned long length;
	int prot;
};

/**
 * struct bg_dmabuf_uva_info - bg_reclaim ACPU maped User-VA record struct
 *
 * @list: Used by the info_manager to keep a list of record uva
 * @state: this va info has been mapped or not
 * @task_name: Task's name of maped uva
 * @vma: VMA corresponding to uva and task
 * @mm: MM struct corresponding to uva and task
 */
#define BG_MAX_TASK_NAME_LEN	20
struct bg_dmabuf_uva_info {
	struct list_head list;
	enum bg_dmabuf_map_state state;
	/* just for mntn */
	char task_name[BG_MAX_TASK_NAME_LEN];
	struct vm_area_struct *vma;
	struct mm_struct *mm;
};

/**
 * struct bg_dmabuf_kva_info - bg_reclaim ACPU maped Kernel-VA record struct
 *
 * @list: Used by the info_manager to keep a list of record kva
 * @state: this va info has been mapped or not
 * @vaddr: VA by ACPU Kernel-driver map
 */
struct bg_dmabuf_kva_info {
	struct list_head list;
	enum bg_dmabuf_map_state state;
	void *vaddr;
};

/**
 * struct bg_dmabuf_info - bg_reclaim dmabuf info struct
 * is_release\is_missing_info\attach_num\uva_info\kva_info protect by buffer-lock
 * iova_info or other func protected by rwsem
 *
 * @is_release: buffer has reclaimed memory or not
 * @is_missing_info: info has missed some va-record or not
 * @attach_num: numbers of device attach
 * @pin_state: add pin_state for vdec to prevent TOCTOU, bg_buffer is pinned means cannot be reclaimed.
 * @rwsem: sync exec sequence of processes that can't hold lock, such as is_release, iova
 * @iova_info: List of record iova
 * @uva_info: List of record uva
 * @kva_info: List of record kva
 * @iova_info_mutex: Mutex of iova_info list
 * @uva_info_mutex: Mutex of uva_info list
 * @kva_info_mutex: Mutex of kva_info list
 * @unmap_lock: Mutex of uva munmap free pagetable
 */
struct bg_dmabuf_info {
	atomic_t is_release;
	atomic_t is_missing_info;
	atomic_t attach_num;
	atomic_t pin_state;
	struct rw_semaphore rwsem;
	struct list_head iova_info;
	struct list_head uva_info;
	struct list_head kva_info;
	struct mutex iova_info_mutex;
	struct mutex uva_info_mutex;
	struct mutex kva_info_mutex;
	struct mutex unmap_lock;
};

#ifdef CONFIG_BG_DMABUF_RECLAIM
bool is_info_missing_va_record(struct bg_dmabuf_info *info);
struct bg_dmabuf_info *get_info_from_bg_buffer(void *buffer);
int alloc_bg_buffer_info(void *buffer);
void free_bg_buffer_info(void *buffer);

int record_bg_buffer_uva_info(void *buffer, struct vm_area_struct *vma);
void delete_bg_buffer_uva_info(void *buffer, struct vm_area_struct *vma);

int record_bg_buffer_kva_info(void *buffer, void *vaddr);
void delete_bg_buffer_kva_info(void *buffer, void *vaddr);

int record_bg_buffer_iova_info(struct dma_buf *dmabuf, struct device *dev,
	unsigned long iova, unsigned long length, int prot);
void delete_bg_buffer_iova_info(struct dma_buf *dmabuf,
	struct device *dev, unsigned long iova, bool is_delete_all);
bool is_bg_buffer_release(void *buffer);
int bg_dmabuf_attach(void *buffer);
void bg_dmabuf_detach(void *buffer);
void dump_bg_buffer_info(struct dma_buf *dmabuf);
int bg_buffer_vma_unmap_lock(struct vm_area_struct *vma, void **dma_buf);
void bg_buffer_vma_unmap_unlock(void *dma_buf);

extern const struct vm_operations_struct bg_vm_ops;
static inline bool vma_is_bg_buffer_vma(struct vm_area_struct *vma)
{
	if (vma->vm_ops == &bg_vm_ops)
		return true;
	return false;
}
#else
static inline bool is_info_missing_va_record(struct bg_dmabuf_info *info)
{
	return true;
}
static inline struct bg_dmabuf_info *get_info_from_bg_buffer(void *buffer)
{
	return NULL;
}
static inline int alloc_bg_buffer_info(void *buffer)
{
	return 0;
}
static inline void free_bg_buffer_info(void *buffer)
{
}
static inline int record_bg_buffer_uva_info(void *buffer,
	struct vm_area_struct *vma)
{
	return 0;
}
static inline void delete_bg_buffer_uva_info(void *buffer,
	struct vm_area_struct *vma)
{
}
static inline int record_bg_buffer_kva_info(void *buffer, void *vaddr)
{
	return 0;
}
static inline void delete_bg_buffer_kva_info(void *buffer, void *vaddr)
{
}
static inline int record_bg_buffer_iova_info(struct dma_buf *dmabuf,
	struct device *dev, unsigned long iova, unsigned long length, int prot)
{
	return 0;
}
static inline void delete_bg_buffer_iova_info(struct dma_buf *dmabuf,
	struct device *dev, unsigned long iova, bool is_delete_all)
{
}
static inline bool is_bg_buffer_release(void *buffer)
{
	return false;
}
static inline int bg_dmabuf_attach(void *buffer)
{
	return 0;
}
static inline void bg_dmabuf_detach(void *buffer)
{
}
static inline void dump_bg_buffer_info(struct dma_buf *dmabuf)
{
}
static inline bool vma_is_bg_buffer_vma(struct vm_area_struct *vma)
{
	return false;
}
static inline int bg_buffer_vma_unmap_lock(struct vm_area_struct *vma, void **dma_buf)
{
	return 0;
}
static inline void bg_buffer_vma_unmap_unlock(void *dma_buf)
{
}
#endif /* CONFIG_BG_DMABUF_RECLAIM */

#endif /* _BG_DMABUF_INFO_MANAGER_H */
