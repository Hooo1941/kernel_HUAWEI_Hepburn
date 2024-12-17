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
#include "relay_net.h"
#include <linux/kernel.h>
#include <securec.h>
#include <mdrv_udi.h>
#include <bsp_relay.h>
#include <linux/usb/iot/bsp_unet.h>
#include <bsp_wan_eth_stick.h>
#include <bsp_ppp_ndis.h>

#define THIS_MODU mod_relay
#define UNET_MAX_NUM 6

void relay_net_at_recv_cb(unsigned int minor_type, unsigned char *buf, unsigned int len);
#define NET_AT_RECV_CB(_name)                                                \
    static void net_at_recv_cb_##_name(unsigned char *buf, unsigned int len) \
    {                                                                        \
        relay_net_at_recv_cb(_name, buf, len);                               \
    }

NET_AT_RECV_CB(RELAY_DEV_UNET_AT);
#define NET_AT_RECV_CB_NAME(_name) (net_at_recv_cb_##_name)

void relay_net_stat_chg_cb(unsigned int minor_type, enum ncm_ioctrl_connect_stus status, void *buff);
#define NET_STAT_CHG_CB(_name)                                                           \
    static void net_stat_chg_cb_##_name(enum ncm_ioctrl_connect_stus status, void *buff) \
    {                                                                                    \
        relay_net_stat_chg_cb(_name, status, buff);                                      \
    }

NET_STAT_CHG_CB(RELAY_DEV_UNET_CTRL);
NET_STAT_CHG_CB(RELAY_DEV_PNET_CTRL);
#define NET_STAT_CHG_CB_NAME(_name) (net_stat_chg_cb_##_name)

struct mci_net_stat_chg_info {
    unsigned int net_id;
    unsigned int state;
};

struct relay_net_function {
    int (*stick_open)(void);
    int (*stick_close)(void);
    int (*stick_ioctl)(u32 dev_id, enum ncm_ioctl_cmd_type cmd, void *param);
};

static struct relay_net_function g_net_funcs[RELAY_NET_MAX_TYPE] = {
   [RELAY_DEV_UNET_CTRL] = {
        .stick_open = bsp_unet_stick_open,
        .stick_ioctl = bsp_unet_stick_ioctl,
        .stick_close = bsp_unet_stick_close,
    },
    [RELAY_DEV_UNET_AT] = {
        .stick_open = bsp_unet_stick_open,
        .stick_ioctl = bsp_unet_stick_ioctl,
        .stick_close = bsp_unet_stick_close,
    },
    [RELAY_DEV_PNET_CTRL] = {
        .stick_open = bsp_pnet_stick_open,
        .stick_ioctl = bsp_pnet_stick_ioctl,
        .stick_close = bsp_pnet_stick_close,
    },
};

static struct relay_net_device g_net_devices[RELAY_NET_MAX_TYPE] = {
    [RELAY_DEV_UNET_CTRL] = {
        .adp_dev.name = "USB NET CTRL",
        .funcs.stus_chg_cb = NET_STAT_CHG_CB_NAME(RELAY_DEV_UNET_CTRL),
    },
    [RELAY_DEV_UNET_AT] = {
        .adp_dev.name = "USB NET AT",
        .funcs.at_recv_cb = NET_AT_RECV_CB_NAME(RELAY_DEV_UNET_AT),
    },
    [RELAY_DEV_PNET_CTRL] = {
        .adp_dev.name = "PCIE NET CTRL",
        .funcs.stus_chg_cb = NET_STAT_CHG_CB_NAME(RELAY_DEV_PNET_CTRL),
    },
};

int relay_net_up_deliver(struct relay_net_device *net_dev, unsigned int cmd, unsigned char *buf, unsigned int len)
{
    int ret;

    if (net_dev->adp_dev.opend == RELAY_OPEND_MAGIC) {
        net_dev->stat.recv_cb_shed++;
        if (net_dev->cbs.net_read_cb != NULL) {
            ret = net_dev->cbs.net_read_cb(net_dev->adp_dev.private, cmd, buf, len);
            return ret;
        } else {
            net_dev->stat.no_read_cb++;
        }
    } else {
        net_dev->adp_dev.base_stat.port_not_open++;
    }
    return 0;
}

void relay_net_at_recv_cb(unsigned int minor_type, unsigned char *buf, unsigned int len)
{
    struct relay_net_device *net_dev = NULL;

    if (minor_type >= RELAY_NET_MAX_TYPE) {
        relay_err("minor_type:%d overflow\n", minor_type);
        return;
    }

    net_dev = &g_net_devices[minor_type];
    net_dev->stat.at_recv_done++;
    if (relay_net_up_deliver(net_dev, RELAY_NET_AT_RECV, buf, len) < 0) {
        net_dev->stat.at_up_fail++;
    }
    return;
}

