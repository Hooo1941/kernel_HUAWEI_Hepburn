/*
 * bg_dmabuf_info_manager.c
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
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/sched/mm.h>
#include <linux/mm_types.h>
#include <linux/bg_dmabuf_reclaim/bg_dmabuf_info_manager.h>
#include <securec.h>

#include "bg_internal_buf_interface.h"

static void __dump_uva_info(struct bg_dmabuf_info *info)
{
	struct list_head *pos = NULL;
	struct list_head *q = NULL;

	mutex_lock(&info->uva_info_mutex);
	list_for_each_safe(pos, q, &info->uva_info) {
		struct bg_dmabuf_uva_info *uva_info = list_entry(pos, struct bg_dmabuf_uva_info, list);
		bg_buf_pr_info("%s task-%s stat-%d mm-0x%pK addr-0x%llx len-0x%llx\n", __func__,
			uva_info->task_name, uva_info->state, uva_info->mm, uva_info->vma->vm_start,
			uva_info->vma->vm_end - uva_info->vma->vm_start);
		bg_buf_pr_debug("%s vma-0x%pK vm_mm-0x%pK prev-0x%pK offset-0x%llx end-0x%llx\n",
			__func__, uva_info->vma, uva_info->vma->vm_mm, uva_info->vma->vm_prev,
			uva_info->vma->vm_pgoff * PAGE_SIZE,
			uva_info->vma->vm_end);
	}
	mutex_unlock(&info->uva_info_mutex);
}

static void __dump_kva_info(struct bg_dmabuf_info *info)
{
	struct list_head *pos = NULL;
	struct list_head *q = NULL;

	mutex_lock(&info->kva_info_mutex);
	list_for_each_safe(pos, q, &info->kva_info) {
		struct bg_dmabuf_kva_info *kva_info = list_entry(pos, struct bg_dmabuf_kva_info, list);
		bg_buf_pr_info("%s stat-%d addr-%pK\n", __func__, kva_info->state, kva_info->vaddr);
	}
	mutex_unlock(&info->kva_info_mutex);
}

static void __dump_iova_info(struct bg_dmabuf_info *info)
{
	struct list_head *pos = NULL;
	struct list_head *q = NULL;

	mutex_lock(&info->iova_info_mutex);
	list_for_each_safe(pos, q, &info->iova_info) {
		struct bg_dmabuf_iova_info *iova_info = list_entry(pos, struct bg_dmabuf_iova_info, list);
		bg_buf_pr_info("%s dev-%s state-%d iova-0x%llx length-0x%llx prot-0x%x\n",
			__func__, dev_name(iova_info->dev), iova_info->state,
			iova_info->iova, iova_info->length, iova_info->prot);
	}
	mutex_unlock(&info->iova_info_mutex);
}

bool is_info_missing_va_record(struct bg_dmabuf_info *info)
{
	return (atomic_read(&info->is_missing_info) > 0);
}

static inline void set_info_missing_va_record(struct bg_dmabuf_info *info)
{
	atomic_set(&info->is_missing_info, 1);
	bg_buf_pr_info("%s info-0x%pK missing va record\n", __func__, info);
}

struct bg_dmabuf_info *get_info_from_bg_buffer(void *buffer)
{
	if (unlikely(!buffer)) {
		bg_buf_pr_debug("%s buffer is NULL\n", __func__);
		return NULL;
	}

	/* check dmabuf is ok to reclaim (expect system_heap\flag not fit) */
	if (!is_buffer_valid(buffer))
		return NULL;

	return get_info_from_buffer(buffer);
}

static struct bg_dmabuf_info *__get_no_missing_info(void *buffer)
{
	struct bg_dmabuf_info *info = NULL;

	info = get_info_from_bg_buffer(buffer);
	if (info && (is_info_missing_va_record(info) == false))
		return info;
	else
		return NULL;
}

static inline void __init_va_info(struct list_head *list, struct mutex *list_mutex)
{
	INIT_LIST_HEAD(list);
	mutex_init(list_mutex);
}

