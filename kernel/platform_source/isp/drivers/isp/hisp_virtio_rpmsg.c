// SPDX-License-Identifier: GPL-2.0
/*
 * Virtio-based remote processor messaging bus
 *
 * Copyright (C) 2011 Texas Instruments, Inc.
 * Copyright (C) 2011 Google, Inc.
 *
 * Ohad Ben-Cohen <ohad@wizery.com>
 * Brian Swetland <swetland@google.com>
 */

#include <linux/dma-mapping.h>
#include <linux/idr.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of_device.h>
#include <linux/scatterlist.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/virtio.h>
#include <linux/virtio_byteorder.h>
#include <linux/virtio_ids.h>
#include <linux/virtio_config.h>
#include <linux/wait.h>
#include <linux/rpmsg.h>
#include <securec.h>
#include <rpmsg_internal.h>
#include "hisp_internel.h"

struct hisp_vproc_info {
	struct virtio_device *vdev;
	struct virtqueue *rvq, *svq;
	void *rbufs;
	void *sbufs;
	unsigned int num_bufs;
	unsigned int buf_size;
	int last_sbuf;
	dma_addr_t bufs_dma;
	struct mutex tx_lock;
	struct idr endpoints;
	struct mutex endpoints_lock;
	wait_queue_head_t sendq;
	atomic_t sleepers;
	struct rpmsg_endpoint *ns_ept;
};

#define VIRTIO_RPMSG_F_NS      0 /* RP supports name service notifications */
#define ISP_VIRTIO_EPT_MAX     2 /* MAX Endpoint Num */
#define ISP_VIRTIO_EPT_MIN     1 /* MIN Endpoint Num */

struct rpmsg_ns_msg {
	char name[RPMSG_NAME_SIZE];
	__virtio32 addr;
	__virtio32 flags;
} __packed;

enum rpmsg_ns_flags {
	RPMSG_NS_CREATE		= 0,
	RPMSG_NS_DESTROY	= 1,
};

struct hisp_virtio_rpmsg_ch {
	struct rpmsg_device rpdev;
	struct hisp_vproc_info *vrp;
};

#define to_hisp_virtio_rpmsg_ch(_rpdev) \
	container_of(_rpdev, struct hisp_virtio_rpmsg_ch, rpdev)

#define MAX_RPMSG_NUM_BUFS	(512)
#define MAX_RPMSG_BUF_SIZE	(512)

#define RPMSG_RESERVED_ADDRESSES	(1024)

#define RPMSG_NS_ADDR			(53)

static void hisp_vr_destroy_ept(struct rpmsg_endpoint *ept);
static int hisp_vr_send(struct rpmsg_endpoint *ept, void *data, int len);
static int hisp_vr_sendto(struct rpmsg_endpoint *ept, void *data, int len,
			       u32 dst);
static int hisp_vr_send_offchannel(struct rpmsg_endpoint *ept, u32 src,
					u32 dst, void *data, int len);
static int hisp_vr_trysend(struct rpmsg_endpoint *ept, void *data, int len);
static int hisp_vr_trysendto(struct rpmsg_endpoint *ept, void *data,
				  int len, u32 dst);
static int hisp_vr_trysend_offchannel(struct rpmsg_endpoint *ept, u32 src,
					   u32 dst, void *data, int len);

static const struct rpmsg_endpoint_ops hisp_vr_ept_ops = {
	.destroy_ept = hisp_vr_destroy_ept,
	.send = hisp_vr_send,
	.sendto = hisp_vr_sendto,
	.send_offchannel = hisp_vr_send_offchannel,
	.trysend = hisp_vr_trysend,
	.trysendto = hisp_vr_trysendto,
	.trysend_offchannel = hisp_vr_trysend_offchannel,
};

static void hisp_vr_sg_init(struct scatterlist *sg, void *cpu_addr, unsigned int len)
{
	if (is_vmalloc_addr(cpu_addr)) {
		sg_init_table(sg, 1);
		sg_set_page(sg, vmalloc_to_page(cpu_addr), len,
			    offset_in_page((uintptr_t)cpu_addr));
	} else {
		WARN_ON(virt_addr_valid((uintptr_t)cpu_addr) == 0);
		sg_init_one(sg, cpu_addr, len);
	}
}

