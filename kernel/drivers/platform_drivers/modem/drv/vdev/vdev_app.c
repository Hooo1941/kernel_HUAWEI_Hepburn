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
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/poll.h>
#include <mdrv_timer.h>
#include <securec.h>
#include <bsp_print.h>
#include <bsp_mci.h>
#include <bsp_chr.h>
#include "vdev.h"

struct vcom_app_ctx g_vcom_app = {0};
#define THIS_MODU mod_vcom
#define MAX_U64_STR_SIZE    16
#define MAX_VDEV_SIZE   256
#define MCI_AT_OFFSET 2 // /r/n无用字符

static struct mci_ind_data g_mci_ind_data = {0};
static unsigned int g_mci_chr_index = 0;

static struct vcom_hd *get_hd(struct file *file)
{
    return vcom_get_hd(file->f_inode->i_cdev->dev - g_vcom_app.devt);
}

static struct vcom_cdev *get_vcom_cdev(struct file *file)
{
    if ((file->f_inode->i_cdev->dev - g_vcom_app.devt) >= VCOM_APP_BOTTOM) {
        return NULL;
    }
    return &g_vcom_app.vc[file->f_inode->i_cdev->dev - g_vcom_app.devt];
}

struct vcom_app_ctx *get_vcom_app_ctx(void)
{
    return &g_vcom_app;
}

ssize_t vdev_cdev_write(struct file *file, const char __user *data, size_t size, loff_t *loff)
{
    int ret;
    struct vcom_hd *hd = NULL;
    struct vcom_cdev *vc = NULL;

    VCOM_TRACE(g_vcom_debug);
    hd = get_hd(file);
    if (hd == NULL) {
        VCOM_PRINT_ERROR("hd is NULL\n");
        return 0;
    }

    mci_debug_record(hd, data, size, VCOM_USER_TYPE, 1);

    vc = get_vcom_cdev(file);
    if (vc == NULL) {
        VCOM_PRINT_ERROR("vc is NULL\n");
        return 0;
    }

    if (size > VCOM_MAX_DATA_SIZE) {
        return 0;
    }
    mutex_lock(&vc->write_lock);
    ret = vcom_write(hd, data, size, WRITE_ASYNC);
    mutex_unlock(&vc->write_lock);
    if (ret) {
        return 0;
    } else {
        return size;
    }
}

void vdev_app_record(struct vcom_hd *hd, struct vcom_buf *vb, unsigned int size)
{
    unsigned int record_size;

    record_size = size > MCI_RECORD_LEN - 1 ? MCI_RECORD_LEN - 1 : size;
    if (memcpy_s(hd->map->stax.app_record, MCI_RECORD_LEN - 1,
        &vb->data[vb->payload_start], record_size) != EOK) {
        hd->map->stax.app_size = 0;
        VCOM_PRINT_ERROR("write record fail\n");
    } else {
        hd->map->stax.app_size = record_size;
    }
}

ssize_t vdev_read_data(struct file *file, char __user *data, size_t size,
    struct vcom_buf *vb, unsigned int len)
{
    struct vcom_hd *hd = get_hd(file);
    struct vcom_cdev *vc = get_vcom_cdev(file);

    if (len > size) { // Multiple copies
        vdev_app_record(hd, vb, size);
        if (copy_to_user(data, &vb->data[vb->payload_start], size)) {
            VCOM_PRINT_ERROR("map %s, copy_to_user out\n", hd->name);
            hd->map->stax.copy_fail++;
            goto read_fail;
        }
        vb->payload_start += size;
        vb->payload_len -= size;
        hd->map->stax.remain++;
        if (vcom_requeue(hd, vb) < 0) {
            VCOM_PRINT_ERROR("map %s, requeue fail\n", hd->name);
            hd->map->stax.req_fail++;
            goto read_fail;
        }
        wake_up_interruptible(&vc->wait);
        return size;
    } else { // Single copy
        vdev_app_record(hd, vb, len);
        if (copy_to_user(data, &vb->data[vb->payload_start], len)) {
            hd->map->stax.copy_fail++;
            VCOM_PRINT_ERROR("map %s, copy_to_user out\n", hd->name);
            goto read_fail;
        }
        hd->map->stax.succ_free++;
        kfree(vb);
        return len;
    }

read_fail:
    hd->map->stax.fail_free++;
    kfree(vb);
    return 0;
}

