/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
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
#include <securec.h>
#include <linux/workqueue.h>
#include <osl_types.h>
#include <osl_list.h>
#include <osl_spinlock.h>
#include <osl_sem.h>
#include <osl_malloc.h>
#include <bsp_homi.h>
#include "homi_core.h"
#include "homi_debug.h"
#include "homi_genl.h"

static struct homi_ctrl *g_homi_ctrl = NULL;

static struct homi_ctrl *homi_ctrl_get(void)
{
    return g_homi_ctrl;
}

/* get chan reference must in valid state including initing/
 * disconnected/connected state.
 */
static void homi_chan_get(struct homi_chan *chan)
{
    unsigned long flags;

    spin_lock_irqsave(&chan->ref_lock, flags);
    chan->refcnt++;
    spin_unlock_irqrestore(&chan->ref_lock, flags);
}

void homi_chan_put(struct homi_chan *chan)
{
    unsigned long flags;

    spin_lock_irqsave(&chan->ref_lock, flags);
    chan->refcnt--;
    spin_unlock_irqrestore(&chan->ref_lock, flags);
}

/* if chan is in state, then get chan.
 * if got, must put chan after used.
 */
bool homi_chan_test_and_get(struct homi_chan *chan, enum homi_chan_state state)
{
    bool flag = false;

    osl_sem_down(&chan->chsem);
    if (chan->state == state) {
        homi_chan_get(chan);
        flag = true;
    }
    osl_sem_up(&chan->chsem);

    return flag;
}

/* valid chan include initing/disconnected/connected state chan,
 * not include removed chan. must put chan after used.
 */
static struct homi_chan *homi_get_valid_chan(struct homi_port *port, enum homi_chan_type type)
{
    unsigned long flags;
    struct homi_chan *tmp = NULL;
    struct homi_chan *chan = NULL;
    struct homi_chan *real_chan = NULL;

    spin_lock_irqsave(&port->lock, flags);
    list_for_each_entry_safe(chan, tmp, &port->list, list) {
        if (chan->type == type) {
            real_chan = chan;
            homi_chan_get(real_chan);
            break;
        }
    }
    spin_unlock_irqrestore(&port->lock, flags);

    if (real_chan == NULL) {
        return NULL;
    }

    osl_sem_down(&real_chan->chsem);
    if (real_chan->state == HOMI_CHSTATE_REMOVED) {
        homi_chan_put(real_chan);
        osl_sem_up(&real_chan->chsem);
        return NULL;
    }
    osl_sem_up(&real_chan->chsem);

    return real_chan;
}

/* find not removed homi chan in port list and homi gc_list */
static struct homi_chan *homi_chan_find(struct homi_port *port, enum homi_chan_type type)
{
    unsigned long flags;
    struct homi_chan *tmp = NULL;
    struct homi_chan *chan = NULL;
    struct homi_ctrl *hctrl = port->hctrl;

    spin_lock_irqsave(&port->lock, flags);
    list_for_each_entry_safe(chan, tmp, &port->list, list) {
        if (chan->type == type) {
            return chan;
        }
    }
    spin_unlock_irqrestore(&port->lock, flags);

    spin_lock_irqsave(&hctrl->gc_lock, flags);
    list_for_each_entry_safe(chan, tmp, &hctrl->gc_list, list) {
        if (chan->type == type) {
            return chan;
        }
    }
    spin_unlock_irqrestore(&hctrl->gc_lock, flags);

    return NULL;
}

static struct homi_chan *homi_chan_alloc(void)
{
    struct homi_chan *chan;

    chan = (struct homi_chan *)osl_malloc(sizeof(struct homi_chan));
    if (chan == NULL) {
        homi_err("homi chan malloc err\n");
        return NULL;
    }

    (void)memset_s(chan, sizeof(struct homi_chan), 0, sizeof(struct homi_chan));

    chan->refcnt = 0;
    spin_lock_init(&chan->ref_lock);

    osl_sem_init(1, &chan->chsem);
    osl_sem_init(1, &chan->rdsem);

    INIT_LIST_HEAD(&chan->list);

    return chan;
}

struct homi_chan *homi_chan_init(struct homi_port *port, struct homi_chan_ops *ops, struct homi_cfg *cfg)
{
    int ret;
    unsigned long flags;
    struct homi_chan *chan;

    /* find chan in port list and homi gc list */
    chan = homi_chan_find(port, cfg->ctype);
    if (chan != NULL) {
        homi_err("homi port %d chan have alloc\n", cfg->ctype);
        return NULL;
    }

    chan = homi_chan_alloc();
    if (chan == NULL) {
        homi_err("homi port %d chan alloc err\n", cfg->ctype);
        return NULL;
    }

    chan->type = cfg->ctype;
    chan->port = port;
    chan->ops = ops;

    chan->blength = cfg->blength;
    chan->bnum = cfg->bnum;

    chan->cb.sendcb = cfg->cb.sendcb;
    chan->cb.recvcb = cfg->cb.recvcb;
    chan->cb.eventcb = cfg->cb.eventcb;

    if ((chan->ops->init == NULL) || \
            (chan->ops->send == NULL)) {
        osl_free(chan);
        homi_err("homi chan ops register err\n");
        return NULL;
    }