static void hisp_vr_ept_release(struct kref *kref)
{
	struct rpmsg_endpoint *ept = container_of(kref, struct rpmsg_endpoint,
						  refcount);

	kfree(ept);
}

static struct rpmsg_endpoint *__hisp_vr_create_ept(struct hisp_vproc_info *vrp,
		struct rpmsg_device *rpdev, rpmsg_rx_cb_t cb, void *priv, u32 addr)
{
	int id_min, id_max, id;
	struct rpmsg_endpoint *ept;
	struct device *dev = rpdev ? &rpdev->dev : &vrp->vdev->dev;

	ept = kzalloc(sizeof(*ept), GFP_KERNEL);
	if (!ept)
		return NULL;

	kref_init(&ept->refcount);
	mutex_init(&ept->cb_lock);

	ept->rpdev = rpdev;
	ept->cb = cb;
	ept->priv = priv;
	ept->ops = &hisp_vr_ept_ops;

	if (addr == RPMSG_ADDR_ANY) {
		id_min = RPMSG_RESERVED_ADDRESSES;
		id_max = 0;
	} else {
		id_min = (int)addr;
		id_max = (int)addr + 1;
	}

	mutex_lock(&vrp->endpoints_lock);

	id = idr_alloc(&vrp->endpoints, ept, id_min, id_max, GFP_KERNEL);
	if (id < 0) {
		dev_err(dev, "idr_alloc failed: %d\n", id);
		goto free_ept;
	}
	ept->addr = (u32)id;

	mutex_unlock(&vrp->endpoints_lock);

	return ept;

free_ept:
	mutex_unlock(&vrp->endpoints_lock);
	kref_put(&ept->refcount, hisp_vr_ept_release);
	return NULL;
}

static struct rpmsg_endpoint *hisp_vr_create_ept(struct rpmsg_device *rpdev,
						      rpmsg_rx_cb_t cb,
						      void *priv,
						      struct rpmsg_channel_info chinfo)
{
	struct hisp_virtio_rpmsg_ch *vch = to_hisp_virtio_rpmsg_ch(rpdev);
	struct rpmsg_endpoint *ept = NULL;
	ept = __hisp_vr_create_ept(vch->vrp, rpdev, cb, priv, chinfo.src);
	if (ept == NULL)
		return NULL;
	kref_get(&ept->refcount);
	hisp_msg_service_init(ept);
	return ept;
}

static void __hisp_vr_destroy_ept(struct hisp_vproc_info *vrp, struct rpmsg_endpoint *ept)
{
	mutex_lock(&vrp->endpoints_lock);
	idr_remove(&vrp->endpoints, ept->addr);
	mutex_unlock(&vrp->endpoints_lock);

	mutex_lock(&ept->cb_lock);
	ept->cb = NULL;
	mutex_unlock(&ept->cb_lock);

	kref_put(&ept->refcount, hisp_vr_ept_release);
}

static void hisp_vr_destroy_ept(struct rpmsg_endpoint *ept)
{
	struct hisp_virtio_rpmsg_ch *vch = to_hisp_virtio_rpmsg_ch(ept->rpdev);
	hisp_msg_service_exit();

	switch (kref_read(&ept->refcount)) {
	case ISP_VIRTIO_EPT_MAX:
		kref_put(&ept->refcount, hisp_vr_ept_release);
		__hisp_vr_destroy_ept(vch->vrp, ept);
		break;
	case ISP_VIRTIO_EPT_MIN:
		__hisp_vr_destroy_ept(vch->vrp, ept);
		break;
	default:
		pr_err("%s: refcount: %d\n", __func__, kref_read(&ept->refcount));
		break;
	}
}