ssize_t vdev_cdev_read(struct file *file, char __user *data, size_t size, loff_t *loff)
{
    struct vcom_hd *hd = NULL;
    struct vcom_buf *vb = NULL;
    struct vcom_cdev *vc = NULL;
    unsigned int len;
    int ret;

    VCOM_TRACE(g_vcom_debug);
    hd = get_hd(file);
    if (hd == NULL) {
        VCOM_PRINT_ERROR("hd is NULL\n");
        return 0;
    }
    vc = get_vcom_cdev(file);
    if (vc == NULL) {
        VCOM_PRINT_ERROR("vc is NULL\n");
        return 0;
    }
    VCOM_PRINT("size: %lu\n", size);
    VCOM_PRINT("hd name: %s\n", vc->hd->name);
    mutex_lock(&vc->read_lock);
    hd->map->stax.read++;
    if (wait_event_interruptible(vc->wait, !(hd->map->link_ops->idle(hd->map)))) {
        VCOM_PRINT("wait_event_interruptible out\n");
        mutex_unlock(&vc->read_lock);
        return -ERESTARTSYS;
    }

    vb = (struct vcom_buf *)vcom_read(hd, &len);
    if (vb == NULL) {
        VCOM_PRINT_ERROR("map %s, mci read null\n", hd->name);
        mutex_unlock(&vc->read_lock);
        return 0;
    }
    VCOM_PRINT("map %s, mci read len: %u\n", hd->name, len);
    ret = vdev_read_data(file, data, size, vb, len);
    mutex_unlock(&vc->read_lock);
    mci_debug_record(hd, data, ret, VCOM_USER_TYPE, ret == 0 ? 0 : 1);
    return ret;
}

unsigned int vdev_cdev_poll(struct file *file, struct poll_table_struct *poll)
{
    struct vcom_hd *hd = NULL;
    struct vcom_cdev *vc = NULL;

    hd = get_hd(file);
    if (hd == NULL) {
        VCOM_PRINT_ERROR("hd is NULL\n");
        return 0;
    }
    vc = get_vcom_cdev(file);
    if (vc == NULL) {
        VCOM_PRINT_ERROR("vc is NULL\n");
        return 0;
    }

    poll_wait(file, &vc->wait, poll);
    if (hd->map->link_ops->idle(hd->map)) {
        VCOM_PRINT("map %s, mci idle\n", hd->name);
        return 0;
    } else {
        VCOM_PRINT("map %s, mci not idle\n", hd->name);
        return (POLLIN | POLLRDNORM);
    }
}


static struct file_operations g_vcom_cdev_fops = {
    .owner = THIS_MODULE,
    .open = 0,
    .write = vdev_cdev_write,
    .read = vdev_cdev_read,
    .release = 0,
    .poll = vdev_cdev_poll,
    .unlocked_ioctl = 0,
};

int vdev_cdev_init(void)
{
    unsigned int size;

    VCOM_PRINT_ERROR("[init] cdev start\n");
    VCOM_TRACE(g_vcom_debug);
    size = vcom_get_hd_num();
    alloc_chrdev_region(&g_vcom_app.devt, 0, MAX_VDEV_SIZE, "vdev_app");
    g_vcom_app.class = class_create(THIS_MODULE, "vdev_app");
    if (IS_ERR(g_vcom_app.class)) {
        VCOM_PRINT_ERROR("device create err\n");
        return -1;
    }

    g_vcom_app.vc = kzalloc(VCOM_APP_BOTTOM * sizeof(struct vcom_cdev), GFP_KERNEL);
    if (g_vcom_app.vc == NULL) {
        VCOM_PRINT_ERROR("kzalloc err\n");
        return -1;
    }
    g_vcom_app.fop = &g_vcom_cdev_fops;
    VCOM_PRINT_ERROR("[init] cdev ok\n");
    return 0;
}

