/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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
#include <linux/errno.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/poll.h>
#include <securec.h>
#include <bsp_print.h>
#include <bsp_mci.h>
#include "vdev.h"

#define THIS_MODU mod_vcom
struct vcom_bindpid_ctx_s g_vcom_bindpid_ctx = {{0}};

int notify_netmanager(struct vcom_map_s *map, char *buf, size_t size, int flag)
{
    struct vcom_app_ctx *app_ctx = NULL;
    struct vcom_cdev *vc = NULL;
    struct vcom_buf *vb = NULL;
    int ret;
    unsigned int len;
    VCOM_PRINT("start\n");
    app_ctx = get_vcom_app_ctx();
    vc = &app_ctx->vc[VCOM_APP_NMCTRL];
    if ((size > 0xffff) || (size == 0)) {
        VCOM_PRINT_ERROR("buf size err, size = %zu\n", size);
        return -1;
    }
    len = sizeof(struct vcom_buf) + size;
    vb = (struct vcom_buf *)kmalloc(len, GFP_KERNEL);
    if (vb == NULL) {
        VCOM_PRINT_ERROR("kmalloc failed\n");
        return -ENOMEM;
    }

    vb->payload_start = 0;
    vb->payload_len = size;
    if (memcpy_s(vb->data, size, buf, size) != EOK) {
        VCOM_PRINT_ERROR("memcpy_s failed\n");
        kfree(vb);
        return -EIO;
    }
    ret = map->link_ops->enqueue(map, (unsigned char *)vb);
    VCOM_PRINT("notify_netmanager enqueue succ\n"); // debug_info, need to delete
    if (!ret) {
        wake_up_interruptible(&vc->wait);
    }
    VCOM_PRINT("notify_netmanager wake_up_interruptible succ\n"); // debug_info, need to delete
    return ret;
}

ssize_t vdev_write_bindpid(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
    ssize_t ret;
    struct vcom_hd *nmctrl_hd = NULL;

    VCOM_PRINT("count = %zu\n", count);

    if (filp == NULL) {
        return -ENOENT;
    }

    if ((buf == NULL) || (ppos == NULL)) {
        return -EFAULT;
    }

    if (*ppos > 0) {
        VCOM_PRINT_ERROR("only allow write from start.");
        return 0;
    }

    if (count >= VCOM_PID_LEN || !count) {
        VCOM_PRINT_ERROR("pid size illegal, count = %zu\n", count);
        return -EIO;
    }

    nmctrl_hd = vcom_get_hd(VCOM_APP_NMCTRL);

    mutex_lock(&g_vcom_bindpid_ctx.write_lock);
    if (copy_from_user(g_vcom_bindpid_ctx.pid, buf, count)) {
        mutex_unlock(&g_vcom_bindpid_ctx.write_lock);
        return -EFAULT;
    }
    if (kstrtoint(g_vcom_bindpid_ctx.pid, VCOM_PID_LEN, &g_vcom_bindpid_ctx.pid_msg.ulBindPid)) {
        pr_err("trtoi invalid\n");
        mutex_unlock(&g_vcom_bindpid_ctx.write_lock);
        return -EIO;
    }
    VCOM_PRINT("ulBindPid:%u\n", g_vcom_bindpid_ctx.pid_msg.ulBindPid);
    VCOM_PRINT("sizeof(g_vcom_bindpid_ctx.pid_msg):%zu\n", sizeof(g_vcom_bindpid_ctx.pid_msg));
    g_vcom_bindpid_ctx.pid_msg.enMsgId = VCOM_BIND_PID_CONFIG_IND;
    g_vcom_bindpid_ctx.pid_msg.ulMsgLen = sizeof(unsigned int);
    ret = notify_netmanager(nmctrl_hd->map, (char*)&g_vcom_bindpid_ctx.pid_msg, sizeof(g_vcom_bindpid_ctx.pid_msg), 1);
    mutex_unlock(&g_vcom_bindpid_ctx.write_lock);
    if (ret) {
        return 0;
    } else {
        *ppos += sizeof(g_vcom_bindpid_ctx.pid_msg);
        return sizeof(g_vcom_bindpid_ctx.pid_msg);
    }
}

static struct proc_ops g_vcom_bindpid_fops = {
    .proc_open = 0,
    .proc_write = vdev_write_bindpid,
    .proc_read=  0,
    .proc_release = 0,
    .proc_poll = 0,
};

int vcom_hd_bindpid_init(struct vcom_hd *hd)
{
    if (!proc_create_data(hd->name, S_IWUSR | S_IWGRP, 0, &g_vcom_bindpid_fops, hd)) {
        VCOM_PRINT_ERROR("proc_create_data err\n");
        return -1;
    }

    mutex_init(&g_vcom_bindpid_ctx.write_lock);
    return 0;
}