void relay_net_stat_chg_cb(unsigned int minor_type, enum ncm_ioctrl_connect_stus status, void *buff)
{
    struct relay_net_device *net_dev = NULL;
    struct mci_net_stat_chg_info info = { 0 };

    if (minor_type >= RELAY_NET_MAX_TYPE) {
        relay_err("minor_type:%d overflow\n", minor_type);
        return;
    }

    info.state = status == NCM_IOCTL_STUS_BREAK ? 0 : 1;
    info.net_id = *(unsigned int *)buff;

    net_dev = &g_net_devices[minor_type];

    if (net_dev->adp_dev.b_enable != RELAY_ENABLE_MAGIC) {
        relay_info("minor_type: %u no need to notify state change\n", minor_type);
        return;
    }

    net_dev->stat.conn_chg_num++;
    if (relay_net_up_deliver(net_dev, RELAY_NET_STAT_CHG, (void *)&info, sizeof(info)) < 0) {
        net_dev->stat.conn_chg_fail++;
    }
    return;
}

int relay_net_set_udi_ioctl(struct relay_net_device *net_dev, unsigned int minor_type)
{
    int ret = 0;

    /* set usb read cb */
    if (minor_type == RELAY_DEV_UNET_AT) {
        ret = g_net_funcs[minor_type].stick_ioctl(0, NCM_IOCTL_REG_AT_PROCESS_FUNC, net_dev->funcs.at_recv_cb);
        if (ret) {
            relay_err("usb net set at recv cb failed\n");
            return ret;
        }
    } else {
        /* set connect status change cb */
        ret = g_net_funcs[minor_type].stick_ioctl(0, NCM_IOCTL_REG_CONNECT_STUS_CHG_FUNC, net_dev->funcs.stus_chg_cb);
        if (ret) {
            relay_err("net set stat chg cb failed\n");
            return ret;
        }
    }

    return 0;
}

int relay_net_dev_stat_init(struct relay_net_device *net_dev, unsigned int minor_type)
{
    int ret;

    ret = memset_s(&net_dev->stat, sizeof(net_dev->stat), 0, sizeof(struct relay_net_stat));
    if (ret) {
        relay_info("minor_type:%d memset_s fail:%d\n", minor_type, ret);
        return ret;
    }

    ret = memset_s(&net_dev->adp_dev.base_stat, sizeof(net_dev->adp_dev.base_stat), 0, sizeof(struct relay_base_stat));
    if (ret) {
        relay_info("minor_type:%d memset_s fail:%d\n", minor_type, ret);
        return ret;
    }
    return 0;
}

int relay_net_dev_open(unsigned int minor_type)
{
    struct relay_net_device *net_dev = NULL;
    int ret = -EINVAL;

    if (minor_type >= RELAY_NET_MAX_TYPE) {
        relay_err("minor_type:%d overflow\n", minor_type);
        return ret;
    }

    net_dev = &g_net_devices[minor_type];
    if (net_dev->adp_dev.opend == RELAY_OPEND_MAGIC) {
        relay_info("minor_type:%d has opend\n", minor_type);
        net_dev->stat.droped++;
        return ret;
    }

    ret = g_net_funcs[minor_type].stick_open();
    if (ret < 0) {
        relay_info("stick channel open fail:%d\n", ret);
        return ret;
    }

    ret = relay_net_set_udi_ioctl(net_dev, minor_type);
    if (ret) {
        relay_info("minor_type:%d set ioctl fail:%d\n", minor_type, ret);
        return ret;
    }

    ret = relay_net_dev_stat_init(net_dev, minor_type);
    if (ret < 0) {
        return ret;
    }

    net_dev->adp_dev.opend = RELAY_OPEND_MAGIC;
    net_dev->adp_dev.base_stat.open++;
    relay_info("minor_type:%d open succ\n", minor_type);
    return ret;
}

int relay_net_dev_close(unsigned int minor_type)
{
    struct relay_net_device *net_dev = NULL;
    int ret;

    if (minor_type >= RELAY_NET_MAX_TYPE) {
        relay_err("minor_type:%d overflow\n", minor_type);
        return -EINVAL;
    }

    net_dev = &g_net_devices[minor_type];
    if (net_dev->adp_dev.opend != RELAY_OPEND_MAGIC) {
        relay_info("minor_type:%d not open\n", minor_type);
        net_dev->stat.droped++;
        return -EINVAL;
    }

    ret = g_net_funcs[minor_type].stick_close();
    if (ret < 0) {
        relay_err("minor_type:%d close fail:%d\n", minor_type, ret);
    }

    net_dev->adp_dev.opend = 0;
    net_dev->adp_dev.base_stat.close++;
    return ret;
}

