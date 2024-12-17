/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2021. All rights reserved.
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
#include <linux/workqueue.h>
#include <linux/spinlock.h>
#include <securec.h>
#include <bsp_homi.h>
#include <bsp_print.h>
#include "vdev.h"

#define THIS_MODU mod_vcom
#define SOCKET_AT_BUFSIZE 1024
#define SOCKET_OM_BUFSIZE 8192

enum relay_socket_state {
    SOCKET_DISCONNECT,
    SOCKET_CONNECT,
    SOCKET_UNKNOW,
};

struct relay_socket_stat {
    unsigned int rx_num;
    unsigned int tx_drop;
    unsigned int tx_queue;
    unsigned int tx_success_num;
    unsigned int tx_fail_num;
    unsigned int evt_num;
    unsigned int init_success_num;
    unsigned int init_fail_num;
};

struct relay_socket_ctx {
    struct vcom_hd *hd;
    struct homi_chan *chan;
    struct list_head tx_queue;
    spinlock_t tx_lock;
    enum relay_socket_state conn_state;
    struct delayed_work write_work;
    struct workqueue_struct *socket_work_queue;
    struct relay_socket_stat stat;
    struct homi_cfg cfg;
};

struct relay_socket_packet {
    struct list_head list;
    void *buf;
    unsigned int buf_len;
};

static void vcom_socket_read_cb(unsigned int minor_type, void *data, int len);
static void vcom_socket_at_read_cb(struct homi_chan *chan, void *data, int len)
{
    vcom_socket_read_cb(MCI_SOCKET_AT, data, len);
}
static void vcom_socket_om_read_cb(struct homi_chan *chan, void *data, int len)
{
    vcom_socket_read_cb(MCI_SOCKET_OM, data, len);
}


static void vcom_socket_event_cb(unsigned int minor_type, enum homi_port_type ptype, enum homi_chan_state state);
static void vcom_socket_at_event_cb(enum homi_port_type ptype, enum homi_chan_state state)
{
    vcom_socket_event_cb(MCI_SOCKET_AT, ptype, state);
}
static void vcom_socket_om_event_cb(enum homi_port_type ptype, enum homi_chan_state state)
{
    vcom_socket_event_cb(MCI_SOCKET_OM, ptype, state);
}

struct relay_socket_ctx g_relay_socket_ctx[MCI_SOCKET_MAX] = {
    [MCI_SOCKET_AT] = {
        .cfg.cb.recvcb = vcom_socket_at_read_cb,
        .cfg.cb.eventcb = vcom_socket_at_event_cb,
        .cfg.ptype = HOMI_PORT_SOCKET,
        .cfg.ctype = HOMI_CH_SK_20249,
        .cfg.blength = SOCKET_AT_BUFSIZE,
    },
    [MCI_SOCKET_OM] = {
        .cfg.cb.recvcb = vcom_socket_om_read_cb,
        .cfg.cb.eventcb = vcom_socket_om_event_cb,
        .cfg.ptype = HOMI_PORT_SOCKET,
        .cfg.ctype = HOMI_CH_SK_20250,
        .cfg.blength = SOCKET_OM_BUFSIZE,
    },
};

static void vcom_socket_read_cb(unsigned int minor_type, void *data, int len)
{
    struct relay_socket_ctx *ctx = &g_relay_socket_ctx[minor_type];
    vcom_write(ctx->hd, data, len, WRITE_ASYNC);
    ctx->stat.rx_num++;
}

static void vcom_socket_event_cb(unsigned int minor_type, enum homi_port_type ptype, enum homi_chan_state state)
{
    struct relay_socket_ctx *ctx = &g_relay_socket_ctx[minor_type];

    if (state == HOMI_CHSTATE_CONNECTED) {
        ctx->conn_state = SOCKET_CONNECT;
    } else if (state == HOMI_CHSTATE_DISCONNECTED) {
        ctx->conn_state = SOCKET_DISCONNECT;
    } else {
        bsp_err("<%s> invalid event state %d\n", __func__, state);
        return;
    }
    ctx->stat.evt_num++;
}

static void relay_socket_send_data(struct work_struct *work)
{
    int ret;
    unsigned long flags;
    struct relay_socket_packet *packet = NULL;
    struct relay_socket_ctx *ctx = container_of(work, struct relay_socket_ctx, write_work.work);
    struct homi_chan *chan = ctx->chan;

    while (1) {
        spin_lock_irqsave(&(ctx->tx_lock), flags);
        packet = list_first_entry_or_null(&ctx->tx_queue, struct relay_socket_packet, list);
        if (packet == NULL) {
            spin_unlock_irqrestore(&(ctx->tx_lock), flags);
            return;
        }
        list_del(&packet->list);
        spin_unlock_irqrestore(&(ctx->tx_lock), flags);

        ret = chan->ops->send(chan, packet->buf, packet->buf_len);
        if (ret) {
            ctx->stat.tx_fail_num++;
            bsp_err("<%s> data send fail:%d\n", __func__, ret);
        } else {
            ctx->stat.tx_success_num++;
        }
        kfree(packet);
    }
}

