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
#include <linux/kernel.h>
#include <linux/etherdevice.h>
#include <linux/usb/iot/usb_nv.h>
#include <linux/usb/iot/bsp_unet.h>
#include <osl_types.h>
#include <mdrv_pfa.h>
#include <bsp_pfa.h>
#include <bsp_dra.h>
#include <bsp_ppp_ndis.h>
#include <bsp_efuse.h>
#include <bsp_wan_eth_stick.h>
#include <securec.h>

#include "ppp_ndis_net.h"
#include "ppp_ndis_address.h"
#include "ppp_ndis_adp.h"

#define THIS_MODU mod_ppp_ndis

#define ETHERNET_MAX_ADDR_NUM 30
#define ETHERNET_MAX_ADDR_BYTE 255

struct ppp_ndis_host_mac {
    bool init;
    bool used;
    unsigned char mac_addr[ETH_ALEN];
};

struct ppp_ndis_efuse_mac_format {
    unsigned char mac_addr1[ETH_ALEN];
    unsigned char addr1_continuous_num;
    unsigned char mac_addr2[ETH_ALEN];
    unsigned char addr2_continuous_num;
    unsigned char reserve1;
    unsigned char reserve2;
};

static struct ppp_ndis_host_mac g_efuse_mac[ETHERNET_MAX_ADDR_NUM] = {{0}};
/* initial value, changed by "ifconfig usb0 hw ether xx:xx:xx:xx:xx:xx" */
static char *g_usb_dev_addr[UNET_MAX_NUM] = {
    "0A:5B:8F:27:9A:64",
    "0A:5B:8F:27:9A:65",
    "0A:5B:8F:27:9A:66",
    "0A:5B:8F:27:9A:67",
    "0A:5B:8F:27:9A:68",
    "0A:5B:8F:27:9A:69",
};

/* this address is invisible to ifconfig */
static char *g_usb_host_addr[UNET_MAX_NUM] = {
    "0C:5B:8F:27:9A:64",
    "0C:5B:8F:27:9A:65",
    "0C:5B:8F:27:9A:66",
    "0C:5B:8F:27:9A:67",
    "0C:5B:8F:27:9A:68",
    "0C:5B:8F:27:9A:69",
};

/* initial value, changed by "ifconfig usb0 hw ether xx:xx:xx:xx:xx:xx" */
static char *g_pcie_dev_addr[PNET_MAX_NUM] = {
    "0A:5B:8F:27:9A:70",
    "0A:5B:8F:27:9A:71",
    "0A:5B:8F:27:9A:72",
    "0A:5B:8F:27:9A:73",
    "0A:5B:8F:27:9A:74",
    "0A:5B:8F:27:9A:75",
    "0A:5B:8F:27:9A:76",
    "0A:5B:8F:27:9A:77",
};

/* this address is invisible to ifconfig */
static char *g_pcie_host_addr[PNET_MAX_NUM] = {
    "0C:5B:8F:27:9A:70",
    "0C:5B:8F:27:9A:71",
    "0C:5B:8F:27:9A:72",
    "0C:5B:8F:27:9A:73",
    "0C:5B:8F:27:9A:74",
    "0C:5B:8F:27:9A:75",
    "0C:5B:8F:27:9A:76",
    "0C:5B:8F:27:9A:77",
};

static char *g_usb_gw_addr[UNET_MAX_NUM] = {
    "00:11:09:64:01:01",
    "00:11:09:64:01:02",
    "00:11:09:64:01:03",
    "00:11:09:64:01:04",
    "00:11:09:64:01:05",
    "00:11:09:64:01:06",
};

static char *g_pcie_gw_addr[PNET_MAX_NUM] = {
    "00:11:09:64:01:07",
    "00:11:09:64:01:08",
    "00:11:09:64:01:09",
    "00:11:09:64:01:0A",
    "00:11:09:64:01:0B",
    "00:11:09:64:01:0C",
    "00:11:09:64:01:0D",
    "00:11:09:64:01:0E",
};

