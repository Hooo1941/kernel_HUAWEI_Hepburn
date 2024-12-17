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
#include <linux/skbuff.h>
#include <linux/netlink.h>
#include <net/netlink.h>
#include <net/sock.h>
#include <osl_types.h>
#include <osl_malloc.h>
#include <osl_list.h>
#include <osl_spinlock.h>
#include <osl_sem.h>
#include <bsp_dt.h>
#include <bsp_homi.h>
#include "homi_core.h"
#include "homi_debug.h"
#include "homi_netlink.h"

/* must same order to homi_chan_type of netlink */
static int g_netlinkport[0x2] = {29, 31};
u32 g_homi_netlink_get_cnt = 0;
u32 g_homi_netlink_put_succ_cnt = 0;
u32 g_homi_netlink_put_fail_cnt = 0;

void netlink_debug_show_refcnt(void)
{
    homi_err("netlink get cnt: 0x%x\n", g_homi_netlink_get_cnt);
    homi_err("netlink put succ cnt: 0x%x\n", g_homi_netlink_put_succ_cnt);
    homi_err("netlink put fail cnt: 0x%x\n", g_homi_netlink_put_fail_cnt);
}

/* must put chan after used */
static struct homi_chan *netlink_get_pair(struct homi_chan *chan)
{
    struct homi_chan *pchan = NULL;
    enum homi_chan_type ctype;

    switch (chan->type) {
        case HOMI_CH_NL_31:
            ctype = HOMI_CH_NL_29;
            break;
        default:
            homi_err("use invalid pair type:%d get pair socket\n", chan->type);
            return NULL;
    }

    /* must put chan after used */
    pchan = homi_get_valid_chan_by_type(HOMI_PORT_NETLINK, ctype);
    if (pchan == NULL) {
        homi_err("get pair socket chan fail\n");
        return NULL;
    }

    return pchan;
}

static void netlink_bind_pair(struct homi_chan *chan, unsigned int rcvpid)
{
    struct homi_chan *pchan;
    struct netlink_chan_cfg *nlcfg = NULL;

    /* must put chan after usded */
    pchan = netlink_get_pair(chan);
    if (pchan == NULL) {
        homi_err("bind %d netlink chan get null pair chan\n", chan->type);
        return;
    }

    nlcfg = (struct netlink_chan_cfg *)pchan->private;
    nlcfg->rcvpid = rcvpid;

    osl_sem_down(&pchan->chsem);
    pchan->state = HOMI_CHSTATE_CONNECTED;
    osl_sem_up(&pchan->chsem);

    if (pchan->cb.eventcb != NULL) {
        pchan->cb.eventcb(HOMI_PORT_NETLINK, HOMI_CHSTATE_CONNECTED);
    }

    homi_chan_put(pchan);
    return;
}

static void netlink_input(struct sk_buff *skb)
{
    struct homi_chan *chan = NULL;
    struct netlink_chan_cfg *nlcfg = NULL;
    struct sock *sk = NULL;
    struct nlmsghdr *nlh = NULL;
    void *data = NULL;
    int len;

    if ((skb == NULL) || (skb->sk == NULL)) {
        homi_err("netlink input get null skb\n");
        return;
    }

    sk = skb->sk;
    nlh = nlmsg_hdr(skb);
    if (!NLMSG_OK(nlh, skb->len)) {
        homi_err("netlink input len invalid, nlh_len=0x%x, skb_len=0x%x\n", nlh->nlmsg_len, skb->len);
        return;        
    }
	
    if (nlh->nlmsg_pid == 0) {
        homi_err("netlink nlmsg_pid is 0\n");
        return;
    }
	
    /* if sk user data not null and not homi chan, will error */
    chan = (struct homi_chan *)rcu_dereference_sk_user_data(sk);
    if ((chan == NULL) || (chan->flag != HOMI_CHAN_MAGICNUM)) {
        homi_err("netlink user data not homi chan\n");
        return;
    }
    homi_debug(HLEVDBG, "%d netlink chan get data\n", chan->type);

    nlcfg = (struct netlink_chan_cfg *)chan->private;

    osl_sem_down(&chan->chsem);
    chan->state = HOMI_CHSTATE_CONNECTED;
    osl_sem_up(&chan->chsem);

    if (chan->cb.eventcb != NULL) {
        chan->cb.eventcb(HOMI_PORT_NETLINK, HOMI_CHSTATE_CONNECTED);
    }
    
    data = nlmsg_data(nlh);
    len = nlmsg_len(nlh);

    nlcfg->rcvpid = nlh->nlmsg_pid;

    /* need decoupe pid from another chan */
    netlink_bind_pair(chan, nlh->nlmsg_pid);

    if (chan->cb.recvcb != NULL) {
        chan->cb.recvcb(chan, data, len);
    }

    homi_debug(HLEVDBG, "%d netlink chan handler data end\n", chan->type);
    return;
}