    spin_lock_irqsave(&port->lock, flags);
    list_add_tail(&chan->list, &port->list);
    spin_unlock_irqrestore(&port->lock, flags);

    ret = chan->ops->init(chan);
    if (ret) {
        spin_lock_irqsave(&port->lock, flags);
        list_del(&chan->list);
        spin_unlock_irqrestore(&port->lock, flags);
        osl_free(chan);
        homi_err("homi chan ops init err\n");
        return NULL;
    }

    chan->flag = HOMI_CHAN_MAGICNUM;

    return chan;
}

struct homi_port *homi_get_port(enum homi_port_type type)
{
    unsigned long flags;
    struct homi_port *p = NULL;
    struct homi_ctrl *homi_ctrl = homi_ctrl_get();

    if (homi_ctrl == NULL) {
        return NULL;
    }

    spin_lock_irqsave(&homi_ctrl->r_lock, flags);
    for (p = homi_ctrl->hport; p != NULL; p = p->next) {
        if (p->type == type) {
            break;
        }
    }
    spin_unlock_irqrestore(&homi_ctrl->r_lock, flags);

    return p;
}

struct homi_port *homi_port_alloc(void)
{
    struct homi_port *port;

    port = (struct homi_port *)osl_malloc(sizeof(struct homi_port));
    if (port == NULL) {
        homi_err("homi port malloc err\n");
        return NULL;
    }

    (void)memset_s(port, sizeof(struct homi_port), 0, sizeof(struct homi_port));

    port->type = HOMI_PORT_MAX;
    INIT_LIST_HEAD(&port->list);
    spin_lock_init(&port->lock);

    return port;
}

/* must put chan after used */
struct homi_chan *homi_get_valid_chan_by_type(enum homi_port_type ptype, enum homi_chan_type ctype)
{
    struct homi_port *port;
    struct homi_chan *chan = NULL;

    port = homi_get_port(ptype);
    if (port == NULL) {
        homi_err("homi get chan find port err\n");
        return NULL;
    }

    chan = homi_get_valid_chan(port, ctype);
    if (chan == NULL) {
        homi_err("homi get chan find chan err\n");
        return NULL;
    }

    return chan;
}

int homi_port_register(struct homi_port *port)
{
    unsigned long flags;
    struct homi_port **p = NULL;
    struct homi_ctrl *homi_ctrl = homi_ctrl_get();

    if (homi_ctrl == NULL) {
        homi_err("homi ctrl is null err\n");
        return BSP_HOMI_INIT_ERR;
    }

    spin_lock_irqsave(&homi_ctrl->r_lock, flags);
    for (p = &homi_ctrl->hport; *p != NULL; p = &(*p)->next) {
        if ((*p)->type == port->type) {
            spin_unlock_irqrestore(&homi_ctrl->r_lock, flags);
            return BSP_HOMI_REGISTER_PORT_ERR;
        }
    }

    *p = port;
    port->hctrl = homi_ctrl;
    homi_ctrl->h_cnt++;
    spin_unlock_irqrestore(&homi_ctrl->r_lock, flags);

    return 0;
}

static void homi_chan_gc_handler(struct work_struct *work)
{
    int refcnt;
    unsigned long flags;
    int need_resched = 0;
    struct homi_chan *tmp = NULL;
    struct homi_chan *chan = NULL;
    struct homi_ctrl *homi_ctrl = container_of(work, struct homi_ctrl, gc_work.work);

    spin_lock_irqsave(&homi_ctrl->gc_lock, flags);
    list_for_each_entry_safe(chan, tmp, &homi_ctrl->gc_list, list) {
        spin_lock_irq(&chan->ref_lock);
        refcnt = chan->refcnt;
        spin_unlock_irq(&chan->ref_lock);

        if (refcnt != 0) {
            need_resched = 1;
            continue;
        }

        if (chan->ops->release != NULL) {
            chan->ops->release(chan);
        }

        list_del(&chan->list);
        kfree(chan);
    }
    spin_unlock_irqrestore(&homi_ctrl->gc_lock, flags);

    if (need_resched) {
        queue_delayed_work(system_wq, &homi_ctrl->gc_work, msecs_to_jiffies(HOMI_CH_GC_TIME));
    }
}

int homi_init(void)
{
    struct homi_ctrl *homi_ctrl;
    int ret;

    homi_ctrl = (struct homi_ctrl *)osl_malloc(sizeof(struct homi_ctrl));
    if (homi_ctrl == NULL) {
        homi_err("homi_ctrl malloc err\n");
        return BSP_HOMI_MALLOC_ERR;
    }

    (void)memset_s(homi_ctrl, sizeof(struct homi_ctrl), 0, sizeof(struct homi_ctrl));

    INIT_LIST_HEAD(&homi_ctrl->gc_list);
    spin_lock_init(&homi_ctrl->gc_lock);
    INIT_DELAYED_WORK(&homi_ctrl->gc_work, homi_chan_gc_handler);

    spin_lock_init(&homi_ctrl->r_lock);

    g_homi_ctrl = homi_ctrl;

    ret = homi_genl_init();
    if (ret) {
        return ret;
    }

    homi_printf("homi core init success\n");
    return 0;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(homi_init);
#endif