int relay_net_dev_write(unsigned int minor_type, char *addr, unsigned int size)
{
    return -1;
}

int relay_net_set_iface_info(unsigned int minor_type, struct netif_ctrl_param *net_param)
{
    int ret;
    struct ppp_ndis_field_info field_info;

    if (minor_type == RELAY_DEV_UNET_CTRL) {
        field_info.chan_type = PPP_NDIS_DEV_UNET;
    } else if (minor_type == RELAY_DEV_PNET_CTRL) {
        field_info.chan_type = PPP_NDIS_DEV_PNET;
    } else {
        relay_err("minor type error\n");
        return -1;
    }
    field_info.chan_id = net_param->dev_id;
    field_info.data_len = sizeof(net_param->iface_info);
    bsp_set_ndis_iface_info(&field_info, &net_param->iface_info);
    ret = g_net_funcs[minor_type].stick_ioctl(net_param->dev_id, NCM_IOCTL_SET_PDUSESSION, (void *)&net_param->iface_info);
    return ret;
}

int relay_net_get_mac_header(unsigned int minor_type, struct netif_ctrl_param *net_param)
{
    struct ppp_ndis_field_info field_info;
    struct relay_net_device *net_dev = NULL;

    if (minor_type == RELAY_DEV_UNET_CTRL) {
        field_info.chan_type = PPP_NDIS_DEV_UNET;
    } else if (minor_type == RELAY_DEV_PNET_CTRL) {
        field_info.chan_type = PPP_NDIS_DEV_PNET;
    } else {
        relay_err("minor type error\n");
        return -1;
    }
    field_info.chan_id = net_param->dev_id;
    field_info.data_len = sizeof(net_param->mac_header);
    bsp_ppp_ndis_get_mac_header(&field_info, &net_param->mac_header);

    net_dev = &g_net_devices[minor_type];
    if (relay_net_up_deliver(net_dev, RELAY_NET_ACK_CFG, (void *)net_param, sizeof(struct netif_ctrl_param)) < 0) {
        net_dev->stat.up_cfg_fail++;
        relay_err("net cfg up deliver fail\n");
    }
    return 0;
}

int relay_net_get_netdev_cfg(unsigned int minor_type, struct netif_ctrl_param *net_param)
{
    struct relay_net_device *net_dev = NULL;
    unsigned char chan_type;

    if (minor_type == RELAY_DEV_UNET_CTRL) {
        chan_type = PPP_NDIS_DEV_UNET;
    } else if (minor_type == RELAY_DEV_PNET_CTRL) {
        chan_type = PPP_NDIS_DEV_PNET;
    } else {
        relay_err("minor type error\n");
        return -1;
    }
    bsp_ppp_ndis_get_netdev_cfg(chan_type, &net_param->netdev_cfg);

    net_dev = &g_net_devices[minor_type];
    if (relay_net_up_deliver(net_dev, RELAY_NET_ACK_CFG, (void *)net_param, sizeof(struct netif_ctrl_param)) < 0) {
        net_dev->stat.up_cfg_fail++;
        relay_err("net cfg up deliver fail\n");
    }
    return 0;
}

int relay_net_write_cfg(unsigned int minor_type, void *para, unsigned int para_size)
{
    int ret = -EINVAL;
    struct netif_ctrl_param *net_param = NULL;
    struct relay_net_device *net_dev = &g_net_devices[minor_type];

    if (para_size != sizeof(struct netif_ctrl_param)) {
        relay_err("minor_type:%d buffer size error\n", minor_type);
        return ret;
    }

    net_param = (struct netif_ctrl_param *)para;
    switch (net_param->cmd) {
        case NETIF_SET_SPEED_CHANGE:
            ret = g_net_funcs[minor_type].stick_ioctl(net_param->dev_id, NCM_IOCTL_CONNECTION_SPEED_CHANGE_NOTIF, (void *)&net_param->conn_speed);
            break;
        case NETIF_SET_CONNECT_STATE:
            ret = g_net_funcs[minor_type].stick_ioctl(net_param->dev_id, NCM_IOCTL_NETWORK_CONNECTION_NOTIF, (void *)&net_param->conn_notify);
            break;
        case NETIF_SET_IFACE_INFO:
            ret = relay_net_set_iface_info(minor_type, net_param);
            break;
        case NETIF_QUERY_MAC_HEADER:
            ret = relay_net_get_mac_header(minor_type, net_param);
            break;
        case NETIF_QUERY_NETDEV_CFG:
            ret = relay_net_get_netdev_cfg(minor_type, net_param);
            break;
        default:
            relay_err("net cmd: %d is error\n", net_param->cmd);
    }

    if (ret < 0) {
        net_dev->stat.write_cfg_fail++;
    }
    return ret;
}