struct device *vcom_create_dev(struct cdev *cdev, char *name, unsigned int id, void *priv)
{
    if (cdev_add(cdev, g_vcom_app.devt + id, 1)) {
        return NULL;
    }
    return device_create(g_vcom_app.class, NULL, g_vcom_app.devt + id, priv, name);
}

int vcom_hd_app_init(struct vcom_hd *hd)
{
    int ret;
    int id;

    VCOM_TRACE(g_vcom_debug);
    id = hd->dev_id;
    g_vcom_app.vc[id].hd = hd;
    mutex_init(&g_vcom_app.vc[id].write_lock);
    mutex_init(&g_vcom_app.vc[id].read_lock);
    init_waitqueue_head(&g_vcom_app.vc[id].wait);
    cdev_init(&g_vcom_app.vc[id].cdev, g_vcom_app.fop);
    ret = cdev_add(&g_vcom_app.vc[id].cdev, g_vcom_app.devt + id, 1);
    if (ret) {
        VCOM_PRINT_ERROR("cdev_add err\n");
        return -1;
    }

    g_vcom_app.vc[id].dev = device_create(g_vcom_app.class, NULL, g_vcom_app.devt + id, hd, hd->name);
    if (IS_ERR(g_vcom_app.vc[id].dev)) {
        VCOM_PRINT_ERROR("device create err\n");
        return -1;
    }
    return 0;
}
ssize_t vdev_getslice_read(struct file *file, char __user *data, size_t size, loff_t *loff)
{
    unsigned int val_l, val_h;
    unsigned long long val;
    ssize_t len;
    unsigned char buf[MAX_U64_STR_SIZE] = {0};
    if (loff == NULL || *loff > 0) {
        return 0;
    }
    if (mdrv_timer_get_accuracy_timestamp(&val_h, &val_l)) {
        VCOM_PRINT_ERROR("timerstamp error\n");
        return 0;
    }
    val = ((unsigned long long)val_h << 32) + val_l;
    len = sprintf_s(buf, MAX_U64_STR_SIZE, "%llx", val);
    if (len == -1) {
        VCOM_PRINT_ERROR("sprintf_s failed\n");
        return 0;
    }
    if (size < len) {
        VCOM_PRINT_ERROR("size:%ld too small\n", size);
        return 0;
    }
    if (copy_to_user(data, buf, len)) {
        VCOM_PRINT_ERROR("copy to user fail\n");
        return 0;
    }
    *loff = len;
    return len;
}

static struct proc_ops g_vcom_getslice_fops = {
    //.owner = THIS_MODULE,
    .proc_read = vdev_getslice_read,
};

struct proc_dir_entry *vcom_proc_init(void)
{
    return proc_mkdir("vdev_hmi", NULL);
}

int bsp_mci_proc_create(const char *name, umode_t mode, struct proc_ops *ops)
{
    struct proc_dir_entry *dir = NULL;
    struct proc_dir_entry *ret_dir = NULL;

    if (name == NULL || ops == NULL) {
        VCOM_PRINT_ERROR("proc para is null\n");
        return -1;
    }

    if (g_vcom_app.dir == NULL) {
        dir = vcom_proc_init();
        if (dir == NULL) {
            VCOM_PRINT_ERROR("make proc dir fail\n");
            return -1;
        }
        g_vcom_app.dir = dir;
    }

    ret_dir = proc_create(name, mode, g_vcom_app.dir, ops);
    if (ret_dir == NULL) {
        VCOM_PRINT_ERROR("make proc node fail\n");
        return -1;
    }

    return 0;
}

