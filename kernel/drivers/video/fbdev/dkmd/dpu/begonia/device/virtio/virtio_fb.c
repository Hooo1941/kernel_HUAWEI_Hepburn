/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/slab.h>
#include <linux/module.h>
#include <linux/virtio_ids.h>
#include <linux/virtio.h>
#include <linux/virtio_config.h>
#include <linux/spinlock.h>
#include <asm/sysreg.h>
#define CREATE_TRACE_POINTS
#include <trace/events/vfb.h>

#include "virtio_log.h"
#include "virtio_fb.h"
#include "../vfb/dpu_vfb_api.h"

struct virtio_fb_info {
	int index;
	struct virtio_device *vdev;
	struct virtqueue **vqs;
	unsigned int num_vqs;
};

typedef enum {
	VQ_INFO,
	VQ_PRESENT,
	VQ_SMMU,
	VQ_VIRTIO_FB_MAX,
} virtio_fb_vq_type_t;

static DEFINE_SPINLOCK(g_map_lock);
static DEFINE_MUTEX(g_vfi_lock);
static DEFINE_SPINLOCK(g_vq_lock);
/* only support one virtio fb */
static struct virtio_fb_info *g_vfi = NULL;
static int g_index = 0;

static void virtio_fb_fill_product_config(struct virtio_fb_info *vfi)
{
	struct scatterlist sg;
	struct vfb_product_config *config = NULL;
	struct virtqueue *vq = NULL;
	unsigned int len;

	vfb_debug("%s +\n", __func__);

	vq = vfi->vqs[VQ_INFO];

	config = kzalloc(sizeof(struct vfb_product_config), GFP_KERNEL);
	if (config == NULL) {
		pr_warn("failed to alloc buf\n");
		return;
	}

	sg_init_one(&sg, config, sizeof(struct vfb_product_config));
	virtqueue_add_outbuf(vq, &sg, 1, config, GFP_KERNEL);
	virtqueue_kick(vq);

	if (virtqueue_get_buf(vq, &len)) {
		vfb_fill_product_config(vfi->index, config);
	} else {
		pr_warn("failed to get buf\n");
	}

	kfree(config);
	config = NULL;

	vfb_debug("%s -\n", __func__);
}

int virtio_fb_send_sgl_info(struct sg_table *table, struct iommu_head_info *h_info)
{
	int ret;
	unsigned int len;
	unsigned long flags;

	if ((table == NULL) || (h_info == NULL)) {
		pr_err("table or h_info is null\n");
		return  -1;
	}

	spin_lock_irqsave(&g_map_lock, flags);
	ret = virtqueue_add_inbuf(g_vfi->vqs[VQ_SMMU], table->sgl, table->nents, h_info, GFP_KERNEL);
	if (ret < 0) {
		pr_err("virtqueue add buffer failed\n");
		spin_unlock_irqrestore(&g_map_lock, flags);
		return -1;
	}

	virtqueue_kick(g_vfi->vqs[VQ_SMMU]);

	if (!virtqueue_get_buf(g_vfi->vqs[VQ_SMMU], &len)) {
		pr_err("failed to get iova info\n");
		spin_unlock_irqrestore(&g_map_lock, flags);
		return -1;
	}
	spin_unlock_irqrestore(&g_map_lock, flags);

	return 0;
}

static void virtio_fb_callback_present(struct virtqueue *vq)
{
	unsigned int len;
	struct virtio_fb_present_data *data = NULL;
	struct virtio_fb_info *vfi = NULL;
	unsigned long flags;

	if ((vq->vdev != NULL) && (vq->vdev->priv != NULL)) {
		vfi = vq->vdev->priv;
		vfb_debug("%s: index=%d, num_vqs=%d\n", __func__, vfi->index, vfi->num_vqs);
	}

	spin_lock_irqsave(&g_vq_lock, flags);
	data = virtqueue_get_buf(vq, &len);
	spin_unlock_irqrestore(&g_vq_lock, flags);
	while(data != NULL) {
		vfb_debug("%s: vq->index=%d, len=%d, frame_no=%u\n",
			  __func__, vq->index, len, data->frame_no);
		vfb_present_done(vfi->index, data);
		spin_lock_irqsave(&g_vq_lock, flags);
		data = virtqueue_get_buf(vq, &len);
		spin_unlock_irqrestore(&g_vq_lock, flags);
	}
}

static void virtio_fb_callback(struct virtqueue *vq)
{
	if (vq == NULL) {
		pr_warn("vq is null\n");
		return;
	}

	switch (vq->index) {
	case VQ_PRESENT:
		virtio_fb_callback_present(vq);
		break;
	default:
		pr_warn("invalid vq index: %u\n", vq->index);
		break;
	}
}

