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
#include <linux/uaccess.h>
#include <linux/jiffies.h>
#include <securec.h>
#include <bsp_mci.h>
#include <bsp_print.h>
#include "vdev.h"

#define THIS_MODU mod_vcom
#define ATP_DEV_T   100   /* ATP dev_t */

#define ATP_MOD_MAX_NUM 10

struct atp_mod_proc {
    unsigned long timeout_jiff;
    unsigned int data_max_size;
    bsp_mci_atp_mod_callback func;
};

struct atp_mod_ctx {
    unsigned int id;
    struct atp_mod_proc proc;
};

struct mci_atp_mod_pam {
    struct atp_mod_proc *proc;
    unsigned char *buffer;
    unsigned int len;
    int ret;
};

struct mci_atp_ctx {
    struct vcom_cdev dev;
    struct workqueue_struct *wq;
    struct work_struct ioctl_work;
    struct completion ioctl_comp;

    struct atp_mod_ctx mod_sets[ATP_MOD_MAX_NUM];
    unsigned int mod_cnt;
    struct mci_atp_mod_pam mod_proc_pam;
};

struct mci_atp_debug_ctx {
    unsigned int msg_len_err;
    unsigned int msg_buf_err;
    unsigned int unregister;
    unsigned int timeout;
    unsigned int no_mem;
};

struct mci_atp_debug_ctx g_mci_atp_debug_ctx = { 0 };
struct mci_atp_ctx g_mci_atp_ctx = { 0 };

int bsp_mci_atp_mod_register(unsigned int id, bsp_mci_atp_mod_callback func, unsigned int timeout_ms,
    unsigned int data_max_size)
{
    struct mci_atp_ctx *pctx = &g_mci_atp_ctx;

    if (pctx->mod_cnt >= ATP_MOD_MAX_NUM) {
        VCOM_PRINT_ERROR("atp mod is full\n");
        return -1;
    }

    if (func == NULL || timeout_ms == 0 || data_max_size == 0) {
        VCOM_PRINT_ERROR("pam error\n");
        return -1;
    }

    pctx->mod_sets[pctx->mod_cnt].id = id;
    pctx->mod_sets[pctx->mod_cnt].proc.func = func;
    pctx->mod_sets[pctx->mod_cnt].proc.timeout_jiff = msecs_to_jiffies(timeout_ms);
    pctx->mod_sets[pctx->mod_cnt].proc.data_max_size = data_max_size;
    pctx->mod_cnt++;

    return 0;
}

struct atp_mod_proc *mci_atp_get_mod(unsigned int id)
{
    unsigned int i;
    struct mci_atp_ctx *pctx = &g_mci_atp_ctx;

    for (i = 0; i < pctx->mod_cnt; i++) {
        if (pctx->mod_sets[i].id == id) {
            return &pctx->mod_sets[i].proc;
        }
    }

    return NULL;
}

static int mci_atp_get_msg(struct mci_atp_msg *msg, unsigned long data, unsigned int data_max_size)
{
    int ret;

    ret = copy_from_user(msg, (void *)(uintptr_t)data, sizeof(struct mci_atp_msg));
    if (ret) {
        VCOM_PRINT_ERROR("copy_from_user failed\n");
        return -EIO;
    }

    if (msg->len == 0 || msg->len > data_max_size) {
        g_mci_atp_debug_ctx.msg_len_err++;
        VCOM_PRINT_ERROR("msg data len error\n");
        return -EIO;
    }

    if (msg->buffer == NULL) {
        g_mci_atp_debug_ctx.msg_buf_err++;
        VCOM_PRINT_ERROR("msg buffer is NULL\n");
        return -EIO;
    }

    return 0;
}

static void mci_atp_ioctl_work(struct work_struct *work)
{
    struct mci_atp_ctx *pctx = container_of(work, struct mci_atp_ctx, ioctl_work);
    struct mci_atp_mod_pam *mod_pam = &(pctx->mod_proc_pam);

    if (mod_pam->proc->func == NULL || mod_pam->buffer == NULL || mod_pam->len == 0) {
        return;
    }
    mod_pam->ret = mod_pam->proc->func(mod_pam->buffer, mod_pam->len);

    complete(&(pctx->ioctl_comp));
}

