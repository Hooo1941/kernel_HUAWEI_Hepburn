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

#include <linux/module.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/smp.h>
#include <linux/printk.h>
#include <linux/module.h>
#include <linux/kfifo.h>
#include <osl_malloc.h>
#include <msg_id.h>
#include <bsp_msg.h>
#include <bsp_slice.h>
#include <bsp_print.h>
#include "virtshell.h"
#include "securec.h"

#undef THIS_MODU
#define THIS_MODU mod_cshell
msg_chn_t g_cshell_msg_hdl;
struct cshell_info g_cshell_ap = {
    .start_tx = cshell_starttx,
    .start_shell = virtshl_start_shell
};
EXPORT_SYMBOL(g_cshell_ap);
struct kfifo g_cshell_logbuff;
struct kfifo g_cshell_cmdbuff;

int cshell_msg_cb(const struct msg_addr *src, void *buf, u32 len)
{
    unsigned int buf_len = len;
    if (buf_len >= (1U << LOG_BUF_SHIFT) || buf == NULL) {
        return 0;
    }

    g_cshell_ap.logbuff_in(&g_cshell_ap, (char *)buf, buf_len);
    g_cshell_ap.start_tx(&g_cshell_ap);
    return 0;
}

int cshell_send_to_ccore(void *data, u32 len)
{
    int ret;
    struct msg_addr dst;
    dst.core = MSG_CORE_TSP;
    dst.chnid = MSG_CHN_CSHELL;
    ret = bsp_msg_lite_sendto(g_cshell_msg_hdl, &dst, data, len);
    if (ret) {
        cshell_port_err("fail to send msg to ccore");
    }
    return ret;
}

int cshell_msg_init(void)
{
    int ret;
    struct msgchn_attr lite_attr = { 0 };
    bsp_msgchn_attr_init(&lite_attr);
    lite_attr.coremask = MSG_CORE_MASK(MSG_CORE_TSP);
    lite_attr.chnid = MSG_CHN_CSHELL;
    lite_attr.lite_notify = cshell_msg_cb;
    ret = bsp_msg_lite_open(&g_cshell_msg_hdl, &lite_attr);
    if (ret) {
        cshell_port_err("cshell msg open err, ret = %d\n", ret);
        return -1;
    }
    return 0;
}

static int virtshl_send_list(void)
{
    static unsigned char tmpbuf[VIRTSHL_MAX_PACKAGE_LEN] = { 0, };
    unsigned char *outbuf = tmpbuf;
    int len;
    do {
        outbuf = tmpbuf;
        len = g_cshell_ap.cmdbuff_out(&g_cshell_ap, outbuf, VIRTSHL_MAX_PACKAGE_LEN);
        if (len <= 0) {
            break;
        }
        (void)cshell_send_to_ccore(outbuf, (unsigned int)len);
    } while (len <= 0);
    return 0;
}

int virtshl_send_tsk_func(void *data)
{
    while (!kthread_should_stop()) {
        down(&g_cshell_ap.send_tsk_sem);
        (void)virtshl_send_list();
    }
    return 0;
}

int virtshl_start_shell(struct cshell_info *cshell)
{
    up(&g_cshell_ap.send_tsk_sem);
    return 0;
}
EXPORT_SYMBOL(virtshl_start_shell);

static void cshell_logbuff_in(struct cshell_info *cshell, unsigned char *inbuf, unsigned int size)
{
    kfifo_in(cshell->logbuf, (void *)(inbuf), size);
}

static int cshell_cmdbuff_out(struct cshell_info *cshell, unsigned char *outbuf, unsigned int size)
{
    return kfifo_out(cshell->cmdbuf, (void *)(outbuf), size);
}

static int cshell_logbuff_out(struct cshell_info *cshell, unsigned char *outbuf, unsigned int size)
{
    return kfifo_out(cshell->logbuf, (void *)(outbuf), size);
}

static int cshell_cmdbuff_in(struct cshell_info *cshell, unsigned char *inbuf, unsigned int sz)
{
    int ret;
    ret = kfifo_in(cshell->cmdbuf, (void *)(inbuf), sz);
    (void)cshell->start_shell(cshell);
    return ret;
}

int cshell_info_init(struct cshell_info *cshell)
{
    if (cshell == NULL) {
        return -1;
    }
    cshell->logbuff_out = cshell_logbuff_out;
    cshell->cmdbuff_in = cshell_cmdbuff_in;
    cshell->logbuff_in = cshell_logbuff_in;
    cshell->cmdbuff_out = cshell_cmdbuff_out;

    return 0;
}
EXPORT_SYMBOL(cshell_info_init);

int virtshl_init(void)
{
    int ret;
    ret = kfifo_alloc(&g_cshell_logbuff, 1U << LOG_BUF_SHIFT, GFP_KERNEL);
    ret = kfifo_alloc(&g_cshell_cmdbuff, 1U << CMD_BUF_SHIFT, GFP_KERNEL);
    if (ret) {
        cshell_port_err("kfifo creat failed ret = %d\n", ret);
    }
    g_cshell_ap.cmdbuf = &g_cshell_cmdbuff;
    g_cshell_ap.logbuf = &g_cshell_logbuff;
    cshell_info_init(&g_cshell_ap);

    sema_init(&g_cshell_ap.send_tsk_sem, 0);
    cshell_msg_init();
    g_cshell_ap.send_tsk_id = kthread_run(virtshl_send_tsk_func, NULL, "virtshl_sfunc");
    if (IS_ERR_OR_NULL(g_cshell_ap.send_tsk_id)) {
        cshell_port_err("fail to create virtshl_sfunc, ret = %ld\n", PTR_ERR(g_cshell_ap.send_tsk_id));
        return -1;
    }
    return 0;
}
#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(virtshl_init);
#endif
