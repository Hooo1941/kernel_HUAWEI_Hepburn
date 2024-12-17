/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/netfilter_bridge.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/switchdev.h>
#include "pfa.h"
#include "pfa_ip_entry.h"
#include "pfa_mac_entry.h"
#include "pfa_port.h"
#include "pfa_dbg.h"
#include "pfa_port.h"
#include "pfa_direct_fw.h"

#ifdef CONFIG_PFA_FW
#include <net/netfilter/br_netfilter.h>
#endif

#ifdef CONFIG_PFA_FW
struct pfa_hook_ops g_pfa_hook = {0};

int pfa_skb_is_updateonly(void *skb)
{
    struct sk_buff *pfa_skb = NULL;

    pfa_skb = (struct sk_buff *)skb;

    if (pfa_skb->update_only) {
        g_pfa.cpuport.cpu_updonly_comp++;
        if (unlikely(g_pfa.msg_level & PFA_MSG_TUPLE)) {
            PFA_ERR("update only complete! \n");
        }
    }
    return (int)(pfa_skb->update_only);
}

unsigned int pfa_ip_hook_func(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    struct nf_conn *ct = NULL;
    enum ip_conntrack_info ctinfo;

    ct = nf_ct_get(skb, &ctinfo);
    if (ct != NULL) {
        ct->status |= IPS_PFA_FORWARD;
    } else {
        g_pfa.stat.ip_hook_ct_null++;
    }

    return NF_ACCEPT;
}

int bsp_pfa_enable_macfw_to_ip(unsigned int vir_portno, unsigned int enable)
{
    struct pfa *pfa = &g_pfa;
    struct pfa_port_ctrl *port_ctrl = NULL;
    unsigned int portno = PFA_GET_PHY_PORT(vir_portno);

    if (portno >= PFA_PORT_NUM) {
        return -1;
    }

    port_ctrl = &pfa->ports[portno].ctrl;
    port_ctrl->port_flags.mac_to_ip_enable = (enable == 0) ? 0 : 1;

    pfa->hal->enable_macfw_to_ip(pfa, portno, port_ctrl->port_flags.mac_to_ip_enable);

    if (pfa->direct_fw.direct_fw_timer_add == 0) {
        add_timer(&pfa->direct_fw.direct_fw_timer);
        pfa->direct_fw.direct_fw_timer_add = 1;
    }

    return 0;
}

unsigned int pfa_mac_to_ip_hook_func(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    struct pfa_direct_fw_entry_ctx ctx;
    struct ethhdr *eth_hdr;
    struct pfa *pfa = &g_pfa;
    struct pfa_port_ctrl *in_port_ctrl = NULL;
    struct pfa_port_ctrl *out_port_ctrl = NULL;
    unsigned int in_port, out_port;
    struct pfa_build_fw_entry_info info = {0};

    // filter skb
    in_port = PFA_GET_PHY_PORT(state->in->pfa_portno);
    out_port = PFA_GET_PHY_PORT(state->out->pfa_portno);
    in_port_ctrl = &pfa->ports[in_port].ctrl;
    out_port_ctrl = &pfa->ports[out_port].ctrl;
    if (!(in_port_ctrl->port_flags.mac_to_ip_enable || out_port_ctrl->port_flags.mac_to_ip_enable) ||
        (skb->update_only == 1)) {
        return NF_ACCEPT;
    }

    if (pfa->ops.pfa_ip_fw_add_filter && pfa->ops.pfa_ip_fw_add_filter(skb)) {
        pfa->stat.ipfw_mac_to_ip_filter++;
        return NF_ACCEPT;
    }

    clear_bit(PFA_EVENT_MAC_TO_IP_STOPED_BIT, (void *)&pfa->event);
    if (test_bit(PFA_EVENT_RESETTING_BIT, (void *)&pfa->event)) {
        set_bit(PFA_EVENT_MAC_TO_IP_STOPED_BIT, (void *)&pfa->event);
        wake_up(&pfa->wqueue);
        return NOTIFY_DONE;
    }

    ctx.net_id = 0;
    ctx.maped = 1;
    ctx.skb_cb_info = 0;
    ctx.add_wan_info = 0;
    ctx.l2_hdr_len = ETH_HLEN;

    skb_push(skb, ETH_HLEN);
    eth_hdr = (struct ethhdr *)(uintptr_t)skb->data;
    ether_addr_copy(ctx.dest_hd, eth_hdr->h_dest);
    ether_addr_copy(ctx.src_hd, eth_hdr->h_source);

    info.skb = skb;
    info.ctx = &ctx;
    info.portno = state->out->pfa_portno;
    info.macfw_to_ip = 1;
    info.state = state;
    pfa_build_fw_entry(&info);
    PFA_INFO("pf:0x%x, hook:0x%x, in_port:%u, out_port:%u\n",state->pf, state->hook, state->in->pfa_portno, state->out->pfa_portno);
    skb_pull(skb, ETH_HLEN);

    set_bit(PFA_EVENT_MAC_TO_IP_STOPED_BIT, (void *)&pfa->event);
    wake_up(&pfa->wqueue);
    return NF_ACCEPT;
}