static int ppp_ndis_get_addr(const char *str, unsigned char *dev_addr)
{
    if (str) {
        unsigned i;
        int ret = 0;

        for (i = 0; i < ETH_ALEN; i++) {
            unsigned char num = 0;

            if ((*str == '.') || (*str == ':')) {
                str++;
            }
            ret = hex_to_bin(*str++);
            if (ret >= 0)
                num = (u32)ret << 4; /* 4 means left shifting four bits */
            ret = hex_to_bin(*str++);
            if (ret >= 0)
                num |= (u32)ret;
            dev_addr[i] = num;
        }
        if (is_valid_ether_addr(dev_addr)) {
            return 0;
        }
    }
    ppp_ndis_err("ether addr is invalid, use random addr\n");
    eth_random_addr(dev_addr);
    return 0;
}

static int ppp_ndis_get_dev_mac_address(struct ppp_ndis_field_info *field_info, unsigned char *mac_addr)
{
    if (field_info->chan_type == PPP_NDIS_DEV_UNET) {
        return ppp_ndis_get_addr(g_usb_dev_addr[field_info->chan_id], mac_addr);
    } else {
        return ppp_ndis_get_addr(g_pcie_dev_addr[field_info->chan_id], mac_addr);
    }
}

/* 待实现 */
static int ppp_ndis_get_efuse_mac_address(unsigned char *mac_addr)
{
    int ret;
    unsigned int index = 0;

    while (index < ETHERNET_MAX_ADDR_NUM) {
        if (g_efuse_mac[index].init == true && g_efuse_mac[index].used == false) {
            break;
        }
        index++;
    }

    if (index == ETHERNET_MAX_ADDR_NUM) {
        eth_random_addr(mac_addr);
    } else {
        ret = memcpy_s(mac_addr, ETH_ALEN, g_efuse_mac[index].mac_addr, ETH_ALEN);
        if (ret < 0) {
            ppp_ndis_err("memcpy out of size\n");
            return ret;
        }
        g_efuse_mac[index].used = true;
    }

    return 0;
}

static int ppp_ndis_get_constant_mac_address(struct ppp_ndis_field_info *field_info, unsigned char *mac_addr)
{
    if (field_info->chan_type == PPP_NDIS_DEV_UNET) {
        return ppp_ndis_get_addr(g_usb_host_addr[field_info->chan_id], mac_addr);
    } else {
        return ppp_ndis_get_addr(g_pcie_host_addr[field_info->chan_id], mac_addr);
    }
}

static bool ppp_ndis_check_support_5g_lan(struct ppp_ndis_field_info *field_info)
{
    if (field_info->chan_type == PPP_NDIS_DEV_UNET) {
        return bsp_unet_support_5g_lan(field_info->chan_id);
    } else {
        return bsp_pcie_support_5g_lan(field_info->chan_id);
    }
}

static int ppp_ndis_get_host_mac_address(struct ppp_ndis_field_info *field_info, unsigned char *mac_addr)
{
    if (ppp_ndis_check_support_5g_lan(field_info)) {
        return ppp_ndis_get_efuse_mac_address(mac_addr);
    } else {
        return ppp_ndis_get_constant_mac_address(field_info, mac_addr);
    }
}

static int ppp_ndis_get_gw_mac_address(struct ppp_ndis_field_info *field_info, unsigned char *mac_addr)
{
    if (field_info->chan_type == PPP_NDIS_DEV_UNET) {
        return ppp_ndis_get_addr(g_usb_gw_addr[field_info->chan_id], mac_addr);
    } else {
        return ppp_ndis_get_addr(g_pcie_gw_addr[field_info->chan_id], mac_addr);
    }
}

int ppp_ndis_get_mac_address(struct ppp_ndis_mac_index *index_info, unsigned char *mac_addr)
{
    int ret;

    if (index_info->field_info.chan_type != PPP_NDIS_DEV_UNET &&
        index_info->field_info.chan_type != PPP_NDIS_DEV_PNET) {
        ppp_ndis_err("[get_mac_addr]ppp_ndis card type error\n");
        return -1;
    }

    switch (index_info->addr_type) {
        case DEV_ADDR_TYPE:
            ret = ppp_ndis_get_dev_mac_address(&index_info->field_info, mac_addr);
            break;
        case HOST_ADDR_TYPE:
            ret = ppp_ndis_get_host_mac_address(&index_info->field_info, mac_addr);
            break;
        case GW_ADDR_TYPE:
            ret = ppp_ndis_get_gw_mac_address(&index_info->field_info, mac_addr);
            break;
        default:
            ppp_ndis_err("ppp_ndis addr type error\n");
            ret = -1;
    }
    return ret;
}