static int netlink_cfg_init(struct homi_chan *chan, struct netlink_chan_cfg *nlcfg)
{
    switch (chan->type) {
        case HOMI_CH_NL_29:
            nlcfg->nonblock = 0;
            break;
        case HOMI_CH_NL_31:
            nlcfg->nonblock = MSG_DONTWAIT;
            break;
        default:
            homi_err("netlink cfg invalid, chan type:%d\n", chan->type);
            return BSP_HOMI_NL_TYPE_ERR;
    }

    nlcfg->port = g_netlinkport[chan->type - HOMI_CH_NL_BASE];

    nlcfg->kncfg.input = netlink_input;

    return 0;
}

static int netlink_chan_init(struct homi_chan *chan)
{
    int ret;
    struct netlink_chan_cfg *nlcfg;

    nlcfg = (struct netlink_chan_cfg *)osl_malloc(sizeof(struct netlink_chan_cfg));
    if (nlcfg == NULL) {
        homi_err("netlink chan cfg malloc fail\n");
        return BSP_HOMI_NL_CFG_MALLOC_ERR;
    }

    (void)memset_s(nlcfg, sizeof(struct netlink_chan_cfg), 0, sizeof(struct netlink_chan_cfg));

    ret = netlink_cfg_init(chan, nlcfg);
    if (ret) {
        homi_err("netlink chan cfg init fail:%d\n", ret);
        goto cfg_release;
    }

    nlcfg->sk = netlink_kernel_create(&init_net, nlcfg->port, &nlcfg->kncfg);
    if (nlcfg->sk == NULL) {
        homi_err("netlink kernel create fail\n");
        ret = BSP_HOMI_NL_CREATE_ERR;
        goto cfg_release;
    }

    chan->private = (void *)nlcfg;
    rcu_assign_sk_user_data(nlcfg->sk, chan);

    homi_printf("%d netlink chan init success\n", chan->type);
    return 0;

cfg_release:
    osl_free(nlcfg);
    return ret;
}

static int netlink_chan_send(struct homi_chan *chan, void *data, unsigned int length)
{
    int ret;
    void *sdata = NULL;
    struct sk_buff *skb = NULL;
    struct nlmsghdr *nlh = NULL;
    int nlength = NLMSG_ALIGN(length);
    struct netlink_chan_cfg *nlcfg = (struct netlink_chan_cfg *)chan->private;

    /* must put chan after used */
    if (!homi_chan_test_and_get(chan, HOMI_CHSTATE_CONNECTED)) {
        homi_err("%d netlink chan not connected, can't send data\n", chan->type);
        return BSP_HOMI_CHAN_NOT_CONNECT_ERR;
    }
    g_homi_netlink_get_cnt++;

    skb = nlmsg_new(nlength, GFP_KERNEL);
    if (skb == NULL) {
        homi_err("%d chan new netlink skb fail\n", chan->type);
        ret = BSP_HOMI_NL_NEW_SKB_ERR;
        goto put_chan;
    }

    nlh = nlmsg_put(skb, 0, 0, NLMSG_DONE, nlength, 0);
    if (nlh == NULL) {
        homi_err("%d chan put netlink nlh fail\n", chan->type);
        ret = BSP_HOMI_NL_PUT_NLH_ERR;
        goto skb_free;
    }

    sdata = nlmsg_data(nlh);
    if (memcpy_s(sdata, length, data, length)) {
        homi_err("%d chan netlink memcpy fail\n", chan->type);
        ret = BSP_HOMI_MEMCPY_ERR;
        goto skb_free;
    }

    ret = netlink_unicast(nlcfg->sk, skb, nlcfg->rcvpid, nlcfg->nonblock);
    if (ret < 0) {
        homi_err("%d chan netlink send data fail\n", chan->type);
        goto put_chan;
    }

    homi_debug(HLEVDBG, "%d chan netlink send data success\n", chan->type);
    homi_chan_put(chan);
    g_homi_netlink_put_succ_cnt++;
    return 0;

skb_free:
    kfree_skb(skb);
put_chan:
    homi_chan_put(chan);
    g_homi_netlink_put_fail_cnt++;
    return ret;
}

