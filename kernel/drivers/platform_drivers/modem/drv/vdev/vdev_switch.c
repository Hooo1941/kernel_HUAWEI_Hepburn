/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <securec.h>
#include <mdrv_bmi_mci_switch.h>
#include <bsp_print.h>
#include <bsp_nva.h>
#include <nva_stru.h>
#include <nva_id.h>
#include "vdev.h"

#define THIS_MODU mod_vcom
#define MCI_SWITCH_SUPPORT_MAX_NUM 16
#define MCI_SWITCH_QUEUE_DEPTH_MAX 0xFFFF

struct mci_switch_debug_ctx {
    unsigned int ioctl;
    unsigned int ioctl_succ;
    unsigned int ioctl_fail;
    unsigned int read;
    unsigned int read_succ;
    unsigned int read_fail;
    unsigned int write;
    unsigned int write_succ;
    unsigned int write_fail;
    unsigned int poll;
    unsigned int queue;
    unsigned int queue_drop;
    unsigned int queue_fail;
    unsigned int queue_succ;
};

struct mci_switch_ctx {
    struct cdev cdev;
    struct device *dev;
    spinlock_t lock;
    wait_queue_head_t wait;
    struct list_head queue;
    unsigned int queue_num;
    unsigned int queue_depth;
    struct mutex read_mutex;
    struct mutex write_mutex;
    struct file_operations *fop;
    char *cdev_name;
    struct mci_switch_debug_ctx debug_ctx;
};

struct mci_switch_vb_node {
    struct vcom_buf *vb;
    struct list_head list;
};

enum mci_switch_dev_type {
    MCI_SWITCH_TE_TYPE,
    MCI_SWITCH_MT_TYPE,
    MCI_SWITCH_MAX_TYPE,
};

nv_mci_feature_s g_mci_nv_feature = { 0 };
ssize_t mci_switch_te_write(struct file *file, const char __user *data, size_t size, loff_t *loff);
ssize_t mci_switch_te_read(struct file *file, char __user *data, size_t size, loff_t *loff);
unsigned int mci_switch_te_poll(struct file *file, struct poll_table_struct *poll);
long mci_switch_te_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

ssize_t mci_switch_mt_write(struct file *file, const char __user *data, size_t size, loff_t *loff);
ssize_t mci_switch_mt_read(struct file *file, char __user *data, size_t size, loff_t *loff);
unsigned int mci_switch_mt_poll(struct file *file, struct poll_table_struct *poll);
long mci_switch_mt_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

static struct file_operations g_mci_switch_te_fops = {
    .owner = THIS_MODULE,
    .open = NULL,
    .write = mci_switch_te_write,
    .read = mci_switch_te_read,
    .poll = mci_switch_te_poll,
    .unlocked_ioctl = mci_switch_te_unlocked_ioctl,
    .compat_ioctl = mci_switch_te_unlocked_ioctl,
};

static struct file_operations g_mci_switch_mt_fops = {
    .owner = THIS_MODULE,
    .open = NULL,
    .write = mci_switch_mt_write,
    .read = mci_switch_mt_read,
    .poll = mci_switch_mt_poll,
    .unlocked_ioctl = mci_switch_mt_unlocked_ioctl,
    .compat_ioctl = mci_switch_mt_unlocked_ioctl,
};

struct mci_switch_ctx g_mci_switch_ctx[MCI_SWITCH_MAX_TYPE] = {
    [MCI_SWITCH_TE_TYPE] = {
        .fop = &g_mci_switch_te_fops,
        .cdev_name = "mci_te",
    },
    [MCI_SWITCH_MT_TYPE] = {
        .fop = &g_mci_switch_mt_fops,
        .cdev_name = "mci_mt"
    },
};

struct mci_cmd_map_item {
    unsigned int payload_type;
    enum mci_switch_cmd_type cmd_type;
};

