/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2018. All rights reserved.
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
#include "msg_plat.h"
#include "msg_base.h"
#include "msg_mem.h"
#include "msg_core.h"
#include "msg_cmsg.h"
#include "msg_lite.h"

static struct msg_chn_hdl g_msg_fd_info[MSG_CHN_CNT_MAX];
static u8 g_msg_init_flag = 0;

struct msg_chn_hdl *get_msgchn_hdl(u32 msgid)
{
    if (msgid >= MSG_CHN_CNT_MAX) {
        return NULL;
    }
    return &g_msg_fd_info[msgid];
}

int msg_lite_callback(const struct msg_addr *src_addr, const struct msg_addr *dst_addr, void *buf, u32 len)
{
    if (src_addr == NULL || dst_addr == NULL || buf == NULL || len == 0) {
        return MSG_ERR_EINVAL;
    }

    if (dst_addr->chnid >= MSG_CHN_CNT_MAX || g_msg_fd_info[dst_addr->chnid].state != MSG_CHN_OPEND) {
        return MSG_ERR_EHOSTUNREACH;
    }
    if ((g_msg_fd_info[dst_addr->chnid].core_mask & MSG_CORE_MASK(src_addr->core)) == 0) {
        return MSG_ERR_EHOSTUNREACH;
    }

    if (g_msg_fd_info[dst_addr->chnid].func == NULL) {
        return MSG_ERR_EHOSTUNREACH;
    }

    g_msg_fd_info[dst_addr->chnid].func(src_addr, buf, len);

    return 0;
}

void msg_lite_wakeup_callback(const struct msg_addr *src_addr, const struct msg_addr *dst_addr)
{
    struct msg_wakeup_event *wakeup_event = NULL;

    if (src_addr == NULL || dst_addr == NULL) {
        return;
    }

    if (dst_addr->chnid >= MSG_CHN_CNT_MAX || g_msg_fd_info[dst_addr->chnid].state != MSG_CHN_OPEND) {
        return;
    }

    if ((g_msg_fd_info[dst_addr->chnid].core_mask & MSG_CORE_MASK(src_addr->core)) == 0) {
        return;
    }

    wakeup_event = g_msg_fd_info[dst_addr->chnid].wakeup_event;
    if (wakeup_event == NULL || wakeup_event->wakeup_func == NULL) {
        return;
    }
    wakeup_event->wakeup_func(wakeup_event->arg);
}

void bsp_msgchn_attr_init(struct msgchn_attr *pattr)
{
    if (pattr == NULL) {
        return;
    }
    pattr->magic = MID_ATTR_INIT_MAGIC;
}

int bsp_msg_lite_open(struct msg_chn_hdl **ppchn_hdl, struct msgchn_attr *pattr)
{
    u32 msgid;
    if (g_msg_init_flag != MSG_INIT_OK) {
        return MSG_ERR_UNINIT;
    }
    if (ppchn_hdl == NULL || pattr == NULL) {
        return MSG_ERR_EINVAL;
    }
    if (pattr->magic != MID_ATTR_INIT_MAGIC || pattr->chnid >= MSG_CHN_CNT_MAX) {
        return MSG_ERR_ENXIO;
    }

    msgid = pattr->chnid;

    if (g_msg_fd_info[msgid].state == MSG_CHN_OPEND) {
        return MSG_ERR_EEXIST;
    }
    g_msg_fd_info[msgid].msgid = pattr->chnid;
    g_msg_fd_info[msgid].core_mask = pattr->coremask;
    g_msg_fd_info[msgid].func = pattr->lite_notify;
    g_msg_fd_info[msgid].state = MSG_CHN_OPEND;

    lite_assign_dump_pos(&g_msg_fd_info[msgid]);
    *ppchn_hdl = &g_msg_fd_info[msgid];
    return 0;
}