static int netlink_chan_release(struct homi_chan *chan)
{
    struct netlink_chan_cfg *nlcfg = (struct netlink_chan_cfg *)chan->private;

    osl_free(nlcfg);

    return 0;
}

static struct homi_chan_ops g_netlink_chan_ops = {
    .init = netlink_chan_init,
    .send = netlink_chan_send,
    .release = netlink_chan_release,
};

static struct homi_chan *homi_netlink_chan_alloc(struct homi_port *port, struct homi_cfg *cfg)
{
    struct homi_chan *chan;

    chan = homi_chan_init(port, &g_netlink_chan_ops, cfg);
    if (chan == NULL) {
        homi_err("homi socket chan alloc init fail\n");
        return NULL;
    }

    osl_sem_down(&chan->chsem);
    if (chan->state == HOMI_CHSTATE_INITING) {
        chan->state = HOMI_CHSTATE_DISCONNECTED;
    }
    osl_sem_up(&chan->chsem);

    return chan;
}

static int homi_netlink_chan_release(struct homi_port *port, struct homi_chan *chan)
{
    unsigned long flags;
    int refcnt;
    struct homi_ctrl *hctrl = port->hctrl;

    spin_lock_irqsave(&chan->ref_lock, flags);
    refcnt = chan->refcnt;
    spin_unlock_irqrestore(&chan->ref_lock, flags);

    if (refcnt != 0) {
        /* chan must have removed from port */
        spin_lock_irqsave(&hctrl->gc_lock, flags);
        list_add_tail(&chan->list, &hctrl->gc_list);
        spin_unlock_irqrestore(&hctrl->gc_lock, flags);
        queue_delayed_work(system_wq, &hctrl->gc_work, msecs_to_jiffies(HOMI_CH_GC_TIME));
        homi_err("homi chan refcount is not 0, add to gc list\n");
        return 0;
    }

    if (chan->ops->release != NULL) {
        chan->ops->release(chan);
    }

    kfree(chan);

    return 0;
}

int homi_netlink_init(void)
{
    int ret;
    struct homi_port *port;
    const char *dtspath = "/homi_cust";
    device_node_s *custnd = NULL;

    custnd = bsp_dt_find_node_by_path(dtspath);
    if (custnd == NULL) {
        homi_err("homi netlink port read dts path fail\n");
        return BSP_HOMI_NL_DTS_PORT_CUST_ERR;
    }

    ret = bsp_dt_property_read_u32_array(custnd, "netlinkport", g_netlinkport, sizeof(g_netlinkport) / sizeof(g_netlinkport[0]));
    if (ret) {
        homi_err("homi netlink port read dts fail\n");
        return BSP_HOMI_NL_DTS_PORT_CUST_ERR;
    }

    port = homi_port_alloc();
    if (port == NULL) {
        homi_err("homi netlink port alloc fail\n");
        return BSP_HOMI_PORT_MALLOC_ERR;
    }

    port->type = HOMI_PORT_NETLINK;

    port->alloc = homi_netlink_chan_alloc;
    port->release = homi_netlink_chan_release;

    /* register port at last */
    ret = homi_port_register(port);
    if (ret) {
        osl_free(port);
        homi_err("homi netlink port register fail, ret:%d\n", ret);
        return ret;
    }

    homi_printf("homi netlink port init success\n");
    return 0;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
module_init(homi_netlink_init);
#endif