struct mci_cmd_map_item g_mci_cmd_map_table[] = {
    {QUERY_READY, SWITCH_QUERY_READY},
    {NOTIFY_READY, SWITCH_NOTIFY_READY},
    {WRITE_ASYNC, SWITCH_TRANS_DATA},
    {WRITE_MSC, SWITCH_WRITE_MSC},
    {WRITE_FLOW_CTRL, SWITCH_WRITE_FLOW_CTRL},
    {WRITE_WLEN, SWITCH_WRITE_WLEN},
    {WRITE_STP, SWITCH_WRITE_STP},
    {WRITE_EPS, SWITCH_WRITE_EPS},
    {WRITE_BAUD, SWITCH_WRITE_BAUD},
    /* SWITCH_CLEAR_WRITE_BUFF 暂未实现 */
    {NOTIFY_WATER_LINE, SWITCH_NOTIFY_WATER_LINE},
    {NOTIFY_ESCAPE, SWITCH_NOTIFY_ESCAPE},
    {ENABLE_ESCAPE_DETECT, SWITCH_ENABLE_ESCAPE_DETECT},
    {SET_TRANS_MODE, SWITCH_SET_TRANS_MODE},
    {SET_PPP_INFO, SWITCH_SET_PPP_INFO},
    {ENABLE_CMUX, SWITCH_ENABLE_CMUX},
    {SET_CMUX_INFO, SWITCH_SET_CMUX_INFO},
    {SET_CMUX_DLC, SWITCH_SET_CMUX_DLC},
};

bool mci_switch_channel_support(unsigned int channel_id)
{
    struct vcom_hd *hd = NULL;

    hd = vcom_get_hd(channel_id);
    if (hd == NULL || hd->switch_support == false) {
        return false;
    }
    return true;
}

enum mci_switch_cmd_type mci_payload_type_trans_siwtch_type(unsigned int payload_type)
{
    unsigned int index;
    unsigned table_size = sizeof(g_mci_cmd_map_table) / sizeof(struct mci_cmd_map_item);
    for (index = 0; index < table_size; index++) {
        if (g_mci_cmd_map_table[index].payload_type == payload_type) {
            return g_mci_cmd_map_table[index].cmd_type;
        }
    }
    return SWITCH_TYPE_MAX;
}

unsigned int mci_siwtch_type_trans_payload_type(enum mci_switch_cmd_type cmd_type)
{
    unsigned int index;
    unsigned table_size = sizeof(g_mci_cmd_map_table) / sizeof(struct mci_cmd_map_item);
    for (index = 0; index < table_size; index++) {
        if (g_mci_cmd_map_table[index].cmd_type == cmd_type) {
            return g_mci_cmd_map_table[index].payload_type;
        }
    }

    return PAYLOAD_TYPE_MAX;
}

int mci_switch_vb_receive(struct mci_switch_ctx *ctx, struct vcom_hd *hd, struct vcom_buf *vb)
{
    unsigned long flags;
    struct mci_switch_vb_node *node = NULL;

    if (g_mci_nv_feature.switch_support == 0 || hd->switch_support == false) {
        return -EINVAL;
    }

    ctx->debug_ctx.queue++;
    if (ctx->queue_num > ctx->queue_depth) {
        ctx->debug_ctx.queue_drop++;
        kfree(vb);
        return 0;
    }

    node = (struct mci_switch_vb_node *)kzalloc(sizeof(struct mci_switch_vb_node), GFP_KERNEL);
    if (node == NULL) {
        VCOM_PRINT_ERROR("vb node alloc fail\n");
        ctx->debug_ctx.queue_fail++;
        kfree(vb);
        return 0;
    }

    node->vb = vb;
    spin_lock_irqsave(&ctx->lock, flags);
    list_add_tail(&node->list, &ctx->queue);
    ctx->queue_num++;
    spin_unlock_irqrestore(&ctx->lock, flags);

    wake_up_interruptible(&ctx->wait);
    ctx->debug_ctx.queue_succ++;
    return 0;
}

int mci_switch_te_vb_receive(struct vcom_hd *hd, struct vcom_buf *vb)
{
    struct mci_switch_ctx *ctx = &g_mci_switch_ctx[MCI_SWITCH_TE_TYPE];
    return mci_switch_vb_receive(ctx, hd, vb);
}