#define __deinit_va_info(pos, q, info, node, type, lock) ({ \
	mutex_lock(lock); \
	list_for_each_safe(pos, q, info) { \
		node = list_entry(pos, type, list); \
		list_del(pos); \
		kfree(node); \
	} \
	mutex_unlock(lock); \
})

int alloc_bg_buffer_info(void *buffer)
{
	struct bg_dmabuf_info *info = NULL;

	if (!buffer || !is_buffer_valid(buffer))
		return -EINVAL;

	info = kzalloc(sizeof(*info), GFP_KERNEL);
	if (!info) {
		bg_buf_pr_err("%s info alloc failed\n", __func__);
		return -ENOMEM;
	}

	atomic_set(&info->is_release, 0);
	atomic_set(&info->is_missing_info, 0);
	atomic_set(&info->attach_num, 0);
	atomic_set(&info->pin_state, BG_BUFFER_INIT);
	init_rwsem(&info->rwsem);

	__init_va_info(&info->iova_info, &info->iova_info_mutex);
	__init_va_info(&info->uva_info, &info->uva_info_mutex);
	__init_va_info(&info->kva_info, &info->kva_info_mutex);
	mutex_init(&info->unmap_lock);

	set_info_to_buffer(buffer, info);
	bg_buf_pr_debug("%s alloc ok, buffer-0x%pK\n", __func__, buffer);

	return 0;
}

static void __bg_buffer_mmdrop(struct bg_dmabuf_info *info)
{
	struct list_head *pos = NULL;
	struct list_head *q = NULL;

	mutex_lock(&info->uva_info_mutex);
	list_for_each_safe(pos, q, &info->uva_info) {
		struct bg_dmabuf_uva_info *uva_in = list_entry(pos, struct bg_dmabuf_uva_info, list);
		mmdrop(uva_in->mm);
		bg_buf_pr_info("%s drop mm-0x%pK vma-0x%pK addr-0x%llx\n",
			__func__, uva_in->mm, uva_in->vma, uva_in->vma->vm_start);
	}
	mutex_unlock(&info->uva_info_mutex);
}

void free_bg_buffer_info(void *buffer)
{
	struct list_head *pos = NULL;
	struct list_head *q = NULL;
	struct bg_dmabuf_kva_info *kva_info = NULL;
	struct bg_dmabuf_uva_info *uva_info = NULL;
	struct bg_dmabuf_iova_info *iova_info = NULL;
	struct bg_dmabuf_info *info = NULL;

	info = get_info_from_bg_buffer(buffer);
	if (!info)
		return;

	set_info_to_buffer(buffer, NULL);

	__deinit_va_info(pos, q, &info->iova_info,
		iova_info, struct bg_dmabuf_iova_info, &info->iova_info_mutex);

	/* uva record need mmdrop all mm struct */
	__bg_buffer_mmdrop(info);
	__deinit_va_info(pos, q, &info->uva_info,
		uva_info, struct bg_dmabuf_uva_info, &info->uva_info_mutex);

	__deinit_va_info(pos, q, &info->kva_info,
		kva_info, struct bg_dmabuf_kva_info, &info->kva_info_mutex);

	kfree(info);
	bg_buf_pr_debug("%s info free ok, buffer-0x%pK\n", __func__, buffer);
}

enum va_ops_type {
	__ADD_VA,
	__DELETE_VA,
	__DELETE_ALL_VA,
};

static int __modify_uva_in_list(struct bg_dmabuf_uva_info *node,
	struct list_head *info_list, struct mutex *list_mutex,
	struct vm_area_struct *vma, enum va_ops_type type)
{
	struct list_head *pos = NULL;
	struct list_head *q = NULL;
	struct bg_dmabuf_uva_info *temp = NULL;
	struct bg_dmabuf_uva_info *uva_in_list = NULL;
	struct mm_struct *mm = NULL;
	int ret = 0;

	mutex_lock(list_mutex);
	list_for_each_safe(pos, q, info_list) {
		temp = list_entry(pos, struct bg_dmabuf_uva_info, list);
		if (temp->vma == vma) {
			uva_in_list = temp;
			break;
		}
	}