static int init_vqs(struct virtio_fb_info *vfi)
{
	int ret;
	struct virtio_device *vdev = vfi->vdev;
	unsigned int num_vqs = vfi->num_vqs;

	unsigned int i;
	vq_callback_t **callbacks = NULL;
	const char *names[VQ_VIRTIO_FB_MAX] = {
		"vq_info",
		"vq_present",
		"vq_smmu",
	};

	if (num_vqs > VQ_VIRTIO_FB_MAX) {
		pr_err("too many vqs\n");
		return -EINVAL;
	}

	vfi->vqs = kmalloc_array(num_vqs, sizeof(struct virtqueue *), GFP_KERNEL);
	callbacks = kmalloc_array(num_vqs, sizeof(vq_callback_t *), GFP_KERNEL);
	if (vfi->vqs == NULL || callbacks == NULL) {
		pr_warn("failed to allocate vqs or callbacks\n");
		ret = -ENOMEM;
		goto err;
	}

	for (i = 0; i < num_vqs; i++) {
		if (i == VQ_INFO || i == VQ_SMMU) {
			callbacks[i] = NULL;
			continue;
		}
		callbacks[i] = virtio_fb_callback;
	}

	ret = virtio_find_vqs(vdev, num_vqs, vfi->vqs, callbacks, names, NULL);
	if (ret != 0) {
		pr_warn("failed to find vqs\n");
		goto err;
	}

	kfree(callbacks);
	callbacks = NULL;
	return 0;

err:
	if (callbacks != NULL) {
		kfree(callbacks);
		callbacks = NULL;
	}

	if (vfi->vqs != NULL) {
		kfree(vfi->vqs);
		vfi->vqs = NULL;
	}

	return ret;
}

static int virtio_fb_probe(struct virtio_device *vdev)
{
	int ret = 0;
	struct virtio_fb_info *vfi = NULL;
	int index;

	if (vdev == NULL) {
		pr_err("vdev is null\n");
		return -EINVAL;
	}

	vfb_debug("%s +\n", __func__);

	mutex_lock(&g_vfi_lock);
	if (g_index > 0) {
		ret = -EINVAL;
	} else {
		index = g_index;
	}
	mutex_unlock(&g_vfi_lock);

	if (ret != 0) {
		pr_warn("too many virtio fb devices\n");
		return ret;
	}

	vfi = kmalloc(sizeof(struct virtio_fb_info), GFP_KERNEL);
	if (unlikely(!vfi)) {
		pr_warn("failed to allocate vfi\n");
		return -ENOMEM;
	}

	vdev->priv = vfi;
	vfi->vdev = vdev;
	vfi->num_vqs = VQ_VIRTIO_FB_MAX;
	vfi->vqs = NULL;
	vfi->index = index;

	ret = init_vqs(vfi);
	if (ret != 0) {
		pr_warn("failed to init vqs\n");
		kfree(vfi);
		vfi = NULL;
		return ret;
	}

	virtio_device_ready(vdev);

	virtio_fb_fill_product_config(vfi);

	mutex_lock(&g_vfi_lock);
	g_vfi = vfi;
	g_index++;
	mutex_unlock(&g_vfi_lock);

	vfb_debug("%s -\n", __func__);

	return 0;
}

static void virtio_fb_remove(struct virtio_device *vdev)
{
	struct virtio_fb_info *vfi = NULL;

	if (vdev == NULL) {
		pr_err("vdev is null\n");
		return;
	}

	mutex_lock(&g_vfi_lock);
	g_vfi = NULL;
	if (g_index <= 0) {
		mutex_unlock(&g_vfi_lock);
		return;
	}
		
	g_index--;
	mutex_unlock(&g_vfi_lock);

	vfi = vdev->priv;

	vdev->config->del_vqs(vdev);
	kfree(vfi->vqs);
	vfi->vqs = NULL;
	kfree(vfi);
	vfi = NULL;
}

static const struct virtio_device_id id_table[] = {
	{ VIRTIO_ID_FB_V2, VIRTIO_DEV_ANY_ID },
	{ 0 },
};

static struct virtio_driver virtio_fb = {
	.driver.name		= KBUILD_MODNAME,
	.driver.owner		= THIS_MODULE,
	.id_table		= id_table,
	.probe			= virtio_fb_probe,
	.remove			= virtio_fb_remove,
};

static int __init virtio_fb_init(void)
{
	vfb_debug("virtio fb v2 init\n");
	return register_virtio_driver(&virtio_fb);
}

module_init(virtio_fb_init);

MODULE_DEVICE_TABLE(virtio, id_table);
MODULE_DESCRIPTION("Virtio fb driver");

int virtio_fb_present(int index, struct virtio_fb_present_data *data)
{
	int ret = 0;
	struct scatterlist sg;
	struct virtqueue *vq = NULL;
	unsigned long flags;

	if (data == NULL) {
		pr_err("data is null\n");
		return -ENODEV;
	}

	sg_init_one(&sg, data, sizeof(struct virtio_fb_present_data));

	mutex_lock(&g_vfi_lock);

	if (g_vfi == NULL || index != g_vfi->index)
		ret = -ENODEV;

	spin_lock_irqsave(&g_vq_lock, flags);
	if (ret == 0) {
		vq = g_vfi->vqs[VQ_PRESENT];

		ret = virtqueue_add_inbuf(vq, &sg, 1, data, GFP_KERNEL);
	}

	if (ret == 0) {
		trace_vfb_present_kick(true, current->pid, (int32_t)data->frame_no, read_sysreg(cntvct_el0));
		virtqueue_kick(vq);
		trace_vfb_present_kick(false, current->pid, (int32_t)data->frame_no, read_sysreg(cntvct_el0));
	}
	spin_unlock_irqrestore(&g_vq_lock, flags);

	mutex_unlock(&g_vfi_lock);

	if (ret == 0)
		vfb_debug("%s: frame_no=%u\n", __func__, data->frame_no);
	else
		pr_warn("failed to present: ret=%d, frame_no = %u\n", ret, data->frame_no);

	return ret;
}
