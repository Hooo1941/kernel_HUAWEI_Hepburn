/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <securec.h>
#include "ion.h"
#include "dkmd_log.h"
#include "virtio_iommu.h"
#include "virtio_fb.h"

struct buffer_info {
	struct list_head node;
	uint64_t vfb_buf_key;
	uint64_t refcnt;
	struct dma_buf *buf;
	bool decrease_flag;
};

struct map_entry {
	uint64_t pa;
	uint64_t size;
};

enum smmu_option_type {
	SMMU_MAP,
	SMMU_UNMAP
};

#define ENTRY_NUMS_PAGE		(PAGE_SIZE / sizeof(struct map_entry))
/* The buffer original display refcnt, when decrease to 0, unmap the buffer */
#define BUFFER_REFCNT				8
#define MAX_ENTRY				512

static LIST_HEAD(buf_map_list);
static DEFINE_SPINLOCK(map_list_lock);

void dpu_vfb_put_dmabuf(struct dma_buf *buf)
{
	if (IS_ERR_OR_NULL(buf)) {
		dpu_pr_err("Invalid dmabuf\n");
		return;
	}

	dma_buf_put(buf);
}

static size_t sg_size_get(struct scatterlist *sgl, int nents)
{
	int i;
	size_t iova_size = 0;
	struct scatterlist *sg = NULL;

	for_each_sg(sgl, sg, nents, i)
		iova_size += (size_t)ALIGN(sg->length, PAGE_SIZE);

	return iova_size;
}

static int map_sg_nents(int sg_nents)
{
	int left_nents = 0;
	int page_nents = 0;

	left_nents = sg_nents % ENTRY_NUMS_PAGE;
	page_nents = sg_nents / ENTRY_NUMS_PAGE;

	return ((left_nents > 0) ? page_nents + 1 : page_nents);
}

static struct sg_table *dup_sg_table(struct sg_table *table)
{
	int ret;
	unsigned int i;
	struct sg_table *new_table = NULL;
	struct scatterlist *new_sg = NULL;
	struct scatterlist *sg = NULL;

	new_table = kzalloc(sizeof(*new_table), GFP_KERNEL);
	if (!new_table)
		return NULL;

	ret = sg_alloc_table(new_table, table->nents, GFP_KERNEL);
	if (ret) {
		kfree(new_table);
		new_table = NULL;
		return NULL;
	}

	new_sg = new_table->sgl;
	for_each_sg(table->sgl, sg, table->nents, i) {
		memcpy_s(new_sg, sizeof(struct scatterlist), sg, sizeof(struct scatterlist));
		new_sg = sg_next(new_sg);
	}

	return new_table;
}

static void init_header_info(struct iommu_head_info *h_info, uint64_t buf_key,
						int ops_code, int sg_nents, size_t map_size)
{
	h_info->vfb_buf_key = buf_key;
	h_info->ops_code = ops_code;
	h_info->sg_nents = sg_nents;
	h_info->map_size = map_size;
	h_info->err_code = 0;
}

static struct sg_table *create_sgtable(struct iommu_head_info *h_info)
{
	int ret;
	int nents = 1;
	struct sg_table *table = NULL;

	nents += map_sg_nents(h_info->sg_nents);
	table = kmalloc(sizeof(*table), GFP_KERNEL);
	if (table == NULL) {
		dpu_pr_err("%s, sgtable alloc fail\n", __func__);
		return NULL;
	}
	ret = sg_alloc_table(table, (uint32_t)nents, GFP_KERNEL);
	if (ret) {
		dpu_pr_err("%s, sglist alloc fail, nents = %d\n", __func__, nents);
		goto free_table;
	}

	return table;

free_table:
	kfree(table);
	table = NULL;

	return NULL;
}

static void iommu_header_info_add_sglist(struct scatterlist *sgl,
				struct iommu_head_info *head_info)
{
	struct page *page = NULL;
	unsigned int offset = 0;

	page = vmalloc_to_page(head_info);
	offset = ((uintptr_t)head_info) & (~PAGE_MASK);
	sg_set_page(sgl, page, sizeof(*head_info), offset);
}

struct dma_buf *dpu_vfb_get_dmabuf(int sharefd)
{
	struct dma_buf *buf = NULL;

	/* dim layer share fd -1 */
	if (sharefd < 0) {
		dpu_pr_err("Invalid file sharefd = %d\n", sharefd);
		return NULL;
	}

	buf = dma_buf_get(sharefd);
	if (IS_ERR_OR_NULL(buf)) {
		dpu_pr_err("Invalid file buf, sharefd = %d\n", sharefd);
		return NULL;
	}

	return buf;
}

static void sglist_to_map_entry(struct scatterlist *sg, int nents,
				struct map_entry *m_entry, int entry_nums)
{
	int i;
	struct scatterlist *s = NULL;

	for_each_sg(sg, s, nents, i) {
		if (i >= entry_nums)
			break;
		m_entry[i].pa = sg_phys(s);
		m_entry[i].size = s->length;
	}
}