int mci_switch_mt_vb_receive(struct vcom_hd *hd, struct vcom_buf *vb)
{
    struct mci_switch_ctx *ctx = &g_mci_switch_ctx[MCI_SWITCH_MT_TYPE];
    return mci_switch_vb_receive(ctx, hd, vb);
}

int mci_switch_fill_vb(struct vcom_buf *vb, struct mci_switch_trans_msg *msg, const char __user *data)
{
    int ret;
    unsigned int payload_type;

    vb->dev_type = msg->channel_type;
    vb->id = msg->channel_id;
    payload_type = mci_siwtch_type_trans_payload_type(msg->cmd_type);
    if (payload_type == PAYLOAD_TYPE_MAX) {
        return -EINVAL;
    }

    vb->payload_type = payload_type;
    vb->payload_start = 0;
    vb->payload_len = msg->buffer_len;

    ret = copy_from_user(vb->data, data + sizeof(struct mci_switch_trans_msg), msg->buffer_len);
    if (ret) {
        VCOM_PRINT_ERROR("copy_from_user fail\n");
        return ret;
    }
    return 0;
}

struct vcom_buf *mci_switch_build_vb(const char __user *data, size_t size)
{
    int ret;
    unsigned int total_len;
    struct vcom_buf *vb = NULL;
    struct mci_switch_trans_msg msg = {0};

    if (size < sizeof(struct mci_switch_trans_msg) || size > VCOM_MAX_DATA_SIZE + sizeof(struct mci_switch_trans_msg)) {
        VCOM_PRINT_ERROR("user data size error\n");
        return NULL;
    }

    if (copy_from_user(&msg, data, sizeof(struct mci_switch_trans_msg))) {
        VCOM_PRINT_ERROR("copy from user fail\n");
        return NULL;
    }

    if (!mci_switch_channel_support(msg.channel_id)) {
        VCOM_PRINT_ERROR("channel id: %u not support\n", msg.channel_id);
        return NULL;
    }

    if (msg.buffer_len != size - sizeof(struct mci_switch_trans_msg)) {
        VCOM_PRINT_ERROR("channel id: %u buffer len check fail\n", msg.channel_id);
        return NULL;
    }

    total_len = sizeof(struct vcom_buf) + msg.buffer_len;
    vb = (struct vcom_buf *)kzalloc(total_len, GFP_KERNEL);
    if (vb == NULL) {
        VCOM_PRINT_ERROR("vb alloc fail\n");
        return NULL;
    }

    ret = mci_switch_fill_vb(vb, &msg, data);
    if (ret) {
        kfree(vb);
        return NULL;
    }
    return vb;
}

int mci_switch_te_vb_deliver(struct vcom_buf *vb)
{
    if (mci_msg_dispatch(vb) < 0) {
        kfree(vb);
        return -EINVAL;
    }

    return 0;
}

ssize_t mci_switch_te_write(struct file *file, const char __user *data, size_t size, loff_t *loff)
{
    int ret;
    struct vcom_buf *vb = NULL;
    struct mci_switch_ctx *ctx = &g_mci_switch_ctx[MCI_SWITCH_TE_TYPE];

    if (data == NULL || size == 0) {
        VCOM_PRINT_ERROR("param error\n");
        return -EINVAL;
    }

    ctx->debug_ctx.write++;
    mutex_lock(&ctx->write_mutex);
    vb = mci_switch_build_vb(data, size);
    if (vb == NULL) {
        mutex_unlock(&ctx->write_mutex);
        ctx->debug_ctx.write_fail++;
        return -EINVAL;
    }

    /* te侧下发流程接口 */
    ret = mci_switch_te_vb_deliver(vb);
    if (ret < 0) {
        mutex_unlock(&ctx->write_mutex);
        ctx->debug_ctx.write_fail++;
        return -EPERM;
    }

    mutex_unlock(&ctx->write_mutex);

    ctx->debug_ctx.write_succ++;
    return size;
}