static int mci_atp_proc_func(struct mci_atp_ctx *pctx, struct mci_atp_mod_pam *mod_pam)
{
    int ret;

    (void)memcpy_s(&(pctx->mod_proc_pam), sizeof(struct mci_atp_mod_pam), mod_pam, sizeof(struct mci_atp_mod_pam));
    queue_work(pctx->wq, &(pctx->ioctl_work));

    if (wait_for_completion_timeout(&(pctx->ioctl_comp), mod_pam->proc->timeout_jiff) == 0) {
        g_mci_atp_debug_ctx.timeout++;
        VCOM_PRINT_ERROR("atp ioctl queue work timeout\n");
        ret = -1;
        goto exit;
    }
    ret = pctx->mod_proc_pam.ret;

exit:
    (void)memset_s(&(pctx->mod_proc_pam), sizeof(struct mci_atp_mod_pam), 0, sizeof(struct mci_atp_mod_pam));

    return ret;
}

static long mci_atp_ioctl(struct file *file __attribute__((unused)), unsigned int cmd, unsigned long data)
{
    int ret;
    struct mci_atp_msg msg;
    struct mci_atp_ctx *pctx = &g_mci_atp_ctx;
    struct mci_atp_mod_pam mod_pam;

    if (data == (unsigned long)NULL) {
        VCOM_PRINT_ERROR("data is NULL\n");
        return -EIO;
    }

    mod_pam.proc = mci_atp_get_mod(cmd);
    if (mod_pam.proc == NULL) {
        g_mci_atp_debug_ctx.unregister++;
        VCOM_PRINT_ERROR("module not register\n");
        return -EIO;
    }

    if (mci_atp_get_msg(&msg, data, mod_pam.proc->data_max_size)) {
        return -EIO;
    }
    mod_pam.len = msg.len;

    mod_pam.buffer = kmalloc(mod_pam.len, GFP_KERNEL);
    if (mod_pam.buffer == NULL) {
        g_mci_atp_debug_ctx.no_mem++;
        return -ENOMEM;
    }

    ret = copy_from_user(mod_pam.buffer, msg.buffer, mod_pam.len);
    if (ret) {
        VCOM_PRINT_ERROR("copy_from_user failed\n");
        goto exit;
    }

    mutex_lock(&pctx->dev.write_lock);
    ret = mci_atp_proc_func(pctx, &mod_pam);
    mutex_unlock(&pctx->dev.write_lock);

    if (copy_to_user(msg.buffer, mod_pam.buffer, mod_pam.len)) {
        VCOM_PRINT_ERROR("copy_to_user failed\n");
        ret = -1;
        goto exit;
    }

exit:
    kfree(mod_pam.buffer);
    return ret;
}

struct file_operations g_mci_atp_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = mci_atp_ioctl,
};

int mci_atp_init(struct vcom_hd *hd)
{
    unsigned int id = ATP_DEV_T;
    struct vcom_cdev *dev = &(g_mci_atp_ctx.dev);

    g_mci_atp_ctx.wq = create_singlethread_workqueue("mci_atp_wq");
    if (g_mci_atp_ctx.wq == NULL) {
        VCOM_PRINT_ERROR("create work queue fail!");
        return -1;
    }
    INIT_WORK(&g_mci_atp_ctx.ioctl_work, mci_atp_ioctl_work);
    init_completion(&g_mci_atp_ctx.ioctl_comp);

    mutex_init(&dev->write_lock);
    cdev_init(&dev->cdev, &g_mci_atp_fops);
    dev->dev = vcom_create_dev(&dev->cdev, hd->name, id, hd);
    if (IS_ERR(dev->dev)) {
        VCOM_PRINT_ERROR("device create err\n");
        return -1;
    }

    return 0;
}

#ifdef CONFIG_VDEV_DEBUG
void mci_atp_show(void)
{
    struct mci_atp_debug_ctx *ctx = &g_mci_atp_debug_ctx;
    unsigned int i;

    for (i = 0; i < g_mci_atp_ctx.mod_cnt; i++) {
        bsp_err("module id 0x%x\n", g_mci_atp_ctx.mod_sets[i].id);
    }

    bsp_err("buffer err cnt %d\n", ctx->msg_buf_err);
    bsp_err("len err    cnt %d\n", ctx->msg_len_err);
    bsp_err("unregister cnt %d\n", ctx->unregister);
    bsp_err("timeout    cnt %d\n", ctx->timeout);
    bsp_err("no memory  cnt %d\n", ctx->no_mem);
}
#endif