static int map_entry_to_sglist(struct map_entry *m_entry, int entry_nums,
				struct scatterlist *sgl, int nents)
{
	int i;
	int nums = entry_nums;
	struct map_entry *entry = m_entry;
	struct page *page = NULL;
	struct scatterlist *s = sgl;

	for (i = 0; i < nents; i++) {
		unsigned long len;

		if (nums >= ENTRY_NUMS_PAGE) {
			len = PAGE_SIZE;
			nums -= ENTRY_NUMS_PAGE;
		} else {
			len = (unsigned long)nums * sizeof(struct map_entry);
		}

		page = vmalloc_to_page(entry);
		if (!page) {
			dpu_pr_err("%s, vmalloc_to_page err\n", __func__);
			return -EINVAL;
		}
		sg_set_page(s, page, (uint32_t)len, 0);
		s = sg_next(s);

		entry += ENTRY_NUMS_PAGE;
	}

	return 0;
}

static int iommu_virtqueue_add_inbuf(struct iommu_head_info *h_info, struct scatterlist *sgl)
{
	int ret;
	struct sg_table *table = NULL;
	struct map_entry *m_entry = NULL;

	table = create_sgtable(h_info);
	if (table == NULL) {
		dpu_pr_err("%s, creat table fail\n", __func__);
		return -ENOMEM;
	}

	iommu_header_info_add_sglist(table->sgl, h_info);
	/* For SMMU_MAP_SYNC, sglist data should be transmitted by virtio */
	if (h_info->ops_code == SMMU_MAP) {
		int entry_nums = h_info->sg_nents;
		if ((entry_nums <= 0) || (entry_nums > MAX_ENTRY * ENTRY_NUMS_PAGE)) {
			dpu_pr_err("entry_nums's value cannot be zero!\n");
			return -ENOMEM;
		}

		m_entry = vmalloc((unsigned long)entry_nums *
						sizeof(struct map_entry));
		if (!m_entry) {
			dpu_pr_err("%s, map entry alloc fail, numbers = 0x%lx\n",
						__func__, entry_nums);
			ret = -ENOMEM;
			goto free_sglist;
		}
		/*
		 * copy map sglist info to new table->sgl,
		 * table->sgl[0] save header info,
		 * map_entry save sglist addr and size info,
		 * copy map_entry to table->sgl start from table->sgl[1],
		 * virtio send table->sgl to uvmm.
		 */
		sglist_to_map_entry(sgl, h_info->sg_nents,
				m_entry, entry_nums);
		ret = map_entry_to_sglist(m_entry, entry_nums,
				sg_next(table->sgl), (int32_t)table->nents - 1);
		if (ret) {
			dpu_pr_err("%s, map_entry to sglist fail, ret[%d]\n",
								__func__, ret);
			ret = -EFAULT;
			goto free_buffer;
		}
	}
	ret = virtio_fb_send_sgl_info(table, h_info);
	if (ret < 0) {
		dpu_pr_err("vfb send sgl info failed, ret %d\n", ret);
		ret = -EFAULT;
		goto free_buffer;
	}

free_buffer:
	if (h_info->ops_code == SMMU_MAP)
		vfree(m_entry);
free_sglist:
	sg_free_table(table);
	kfree(table);
	table = NULL;
	return ret;
}

static int vfb_iommu_map_dmabuf(struct dma_buf *dmabuf, uint64_t vfb_buf_key)
{
	int ret;
	int code_flag = -EINVAL;
	struct sg_table *dmabuf_sgt = NULL;
	struct sg_table *sgt = NULL;
	struct iommu_head_info *h_info = NULL;

	if (IS_ERR_OR_NULL(dmabuf) || IS_ERR_OR_NULL(dmabuf->priv)) {
		dpu_pr_err("%s, dmabuf or dmabuf->priv is NULL!\n", __func__);
		return -EINVAL;
	}

	dmabuf_sgt = ((struct ion_buffer *)dmabuf->priv)->sg_table;
	if (!dmabuf_sgt) {
		dpu_pr_err("%s, dmabuf_sgt is NULL!\n", __func__);
		return -EINVAL;
	}

	sgt = dup_sg_table(dmabuf_sgt);
	if (!sgt) {
		dpu_pr_err("%s, dup_sg_table fail, nents = %u\n",
					__func__, dmabuf_sgt->nents);
		return -EINVAL;
	}

	h_info = vzalloc(sizeof(*h_info));
	if (!h_info) {
		dpu_pr_err("%s, header info alloc fail!\n", __func__);
		goto free_sgt;
	}

	init_header_info(h_info, vfb_buf_key, SMMU_MAP, (int32_t)sgt->nents, sg_size_get(sgt->sgl, (int32_t)sgt->nents));
	dpu_pr_debug("buffer %llu sg_nents is %u, sg_size is %llu\n",
		vfb_buf_key, sgt->nents, sg_size_get(sgt->sgl, (int32_t)sgt->nents));
	ret = iommu_virtqueue_add_inbuf(h_info, sgt->sgl);
	if ((ret != 0) || (h_info->err_code != 0)) {
		dpu_pr_err("%s, map fail, ret[%d], err_code[%d]\n",
				__func__, ret, h_info->err_code);
		goto free_hinfo;
	}
	code_flag = h_info->err_code;

free_hinfo:
	vfree(h_info);
free_sgt:
	sg_free_table(sgt);
	kfree(sgt);
	sgt = NULL;
	return code_flag;
}