int mci_switch_mt_vb_deliver(struct vcom_buf *vb)
{
    int ret;
    struct vcom_hd *hd = NULL;

    if ((hd = vcom_get_hd(vb->id)) == NULL) {
        kfree(vb);
        return -EINVAL;
    }

    ret = mci_msg_send(hd, vb);
    kfree(vb);
    return ret;
}

ssize_t mci_switch_mt_write(struct file *file, const char __user *data, size_t size, loff_t *loff)
{
    int ret;
    struct vcom_buf *vb = NULL;
    struct mci_switch_ctx *ctx = &g_mci_switch_ctx[MCI_SWITCH_MT_TYPE];

    if (data == NULL || size == 0) {
        VCOM_PRINT_ERROR("param error\n");
        return -EINVAL;
    }

    ctx->debug_ctx.write++;
    mutex_lock(&ctx->write_mutex);
    vb = mci_switch_build_vb(data, size);
    if (vb == NULL) {
        mutex_unlock(&ctx->write_mutex);
        ctx->debug_ctx.write_fail++;
        return -EINVAL;
    }

    /* mt侧下发流程接口 */
    ret = mci_switch_mt_vb_deliver(vb);
    if (ret < 0) {
        mutex_unlock(&ctx->write_mutex);
        ctx->debug_ctx.write_fail++;
        return -EINVAL;
    }

    mutex_unlock(&ctx->write_mutex);
    ctx->debug_ctx.write_succ++;
    return size;
}

bool mci_switch_msg_available(struct list_head *queue, spinlock_t *lock)
{
    unsigned long flags;
    bool ret;

    spin_lock_irqsave(lock, flags);
    ret = !list_empty(queue);
    spin_unlock_irqrestore(lock, flags);
    return ret;
}

int mci_switch_fill_msg_head(struct vcom_buf *vb, char __user *data)
{
    int ret;
    struct mci_switch_trans_msg msg = {0};
    enum mci_switch_cmd_type cmd_type;

    cmd_type = mci_payload_type_trans_siwtch_type(vb->payload_type);
    if (cmd_type == SWITCH_TYPE_MAX) {
        VCOM_PRINT_ERROR("payload type error\n");
        return -EINVAL;
    }

    msg.cmd_type = cmd_type;
    msg.channel_type = vb->dev_type;
    msg.channel_id = vb->id;
    msg.reserved = vb->custom;
    msg.buffer_len = vb->payload_len;
    ret = copy_to_user(data, &msg, sizeof(struct mci_switch_trans_msg));
    if (ret) {
        VCOM_PRINT_ERROR("copy to user fail\n");
        return ret;
    }
    return 0;
}

int mci_switch_fill_msg_body(struct vcom_buf *vb, char __user *data)
{
    int ret;

    ret = copy_to_user(data + sizeof(struct mci_switch_trans_msg), vb->data, vb->payload_len);
    if (ret) {
        VCOM_PRINT_ERROR("copy to user fail\n");
        return ret;
    }
    return 0;
}