u32 get_lite_dump_pos(u32 chn_id)
{
    if (chn_id >= MSG_CHN_CNT_MAX) {
        return (u32)MSG_ERR_EHOSTUNREACH;
    }
    return g_msg_fd_info[chn_id].dump_pos;
}

int bsp_msg_lite_sendto(struct msg_chn_hdl *chn_hdl, const struct msg_addr *dst, void *msg, unsigned len)
{
    int ret = -1;
    s16 trace_code = MSG_TRACE_LITE_AFTER;
    struct msg_addr src_addr;
    struct channel_ctrl_t *chn_ctrl = NULL;
    msg_blkx3_desc_t send_desc;
    if (chn_hdl == NULL || dst == NULL || msg == NULL || len == 0) {
        return MSG_ERR_EINVAL;
    }
    if (chn_hdl->state != MSG_CHN_OPEND) {
        return MSG_ERR_EIO;
    }
    src_addr.core = THIS_CORE;
    src_addr.chnid = chn_hdl->msgid;
    lite_dump_record(&src_addr, dst, len, get_lite_dump_pos(src_addr.chnid), MSG_TRACE_LITE_BEFORE);
    if (dst->core != THIS_CORE) {
        send_desc.cnt = 0x1;
        send_desc.datablk[0x0].len = len;
        send_desc.datablk[0x0].buf = msg;
        if (chn_hdl->cmsg_hdl != NULL) {
            chn_ctrl = (struct channel_ctrl_t *)(chn_hdl->cmsg_hdl);
            ret = chn_ctrl->ops->send(chn_ctrl, &send_desc);
        } else {
            ret = msg_crosscore_send_lite(&src_addr, dst, msg, len);
        }
    } else {
        ret = msg_lite_callback(&src_addr, dst, msg, len);
    }

    if (ret) {
        trace_code = ret;
        lite_tx_record(get_lite_dump_pos(src_addr.chnid), trace_code);
    }
    lite_dump_record(&src_addr, dst, len, get_lite_dump_pos(src_addr.chnid), trace_code);
    return ret;
}

int bsp_msg_wakeup_register(struct msg_chn_hdl *chn_hdl, wakeup_notify func, void *arg)
{
    struct msg_wakeup_event *wakeup_event = NULL;

    if (chn_hdl == NULL || func == NULL) {
        return MSG_ERR_EINVAL;
    }

    if (chn_hdl->state != MSG_CHN_OPEND) {
        return MSG_ERR_EIO;
    }

    if (chn_hdl->wakeup_event != NULL) {
        return MSG_ERR_EEXIST;
    }

    wakeup_event = (struct msg_wakeup_event *)osl_malloc(sizeof(struct msg_wakeup_event));
    if (wakeup_event == NULL) {
        return MSG_ERR_ENOMEM;
    }

    wakeup_event->wakeup_func = func;
    wakeup_event->arg = arg;
    chn_hdl->wakeup_event = (void *)wakeup_event;

    return 0;
}

void msg_lite_init(void)
{
    g_msg_init_flag = MSG_INIT_OK;
}

int bsp_msg_lite_close(struct msg_chn_hdl *chn_hdl)
{
    u32 chn_id;
    if (chn_hdl == NULL) {
        return MSG_ERR_EINVAL;
    }
    if (chn_hdl->state != MSG_CHN_OPEND) {
        return MSG_ERR_EIO;
    }
    chn_id = chn_hdl->msgid;
    if (chn_id >= MSG_CHN_CNT_MAX) {
        return MSG_ERR_EIO;
    }

    g_msg_fd_info[chn_id].state = 0;
    g_msg_fd_info[chn_id].core_mask = 0;
    return 0;
}
EXPORT_SYMBOL(bsp_msg_lite_open);
EXPORT_SYMBOL(bsp_msg_lite_close);
EXPORT_SYMBOL(bsp_msg_lite_sendto);
EXPORT_SYMBOL(bsp_msgchn_attr_init);