	if ((type == __ADD_VA) && (!uva_in_list)) {
		mm = vma->vm_mm;
		if (!mm) {
			WARN_ON(1);
			bg_buf_pr_err("%s vma-%pK mm not exist\n", __func__, vma);
			ret = -1;
			goto out;
		}
		mmgrab(mm);
		node->mm = mm;
		list_add_tail(&node->list, info_list);
		bg_buf_pr_debug("%s add_va, mm-0x%pK, vma-0x%pK vm_start-0x%llx\n",
			__func__, mm, vma, vma->vm_start);
	} else if ((type == __DELETE_VA) && uva_in_list) {
		bg_buf_pr_debug("%s delete_va, mm-0x%pK, vma-0x%pK vm_start-0x%llx\n",
			__func__, uva_in_list->mm, vma, vma->vm_start);
		mmdrop(uva_in_list->mm);
		list_del(&uva_in_list->list);
		kfree(uva_in_list);
	} else {
		/* something wrong in modify operation */
		ret = -1;
	}
out:
	mutex_unlock(list_mutex);

	return ret;
}

int record_bg_buffer_uva_info(void *buffer, struct vm_area_struct *vma)
{
	struct bg_dmabuf_uva_info *uva_info = NULL;
	struct bg_dmabuf_info *info = NULL;
	int ret;

	info = __get_no_missing_info(buffer);
	if (!info)
		return -EINVAL;

	uva_info = kzalloc(sizeof(*uva_info), GFP_KERNEL);
	if (!uva_info) {
		set_info_missing_va_record(info);
		bg_buf_pr_err("%s alloc failed\n", __func__);
		return -ENOMEM;
	}
	uva_info->state = BG_BUFFER_MAP;
	uva_info->vma = vma;
	(void)memcpy_s(uva_info->task_name, BG_MAX_TASK_NAME_LEN,
		current->comm, TASK_COMM_LEN);

	ret = __modify_uva_in_list(uva_info, &info->uva_info, &info->uva_info_mutex,
		vma, __ADD_VA);
	if (ret) {
		set_info_missing_va_record(info);
		kfree(uva_info);
		bg_buf_pr_info("%s vma-0x%pK vm_start-0x%llx vma_mm-0x%pK has already in info_list\n",
			__func__, vma, vma->vm_start, vma->vm_mm);
	} else {
		bg_buf_pr_debug("%s ok, buffer-0x%pK vma-0x%pK vm_start-0x%llx vma_mm-0x%pK\n",
			__func__, buffer, vma, vma->vm_start, vma->vm_mm);
	}
	return ret;
}

void delete_bg_buffer_uva_info(void *buffer, struct vm_area_struct *vma)
{
	struct bg_dmabuf_info *info = NULL;
	int ret;

	info = __get_no_missing_info(buffer);
	if (!info)
		return;

	ret = __modify_uva_in_list(NULL, &info->uva_info, &info->uva_info_mutex,
		vma, __DELETE_VA);
	if (ret) {
		__dump_uva_info(info);
		bg_buf_pr_err("%s caller-%pS buffer-0x%pK vma-0x%pK vm_start-0x%llx vma_mm-0x%pK has not find in info_list\n",
			__func__, (void *)_RET_IP_, buffer, vma, vma->vm_start, vma->vm_mm);
	} else {
		bg_buf_pr_debug("%s task-%s buffer-0x%pK vma-0x%pK addr-0x%llx vma_mm-0x%pK uva delete ok\n",
			__func__, current->comm, buffer, vma, vma->vm_start, vma->vm_mm);
	}
}

static int __modify_kva_in_list(struct bg_dmabuf_kva_info *node,
	struct list_head *info_list, struct mutex *list_mutex,
	void *vaddr, enum va_ops_type type)
{
	struct list_head *pos = NULL;
	struct list_head *q = NULL;
	struct bg_dmabuf_kva_info *temp = NULL;
	struct bg_dmabuf_kva_info *kva_in_list = NULL;
	int ret = 0;

	mutex_lock(list_mutex);
	list_for_each_safe(pos, q, info_list) {
		temp = list_entry(pos, struct bg_dmabuf_kva_info, list);
		if (temp->vaddr == vaddr) {
			kva_in_list = temp;
			break;
		}
	}

	if ((type == __ADD_VA) && (!kva_in_list)) {
		list_add_tail(&node->list, info_list);
	} else if ((type == __DELETE_VA) && kva_in_list) {
		list_del(&kva_in_list->list);
		kfree(kva_in_list);
	} else {
		/* something wrong in modify operation */
		ret = -1;
	}
	mutex_unlock(list_mutex);

	return ret;
}