ssize_t mci_switch_read(struct mci_switch_ctx *ctx, struct file *file, char __user *data, size_t size)
{
    ssize_t ret;
    unsigned long flags;
    struct mci_switch_vb_node *node = NULL;

    ctx->debug_ctx.read++;
    mutex_lock(&ctx->read_mutex);
    if (wait_event_interruptible(ctx->wait, mci_switch_msg_available(&ctx->queue, &ctx->lock))) {
        mutex_unlock(&ctx->read_mutex);
        VCOM_PRINT_ERROR("%s, wait_event_interruptible out\n", ctx->cdev_name);
        ctx->debug_ctx.read_fail++;
        return -ERESTARTSYS;
    }

    spin_lock_irqsave(&ctx->lock, flags);
    node = list_first_entry(&ctx->queue, struct mci_switch_vb_node, list);
    if (node == NULL) {
        spin_unlock_irqrestore(&ctx->lock, flags);
        mutex_unlock(&ctx->read_mutex);
        VCOM_PRINT_ERROR("%s, no vb node for read\n", ctx->cdev_name);
        ctx->debug_ctx.read_fail++;
        return -EINVAL;
    }
    list_del(&node->list);
    ctx->queue_num--;
    spin_unlock_irqrestore(&ctx->lock, flags);

    if (size < node->vb->payload_len + sizeof(struct mci_switch_trans_msg)) {
        ret = -EINVAL;
        ctx->debug_ctx.read_fail++;
        goto free_mem;
    }

    ret = mci_switch_fill_msg_head(node->vb, data);
    if (ret) {
        ctx->debug_ctx.read_fail++;
        goto free_mem;
    }

    ret = mci_switch_fill_msg_body(node->vb, data);
    if (ret) {
        ctx->debug_ctx.read_fail++;
        goto free_mem;
    }

    ret = node->vb->payload_len + sizeof(struct mci_switch_trans_msg);
    ctx->debug_ctx.read_succ++;
free_mem:
    kfree(node->vb);
    kfree(node);
    mutex_unlock(&ctx->read_mutex);
    return ret;
}

ssize_t mci_switch_te_read(struct file *file, char __user *data, size_t size, loff_t *loff)
{
    struct mci_switch_ctx *ctx = &g_mci_switch_ctx[MCI_SWITCH_TE_TYPE];
    if (data == NULL || size == 0) {
        VCOM_PRINT_ERROR("param error\n");
        return -EINVAL;
    }

    return mci_switch_read(ctx, file, data, size);
}

ssize_t mci_switch_mt_read(struct file *file, char __user *data, size_t size, loff_t *loff)
{
    struct mci_switch_ctx *ctx = &g_mci_switch_ctx[MCI_SWITCH_MT_TYPE];
    if (data == NULL || size == 0) {
        VCOM_PRINT_ERROR("param error\n");
        return -EINVAL;
    }

    return mci_switch_read(ctx, file, data, size);
}

unsigned int mci_switch_poll(struct mci_switch_ctx *ctx, struct file *file, struct poll_table_struct *poll)
{
    unsigned int mask = 0;
    unsigned long flags;

    poll_wait(file, &ctx->wait, poll);
    spin_lock_irqsave(&ctx->lock, flags);
    if (!list_empty(&ctx->queue)) {
        mask |= (POLLIN | POLLRDNORM);
    }
    spin_unlock_irqrestore(&ctx->lock, flags);
    return mask;
}

unsigned int mci_switch_te_poll(struct file *file, struct poll_table_struct *poll)
{
    struct mci_switch_ctx *ctx = &g_mci_switch_ctx[MCI_SWITCH_TE_TYPE];
    return mci_switch_poll(ctx, file, poll);
}

unsigned int mci_switch_mt_poll(struct file *file, struct poll_table_struct *poll)
{
    struct mci_switch_ctx *ctx = &g_mci_switch_ctx[MCI_SWITCH_MT_TYPE];
    return mci_switch_poll(ctx, file, poll);
}

long mci_switch_unlocked_ioctl(struct mci_switch_ctx *ctx, struct file *file, unsigned int cmd, unsigned long arg)
{
    long ret;
    struct mci_switch_ioctl_msg msg = { 0 };

    ctx->debug_ctx.ioctl++;
    if (file == NULL || arg == 0) {
        ctx->debug_ctx.ioctl_fail++;
        return -EINVAL;
    }

    ret = copy_from_user(&msg, (void *)(uintptr_t)arg, sizeof(struct mci_switch_ioctl_msg));
    if (ret) {
        ctx->debug_ctx.ioctl_fail++;
        VCOM_PRINT_ERROR("copy from user fail\n");
        return -ENOMEM;
    }

    if (!mci_switch_channel_support(msg.channel_id)) {
        ctx->debug_ctx.ioctl_fail++;
        return -EACCES;
    }

    ctx->debug_ctx.ioctl_succ++;
    return 0;
}

