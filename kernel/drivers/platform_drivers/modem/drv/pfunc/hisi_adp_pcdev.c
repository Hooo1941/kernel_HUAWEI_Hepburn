/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2018-2020. All rights reserved.
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
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
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

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/delay.h>

#include <../fs/internal.h>
#include "bsp_pcdev.h"
#include "pcie_cdev.h"
#include "pcie_cdev_dbg.h"
#include "mdrv_pcdev.h"
#include <osl_types.h>

#define PCDEV_ENABLE_CB_MAX 32
#define ADP_PCDEV_FILP_INVALID(filp) (IS_ERR_OR_NULL((filp)) || !(filp)->private_data)

struct adp_pcdev_stat {
    int stat_open_err;
    void *stat_open_last_err;
    int stat_read_err;
    int stat_read_last_err;
    int stat_write_err;
    int stat_write_last_err;
    int stat_ioctl_err;
    int stat_ioctl_last_err;
    int stat_wait_close;
};

struct adp_pcdev_context {
    char *dev_name;
    int is_open;
    atomic_t opt_cnt;
    struct adp_pcdev_stat stat;
};

typedef struct {
    pcdev_enable_cb_t enum_done_cbs[PCDEV_ENABLE_CB_MAX];
    pcdev_disable_cb_t disable_cbs[PCDEV_ENABLE_CB_MAX];
} PCDEV_ENUM_CB_S;

static struct adp_pcdev_context g_adp_pcdev_ctx[] = {
    { "/dev/pcdev_ctrl",        0, {0}, {0}}, /* UDI_USB_ACM_CTRL */
    { "/dev/pcdev_ttyGS0",      0, {0}, {0}}, /* UDI_USB_ACM_AT */
    { "/dev/pcdev_c_shell",     0, {0}, {0}}, /* UDI_USB_ACM_SHELL */
    { "/dev/pcdev_4g_diag",     0, {0}, {0}}, /* UDI_USB_ACM_LTE_DIAG */
    { "/dev/pcdev_3g_diag",     0, {0}, {0}}, /* UDI_USB_ACM_OM */
    { "/dev/pcdev_mdm",         0, {0}, {0}}, /* UDI_USB_ACM_MODEM */
    { "/dev/pcdev_gps",         0, {0}, {0}}, /* UDI_USB_ACM_GPS */
    { "/dev/pcdev_a_shell",     0, {0}, {0}}, /* UDI_USB_ACM_3G_GPS */
    { "/dev/pcdev_err",         0, {0}, {0}}, /* UDI_USB_ACM_3G_PCVOICE */
    { "/dev/pcdev_voice",       0, {0}, {0}}, /* UDI_USB_ACM_PCVOICE */
    { "/dev/pcdev_skytone",     0, {0}, {0}}, /* UDI_USB_ACM_SKYTONE */
    { "/dev/pcdev_cdma_log",    0, {0}, {0}}, /* UDI_USB_ACM_CDMA_LOG */
    { "/dev/pcdev_agent_nv",    0, {0}, {0}}, /* UDI_PCDEV_AGENT_NV */
    { "/dev/pcdev_agent_om",    0, {0}, {0}}, /* UDI_PCDEV_AGENT_OM */
    { "/dev/pcdev_agent_msg",   0, {0}, {0}}, /* UDI_PCDEV_AGENT_MSG */
    { "/dev/pcdev_appvcom",     0, {0}, {0}}, /* UDI_PCDEV_APPVCOM */
    { "/dev/pcdev_modem",       0, {0}, {0}}, /* UDI_PCDEV_MODEM */
    { "/dev/pcdev_pmom",        0, {0}, {0}}, /* UDI_PCDEV_PMOM */ 
    { "/dev/pcdev_reserve",     0, {0}, {0}}, /* UDI_PCDEV_RESERVE */
};

static int g_pcdev_devid_invalid = 0;
static int g_pcdev_filp_invalid = 0;
static int g_pcdev_enum_done_cur = 0;
static int g_pcdev_disable_cur = 0;
PCDEV_ENUM_CB_S g_pcdev_enum_ctx;
int g_pcdev_enum_done = 0;

unsigned int bsp_pcdev_reg_enablecb(pcdev_enable_cb_t pFunc)
{
    if (g_pcdev_enum_done_cur >= PCDEV_ENABLE_CB_MAX) {
        PCDEV_ERR("pcie cdev enum function already full \n");
        return (unsigned int)(-1);
    }

    if (!pFunc) {
        PCDEV_ERR("bsp_pcdev_reg_enablecb error\n");
        return (unsigned int)(-1);
    }

    g_pcdev_enum_ctx.enum_done_cbs[g_pcdev_enum_done_cur] = pFunc;
    g_pcdev_enum_done_cur++;

    if (g_pcdev_enum_done) {
        pFunc();
    }

    return 0;
}