static int vfb_iommu_unmap_dmabuf(uint64_t buf_key)
{
	int ret = 0;
	struct iommu_head_info *h_info = NULL;

	h_info = kzalloc(sizeof(*h_info), GFP_ATOMIC);
	if (!h_info) {
		dpu_pr_err("%s, header info alloc fail!\n", __func__);
		return -ENOMEM;;
	}

	init_header_info(h_info, buf_key, SMMU_UNMAP, 0, 0);
	ret = iommu_virtqueue_add_inbuf(h_info, NULL);
	if ((ret != 0) || (h_info->err_code != 0))
		dpu_pr_err("%s, unmap fail, ret[%d], err_code[%d]\n", __func__, ret, h_info->err_code);

	kfree(h_info);
	return ret;
}

static bool is_buffer_mapped(uint64_t buf_key)
{
	unsigned long flags;
	bool is_mapped = false;
	struct buffer_info *pos = NULL;

	spin_lock_irqsave(&map_list_lock, flags);
	list_for_each_entry(pos, &buf_map_list, node) {
		if (pos->vfb_buf_key == buf_key) {
			pos->refcnt = BUFFER_REFCNT; // if buffer still in use, reset the refcnt
			is_mapped = true;
			break;
		}
	}
	spin_unlock_irqrestore(&map_list_lock, flags);

	return is_mapped;
}

void decrease_buf_refcnt(uint64_t buf_key)
{
	unsigned long flags;
	struct buffer_info *pos = NULL;
	struct buffer_info *n = NULL;

	spin_lock_irqsave(&map_list_lock, flags);
	list_for_each_entry_safe(pos, n, &buf_map_list, node) {
		if (pos->vfb_buf_key == buf_key)
			pos->decrease_flag = true;

		if (pos->decrease_flag) {
			pos->refcnt--;
			if (pos->refcnt == 0) {
				dpu_pr_info("buffer_key iova unmap ocur");
				dma_buf_put(pos->buf); // if don't use the bufer, put the dma_buf
				vfb_iommu_unmap_dmabuf(pos->vfb_buf_key);
				list_del(&pos->node);
				vfree(pos);
			}
		}
	}
	spin_unlock_irqrestore(&map_list_lock, flags);
}

void set_buf_decrease_flag(uint64_t buf_key)
{
	unsigned long flags;
	struct buffer_info *pos = NULL;
	struct buffer_info *n = NULL;

	spin_lock_irqsave(&map_list_lock, flags);
	list_for_each_entry_safe(pos, n, &buf_map_list, node) {
		if (pos->vfb_buf_key == buf_key)
			pos->decrease_flag = false;
	}
	spin_unlock_irqrestore(&map_list_lock, flags);
}

int dpu_vfb_map_iova(struct dma_buf *dmabuf, int32_t fd)
{
	unsigned long flags;
	uint64_t vfb_buf_key;
	struct buffer_info *info = NULL;
	struct dma_buf *repeat_get_dma_buf = NULL;

	if (IS_ERR_OR_NULL(dmabuf)) {
		dpu_pr_err("dmabuf is err or null\n");
		return -1;
	}

	vfb_buf_key = (uint64_t)(void *)dmabuf;
	if (is_buffer_mapped(vfb_buf_key)) {
		dpu_pr_info("buf has been mapped\n");
		return 0;
	}

	/*
	 * When buf_key is mapped to iova, get dma_buf one more time.
	 * When sf restarts, the same buf_key will not be used again to ensure the uniqueness of buf_key.
	 * When RDA continues to access the buf_key before sf restarts, the content still exists.
	 */
	repeat_get_dma_buf = dma_buf_get(fd);
	if (IS_ERR_OR_NULL(repeat_get_dma_buf)) {
		dpu_pr_err("share_fd %d get dmabuf failed", fd);
		return -1;
	}
	dpu_pr_info("repeat get bufkey");

	info = vzalloc(sizeof(*info));
	if (!info) {
		dma_buf_put(repeat_get_dma_buf);
		dpu_pr_err("alloc for buffer info failed\n");
		return -1;
	}

	if (vfb_iommu_map_dmabuf(dmabuf, vfb_buf_key) != 0) {
		vfree(info);
		dma_buf_put(repeat_get_dma_buf);
		dpu_pr_err("map iova fail\n");
		return -1;
	}

	info->buf = repeat_get_dma_buf;
	info->vfb_buf_key = vfb_buf_key;
	info->refcnt = BUFFER_REFCNT;
	spin_lock_irqsave(&map_list_lock, flags);
	list_add_tail(&info->node, &buf_map_list);
	spin_unlock_irqrestore(&map_list_lock, flags);
	return 0;
}