long mci_switch_te_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct mci_switch_ctx *ctx = &g_mci_switch_ctx[MCI_SWITCH_TE_TYPE];
    return mci_switch_unlocked_ioctl(ctx, file, cmd, arg);
}

long mci_switch_mt_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct mci_switch_ctx *ctx = &g_mci_switch_ctx[MCI_SWITCH_MT_TYPE];
    return mci_switch_unlocked_ioctl(ctx, file, cmd, arg);
}

int mci_switch_nv_init(nv_mci_feature_s *nv_feature)
{
    int ret;

    ret = bsp_nv_read(NVA_ID_DRV_MCI_FEATURE, (unsigned char *)nv_feature, sizeof(nv_mci_feature_s));
    if (ret) {
        VCOM_PRINT_ERROR("mci switch nv read fail\n");
        ret = memset_s(nv_feature, sizeof(nv_mci_feature_s), 0, sizeof(nv_mci_feature_s));
        if (ret) {
            VCOM_PRINT_ERROR("memset fail\n");
        }
    }

    return ret;
}

void mci_switch_dts_channel_init(const char *channel_names[MCI_SWITCH_SUPPORT_MAX_NUM], unsigned int names_num)
{
    unsigned int i;
    unsigned int j;
    unsigned int hd_num = vcom_get_hd_num();
    struct vcom_hd *hd = NULL;

    for (i = 0; i < hd_num; i++) {
        hd = vcom_get_hd_from_set(i);
        hd->switch_support = false;
        for (j = 0; j < names_num; j++) {
            if (strcmp(channel_names[j], hd->name) == 0) {
                hd->switch_support = true;
                break;
            }
        }
    }
}

int mci_switch_dts_init(void)
{
    int ret;
    unsigned int names_num;
    unsigned int queue_depth;
    struct device_node *np = NULL;
    const char *channel_names[MCI_SWITCH_SUPPORT_MAX_NUM] = {NULL};

    np = of_find_node_by_name(NULL, "mci_switch");
    if (np == NULL) {
        VCOM_PRINT_ERROR("mci switch dts get node fail\n");
        return -ENODEV;
    }

    ret = of_property_read_u32(np, "queue_depth", &queue_depth);
    if (ret) {
        VCOM_PRINT_ERROR("mci switch dts get queue depth fail\n");
        of_node_put(np);
        return -EINVAL;
    }

    if (queue_depth >= MCI_SWITCH_QUEUE_DEPTH_MAX) {
        VCOM_PRINT_ERROR("mci switch dts queue depth out of range\n");
        of_node_put(np);
        return -EINVAL;
    }

    g_mci_switch_ctx[MCI_SWITCH_TE_TYPE].queue_depth = queue_depth;
    g_mci_switch_ctx[MCI_SWITCH_MT_TYPE].queue_depth = queue_depth;

    ret = of_property_read_string_array(np, "support_channels", channel_names, ARRAY_SIZE(channel_names));
    if (ret <= 0) {
        VCOM_PRINT_ERROR("mci switch dts get channel names fail\n");
        of_node_put(np);
        return -EINVAL;
    }

    names_num = (unsigned int)ret;
    mci_switch_dts_channel_init(channel_names, names_num);
    of_node_put(np);
    return 0;
}

void mci_switch_ctx_init(enum mci_switch_dev_type type)
{
    struct mci_switch_ctx *ctx = &g_mci_switch_ctx[type];

    mutex_init(&ctx->read_mutex);
    mutex_init(&ctx->write_mutex);
    init_waitqueue_head(&ctx->wait);
    INIT_LIST_HEAD(&ctx->queue);
    spin_lock_init(&ctx->lock);
    ctx->queue_num = 0;
}

void mci_switch_unregister_device(struct class *class, dev_t dev, enum mci_switch_dev_type type)
{
    struct mci_switch_ctx *ctx = &g_mci_switch_ctx[type];
    device_destroy(class, dev + type);
    cdev_del(&ctx->cdev);
}