void pcie_cdev_notify_enum_done(void)
{
    int i;
    g_pcdev_enum_done = 1;

    for (i = 0; i < g_pcdev_enum_done_cur; i++) {
        if (g_pcdev_enum_ctx.enum_done_cbs[i] != NULL) {
            g_pcdev_enum_ctx.enum_done_cbs[i]();
        }
    }

    return;
}

unsigned int bsp_pcdev_reg_disablecb(pcdev_disable_cb_t pFunc)
{
    if (g_pcdev_disable_cur >= PCDEV_ENABLE_CB_MAX) {
        PCDEV_ERR("pcie cdev disable function already full \n");
        return (unsigned int)(-1);
    }

    if (!pFunc) {
        PCDEV_ERR("mdrv_pcdev_reg_disablecb error\n");
        return (unsigned int)(-1);
    }

    g_pcdev_enum_ctx.disable_cbs[g_pcdev_disable_cur] = pFunc;
    g_pcdev_disable_cur++;

    return 0;
}

void pcie_cdev_notify_disable(void)
{
    int i;

    for (i = 0; i < g_pcdev_disable_cur; i++) {
        if (g_pcdev_enum_ctx.disable_cbs[i] != NULL){
            g_pcdev_enum_ctx.disable_cbs[i]();
        }
    }

    return;
}

void *bsp_pcdev_open(u32 dev_id)
{
    mm_segment_t old_fs;
    char *name = NULL;
    struct file *filp = NULL;
    PCDEV_TRACE("in\n");

    if (dev_id >= sizeof(g_adp_pcdev_ctx) / sizeof(struct adp_pcdev_context)) {
        g_pcdev_devid_invalid++;
        PCDEV_ERR("open devID error: %d\n", dev_id);
        return NULL;
    }
    name = g_adp_pcdev_ctx[dev_id].dev_name;

    old_fs = get_fs();
    set_fs(KERNEL_DS);
    filp = filp_open(name, O_RDWR, 0);
    set_fs(old_fs);
    if (IS_ERR_OR_NULL(filp)) {
        g_adp_pcdev_ctx[dev_id].stat.stat_open_err++;
        g_adp_pcdev_ctx[dev_id].stat.stat_open_last_err = filp;
        PCDEV_ERR("devID: %d name %s. filp is NULL\n", dev_id, name);
        return NULL;
    }
    filp->private_data = &g_adp_pcdev_ctx[dev_id];
    g_adp_pcdev_ctx[dev_id].is_open = 1;

    return filp;
}

int bsp_pcdev_close(void *handle)
{
    mm_segment_t old_fs;
    struct file *filp = (struct file *)handle;
    struct adp_pcdev_context *ctx = NULL;

    PCDEV_TRACE("in\n");

    if (unlikely(ADP_PCDEV_FILP_INVALID(filp))) {
        g_pcdev_filp_invalid++;
        return -EINVAL;
    }

    ctx = (struct adp_pcdev_context *)filp->private_data;
    ctx->is_open = 0;

    old_fs = get_fs();
    set_fs(KERNEL_DS);
    filp_close(filp, NULL);
    set_fs(old_fs);
    return 0;
}

int bsp_pcdev_write(void *handle, void *buf, u32 size)
{
    mm_segment_t old_fs;
    struct file *filp = (struct file *)handle;
    struct adp_pcdev_context *ctx = NULL;
    int status;
    loff_t pos = 0;

    if (unlikely(ADP_PCDEV_FILP_INVALID(filp))) {
        g_pcdev_filp_invalid++;
        return -EINVAL;
    }
    ctx = (struct adp_pcdev_context *)filp->private_data;

    atomic_inc(&ctx->opt_cnt);
    if (unlikely(!ctx->is_open || !(filp->f_path.dentry))) {
        status = -ENXIO;
        goto write_ret;
    }
    old_fs = get_fs();
    set_fs(KERNEL_DS);
    status = vfs_write(filp, (void __force __user *)buf, size, &pos);
    set_fs(old_fs);

write_ret:
    atomic_dec(&ctx->opt_cnt);
    if (status < 0) {
        ctx->stat.stat_write_err++;
        ctx->stat.stat_write_last_err = status;
    }
    return status;
}

