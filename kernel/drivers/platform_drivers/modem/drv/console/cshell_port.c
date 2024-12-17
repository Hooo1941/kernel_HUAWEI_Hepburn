/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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

#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/kfifo.h>
#include <product_config.h>
#include <mdrv_udi.h>
#include <mdrv_usb.h>
#include <bsp_print.h>
#include <securec.h>
#include <msg_id.h>
#include <bsp_msg.h>
#include <bsp_slice.h>
#include "virtshell.h"
#include <linux/delay.h>

int g_acm_chn_handle = 0;

struct semaphore g_recv_sem;
struct semaphore g_send_sem;

#undef THIS_MODU
#define THIS_MODU mod_cshell
#define TMP_BUFF_SIZE 128
#define CSHELL_SEND_MAX_NUM 5

int legal_chr(char chr)
{
    if (chr >= 0x20 && chr <= 0x7E) { /* visible character */
        return 1;
    }
    if (chr == '\r' || chr == '\n' || chr == 0x8 || /* backspace */
        chr == 0x7F ||                              /* del */
        chr == 0x1B || /* esc(1b) for up(1b 5b 41)/down(1b 5b 42)/left(1b 5b 44)/right(1b 5b 43) */
        chr == 0x4 ||  /* ctrl+d */
        chr == '\t') {
        return 1;
    }

    return 0;
}

int tsk_recv_func(void *data)
{
    struct acm_wr_async_info acm_wt_info = { 0 };
    int i = 0;

    while (1) {
        down(&g_recv_sem);
        if (g_acm_chn_handle == 0) {
            continue;
        }

        if (mdrv_udi_ioctl(g_acm_chn_handle, ACM_IOCTL_GET_RD_BUFF, &acm_wt_info))
            continue;

        for (i = 0; i < acm_wt_info.size; i++) {
            if (!legal_chr(acm_wt_info.virt_addr[i])) {
                cshell_port_err("[bluetooth port]input contains unvisible data\n");
                break;
            }
        }
        if (i < acm_wt_info.size) {
            (void)mdrv_udi_ioctl(g_acm_chn_handle, ACM_IOCTL_RETURN_BUFF, &acm_wt_info);
            continue;
        }
        g_cshell_ap.cmdbuff_in(&g_cshell_ap, (unsigned char *)acm_wt_info.virt_addr, acm_wt_info.size);

        (void)mdrv_udi_ioctl(g_acm_chn_handle, ACM_IOCTL_RETURN_BUFF, &acm_wt_info);
    }

    return 0;
}

static int tsk_send_func(void *data)
{
    int len = 0;
    int ret = 0;
    int usb_send_fail_cnt = 0;
    static unsigned char g_tmpbuf[TMP_BUFF_SIZE] = { 0, };
    struct acm_wr_async_info info;
    while (!kthread_should_stop()) {
        down(&g_send_sem);
        if (g_acm_chn_handle == 0) {
            continue;
        }
        while ((len = g_cshell_ap.logbuff_out(&g_cshell_ap, g_tmpbuf, TMP_BUFF_SIZE)) > 0) {
            info.virt_addr = g_tmpbuf;
            info.size = len;
            info.phy_addr = 0;
            usb_send_fail_cnt = 0;
            while (mdrv_udi_ioctl(g_acm_chn_handle, ACM_IOCTL_WRITE_ASYNC, &info) < 0) {
                mdelay(5);//5ms后继续发送
                usb_send_fail_cnt++;
                if (usb_send_fail_cnt > CSHELL_SEND_MAX_NUM) {
                    cshell_port_err("mdrv_udi_write fail in tsk_send_func, ret = %d\n", ret);
                    break;
                }
            }
        }
    }
    return 0;
}

void cshell_starttx(struct cshell_info *cshell)
{
    up(&g_send_sem);
}
EXPORT_SYMBOL(cshell_starttx);

void cshell_acm_event_cb(void)
{
    up(&g_recv_sem);
}
EXPORT_SYMBOL(cshell_acm_event_cb);

void acm_open_cb(void)
{
    int acm_chn_handle;
    int retval;
    struct udi_open_param cshell_handle;
    cshell_port_info("acm_open_cb called\n");

    cshell_handle.devid = UDI_ACM_SHELL_ID;

    acm_chn_handle = mdrv_udi_open(&cshell_handle); /*lint !e838*/
    if (!acm_chn_handle) {
        cshell_port_err("A:cshell_udi_open_cb acm open fail: [0x%x]\n", acm_chn_handle);
        return;
    }

    retval = mdrv_udi_ioctl(acm_chn_handle, ACM_IOCTL_SET_READ_CB, cshell_acm_event_cb);
    if (retval) {
        cshell_port_err("A:cshell_udi_open_cb cb register fail: [0x%x]\n", retval);
        return;
    }

    g_acm_chn_handle = acm_chn_handle;
}
EXPORT_SYMBOL(acm_open_cb);

void acm_close_cb(void)
{
    int acm_chn_handle = g_acm_chn_handle;
    cshell_port_info("acm_close_cb called\n");
    (void)mdrv_udi_ioctl(acm_chn_handle, ACM_IOCTL_SET_READ_CB, NULL);
    (void)mdrv_udi_close(acm_chn_handle);
    g_acm_chn_handle = 0;
}
EXPORT_SYMBOL(acm_close_cb);

int cshell_port_init(void)
{
    struct task_struct *tsk_hdl = NULL;
    sema_init(&g_recv_sem, 0);
    sema_init(&g_send_sem, 0);
    tsk_hdl = kthread_run(tsk_recv_func, NULL, "cshell_recv_thread");
    tsk_hdl = kthread_run(tsk_send_func, NULL, "cshell_send_thread");
    if (IS_ERR(tsk_hdl)) {
        cshell_port_err("tsk_hdl creat failed\n");
    }

    mdrv_usb_reg_enablecb(acm_open_cb);
    mdrv_usb_reg_disablecb(acm_close_cb);

    cshell_port_err("[LR:CSHELL] init OK\n");

    return 0;
}
#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
/*lint -e528*/
module_init(cshell_port_init);
#endif