int mci_switch_register_device(struct class *class, dev_t dev, enum mci_switch_dev_type type)
{
    int ret;
    struct mci_switch_ctx *ctx = &g_mci_switch_ctx[type];

    cdev_init(&ctx->cdev, ctx->fop);
    ret = cdev_add(&ctx->cdev, dev + type, 1);
    if (ret) {
        VCOM_PRINT_ERROR("te cdev add fail\n");
        return ret;
    }

    ctx->dev = device_create(class, NULL, dev + type, NULL, ctx->cdev_name);
    if (IS_ERR(ctx->dev)) {
        cdev_del(&ctx->cdev);
        VCOM_PRINT_ERROR("device create fail\n");
        return -ENODEV;
    }
    return 0;
}

int mci_switch_cdev_init(void)
{
    int ret;
    dev_t dev;
    struct class *class;

    alloc_chrdev_region(&dev, 0, MCI_SWITCH_MAX_TYPE, "mci_switch");
    class = class_create(THIS_MODULE, "mci_switch");
    if (class == NULL) {
        VCOM_PRINT_ERROR("class create fail\n");
        return -ENODEV;
    }

    ret = mci_switch_register_device(class, dev, MCI_SWITCH_TE_TYPE);
    if (ret) {
        unregister_chrdev_region(dev, MCI_SWITCH_MAX_TYPE);
        return ret;
    }

    ret = mci_switch_register_device(class, dev, MCI_SWITCH_MT_TYPE);
    if (ret) {
        mci_switch_unregister_device(class, dev, MCI_SWITCH_TE_TYPE);
        unregister_chrdev_region(dev, MCI_SWITCH_MAX_TYPE);
        return ret;
    }
    return 0;
}

int mci_switch_init(void)
{
    int ret;

    ret = mci_switch_nv_init(&g_mci_nv_feature);
    if (ret) {
        return ret;
    }

    if (g_mci_nv_feature.switch_support == 0) {
        return 0;
    }

    ret = mci_switch_dts_init();
    if (ret) {
        return ret;
    }

    ret = mci_switch_cdev_init();
    if (ret) {
        return ret;
    }

    mci_switch_ctx_init(MCI_SWITCH_TE_TYPE);
    mci_switch_ctx_init(MCI_SWITCH_MT_TYPE);
    return 0;
}

void mci_switch_debug_show(void)
{
    enum mci_switch_dev_type type;
    struct mci_switch_ctx *ctx = NULL;

    for (type = 0; type < MCI_SWITCH_MAX_TYPE; type++) {
        ctx = &g_mci_switch_ctx[type];
        bsp_err("debug start: %s\n", ctx->cdev_name);
        bsp_err("queue_depth:%u\n", ctx->queue_depth);
        bsp_err("queue_num:  %u\n", ctx->queue_num);
        bsp_err("ioctl:      %u\n", ctx->debug_ctx.ioctl);
        bsp_err("ioctl_fail: %u\n", ctx->debug_ctx.ioctl_fail);
        bsp_err("ioctl_succ: %u\n", ctx->debug_ctx.ioctl_succ);
        bsp_err("read:       %u\n", ctx->debug_ctx.read);
        bsp_err("read_fail:  %u\n", ctx->debug_ctx.read_fail);
        bsp_err("read_succ:  %u\n", ctx->debug_ctx.read_succ);
        bsp_err("write:      %u\n", ctx->debug_ctx.write);
        bsp_err("write_fail: %u\n", ctx->debug_ctx.write_fail);
        bsp_err("write_succ: %u\n", ctx->debug_ctx.write_succ);
        bsp_err("queue:      %u\n", ctx->debug_ctx.queue);
        bsp_err("queue_drop: %u\n", ctx->debug_ctx.queue_drop);
        bsp_err("queue_fail: %u\n", ctx->debug_ctx.queue_fail);
        bsp_err("queue_succ: %u\n", ctx->debug_ctx.queue_succ);
        bsp_err("debug end:   %s\n", ctx->cdev_name);
    }
}
