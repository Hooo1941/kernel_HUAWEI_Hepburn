/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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
#ifndef RELAY_NET_H
#define RELAY_NET_H

#include <osl_types.h>
#include <mdrv_udi.h>
#include <bsp_relay.h>
#include "relay_def.h"

struct relay_net_func {
    usb_ndis_at_recv_func at_recv_cb;
    usb_ndis_stus_chg_func stus_chg_cb;
};

struct vdev_net_cbs {
    relay_enable_cb enable_cb;
    relay_disable_cb disable_cb;
    relay_net_read_cb net_read_cb;
};

struct relay_net_stat {
    unsigned int at_recv_done;
    unsigned int at_up_fail;
    unsigned int no_read_cb;
    unsigned int at_recv_len;
    unsigned int recv_cb_shed;
    unsigned int conn_chg_num;
    unsigned int conn_chg_fail;
    unsigned int up_cfg_fail;
    unsigned int write_cfg_fail;

    unsigned int droped;
};

struct relay_net_device {
    struct relay_adp_device adp_dev;
    struct relay_net_stat stat;
    struct relay_net_func funcs;
    struct vdev_net_cbs cbs;
};

enum relay_netif_main_cmd {
    NETIF_SET_CMD,
    NETIF_QUERY_CMD,
    NETIF_MAX_CMD,
};

enum relay_netif_set_cmd {
    SET_SPEED_CHANGE, /**< 网卡通道命令字：设置连接速率 */
    SET_CONNECT_STATE, /**< 网卡通道命令字：设置连接状态 */
    SET_IFACE_INFO, /**< 网卡通道命令字：设置iface信息 */
};

enum relay_netif_query_cmd {
    QUERY_NETDEV_CFG, /**< 网卡通道命令字：查询网卡驱动配置项 */
    QUERY_MAC_HEADER, /**< 网卡通道命令字：查询MAC地址 */
};

#define RELAY_BUILD_NETIF_CMD(main_cmd, minor_cmd) (((main_cmd) << 8) | ((minor_cmd) & 0x00ff))

/**
* @brief 网卡接口命令字
*/
enum relay_netif_ioctl_cmd {
    NETIF_SET_SPEED_CHANGE = RELAY_BUILD_NETIF_CMD(NETIF_SET_CMD, SET_SPEED_CHANGE),
    NETIF_SET_CONNECT_STATE = RELAY_BUILD_NETIF_CMD(NETIF_SET_CMD, SET_CONNECT_STATE),
    NETIF_SET_IFACE_INFO = RELAY_BUILD_NETIF_CMD(NETIF_SET_CMD, SET_IFACE_INFO),
    NETIF_QUERY_NETDEV_CFG = RELAY_BUILD_NETIF_CMD(NETIF_QUERY_CMD, QUERY_NETDEV_CFG),
    NETIF_QUERY_MAC_HEADER = RELAY_BUILD_NETIF_CMD(NETIF_QUERY_CMD, QUERY_MAC_HEADER),
    NETIF_IOCTL_MAX_CMD = 0xffff,
};

int relay_net_dev_open(unsigned int minor_type);
int relay_net_dev_close(unsigned int minor_type);
int relay_net_dev_write(unsigned int minor_type, char *addr, unsigned int size);
int relay_net_dev_ioctl(unsigned int minor_type, unsigned int cmd, void *para, unsigned int para_size);
void relay_net_enable(unsigned int minor_type);
void relay_net_disable(unsigned int minor_type);
int relay_net_reg_enablecb(unsigned int minor_type, relay_enable_cb func, void *private);
int relay_net_reg_disablecb(unsigned int minor_type, relay_disable_cb func);
int relay_net_init(void);
#endif /* RELAY_NET_H */