int vcom_hd_proc_init(struct vcom_hd *hd)
{
    int ret;

    if (!proc_create_data(hd->name, S_IRUSR | S_IRGRP, 0, &g_vcom_getslice_fops, hd)) {
        VCOM_PRINT_ERROR("proc_create_data err\n");
        return -1;
    }

    ret = bsp_mci_proc_create(hd->name, S_IRUSR | S_IRGRP, &g_vcom_getslice_fops);
    if (ret) {
        VCOM_PRINT_ERROR("proc create err\n");
    }
    return 0;
}

static void vcom_app_chr_record(struct vcom_buf *vb)
{
    unsigned int index;
    unsigned int record_length;
    unsigned char *at_cmd = vb->data;
    unsigned int at_length = vb->payload_len;
    struct vcom_debug_stax *debug = vcom_get_debug_stax();

    if (at_length <= MCI_AT_OFFSET) {
        return;
    }

    record_length = at_length - MCI_AT_OFFSET > MCI_CHR_RECORD_SIZE - 1 ? MCI_CHR_RECORD_SIZE - 1 :  at_length - MCI_AT_OFFSET;
    debug->chr_record++;
    for (index = 0; index < g_mci_chr_index; index++) {
        if (strncmp(g_mci_ind_data.items[index].at_report, at_cmd + MCI_AT_OFFSET, record_length) == 0) {
            g_mci_ind_data.items[index].count++;
            return;
        }
    }

    if (g_mci_chr_index < MCI_CHR_RECORD_NUM) {
        memcpy_s(g_mci_ind_data.items[g_mci_chr_index].at_report, MCI_CHR_RECORD_SIZE, at_cmd + MCI_AT_OFFSET, record_length);
        g_mci_ind_data.items[g_mci_chr_index].count++;
        g_mci_chr_index++;
    }
}

static void vcom_app_chr_clear(void)
{
    g_mci_chr_index = 0;
    memset_s(&g_mci_ind_data, sizeof(g_mci_ind_data), 0, sizeof(g_mci_ind_data));
}

chr_ind_info_s *vcom_app_alloc_ind_info(unsigned int raw_len)
{
    unsigned int size;
    chr_ind_info_s *ind_info = NULL;

    size = sizeof(chr_ind_info_s) + sizeof(chr_data_header_s) + raw_len;
    ind_info = (chr_ind_info_s *)kzalloc(size, GFP_KERNEL);
    if (ind_info == NULL) {
        VCOM_PRINT_ERROR("kmalloc failed\n");
    }
    return ind_info;
}

static void vcom_app_chr_msg_encode(chr_ind_info_s *ind_info, chr_req_trans_s *req_trans, unsigned int raw_len)
{
    ind_info->msg_type = OM_ERR_LOG_MSG_ERR_REPORT;
    ind_info->msg_sn = req_trans->msg_sn;
    ind_info->msg_len = raw_len + sizeof(chr_data_header_s);
}

static void vcom_app_chr_info_head_encode(chr_ind_info_s *ind_info, chr_req_trans_s *req_trans, unsigned int raw_len)
{
    ind_info->report_head.msg_name = req_trans->msg_name;
    ind_info->report_head.module_id = req_trans->module_id;
    ind_info->report_head.alarm_id = req_trans->alarm_id;
    ind_info->report_head.alarm_type = OM_NO_DIRECT_REPORT;
    vcom_app_chr_msg_encode(ind_info, req_trans, raw_len);
}