int relay_net_at_query_ready_state(struct relay_net_device *net_dev, unsigned int minor_type)
{
    int ret;
    unsigned int ready = net_dev->adp_dev.b_enable == RELAY_ENABLE_MAGIC ? 1 : 0;

    if (net_dev->cbs.net_read_cb != NULL) {
        ret = net_dev->cbs.net_read_cb(net_dev->adp_dev.private, RELAY_NET_NOTIFY_READY,
                                       (void*)&ready, sizeof(unsigned int));
        if (ret < 0) {
            relay_err("minor_type:%d notify ready fail\n", minor_type);
            return ret;
        }
    }
    return 0;
}

int relay_net_ctrl_query_ready_state(struct relay_net_device *net_dev, unsigned int minor_type, unsigned int net_num)
{
    int ret;
    unsigned int i;   
    struct mci_net_stat_chg_info info = { 0 };

    for (i = 0; i < net_num; i++) {
        if (minor_type == RELAY_DEV_UNET_CTRL) {
            info.state = bsp_ppp_ndis_get_ready_state(PPP_NDIS_DEV_UNET, i);
        } else {
            info.state = net_dev->adp_dev.b_enable == RELAY_ENABLE_MAGIC ? 1 : 0;
        }
        info.net_id = i;
        if (net_dev->cbs.net_read_cb != NULL) {
            ret = net_dev->cbs.net_read_cb(net_dev->adp_dev.private, RELAY_NET_STAT_CHG,
                                           (void*)&info, sizeof(info));
            if (ret < 0) {
                relay_err("minor_type:%d notify ready fail\n", minor_type);
                return ret;
            }
        } else {
            return -1;
        }
    }
    return 0;
}

int relay_net_query_ready_state(struct relay_net_device *net_dev, unsigned int minor_type)
{
    if (minor_type == RELAY_DEV_UNET_AT) {
        return relay_net_at_query_ready_state(net_dev, minor_type);
    }

    if (minor_type == RELAY_DEV_UNET_CTRL) {
        return relay_net_ctrl_query_ready_state(net_dev, minor_type, bsp_unet_stick_card_num());
    } else {
        return relay_net_ctrl_query_ready_state(net_dev, minor_type, 8);
    }
}

int relay_net_dev_ioctl(unsigned int minor_type, unsigned int cmd, void *para, unsigned int para_size)
{
    int ret = 0;
    struct relay_net_device *net_dev = NULL;

    if (minor_type >= RELAY_NET_MAX_TYPE || para == NULL || para_size == 0) {
        relay_err("minor_type:%d param err\n", minor_type);
        return -EINVAL;
    }

    net_dev = &g_net_devices[minor_type];
    if (net_dev->adp_dev.opend != RELAY_OPEND_MAGIC) {
        relay_info("minor_type:%d not open\n", minor_type);
        net_dev->stat.droped++;
        return -EINVAL;
    }

    switch (cmd) {
        case RELAY_NET_REG_READ_CB:
            net_dev->cbs.net_read_cb = (relay_net_read_cb)para;
            break;
        case RELAY_NET_WRITE_CFG:
            return relay_net_write_cfg(minor_type, para, para_size);
        case RELAY_NET_AT_RESPONSE: {
            struct ncm_at_rsp rsp_pkt;
            rsp_pkt.at_answer = para;
            rsp_pkt.length = para_size;
            ret = g_net_funcs[minor_type].stick_ioctl(0, NCM_IOCTL_AT_RESPONSE, (void*)&rsp_pkt);
            break;
        }
        case RELAY_NET_QUERY_READY:
            return relay_net_query_ready_state(net_dev, minor_type);
        default:
            return -EINVAL;
    }

    return ret;
}

void relay_net_enable(unsigned int minor_type)
{
    struct relay_net_device *net_dev = NULL;

    net_dev = &g_net_devices[minor_type];
    if (net_dev->cbs.enable_cb != NULL) {
        net_dev->cbs.enable_cb(net_dev->adp_dev.private);
    } else {
        relay_info("minor_type:%d, enable_cb NULL\n", minor_type);
    }

    net_dev->adp_dev.b_enable = RELAY_ENABLE_MAGIC;
    relay_net_query_ready_state(net_dev, minor_type);
    return;
}

