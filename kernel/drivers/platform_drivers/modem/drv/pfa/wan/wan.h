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

#ifndef __WAN_H__
#define __WAN_H__
#include <linux/netdevice.h>
#include <bsp_print.h>
#include <mdrv_pfa.h>
#include <bsp_net_om.h>

#define PFA_WAN_BD_SIZE 32
#define ETH_PKT_LEN_MAX 1518
#define PACKET_HEAD_LEN (16)
#define WAN_CB_HANDLE_MAX 64

struct wan_dev_entry {
    struct wan_dev_info_s info;
    struct list_head list;
};

struct wan_private {
    struct napi_struct napi;
    spinlock_t poll_lock;
    int polling;
    unsigned int evt_rd_done;
    unsigned int evt_td_done;
};

struct wan_debug_info_s {
    unsigned int dbg_pfa_port;
    unsigned int dbg_water_level;
    unsigned int dbg_rx_finish_rd_cnt;
    unsigned int dbg_rx_drop_cnt;
    unsigned int dbg_rx_to_wan_cnt;
    unsigned int dbg_rx_to_net_cnt;
    unsigned int dbg_rx_complete_intr_cnt;
    unsigned int dbg_in_rd;
    unsigned int dbg_rx_wan_nr_drop_stub;

    unsigned int dbg_tx_xmit_cnt;
    unsigned int dbg_tx_skb_null;
    unsigned int dbg_tx_map_fail;
    unsigned int dbg_tx_config_pfa_succ;
    unsigned int dbg_tx_config_pfa_fail;
    unsigned int dbg_tx_finish_td_cnt;
    unsigned int dbg_tx_pkt_len_out_bound;
};

struct wan_ctx_s {
    struct device *dev;
    struct net_device *ndev;
    struct wan_private *priv;
    struct wan_callback_s cb[WAN_CB_HANDLE_MAX];
    unsigned int in_rd[WAN_CB_HANDLE_MAX];
    wan_get_handle_cb get_handle_cb;

    int pfa_port;
    unsigned int water_level;

    unsigned int td_depth;
    unsigned int rd_depth;

    struct list_head dev_list;
    struct net_om_info report_info;
    struct wan_debug_info_s hds_info;

    unsigned int rx_finish_rd_cnt;
    unsigned int rx_drop_cnt;
    unsigned int rx_to_wan_cnt;
    unsigned int rx_to_net_cnt;
    unsigned int rx_complete_intr_cnt;
    unsigned int rx_wan_nr_drop_stub;

    unsigned int tx_xmit_cnt;
    unsigned int tx_skb_null;
    unsigned int tx_map_fail;
    unsigned int tx_config_pfa_succ;
    unsigned int tx_config_pfa_fail;
    unsigned int tx_finish_td_cnt;
    unsigned int tx_pkt_len_out_bound;
    unsigned int tx_skb_mac_len_err;
    unsigned int rx_stick_cnt;
    unsigned int rx_free;
    unsigned int cpu0_stub_en;
};

struct lan_callback_s {
    int (*lan_rx)(struct sk_buff* skb);
    void (*rx_complete)(void);
};

struct lan_ctx_s {
    struct net_device *ndev_init;
    struct net_device *ndev;
    struct lan_callback_s cb;

    int pfa_port;

    unsigned int ndev_null;
    unsigned int skb_null;
    unsigned int rx_finish_rd_cnt;
    unsigned int rx_drop_cnt;
    unsigned int rx_to_lan_cnt;
    unsigned int rx_to_net_cnt;
    unsigned int rx_complete_intr_cnt;
    unsigned int in_rd;
    unsigned int rx_lan_nr_drop_stub;

    unsigned int tx_xmit_cnt;
    unsigned int tx_skb_null;
    unsigned int tx_map_fail;
    unsigned int tx_config_pfa_succ;
    unsigned int tx_config_pfa_fail;
    unsigned int tx_finish_td_cnt;
    unsigned int tx_pkt_len_out_bound;
};

struct pfa_bd_s {
    unsigned int int_en : 1;
    unsigned int mode : 2;
    unsigned int cd_en : 1;
    unsigned int reserved : 2;
    unsigned int with_ip_header : 2;
    unsigned int fc_head : 4;
    unsigned int irq_en : 1;
    unsigned int push_en : 1;
    unsigned int dl_speceal_ind : 1;
    unsigned int high_pri_flag : 1;
    unsigned int pkt_len : 16;

    unsigned long long input_pointer;

    unsigned int pdu_session_id : 8;
    unsigned int reserved2 : 8;
    unsigned int modem_id : 2;
    unsigned int reserved3 : 14;

    unsigned int bypass : 1;
    unsigned int bypass_dst : 4;
    unsigned int eth_analysis_ind : 1;
    unsigned int drop_ack_ind : 1;
    unsigned int ips_id : 8;
    unsigned int reserved4 : 17;

    unsigned int stream_id;

    unsigned int td_info;

    unsigned int user_field0;

    unsigned int user_field1;

    unsigned int user_field2;

    unsigned int pfa_tft_packet_head[PACKET_HEAD_LEN];
};

struct pfa_rd_s {
    unsigned int int_en : 1;
    unsigned int mode : 2;
    unsigned int cd_en : 1;
    unsigned int reserved : 2;
    unsigned int with_ip_header : 2;
    unsigned int fc_head : 4;
    unsigned int irq_en : 1;
    unsigned int push_en : 1;
    unsigned int dl_speceal_ind : 1;
    unsigned int high_pri_flag : 1;
    unsigned int pkt_len : 16;

    unsigned long long output_pointer;

    unsigned int pdu_session_id : 8;
    unsigned int reserved2 : 8;
    unsigned int modem_id : 2;
    unsigned int reserved3 : 14;

    unsigned int bypass : 1;
    unsigned int bypass_dst : 4;
    unsigned int eth_analysis_ind : 1;
    unsigned int drop_ack_ind : 1;
    unsigned int ips_id : 8;
    unsigned int reserved4 : 17;

    unsigned int stream_id;

    unsigned int pfa_tft_result;

    unsigned int user_field0;

    unsigned int user_field1;

    unsigned int user_field2;

    unsigned int pfa_tft_packet_head[PACKET_HEAD_LEN];
};

int wan_init(struct device *dev);
int lan_init(void);
int wan_rd_drop_en(int drop_stub_en);
void wan_set_cpu0_stub_en(unsigned int en);
int wan_info_dump(void);
int wan_dev_dump(void);
void wan_speed(void);
int wan_help(void);
int lan_info_dump(void);
int lan_help(void);
void wan_port_config_init(void);
extern struct wan_ctx_s g_wan_ctx;
#endif