static void vcom_app_chr_data_head_encode(chr_ind_info_s *ind_info, chr_req_trans_s *req_trans, unsigned int raw_len)
{
    chr_data_header_s *data_header = (chr_data_header_s *)ind_info->data;

    data_header->moduleid = req_trans->module_id;
    data_header->modemid = 0;
    data_header->alarm_level = 0x2; /* 0x2, Major level */
    data_header->alarmid = req_trans->alarm_id;
    data_header->alarm_type = OM_NO_DIRECT_REPORT;
    data_header->alarm_len = raw_len;
}

static int vcom_app_chr_data_content_fill(chr_ind_info_s *ind_info, unsigned int raw_len)
{
    int ret;
    chr_data_header_s *data_header = (chr_data_header_s *)ind_info->data;

    ret = memcpy_s(data_header->data, raw_len, &g_mci_ind_data, raw_len);
    if (ret) {
        VCOM_PRINT_ERROR("memcpy failed\n");
    }
    return ret;
}

int vcom_app_chr_report(struct vcom_buf *vb)
{
    int ret;
    chr_ind_info_s *ind_info = NULL;
    chr_req_trans_s *req_trans = NULL;
    unsigned int raw_len;
    struct vcom_debug_stax *debug = vcom_get_debug_stax();

    if (vb->payload_len != sizeof(chr_req_trans_s)) {
        VCOM_PRINT_ERROR("payload length mismatch\n");
        return -1;
    }

    req_trans = (chr_req_trans_s *)vb->data;
    raw_len = sizeof(g_mci_ind_data);
    ind_info = vcom_app_alloc_ind_info(raw_len);
    if (ind_info == NULL) {
        kfree(vb);
        return 0;
    }

    vcom_app_chr_info_head_encode(ind_info, req_trans, raw_len);
    vcom_app_chr_data_head_encode(ind_info, req_trans, raw_len);
    ret = vcom_app_chr_data_content_fill(ind_info, raw_len);
    if (ret) {
        goto free_mem;
    }

    ret = bsp_chr_report(ind_info);
    if (ret) {
        VCOM_PRINT_ERROR("chr report error, %d\n", ret);
    }
    vcom_app_chr_clear();
    debug->chr_report++;

free_mem:
    kfree(ind_info);
    kfree(vb);
    return 0;
}

int mci_app_write_async(struct vcom_map_s *map, struct vcom_buf *vb)
{
    int ret;
    struct vcom_cdev *vc = NULL;

    if (map->hd->wakeup) {
        vcom_app_chr_record(vb);
    }

    vc = &g_vcom_app.vc[vb->id];
    vb->payload_start = 0; /* init payload start offset as 0 */
    ret = map->link_ops->enqueue(map, (unsigned char *)vb);
    wake_up_interruptible(&vc->wait);
    VCOM_PRINT("app enqueue\n");
    return ret;
}

int mci_app_query_ready(struct vcom_buf *vb)
{
    int ret;
    struct vcom_cdev *vc = NULL;
    unsigned int ready_status;

    vc = &g_vcom_app.vc[vb->id];
    kfree(vb);
    ready_status = vc->ready ? 1 : 0;
    ret = vcom_write(vc->hd, &ready_status, sizeof(ready_status), NOTIFY_READY);
    if (ret < 0) {
       VCOM_PRINT_ERROR("notify ready status fail\n");
    }
    return 0;
}

int vcom_app_op(struct vcom_buf *vb)
{
    struct vcom_map_s *map = NULL;

    VCOM_TRACE(g_vcom_debug);

    map = vcom_get_map(vb->id);
    VCOM_PRINT("map: %s\n", map->hd->name);
    if (vb->id >= VCOM_APP_BOTTOM) {
        VCOM_PRINT_ERROR("app id:%u out of range\n", vb->id);
        return -1;
    }

    switch (vb->payload_type) {
        case WRITE_ASYNC:
            return mci_app_write_async(map, vb);
        case QUERY_READY:
            return mci_app_query_ready(vb);
        default:
            VCOM_PRINT_ERROR("app id:%u error payload type\n", vb->id);
            kfree(vb);
            return 0;
    }
}