int record_bg_buffer_kva_info(void *buffer, void *vaddr)
{
	struct bg_dmabuf_kva_info *kva_info = NULL;
	struct bg_dmabuf_info *info = NULL;
	int ret;

	info = __get_no_missing_info(buffer);
	if (!info)
		return -EINVAL;

	kva_info = kzalloc(sizeof(*kva_info), GFP_KERNEL);
	if (!kva_info) {
		set_info_missing_va_record(info);
		bg_buf_pr_err("%s alloc failed\n", __func__);
		return -ENOMEM;
	}
	kva_info->state = BG_BUFFER_MAP;
	kva_info->vaddr = vaddr;

	ret = __modify_kva_in_list(kva_info, &info->kva_info, &info->kva_info_mutex,
		kva_info->vaddr, __ADD_VA);
	if (ret) {
		set_info_missing_va_record(info);
		bg_buf_pr_err("%s vaddr-%pK already exist\n", __func__, vaddr);
		kfree(kva_info);
	} else {
		bg_buf_pr_debug("%s ok, vaddr-%pK\n", __func__, kva_info->vaddr);
	}

	return ret;
}

void delete_bg_buffer_kva_info(void *buffer, void *vaddr)
{
	struct bg_dmabuf_info *info = NULL;
	int ret;

	info = __get_no_missing_info(buffer);
	if (!info)
		return;

	ret = __modify_kva_in_list(NULL, &info->kva_info, &info->kva_info_mutex,
		vaddr, __DELETE_VA);
	if (ret) {
		__dump_kva_info(info);
		bg_buf_pr_info("%s caller-%pS vaddr-%pK has not find in info_list\n",
			__func__, (void *)_RET_IP_, vaddr);
	} else {
		bg_buf_pr_debug("%s ok, vaddr-%pK\n", __func__, vaddr);
	}
}

static int __modify_iova_in_list(struct bg_dmabuf_iova_info *node,
	struct list_head *info_list, struct mutex *list_mutex,
	struct device *dev, unsigned long iova, enum va_ops_type type)
{
	struct list_head *pos = NULL;
	struct list_head *q = NULL;
	struct bg_dmabuf_iova_info *temp = NULL;
	struct bg_dmabuf_iova_info *iova_in_list = NULL;
	int ret = 0;

	mutex_lock(list_mutex);
	list_for_each_safe(pos, q, info_list) {
		temp = list_entry(pos, struct bg_dmabuf_iova_info, list);
		if (type == __DELETE_ALL_VA) {
			list_del(pos);
			kfree(temp);
			continue;
		}

		if (temp->dev == dev && temp->iova == iova) {
			iova_in_list = temp;
			break;
		}
	}

	if ((type == __ADD_VA) && (!iova_in_list)) {
		list_add_tail(&node->list, info_list);
	} else if ((type == __DELETE_VA) && iova_in_list) {
		list_del(&iova_in_list->list);
		kfree(iova_in_list);
	} else if (type == __DELETE_ALL_VA) {
		/* do nothing */
	} else {
		/* something wrong in modify operation */
		ret = -1;
	}
	mutex_unlock(list_mutex);

	return ret;
}