const struct nf_hook_ops g_pfa_nf_ops[] = {
    {
        .hook =     pfa_ip_hook_func,
        .pf =       NFPROTO_IPV4,
        .hooknum =  NF_INET_FORWARD,
        .priority = NF_IP_PRI_FIRST,
    },
    {
        .hook =     pfa_ip_hook_func,
        .pf =       NFPROTO_IPV6,
        .hooknum =  NF_INET_FORWARD,
        .priority = NF_IP_PRI_FIRST,
    },
    {
        .hook =     pfa_mac_to_ip_hook_func,
        .pf =       NFPROTO_BRIDGE,
        .hooknum =  NF_BR_FORWARD,
        .priority = NF_BR_PRI_FIRST,
    },
};

int pfa_hook_init(void)
{
    int ret = 0;

    ret = nf_register_net_hooks(&init_net, g_pfa_nf_ops, ARRAY_SIZE(g_pfa_nf_ops));
    if (ret) {
        PFA_ERR("registering netfilter hooks fail.\n");
    }
    return ret;
}

void __net_exit pfa_hook_exit(void)
{
    nf_unregister_net_hooks(&init_net, g_pfa_nf_ops, ARRAY_SIZE(g_pfa_nf_ops));
    return;
}

int pfa_switchdev_event(struct notifier_block *nb, unsigned long event, void *ptr)
{
    struct net_device *dev = switchdev_notifier_info_to_dev(ptr);
    struct switchdev_notifier_fdb_info *fdb_info = ptr;
    struct net_bridge_port *p = br_port_get_rcu(dev);
    struct net_bridge *br = NULL;
    struct pfa *pfa = &g_pfa;

    if (p == NULL) {
        PFA_ERR(" dev correspond port of br is null, please check.\n");
        return NOTIFY_DONE;
    }

    clear_bit(PFA_EVENT_FDB_STOPED_BIT, (void *)&pfa->event);
    if (test_bit(PFA_EVENT_RESETTING_BIT, (void *)&pfa->event)) {
        set_bit(PFA_EVENT_FDB_STOPED_BIT, (void *)&pfa->event);
        wake_up(&pfa->wqueue);
        return NOTIFY_DONE;
    }

    br = p->br;
    switch (event) {
        case SWITCHDEV_FDB_ADD_TO_DEVICE:
            pfa->macfw.macfw_fdb_dev_add++;
            pfa_mac_fw_add((char *)fdb_info->addr,
                is_vlan_dev(fdb_info->info.dev) ? vlan_dev_vlan_id(fdb_info->info.dev) : 0,
                br->dev->pfa_portno, fdb_info->info.dev->pfa_portno, 0);
            break;
        case SWITCHDEV_FDB_DEL_TO_DEVICE:
            pfa->macfw.macfw_fdb_dev_del++;
            pfa_mac_fw_del((char *)fdb_info->addr,
                is_vlan_dev(fdb_info->info.dev) ? vlan_dev_vlan_id(fdb_info->info.dev) : 0,
                br->dev->pfa_portno, fdb_info->info.dev->pfa_portno);
            break;
        default:
            break;
    }
    set_bit(PFA_EVENT_FDB_STOPED_BIT, (void *)&pfa->event);
    wake_up(&pfa->wqueue);

    return NOTIFY_DONE;
}

struct notifier_block g_pfa_switchdev_notifier = {
    .notifier_call = pfa_switchdev_event,
};

/*
 * Handle changes in state of network devices enslaved to a bridge.
 * Note: don't care about up/down if bridge itself is down, because
 *     port state is checked when bridge is brought up.
 */
 #define PFA_BR_PORT_NUM_MAX  32
static char g_pfa_br_addr[PFA_BR_PORT_NUM_MAX][ETH_ALEN];
int pfa_br_device_event(struct net_device *dev, unsigned long event)
{
    int ret, portno;
    struct pfa *pfa = &g_pfa;

    if (dev == NULL || dev->pfa_portno >= PFA_BR_PORT_NUM_MAX) {
        PFA_ERR("dev is null or dev->pfa_portno err,portno %d\n", dev->pfa_portno);
        return NOTIFY_DONE;
    }

    switch (event) {
        case NETDEV_CHANGEADDR:
            pfa->macfw.macfw_dev_add++;
            pfa_mac_fw_del(g_pfa_br_addr[dev->pfa_portno], 0, dev->pfa_portno, dev->pfa_portno);
            memcpy_s(g_pfa_br_addr[dev->pfa_portno], ETH_ALEN, dev->dev_addr, ETH_ALEN);
            pfa_mac_fw_add(dev->dev_addr, 0, dev->pfa_portno, dev->pfa_portno, 1);
            return NOTIFY_DONE;
        case NETDEV_REGISTER:
            pfa->macfw.macfw_br_add++;
            portno = pfa_alloc_br_port();
            if (!PFA_BR_PORT(portno)) {
                PFA_ERR("br alloc port failed,portno %d\n", portno);
            } else {
                dev->pfa_portno = portno;
                pfa_mac_fw_add(dev->dev_addr, 0, dev->pfa_portno, dev->pfa_portno, 1);
                memcpy_s(g_pfa_br_addr[dev->pfa_portno], ETH_ALEN, dev->dev_addr, ETH_ALEN);
            }
            return NOTIFY_DONE;
        case NETDEV_UNREGISTER:
            pfa->macfw.macfw_br_del++;
            portno = dev->pfa_portno;
            pfa_mac_fw_del(dev->dev_addr, 0, dev->pfa_portno, dev->pfa_portno);
            ret = pfa_free_br_port(portno);
            if (ret) {
                PFA_INFO("br free port failed, portno %d\n", portno);
            }
            return NOTIFY_DONE;
        default:
            return NOTIFY_DONE;
    }
}