int bsp_pcdev_read(void *handle, void *buf, u32 size)
{
    mm_segment_t old_fs;
    struct file *filp = (struct file *)handle;
    struct adp_pcdev_context *ctx = NULL;
    int status;
    loff_t pos = 0;

    if (unlikely(ADP_PCDEV_FILP_INVALID(filp))) {
        g_pcdev_filp_invalid++;
        /* protect system running, usr often use read in while(1). */
        msleep(10); /* sleep 10 miliseconds */
        return -EINVAL;
    }
    ctx = (struct adp_pcdev_context *)filp->private_data;

    atomic_inc(&ctx->opt_cnt);
    if (unlikely(!ctx->is_open || !(filp->f_path.dentry))) {
        status = -ENXIO;
        goto read_ret;
    }

    old_fs = get_fs();
    set_fs(KERNEL_DS);
    status = vfs_read(filp, (void __force __user *)buf, size, &pos);
    set_fs(old_fs);

read_ret:
    atomic_dec(&ctx->opt_cnt);
    if (status <= 0) {
        /* protect system running, usr often use read in while(1). */
        msleep(100); /* sleep 100 miliseconds */
        ctx->stat.stat_read_err++;
        ctx->stat.stat_read_last_err = status;
    }
    return status;
}

int bsp_pcdev_ioctl(void *handle, u32 cmd, void *para)
{
    mm_segment_t old_fs;
    struct file *filp = (struct file *)handle;
    struct adp_pcdev_context *ctx = NULL;
    int status;

    if (unlikely(ADP_PCDEV_FILP_INVALID(filp))) {
        g_pcdev_filp_invalid++;
        return -EINVAL;
    }
    ctx = (struct adp_pcdev_context *)filp->private_data;

    atomic_inc(&ctx->opt_cnt);
    if (unlikely(!ctx->is_open || !(filp->f_path.dentry))) {
        status = -ENXIO;
        goto ioctl_ret;
    }

    old_fs = get_fs();
    set_fs(KERNEL_DS);
    status = (int)pcie_cdev_ioctl(filp, (unsigned int)cmd, (unsigned long)(uintptr_t)para);
    set_fs(old_fs);

ioctl_ret:
    atomic_dec(&ctx->opt_cnt);
    if (status < 0) {
        ctx->stat.stat_ioctl_err++;
        ctx->stat.stat_ioctl_last_err = status;
    }
    return status;
}

void pcdev_adp_dump(void)
{
    unsigned int i;

    PCDEV_ERR("stat_invalid_devid      :%d\n", g_pcdev_devid_invalid);
    PCDEV_ERR("stat_invalid_filp       :%d\n", g_pcdev_devid_invalid);
    for (i = 0; i < sizeof(g_adp_pcdev_ctx) / sizeof(struct adp_pcdev_context); i++) {
        PCDEV_ERR("==== dump dev:%s ====\n", g_adp_pcdev_ctx[i].dev_name);
        PCDEV_ERR("is_open             :%d\n", g_adp_pcdev_ctx[i].is_open);
        PCDEV_ERR("opt_cnt             :%d\n", atomic_read(&g_adp_pcdev_ctx[i].opt_cnt));
        PCDEV_ERR("stat_open_err       :%d\n", g_adp_pcdev_ctx[i].stat.stat_open_err);
        PCDEV_ERR("stat_open_last_err  :%lx\n", (uintptr_t)g_adp_pcdev_ctx[i].stat.stat_open_last_err);
        PCDEV_ERR("stat_read_err       :%d\n", g_adp_pcdev_ctx[i].stat.stat_read_err);
        PCDEV_ERR("stat_read_last_err  :%d\n", g_adp_pcdev_ctx[i].stat.stat_read_last_err);
        PCDEV_ERR("stat_write_err      :%d\n", g_adp_pcdev_ctx[i].stat.stat_write_err);
        PCDEV_ERR("stat_write_last_err :%d\n", g_adp_pcdev_ctx[i].stat.stat_write_last_err);
        PCDEV_ERR("stat_ioctl_err      :%d\n", g_adp_pcdev_ctx[i].stat.stat_ioctl_err);
        PCDEV_ERR("stat_ioctl_last_err :%d\n", g_adp_pcdev_ctx[i].stat.stat_ioctl_last_err);
        PCDEV_ERR("stat_wait_close     :%d\n", g_adp_pcdev_ctx[i].stat.stat_wait_close);
        PCDEV_ERR("\n");
    }
}