int record_bg_buffer_iova_info(struct dma_buf *dmabuf, struct device *dev,
	unsigned long iova, unsigned long length, int prot)
{
	struct bg_dmabuf_info *info = NULL;
	struct bg_dmabuf_iova_info *iova_info = NULL;
	void *buffer = get_buffer_ptr_from_dmabuf(dmabuf);
	int ret;

	info = __get_no_missing_info(buffer);
	if (!info)
		return -EINVAL;

	iova_info = kzalloc(sizeof(*iova_info), GFP_KERNEL);
	if (!iova_info) {
		set_info_missing_va_record(info);
		bg_buf_pr_err("%s alloc failed\n", __func__);
		return -ENOMEM;
	}
	iova_info->state = BG_BUFFER_MAP;
	iova_info->dev = dev;
	iova_info->iova = iova;
	iova_info->length = length;
	iova_info->prot = prot;

	ret = __modify_iova_in_list(iova_info, &info->iova_info, &info->iova_info_mutex,
		dev, iova, __ADD_VA);
	if (ret) {
		set_info_missing_va_record(info);
		kfree(iova_info);
		bg_buf_pr_err("%s dev-%s iova-0x%lx len-0x%lx has already exist\n",
			__func__, dev_name(dev), iova, length);
	} else {
		bg_buf_pr_debug("%s ok dev-%s, iova-0x%lx len-0x%lx\n",
			__func__, dev_name(dev), iova, length);
	}
	return ret;
}

void delete_bg_buffer_iova_info(struct dma_buf *dmabuf,
	struct device *dev, unsigned long iova, bool is_delete_all)
{
	struct bg_dmabuf_info *info = NULL;
	enum va_ops_type va_ops = __DELETE_VA;
	void *buffer = get_buffer_ptr_from_dmabuf(dmabuf);
	int ret;

	info = __get_no_missing_info(buffer);
	if (!info)
		return;

	if (is_delete_all)
		va_ops = __DELETE_ALL_VA;

	ret = __modify_iova_in_list(NULL, &info->iova_info, &info->iova_info_mutex,
		dev, iova, va_ops);
	if (ret) {
		__dump_iova_info(info);
		bg_buf_pr_err("%s dev-%s iova-%lx has not find in info_list\n",
			__func__, dev ? dev_name(dev) : "delete_all", iova);
	} else {
		bg_buf_pr_debug("%s ok, dev-%s iova-%lx\n",
			__func__, dev ? dev_name(dev) : "delete_all", iova);
	}
}

bool is_bg_buffer_release(void *buffer)
{
	struct bg_dmabuf_info *info = get_info_from_bg_buffer(buffer);

	if (!info)
		return false;

	return (atomic_read(&info->is_release) > 0);
}

int bg_dmabuf_attach(void *buffer)
{
	struct bg_dmabuf_info *info = NULL;
	int ret = 0;

	buffer_lock(buffer);
	info = __get_no_missing_info(buffer);
	if (!info) {
		bg_buf_pr_debug("%s get info error\n", __func__);
		goto out;
	}

	if (is_bg_buffer_release((void *)buffer)) {
		bg_buf_pr_err("%s buf has already bg_release\n", __func__);
		ret = -EINVAL;
	} else {
		atomic_inc(&info->attach_num);
	}
out:
	buffer_unlock(buffer);
	return ret;
}

void bg_dmabuf_detach(void *buffer)
{
	struct bg_dmabuf_info *info = NULL;
	int attach_num;

	buffer_lock(buffer);
	info = __get_no_missing_info(buffer);
	if (!info) {
		bg_buf_pr_debug("%s get info error\n", __func__);
		goto out;
	}

	if (is_bg_buffer_release((void *)buffer))
		WARN(1, "%s buf has already bg_release\n", __func__);

	attach_num = atomic_read(&info->attach_num);
	if (attach_num <= 0) {
		bg_buf_pr_err("%s attach num-%d is wrong\n", __func__, attach_num);
		goto out;
	}

	atomic_dec(&info->attach_num);
out:
	buffer_unlock(buffer);
}

void dump_bg_buffer_info(struct dma_buf *dmabuf)
{
	struct bg_dmabuf_info *info = NULL;
	void *buffer = get_buffer_ptr_from_dmabuf(dmabuf);

	info = get_info_from_bg_buffer(buffer);
	if (!info)
		return;

	bg_buf_pr_info("%s buffer-summary: is_release[%d]  is_missing_info[%d]", __func__,
		atomic_read(&info->is_release), is_info_missing_va_record(info));

	__dump_uva_info(info);
	__dump_kva_info(info);
	__dump_iova_info(info);
}