static int hisp_vr_announce_create(struct rpmsg_device *rpdev)
{
	struct hisp_virtio_rpmsg_ch *vch = to_hisp_virtio_rpmsg_ch(rpdev);
	struct hisp_vproc_info *vrp = vch->vrp;
	struct device *dev = &rpdev->dev;
	int err = 0;

	if (rpdev->announce && rpdev->ept &&
	    virtio_has_feature(vrp->vdev, VIRTIO_RPMSG_F_NS)) {
		struct rpmsg_ns_msg nsm;

		err = strncpy_s(nsm.name, RPMSG_NAME_SIZE,
				rpdev->id.name, RPMSG_NAME_SIZE);
		if (err)
			dev_err(dev, "create:strcpy rpmsg name to msg fail. %d\n", err);
		nsm.name[RPMSG_NAME_SIZE - 1] = '\0';
		nsm.addr = cpu_to_virtio32(vrp->vdev, rpdev->ept->addr);
		nsm.flags = cpu_to_virtio32(vrp->vdev, RPMSG_NS_CREATE);

		err = rpmsg_sendto(rpdev->ept, &nsm, sizeof(nsm), RPMSG_NS_ADDR);
		if (err)
			dev_err(dev, "failed to announce service %d\n", err);
	}

	return err;
}

static int hisp_vr_announce_destroy(struct rpmsg_device *rpdev)
{
	struct hisp_virtio_rpmsg_ch *vch = to_hisp_virtio_rpmsg_ch(rpdev);
	struct hisp_vproc_info *vrp = vch->vrp;
	struct device *dev = &rpdev->dev;
	int err = 0;

	if (rpdev->announce && rpdev->ept &&
	    virtio_has_feature(vrp->vdev, VIRTIO_RPMSG_F_NS)) {
		struct rpmsg_ns_msg nsm;

		err = strncpy_s(nsm.name, RPMSG_NAME_SIZE,
				rpdev->id.name, RPMSG_NAME_SIZE);
		if (err)
			dev_err(dev, "destroy:strcpy rpmsg name to msg fail. %d\n", err);
		nsm.name[RPMSG_NAME_SIZE - 1] = '\0';
		nsm.addr = cpu_to_virtio32(vrp->vdev, rpdev->ept->addr);
		nsm.flags = cpu_to_virtio32(vrp->vdev, RPMSG_NS_DESTROY);

		err = rpmsg_sendto(rpdev->ept, &nsm, sizeof(nsm), RPMSG_NS_ADDR);
		if (err)
			dev_err(dev, "failed to announce service %d\n", err);
	}

	return err;
}

static const struct rpmsg_device_ops virtio_rpmsg_ops = {
	.create_ept = hisp_vr_create_ept,
	.announce_create = hisp_vr_announce_create,
	.announce_destroy = hisp_vr_announce_destroy,
};

static void hisp_vr_release_device(struct device *dev)
{
	struct rpmsg_device *rpdev = to_rpmsg_device(dev);
	struct hisp_virtio_rpmsg_ch *vch = to_hisp_virtio_rpmsg_ch(rpdev);

	kfree(vch);
}

static struct rpmsg_device *hisp_rpmsg_create_ch(struct hisp_vproc_info *vrp,
						 struct rpmsg_channel_info *chinfo)
{
	struct hisp_virtio_rpmsg_ch *vch = NULL;
	struct rpmsg_device *rpdev = NULL;
	struct device *tmp = NULL, *dev = &vrp->vdev->dev;
	int ret;

	tmp = rpmsg_find_device(dev, chinfo);
	if (tmp) {
		put_device(tmp);
		dev_err(dev, "channel %s:%x:%x already exist\n",
				chinfo->name, chinfo->src, chinfo->dst);
		return NULL;
	}

	vch = kzalloc(sizeof(*vch), GFP_KERNEL);
	if (!vch)
		return NULL;

	vch->vrp = vrp;

	rpdev = &vch->rpdev;
	rpdev->src = chinfo->src;
	rpdev->dst = chinfo->dst;
	rpdev->ops = &virtio_rpmsg_ops;
	rpdev->announce = rpdev->src != RPMSG_ADDR_ANY;

	ret = strncpy_s(rpdev->id.name, RPMSG_NAME_SIZE,
			chinfo->name, RPMSG_NAME_SIZE);
	if (ret) {
		dev_err(dev, "strcpy channel name to rpmsg id fail. %d\n", ret);
		return NULL;
	}