void relay_net_disable(unsigned int minor_type)
{
    struct relay_net_device *net_dev = NULL;

    net_dev = &g_net_devices[minor_type];
    net_dev->adp_dev.b_enable = 0;
    relay_net_query_ready_state(net_dev, minor_type);

    if (net_dev->cbs.disable_cb != NULL) {
        net_dev->cbs.disable_cb(net_dev->adp_dev.private);
    } else {
        relay_info("minor_type:%d, disable_cb NULL\n", minor_type);
    }
    return;
}

void relay_pnet_enable(void)
{
    relay_net_enable(RELAY_DEV_PNET_CTRL);
}

void relay_pnet_disable(void)
{
    relay_net_disable(RELAY_DEV_PNET_CTRL);
}

int relay_net_init(void)
{
    int ret;

    ret = bsp_pnet_register_state_enable_cb(relay_pnet_enable);
    if (ret < 0) {
        relay_err("minor_type:%d reg enablecb fail\n", RELAY_DEV_PNET_CTRL);
        return ret;
    }

    ret = bsp_pnet_register_state_disable_cb(relay_pnet_disable);
    if (ret < 0) {
        relay_err("minor_type:%d reg disablecb fail\n", RELAY_DEV_PNET_CTRL);
        return ret;
    }
    return 0;
}

int relay_net_reg_enablecb(unsigned int minor_type, relay_enable_cb func, void *private)
{
    struct relay_net_device *net_dev = NULL;

    if (minor_type >= RELAY_NET_MAX_TYPE || func == NULL) {
        relay_err("minor_type:%d param err\n", minor_type);
        return -EINVAL;
    }

    net_dev = &g_net_devices[minor_type];
    net_dev->cbs.enable_cb = func;
    net_dev->adp_dev.private = private;

    if (net_dev->adp_dev.b_enable == RELAY_ENABLE_MAGIC) {
        net_dev->cbs.enable_cb(private);
    }
    return 0;
}

int relay_net_reg_disablecb(unsigned int minor_type, relay_disable_cb func)
{
    struct relay_net_device *net_dev = NULL;

    if (minor_type >= RELAY_NET_MAX_TYPE || func == NULL) {
        relay_err("minor_type:%d param err\n", minor_type);
        return -EINVAL;
    }

    net_dev = &g_net_devices[minor_type];
    net_dev->cbs.disable_cb = func;

    return 0;
}

void relay_net_info_show(unsigned int minor_type)
{
    struct relay_net_device *net_dev = NULL;

    if (minor_type >= RELAY_NET_MAX_TYPE) {
        relay_err("minor_type: %d overflow\n", minor_type);
        return;
    }
    net_dev = &g_net_devices[minor_type];

    relay_err("minor_type       : %u\n", minor_type);
    relay_err("dev name         : %s\n", net_dev->adp_dev.name);

    relay_err("net_read_cb      : %s\n", net_dev->cbs.net_read_cb ? "good" : "null");
    relay_err("enable_cb        : %s\n", net_dev->cbs.enable_cb ? "good" : "null");
    relay_err("disable_cb       : %s\n", net_dev->cbs.disable_cb ? "good" : "null");

    relay_err("open             : %u\n", net_dev->adp_dev.base_stat.open);
    relay_err("open_fail        : %u\n", net_dev->adp_dev.base_stat.open_fail);
    relay_err("close            : %u\n", net_dev->adp_dev.base_stat.close);
    relay_err("port_not_open    : %u\n", net_dev->adp_dev.base_stat.port_not_open);

    relay_err("at_recv_done     : %u\n", net_dev->stat.at_recv_done);
    relay_err("no_read_cb       : %u\n", net_dev->stat.no_read_cb);
    relay_err("at_up_fail       : %u\n", net_dev->stat.at_up_fail);
    relay_err("at_recv_len      : %u\n", net_dev->stat.at_recv_len);
    relay_err("recv_cb_shed     : %u\n", net_dev->stat.recv_cb_shed);
    relay_err("conn_chg_num     : %u\n", net_dev->stat.conn_chg_num);
    relay_err("conn_chg_fail    : %u\n", net_dev->stat.conn_chg_fail);
    relay_err("write_cfg_fail   : %u\n", net_dev->stat.write_cfg_fail);
    relay_err("up_cfg_fail      : %u\n", net_dev->stat.up_cfg_fail);
}