void bsp_ppp_ndis_get_mac_header(struct ppp_ndis_field_info *field_info, struct net_mac_header *mac_header)
{
    if (field_info->chan_type != PPP_NDIS_DEV_UNET && field_info->chan_type != PPP_NDIS_DEV_PNET) {
        ppp_ndis_err("[get_mac_header]ppp_ndis card type error\n");
        return;
    }

    if ((field_info->chan_type == PPP_NDIS_DEV_UNET && field_info->chan_id >= UNET_MAX_NUM) ||
        (field_info->chan_type == PPP_NDIS_DEV_PNET && field_info->chan_id >= PNET_MAX_NUM)) {
        ppp_ndis_err("channel type: %u, channel id: %u out of range\n", field_info->chan_type, field_info->chan_id);
        return;
    }

    ppp_ndis_get_host_mac_address(field_info, &mac_header->host_mac[0]);
    ppp_ndis_get_gw_mac_address(field_info, &mac_header->gw_mac[0]);
    return;
}

unsigned char ppp_ndis_mac_addr_copy(unsigned char index, unsigned char *src_addr, unsigned char addr_num)
{
    int ret;
    unsigned char i;

    for (i = 0; i < addr_num && index < ETHERNET_MAX_ADDR_NUM; i++) {
        ret = memcpy_s(g_efuse_mac[index].mac_addr, ETH_ALEN - 1, src_addr, ETH_ALEN - 1);
        if (ret != 0) {
            ppp_ndis_err("memcpy fail\n");
            break;
        }
        if (src_addr[ETH_ALEN - 1] > ETHERNET_MAX_ADDR_BYTE - i) {
            ppp_ndis_err("mac addr last byte overflow 255\n");
            break;
        }
        g_efuse_mac[index].mac_addr[ETH_ALEN - 1] = src_addr[ETH_ALEN - 1] + i;
        g_efuse_mac[index].init = true;
        index++;
    }
    return index;
}

void ppp_ndis_init_efuse_mac_address(void)
{
    int ret;
    unsigned char index;
    struct ppp_ndis_efuse_mac_format efuse_addr_data = {0};
    efuse_layout_s layout_info = {0};

    ret = bsp_efuse_get_layout_info(EFUSE_DRV_5G_LAN_MAC_ADDR, &layout_info);
    if (ret) {
        ppp_ndis_err("get efuse mac_addr layout_info fail\n");
        return;
    }

    ret = bsp_efuse_read((u32*)&efuse_addr_data, GROUP_INDEX(layout_info.bit), GROUP_LEN(layout_info.bit_len));
    if (ret < 0) {
        ppp_ndis_err("read efuse data fail\n");
        return;
    }
    index = ppp_ndis_mac_addr_copy(0, efuse_addr_data.mac_addr1, efuse_addr_data.addr1_continuous_num);
    index = ppp_ndis_mac_addr_copy(index, efuse_addr_data.mac_addr2, efuse_addr_data.addr2_continuous_num);
}

int bsp_ppp_nids_get_efuse_mac_addr(unsigned char *mac_addr)
{
    if (mac_addr == NULL) {
        ppp_ndis_err("param mac_addr is null\n");
        return -EINVAL;
    }
    return ppp_ndis_get_efuse_mac_address(mac_addr);
}

void ppp_ndis_mac_address_init(void)
{
    bsp_unet_registe_get_mac_cb(ppp_ndis_get_mac_address); /* usb unet */
    bsp_pnet_registe_get_mac_cb(ppp_ndis_get_mac_address);
    ppp_ndis_init_efuse_mac_address();
}