	rpdev->id.name[RPMSG_NAME_SIZE - 1] = '\0';
	rpdev->dev.parent = &vrp->vdev->dev;
	rpdev->dev.release = hisp_vr_release_device;
	ret = rpmsg_register_device(rpdev);
	if (ret)
		return NULL;

	return rpdev;
}

static void *hisp_vr_txbuf_get(struct hisp_vproc_info *vrp)
{
	unsigned int len;
	void *ret = NULL;

	mutex_lock(&vrp->tx_lock);

	if (vrp->last_sbuf < (int)(vrp->num_bufs / 2))
		ret = vrp->sbufs + vrp->buf_size * vrp->last_sbuf++;
	else
		ret = virtqueue_get_buf(vrp->svq, &len);

	mutex_unlock(&vrp->tx_lock);

	return ret;
}

static void hisp_vr_upref_sleepers(struct hisp_vproc_info *vrp)
{
	mutex_lock(&vrp->tx_lock);

	if (atomic_inc_return(&vrp->sleepers) == 1)
		virtqueue_enable_cb(vrp->svq);

	mutex_unlock(&vrp->tx_lock);
}

static void hisp_vr_downref_sleepers(struct hisp_vproc_info *vrp)
{
	mutex_lock(&vrp->tx_lock);

	if (atomic_dec_and_test(&vrp->sleepers))
		virtqueue_disable_cb(vrp->svq);

	mutex_unlock(&vrp->tx_lock);
}

static int __hisp_vr_send_offchannel(struct rpmsg_device *rpdev,
			u32 src, u32 dst, void *data, int len, bool wait)
{
	struct hisp_virtio_rpmsg_ch *vch = to_hisp_virtio_rpmsg_ch(rpdev);
	struct hisp_vproc_info *vrp = vch->vrp;
	struct device *dev = &rpdev->dev;
	struct scatterlist sg;
	struct rpmsg_hdr *msg = NULL;
	int err;

	if (src == RPMSG_ADDR_ANY || dst == RPMSG_ADDR_ANY) {
		dev_err(dev, "invalid addr (src 0x%x, dst 0x%x)\n", src, dst);
		return -EINVAL;
	}

	if (len > (int)(vrp->buf_size - sizeof(struct rpmsg_hdr))) {
		dev_err(dev, "message is too big (%d)\n", len);
		return -EMSGSIZE;
	}

	msg = hisp_vr_txbuf_get(vrp);
	if (!msg && !wait)
		return -ENOMEM;

	while (!msg) {
		hisp_vr_upref_sleepers(vrp);

		err = wait_event_interruptible_timeout(vrp->sendq,
					(msg = hisp_vr_txbuf_get(vrp)),
					(long)msecs_to_jiffies(15000));

		hisp_vr_downref_sleepers(vrp);

		if (err < 0) {
			dev_err(dev, "abort due to a received signal\n");
			return -ERESTARTSYS;
		}

		if (err == 0) {
			dev_err(dev, "timeout waiting for a tx buffer\n");
			return -ERESTARTSYS;
		}
	}

	msg->len = cpu_to_virtio16(vrp->vdev, len);
	msg->flags = 0;
	msg->src = cpu_to_virtio32(vrp->vdev, src);
	msg->dst = cpu_to_virtio32(vrp->vdev, dst);
	msg->reserved = 0;
	err = memcpy_s(msg->data, len, data, len);
	if (err) {
		dev_err(dev, "memcpy data to msg failed: %d\n", err);
		goto out;
	}

	dev_dbg(dev, "TX From 0x%x, To 0x%x, Len %d, Flags %d, Reserved %d\n",
		src, dst, len, msg->flags, msg->reserved);

	hisp_vr_sg_init(&sg, msg, sizeof(*msg) + len);

	mutex_lock(&vrp->tx_lock);

	err = virtqueue_add_outbuf(vrp->svq, &sg, 1, msg, GFP_KERNEL);
	if (err) {
		dev_err(dev, "virtqueue_add_outbuf failed: %d\n", err);
		goto out;
	}

	virtqueue_kick(vrp->svq);
out:
	mutex_unlock(&vrp->tx_lock);
	return err;
}

