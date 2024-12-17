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

#ifndef BSP_PPP_NDIS_H
#define BSP_PPP_NDIS_H

#include <linux/skbuff.h>
#include <linux/types.h>
#include <product_config.h>

#define PPP_NDIS_DEV_MODEM 0x00
#define PPP_NDIS_DEV_UNET 0x01
#define PPP_NDIS_DEV_PNET 0x02

enum nic_address_type {
    HOST_ADDR_TYPE,
    DEV_ADDR_TYPE,
    GW_ADDR_TYPE,
    MAX_ADDR_TYPE,
};

struct ppp_ndis_field_info {
    unsigned short data_len;
    unsigned char chan_type;
    unsigned char chan_id;
}; /* rd_user_field1 */

struct ppp_ndis_mac_index {
    struct ppp_ndis_field_info field_info;
    enum nic_address_type addr_type;
};

struct ppp_ndis_change_info {
    struct ppp_ndis_field_info field_info;
    unsigned int state;
};

struct ppp_ndis_ready_info {
    struct ppp_ndis_field_info field_info;
    unsigned int state;
};

struct net_conn_speed {
    unsigned int downlink; /* 下行速率 */
    unsigned int uplink;   /* 上行速率 */
};

struct net_dev_cfg {
    unsigned int rawip_en : 1;     /* 数据包类型RAWIP配置使能 */
    unsigned int ipv4_auto_en : 1; /* ipv4地址自动配置使能 */
    unsigned int ipv6_auto_en : 1; /* ipv6地址自动配置使能 */
    unsigned int reserve : 29;     /* 保留字段 */
};

#define MAC_ADDR_LEN 6
struct net_mac_header {
    unsigned char host_mac[MAC_ADDR_LEN]; /* 主机端MAC地址 */
    unsigned char gw_mac[MAC_ADDR_LEN];   /* 网关MAC地址 */
};

enum net_iface_mode {
    PPP_NDIS_NET_MODE_IP,
    PPP_NDIS_NET_MODE_ETH,
    PPP_NDIS_NET_MODE_MAX,
};

/**
 * @brief 包标记信息描述结构体
 */
struct net_packet_label {
    unsigned char sess_id;  /* pdusession id */
    unsigned char modem_id; /* modem ID */
    unsigned char fc_head;  /* 过滤链头 */
    union {
        unsigned char reserved; /* packet label 保留字段 */
        struct {
            unsigned char arp_copy_en : 1;
            unsigned char resv : 7;
        };
    };
};

/**
 * @brief net_iface信息描述结构体
 * 对应cmd: VCOM_IOCTL_SET_IFACE_INFO
 */
struct net_iface_info {
    unsigned char iface_mode; /* 对应 enum vcom_net_iface_mode */
    unsigned char iface_id;   /* 全局网卡ID */
    unsigned char link_state; /* 对应 enum vcom_net_link_state */
    unsigned char rsv[1];     /* iface info 保留字段 */
    union {
        struct {
            struct net_packet_label v4_label; /* ipv4 包标识符 */
            struct net_packet_label v6_label; /* ipv6 包标识符 */
        };
        struct net_packet_label eth_label; /* *< ETH mode, 包标识符 */
    };
};

struct netif_ctrl_param {
    unsigned int cmd;    /* 网卡通道命令字，对应PPP_netif_ioctl_cmd */
    unsigned int dev_id; /* 网卡设备ID */
    union {
        struct net_iface_info iface_info; /* 网卡连接信息 */
        struct net_mac_header mac_header; /* mac头 */
        struct net_conn_speed conn_speed; /* 网络连接速率 */
        struct net_dev_cfg netdev_cfg;    /* 网卡驱动配置信息 */
        unsigned int conn_notify;         /* 网络连接状态通知，0:断开/1:连接 */
    };
};

enum ppp_trans_mode {
    PPP_TRANS_MODE_CMD,
    PPP_TRANS_MODE_DATA,
    PPP_TRANS_MODE_MAX,
};

struct ppp_iface_info {
    unsigned char iface_id; // 全局网卡ID
    unsigned char sess_id;  // pdusession id
    unsigned char modem_id; // modem ID
    unsigned char fc_head;  // 过滤链头
};

typedef int (*ppp_ndis_get_mac_cb)(struct ppp_ndis_mac_index *index_info, unsigned char *mac_addr);
typedef void (*ppp_ndis_notify_ready_cb)(struct ppp_ndis_ready_info *ready_info);

#ifdef CONFIG_USB_PPP_NDIS
void bsp_set_ndis_iface_info(struct ppp_ndis_field_info *field_info, struct net_iface_info *iface_info);
void bsp_set_ppp_iface_info(struct ppp_ndis_field_info *field_info, struct ppp_iface_info *iface_info);
void bsp_ppp_ndis_get_mac_header(struct ppp_ndis_field_info *field_info, struct net_mac_header *mac_header);
void bsp_ppp_ndis_get_netdev_cfg(unsigned char chan_type, struct net_dev_cfg *netdev_cfg);
unsigned int bsp_ppp_ndis_get_ready_state(unsigned int dev_type, unsigned int dev_id);
int bsp_ppp_nids_get_efuse_mac_addr(unsigned char *mac_addr);
#else
static inline void bsp_set_ndis_iface_info(struct ppp_ndis_field_info *field_info, struct net_iface_info *iface_info)
{
    return;
}
static inline void bsp_set_ppp_iface_info(struct ppp_ndis_field_info *field_info, struct ppp_iface_info *iface_info)
{
    return;
}
static inline void bsp_ppp_ndis_get_mac_header(struct ppp_ndis_field_info *field_info,
    struct net_mac_header *mac_header)
{
    return;
}
static inline void bsp_ppp_ndis_get_netdev_cfg(unsigned char chan_type, struct net_dev_cfg *netdev_cfg)
{
    return;
}
static inline unsigned int bsp_ppp_ndis_get_ready_state(unsigned int dev_type, unsigned int dev_id)
{
    return 0;
}
static inline int bsp_ppp_nids_get_efuse_mac_addr(unsigned char *mac_addr)
{
    return 0;
}
#endif
#endif // BSP_PPP_NDIS_H