int pfa_net_device_event(struct notifier_block *unused, unsigned long event, void *ptr)
{
    struct net_device *dev = netdev_notifier_info_to_dev(ptr);
    struct netdev_notifier_changeupper_info *changeupper_info = ptr;
    enum pfa_attach_brg_type netdev_action;
    struct pfa *pfa = &g_pfa;

    /* bridge event */
    if (netif_is_bridge_master(dev)) {
        return pfa_br_device_event(dev, event);
    }

    clear_bit(PFA_EVENT_NETDEV_STOPED_BIT, (void *)&pfa->event);
    if (test_bit(PFA_EVENT_RESETTING_BIT, (void *)&pfa->event)) {
        set_bit(PFA_EVENT_NETDEV_STOPED_BIT, (void *)&pfa->event);
        wake_up(&pfa->wqueue);
        return NOTIFY_DONE;
    }

    if (netif_is_bridge_port(dev)) {
         switch (event) {
            case NETDEV_UP: // 先执行brctl addif br00 usb0，再执行ifconfig usb0 up时，触发该事件
                pfa_port_attach_brg_modify(dev->pfa_portno, pfa_attach_brg_brif);
                break;
            case NETDEV_DOWN: // 先执行brctl addif br00 usb0，ifconfig usb0 up，再执行ifconfig usb0 down时，触发该事件
                pfa_port_attach_brg_modify(dev->pfa_portno, pfa_attach_brg_normal);
                break;
            case NETDEV_PRECHANGEUPPER:  // brctl addif/delif br00 usb0, 都触发该envent, 依据changeupper_info->linking的状态
                                        // 区分addif or delif
                netdev_action = pfa_attach_brg_brif;
                if (changeupper_info->linking == false) {
                    netdev_action = pfa_attach_brg_normal;
                }
                pfa_port_attach_brg_modify(dev->pfa_portno, netdev_action);
                break;
            default:
                break;
        }
    }
    set_bit(PFA_EVENT_NETDEV_STOPED_BIT, (void *)&pfa->event);
    wake_up(&pfa->wqueue);
    return NOTIFY_DONE;
}

static struct notifier_block g_pfa_net_device_notifier = {
    .notifier_call = pfa_net_device_event
};

int pfa_macfw_hook_init(void)
{
    int err;

    err = register_netdevice_notifier(&g_pfa_net_device_notifier);
    if (err) {
        PFA_ERR("Failed to register netdevice notifier\n");
        return err;
    }

    err = register_switchdev_notifier(&g_pfa_switchdev_notifier);
    if (err) {
        PFA_ERR("Failed to register switchdev notifier\n");
        return err;
    }

    return 0;
}

void pfa_macfw_hook_exit(void)
{
    unregister_switchdev_notifier(&g_pfa_switchdev_notifier);
    return;
}

void pfa_hook_register(void)
{
    int ret;

    g_pfa_hook.ipfw_entry_add = pfa_ip_fw_add;
    g_pfa_hook.ipfw_entry_del = pfa_ip_fw_del;
    g_pfa_hook.is_updateonly = pfa_skb_is_updateonly;

    ret = pfa_hook_init();
    if (ret) {
        PFA_ERR("pfa hook init fail. \n");
    }
    ret = pfa_macfw_hook_init();
    if (ret) {
        PFA_ERR("pfa macfw hook init fail.\n");
    }
    return;
}

void pfa_hook_unregister(void)
{
    g_pfa_hook.ipfw_entry_add = NULL;
    g_pfa_hook.ipfw_entry_del = NULL;
    g_pfa_hook.is_updateonly = NULL;

    pfa_hook_exit();
    pfa_macfw_hook_exit();
}
EXPORT_SYMBOL(bsp_pfa_enable_macfw_to_ip);
EXPORT_SYMBOL(pfa_mac_to_ip_hook_func);
#endif
MODULE_ALIAS("network hardware accelerator driver");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("smart packet engine(pfa) driver");