static int hisp_vr_send(struct rpmsg_endpoint *ept, void *data, int len)
{
	struct rpmsg_device *rpdev = ept->rpdev;
	u32 src = ept->addr, dst = rpdev->dst;

	return __hisp_vr_send_offchannel(rpdev, src, dst, data, len, true);
}

static int hisp_vr_sendto(struct rpmsg_endpoint *ept, void *data, int len,
			       u32 dst)
{
	struct rpmsg_device *rpdev = ept->rpdev;
	u32 src = ept->addr;

	return __hisp_vr_send_offchannel(rpdev, src, dst, data, len, true);
}

static int hisp_vr_send_offchannel(struct rpmsg_endpoint *ept, u32 src,
					u32 dst, void *data, int len)
{
	struct rpmsg_device *rpdev = ept->rpdev;

	return __hisp_vr_send_offchannel(rpdev, src, dst, data, len, true);
}

static int hisp_vr_trysend(struct rpmsg_endpoint *ept, void *data, int len)
{
	struct rpmsg_device *rpdev = ept->rpdev;
	u32 src = ept->addr, dst = rpdev->dst;

	return __hisp_vr_send_offchannel(rpdev, src, dst, data, len, false);
}

static int hisp_vr_trysendto(struct rpmsg_endpoint *ept, void *data,
				  int len, u32 dst)
{
	struct rpmsg_device *rpdev = ept->rpdev;
	u32 src = ept->addr;

	return __hisp_vr_send_offchannel(rpdev, src, dst, data, len, false);
}

static int hisp_vr_trysend_offchannel(struct rpmsg_endpoint *ept, u32 src,
					   u32 dst, void *data, int len)
{
	struct rpmsg_device *rpdev = ept->rpdev;

	return __hisp_vr_send_offchannel(rpdev, src, dst, data, len, false);
}

static int hisp_vr_recv_single(struct hisp_vproc_info *vrp, struct device *dev,
			     struct rpmsg_hdr *msg, unsigned int len)
{
	struct rpmsg_endpoint *ept = NULL;
	struct scatterlist sg;
	unsigned int msg_len = virtio16_to_cpu(vrp->vdev, msg->len);
	int err;

	dev_dbg(dev, "From: 0x%x, To: 0x%x, Len: %d, Flags: %d, Reserved: %d\n",
		virtio32_to_cpu(vrp->vdev, msg->src),
		virtio32_to_cpu(vrp->vdev, msg->dst), msg_len,
		virtio16_to_cpu(vrp->vdev, msg->flags),
		virtio32_to_cpu(vrp->vdev, msg->reserved));

	if (len > vrp->buf_size ||
	    msg_len > (len - sizeof(struct rpmsg_hdr))) {
		dev_warn(dev, "inbound msg too big: (%d, %d)\n", len, msg_len);
		return -EINVAL;
	}

	mutex_lock(&vrp->endpoints_lock);
	ept = idr_find(&vrp->endpoints, virtio32_to_cpu(vrp->vdev, msg->dst));
	if (ept)
		kref_get(&ept->refcount);

	mutex_unlock(&vrp->endpoints_lock);

	if (ept) {
		mutex_lock(&ept->cb_lock);

		if (hisp_msg_service_recv(msg->data, msg_len,\
				virtio32_to_cpu(vrp->vdev, msg->src)) == 0) {
			if (ept->cb)
				ept->cb(ept->rpdev, msg->data, msg_len, ept->priv,\
					virtio32_to_cpu(vrp->vdev, msg->src));
		}
		mutex_unlock(&ept->cb_lock);

		kref_put(&ept->refcount, hisp_vr_ept_release);
	} else
		dev_warn(dev, "msg received with no recipient\n");

	hisp_vr_sg_init(&sg, msg, vrp->buf_size);

	err = virtqueue_add_inbuf(vrp->rvq, &sg, 1, msg, GFP_KERNEL);
	if (err < 0) {
		dev_err(dev, "failed to add a virtqueue buffer: %d\n", err);
		return err;
	}

	return 0;
}

