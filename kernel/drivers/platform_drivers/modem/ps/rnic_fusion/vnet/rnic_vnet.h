/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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
 */

#ifndef RNIC_VNET_H
#define RNIC_VNET_H

#include <linux/etherdevice.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/if_link.h>
#include <linux/if_vlan.h>
#include "product_config_ps_ap.h"
#include "mdrv_pfa.h"
#include "rnic_data.h"
#include "rnic_client.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CONFIG_RNIC_CUST_NAME
#define RNIC_VNET_NAME	"eth_x"
#else
#define RNIC_VNET_NAME	"rmnet"
#endif

#define RNIC_ETH_PARSE_CHK_LEN	54
#define RNIC_VLAN_PARSE_CHK_LEN	58
#define RNIC_MAX_FIX_SNDBUF	524288

enum rnic_vnet_state_e {
	RNIC_VNET_STATE_RESETTING,
	RNIC_VNET_STATE_DOWN,
	RNIC_VNET_STATE_NOCARRIER,
};

struct rnic_vnet_tx_stats_s {
	u64 tx_bytes;
	u64 tx_pkts;
	u32 tx_linear_errs;
	u32 tx_csum_errs;
	u32 tx_len_errs;
	u32 tx_link_errs;
	u32 tx_carr_errs;
	u32 tx_clone_errors;
};

struct rnic_vnet_rx_stats_s {
	u64 rx_bytes;
	u64 rx_pkts;
	u32 rx_len_errs;
	u32 rx_link_errs;
	u32 rx_carr_errs;
	u32 rx_queue_errs;
	u32 rx_maa_errs;
};

struct rnic_vnet_stats_s {
	struct rnic_vnet_tx_stats_s tx;
	struct rnic_vnet_rx_stats_s rx;
};

struct rnic_vnet_pkt_stats_rpt_s {
	struct {
		u8 ver;
		u8 id;
		u8 reserved[6];
	} hdr;
	struct rnic_vnet_stats_s stats;
};

struct rnic_vnet_ctx_s;

struct rnic_vnet_priv_s {
	struct rnic_handle_s *handle;
	struct net_device *netdev;
	struct device *dev;

	struct rnic_vnet_ctx_s *back;
	struct rnic_vnet_stats_s stats;

	unsigned long vnet_state;
	unsigned long link_state;
	unsigned int fw_mode;
	int port_id;
	struct mutex mutex;

	u32 pif_id;
	u32 vnet_id;
	u32 drop_code;
	bool frag_en;
	bool copy_en;

	struct pfa_tft_info_s tft_infos[RNIC_ADDR_MAX];
	int (*fill_desc)(struct rnic_vnet_priv_s *priv, struct sk_buff *skb);
};

struct rnic_srs_ops_s {
	void (*set_feature_cfg)(const void *);
	void (*rx_skb_list)(struct sk_buff_head *);
	void (*rpt_stats)(void);
	void (*disable_gro)(u32);
	void (*boost_cpu)(u32);
};

struct rnic_vnet_ctx_s {
#ifdef CONFIG_RNIC_LAN_FORWARD
	struct net_device *vlan_netdevs[VLAN_N_VID];
#endif
	struct rnic_vnet_priv_s *privs[RNIC_MAX_PIF_NUM];
	struct rnic_handle_s *handle;
	struct sk_buff_head rx_list;
	const struct rnic_srs_ops_s *srs_ops;
};

struct rnic_vnet_priv_s *rnic_vnet_get_priv(u32 vnet_id);
int rnic_vnet_set_features(struct net_device *dev, netdev_features_t features);

#ifdef __cplusplus
}
#endif

#endif /* RNIC_VNET_H */
