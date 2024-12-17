/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2018-2020. All rights reserved.
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
#ifndef __WAN_ETH_TABLE_H__
#define __WAN_ETH_TABLE_H__

#include "bsp_pcie.h"
#include "wan_eth_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* _cplusplus */


#define WETH_ALIGN_NUM 128
#define WETH_ALIGN_MASK (~(WETH_ALIGN_NUM - 1))
#define WETH_TAB_SETING_FLAG 0x5a5a5a5a
#define WETH_TABLE_DESC 2
#define WETH_DMA_TABLE_DESC 2
struct weth_tab_usr_field {
    unsigned int usr_field0;
    unsigned int usr_field1;
    unsigned int usr_field2;
    unsigned int skb_len;
};

struct weth_tab_head_info {
    unsigned int total;
    unsigned int cur_num;
};

/* header */
struct weth_tab_element_header {
    struct weth_tab_usr_field usr_field[WETH_TAB_ELEMENT_CNT];
    struct weth_tab_head_info head_info;
};

struct weth_tab_element {
    struct pcie_dma_data_element table_desc[WETH_TABLE_DESC];
    struct pcie_dma_data_element dma_table_desc[WETH_DMA_TABLE_DESC];
    struct weth_tab_element_header header;
    struct pcie_dma_data_element data_desc[WETH_TAB_ELEMENT_CNT];
};

struct weth_tab_addr_node {
    unsigned long long table_desc_phy;
    unsigned long long data_desc_phy;
    unsigned long long usr_field_phy;
};

struct weth_tab_addr_table_ep {
    int is_init;
    struct weth_tab_addr_node tx_table[WETH_RC_RX_TAB_MAX_NUM];
    struct weth_tab_addr_node rx_table[WETH_EP_RX_TAB_MAX_NUM];
};

struct weth_tab_addr_table_rc {
    int is_init;
    struct weth_tab_addr_node tx_table[WETH_EP_RX_TAB_MAX_NUM];
    struct weth_tab_addr_node rx_table[WETH_RC_RX_TAB_MAX_NUM];
};

typedef void (*weth_tab_rx_cb_t)(unsigned int dev_id, struct sk_buff *skb, unsigned int num);
typedef void (*weth_tab_table_sync_finish_cb_t)(void);

int weth_tab_init(struct device *g_dev_for_map);

int weth_tab_get_tx_desc(void **handle, char **desc, char **usrfield, unsigned int *total);
/* config data dma by index (B / A) */
int weth_tab_start_tx_data_dma(void *handle, unsigned int num);
int weth_tab_ep_start_tx_dma(void *handle, unsigned int num, int port_no);
void weth_tab_get_rx_peer_usr_field(void *handle, char **peer_usr_field);

/* config table dma by index (B) */
int weth_tab_start_rx_table_dma(unsigned int idx, unsigned int num);

/* config table dma by index (B) */
void weth_tab_regist_rx_cb(weth_tab_rx_cb_t rx_cb);

void weth_tab_set_addr_to_bar(void *addr);

void weth_tab_get_addr_from_bar(void *addr);

void weth_tab_wait_setting_done(void *addr);

void weth_tab_sync_rx_table_complete(unsigned int idx);

void weth_tab_sync_tx_table_complete(unsigned int idx);

void weth_tab_save_tx_skb(void *handle, unsigned int cnt, struct sk_buff *skb);

int weth_tab_put_tx_desc(unsigned int idx, unsigned int num);

void weth_tab_get_rx_desc_by_idx(unsigned int idx, unsigned int num);

void weth_tab_tx_complete(unsigned int idx, unsigned int num);

int weth_tab_start_rx_data_dma(unsigned int idx, unsigned int num);

int weth_tab_tx_refill_event(unsigned int idx, unsigned int num);

int weth_tab_get_free_tab_num(void);

void weth_tab_exit(void);

void weth_tab_exit(void);

void weth_tab_reinit(void);

int weth_tab_get_status(void);

void weth_tab_regist_table_sync_cb(weth_tab_table_sync_finish_cb_t table_sync_finish_cb);

void weth_tab_clear_local_skb(void *handle, unsigned int cnt);
struct weth_tab_ctx *weth_get_tab_ctx(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __WAN_ETH_TABLE_H__ */