static void hisp_vr_recv_done(struct virtqueue *rvq)
{
	struct hisp_vproc_info *vrp = rvq->vdev->priv;
	struct device *dev = &rvq->vdev->dev;
	struct rpmsg_hdr *msg;
	unsigned int len, msgs_received = 0;
	int err;

	msg = virtqueue_get_buf(rvq, &len);
	if (!msg) {
		dev_err(dev, "uhm, incoming signal, but no used buffer ?\n");
		return;
	}

	while (msg) {
		err = hisp_vr_recv_single(vrp, dev, msg, len);
		if (err)
			break;

		msgs_received++;
		msg = virtqueue_get_buf(rvq, &len);
	}

	dev_dbg(dev, "Received %u messages\n", msgs_received);

	if (msgs_received)
		virtqueue_kick(vrp->rvq);
}

static void hisp_vr_xmit_done(struct virtqueue *svq)
{
	struct hisp_vproc_info *vrp = svq->vdev->priv;

	dev_dbg(&svq->vdev->dev, "%s\n", __func__);
	wake_up_interruptible(&vrp->sendq);
}

static int hisp_vr_ns_cb(struct rpmsg_device *rpdev, void *data, int len,
		       void *priv, u32 src)
{
	struct rpmsg_ns_msg *msg = data;
	struct rpmsg_device *newch = NULL;
	struct rpmsg_channel_info chinfo;
	struct hisp_vproc_info *vrp = priv;
	struct device *dev = &vrp->vdev->dev;
	int ret;

	(void)src;

	if (len != (ssize_t)sizeof(*msg)) {
		dev_err(dev, "malformed ns msg (%d)\n", len);
		return -EINVAL;
	}

	if (rpdev) {
		dev_err(dev, "anomaly: ns ept has an rpdev handle\n");
		return -EINVAL;
	}

	msg->name[RPMSG_NAME_SIZE - 1] = '\0';
	ret = strncpy_s(chinfo.name, RPMSG_NAME_SIZE,
			msg->name, RPMSG_NAME_SIZE);
	if (ret) {
		dev_err(dev, "channel name strcpy fail.%d\n", ret);
		return -EINVAL;
	}

	chinfo.src = RPMSG_ADDR_ANY;
	chinfo.dst = virtio32_to_cpu(vrp->vdev, msg->addr);
	dev_info(dev, "%sing channel %s addr 0x%x\n",
		 virtio32_to_cpu(vrp->vdev, msg->flags) & RPMSG_NS_DESTROY ?
		 "destroy" : "creat", msg->name, chinfo.dst);

	if (virtio32_to_cpu(vrp->vdev, msg->flags) & RPMSG_NS_DESTROY) {
		ret = rpmsg_unregister_device(&vrp->vdev->dev, &chinfo);
		if (ret)
			dev_err(dev, "rpmsg_destroy_channel failed: %d\n", ret);
	} else {
		newch = hisp_rpmsg_create_ch(vrp, &chinfo);
		if (!newch)
			dev_err(dev, "hisp_rpmsg_create_ch failed\n");
	}

	return 0;
}