void vcom_socket_free_vb(unsigned int result, struct vcom_hd *hd, struct vcom_buf *vb)
{
    if (result == 0) {
        hd->map->stax.succ_free++;
    } else {
        hd->map->stax.fail_free++;
    }
    kfree(vb);
    return;
}

int vcom_socket_tx_packet(struct vcom_hd *hd, struct relay_socket_ctx *ctx, struct vcom_buf *vb)
{
    int err;
    unsigned long flags;
    struct relay_socket_packet *packet = NULL;

    packet = (struct relay_socket_packet *)kzalloc(sizeof(struct relay_socket_packet) + vb->payload_len, GFP_ATOMIC);
    if (packet == NULL) {
        bsp_err("<%s> kzalloc fail\n", __func__);
        return -ENOMEM;
    }

    packet->buf = packet + 1;
    packet->buf_len = vb->payload_len;
    err = memcpy_s(packet->buf, packet->buf_len, vb->data, vb->payload_len);
    if (err) {
        bsp_err("<%s> memcpy_s err :%d\n", __func__, err);
        kfree(packet);
        return err;
    }
    spin_lock_irqsave(&(ctx->tx_lock), flags);
    list_add_tail(&(packet->list), &(ctx->tx_queue));
    spin_unlock_irqrestore(&(ctx->tx_lock), flags);
    ctx->stat.tx_queue++;
    queue_delayed_work(ctx->socket_work_queue, &ctx->write_work, 0);
    vcom_socket_free_vb(0, hd, vb);
    return 0;
}

int vcom_socket_op(struct vcom_buf *vb)
{
    struct vcom_hd *hd = NULL;
    struct relay_socket_ctx *ctx = NULL;

    hd = vcom_get_hd(vb->id);
    if (hd->relay_type >= MCI_SOCKET_MAX) {
        bsp_err("<%s> socket type %u not support\n", __func__, hd->relay_type);
        return -1;
    }

    ctx = &g_relay_socket_ctx[hd->relay_type];
    if (ctx->conn_state != SOCKET_CONNECT) {
        ctx->stat.tx_drop++;
        goto op_fail;
    }

    if (vb->payload_type == WRITE_ASYNC) {
        if (vcom_socket_tx_packet(hd, ctx, vb) < 0) {
            goto op_fail;
        }
        return 0;
    } else {
        bsp_err("<%s> payload type: %u is not support\n", __func__, vb->payload_type);
        goto op_fail;
    }
op_fail:
    vcom_socket_free_vb(1, hd, vb);
    return 0;
}

void vcom_socket_init_workqueue(struct relay_socket_ctx *ctx)
{
    INIT_DELAYED_WORK(&ctx->write_work, relay_socket_send_data);
    ctx->socket_work_queue = create_singlethread_workqueue("vcom socket");
    if (ctx->socket_work_queue == NULL) {
        bsp_err("<%s> creat workqueue failed\n", __func__);
        return;
    }
}

void vcom_socket_tx_init(struct relay_socket_ctx *ctx)
{
    spin_lock_init(&(ctx->tx_lock));
    INIT_LIST_HEAD(&(ctx->tx_queue));
    vcom_socket_init_workqueue(ctx);
}

int vcom_socket_ready_init(struct vcom_hd *hd)
{
    struct relay_socket_ctx *ctx = NULL;

    if (hd->relay_type >= MCI_SOCKET_MAX) {
        bsp_err("<%s> socket type = %u not support\n", __func__, hd->relay_type);
        return -1;
    }

    ctx = &g_relay_socket_ctx[hd->relay_type];
    ctx->hd = hd;
    ctx->conn_state = SOCKET_DISCONNECT;
    vcom_socket_tx_init(ctx);

    ctx->chan = bsp_homi_chan_alloc(&ctx->cfg);
    if (ctx->chan == NULL) {
        ctx->stat.init_fail_num++;
        bsp_err("<%s> vdev socket ind chan alloc fail\n", __func__);
        return -EINVAL;
    } else {
        ctx->stat.init_success_num++;
        return 0;
    }
}

void vcom_socket_show(unsigned int minor_type)
{
    struct relay_socket_ctx *ctx = NULL;

    if (minor_type >= MCI_SOCKET_MAX) {
        bsp_err("[%s]socket type = %u not support\n", __func__, minor_type);
    }

    ctx = &g_relay_socket_ctx[minor_type];
    bsp_err("socket log start:\n");
    bsp_err("init fail num is        :%u", ctx->stat.init_fail_num);
    bsp_err("init success num is     :%u", ctx->stat.init_success_num);
    bsp_err("rx num is               :%u", ctx->stat.rx_num);
    bsp_err("evt num is              :%u", ctx->stat.evt_num);
    bsp_err("tx drop num is          :%u", ctx->stat.tx_drop);
    bsp_err("tx queue num is         :%u", ctx->stat.tx_queue);
    bsp_err("tx fail num is          :%u", ctx->stat.tx_fail_num);
    bsp_err("tx success num is       :%u", ctx->stat.tx_success_num);
    bsp_err("connect state           :%u", ctx->conn_state);
    bsp_err("socket log end\n");
}