static int hisp_vr_probe(struct virtio_device *vdev)
{
	vq_callback_t *vq_cbs[] = { hisp_vr_recv_done, hisp_vr_xmit_done };
	static const char * const names[] = { "input", "output" };
	struct virtqueue *vqs[2];
	struct hisp_vproc_info *vrp = NULL;
	void *bufs_va = NULL;
	int err = 0;
	unsigned int i;
	size_t total_buf_space;
	bool notify = 0;

	vrp = kzalloc(sizeof(*vrp), GFP_KERNEL);
	if (!vrp)
		return -ENOMEM;

	vrp->vdev = vdev;

	idr_init(&vrp->endpoints);
	mutex_init(&vrp->endpoints_lock);
	mutex_init(&vrp->tx_lock);
	init_waitqueue_head(&vrp->sendq);

	err = virtio_find_vqs(vdev, 2, vqs, vq_cbs, names, NULL);
	if (err)
		goto free_vrp;

	vrp->rvq = vqs[0];
	vrp->svq = vqs[1];

	WARN_ON(virtqueue_get_vring_size(vrp->rvq) !=
		virtqueue_get_vring_size(vrp->svq));

	if (virtqueue_get_vring_size(vrp->rvq) < MAX_RPMSG_NUM_BUFS / 2)
		vrp->num_bufs = virtqueue_get_vring_size(vrp->rvq) * 2;
	else
		vrp->num_bufs = MAX_RPMSG_NUM_BUFS;

	vrp->buf_size = MAX_RPMSG_BUF_SIZE;
	total_buf_space = vrp->num_bufs * vrp->buf_size;
	bufs_va = hisp_vqmem_info_get(vdev, &vrp->bufs_dma, total_buf_space);
	if (!bufs_va) {
		err = -ENOMEM;
		goto vqs_del;
	}

	dev_dbg(&vdev->dev, "buffers: va %pK, dma %pad\n",
		bufs_va, &vrp->bufs_dma);

	vrp->rbufs = bufs_va;
	vrp->sbufs = bufs_va + total_buf_space / 2;
	for (i = 0; i < vrp->num_bufs / 2; i++) {
		struct scatterlist sg;
		void *cpu_addr = vrp->rbufs + i * vrp->buf_size;
		hisp_vr_sg_init(&sg, cpu_addr, vrp->buf_size);
		err = virtqueue_add_inbuf(vrp->rvq, &sg, 1, cpu_addr,
					  GFP_KERNEL);
		WARN_ON(err != 0); /* sanity check; this can't really happen */
	}

	virtqueue_disable_cb(vrp->svq);
	vdev->priv = vrp;
	vrp->ns_ept = __hisp_vr_create_ept(vrp, NULL, hisp_vr_ns_cb,
					vrp, RPMSG_NS_ADDR);
	if (!vrp->ns_ept) {
		dev_err(&vdev->dev, "failed to create the ns ept\n");
		err = -ENOMEM;
		goto vqs_del;
	}

	notify = virtqueue_kick_prepare(vrp->rvq);
	virtio_device_ready(vdev);
	if (notify)
		virtqueue_notify(vrp->rvq);

	dev_info(&vdev->dev, "rpmsg host is online\n");

	return 0;

vqs_del:
	vdev->config->del_vqs(vrp->vdev);
free_vrp:
	kfree(vrp);
	return err;
}

static int hisp_vr_remove_device(struct device *dev, void *data)
{
	(void)data;
	device_unregister(dev);

	return 0;
}

static void hisp_vr_remove(struct virtio_device *vdev)
{
	struct hisp_vproc_info *vrp = vdev->priv;
	int ret;

	vdev->config->reset(vdev);

	ret = device_for_each_child(&vdev->dev, NULL, hisp_vr_remove_device);
	if (ret)
		dev_warn(&vdev->dev, "can't remove rpmsg device: %d\n", ret);

	if (vrp->ns_ept)
		__hisp_vr_destroy_ept(vrp, vrp->ns_ept);

	idr_destroy(&vrp->endpoints);
	vdev->config->del_vqs(vrp->vdev);
	kfree(vrp);
}

static struct virtio_device_id id_table[] = {
	{ VIRTIO_ID_HISP_RPMSG, VIRTIO_DEV_ANY_ID },
	{ 0 },
};

static unsigned int features[] = {
	VIRTIO_RPMSG_F_NS,
};

static struct virtio_driver hisp_vr_driver = {
	.feature_table	= features,
	.feature_table_size = ARRAY_SIZE(features),
	.driver.name	= "HISP VIRTIO RPMSG",
	.driver.owner	= THIS_MODULE,
	.id_table	= id_table,
	.probe		= hisp_vr_probe,
	.remove		= hisp_vr_remove,
};

int hisp_virtio_rpmsg_init(void)
{
	int ret;

	ret = register_virtio_driver(&hisp_vr_driver);
	if (ret)
		pr_err("failed to register virtio driver: %d\n", ret);

	return ret;
}

void hisp_virtio_rpmsg_exit(void)
{
	unregister_virtio_driver(&hisp_vr_driver);
}

MODULE_DESCRIPTION("HISP Virtio-based remote processor messaging bus");
MODULE_LICENSE("GPL v2");
