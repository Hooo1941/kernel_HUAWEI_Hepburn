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

#ifndef RNIC_VNET_DPL_H
#define RNIC_VNET_DPL_H

#include <linux/device.h>
#include <linux/if_ether.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RNIC_VNET_DPL_TX_MAX 1
#define RNIC_VNET_DPL_RX_MAX 1024
#define RNIC_VNET_DPL_CACHE_LEN 128

struct rnic_dpl_mac_info_s {
	u8 dest[ETH_ALEN]; /* destination eth addr */
	u8 source[ETH_ALEN]; /* source ether addr */
	u16 vlan_tci; /* priority and VLAN ID */
	u16 vlan_encap_proto; /* packet type ID field */
};

struct rnic_dpl_ip_info_s {
	u8 ip_ver; /* IP version */
	u8 l4_proto; /* L4 protocol: ICMP,TCP,UDP */
	u8 icmp_type; /* ICMP type */
	u8 rsv0; /* rsv */

	u16 data_len; /* IP packet total length */
	u16 ip4_id; /* IPv4 id */
	u16 src_port; /* IP src port */
	u16 dst_port; /* IP dst port */
	u16 payload_len; /* L4 datagram payload length */
	u16 rsv1; /* rsv */

	u32 l4_id; /* L4 id: ICMP IDENTIFY+SN, TCP SEQ */
	u32 tcp_ack_seq; /* TCP ACK SEQ */
};

struct rnic_dpl_priv_info_s {
	u32 tft_value; /* TFT */
	u32 param0; /* self-defined parameter0 */
	u32 param1; /* self-defined parameter1 */
	u32 param2; /* self-defined parameter2 */
};

struct rnic_dpl_pkt_info_s {
	struct rnic_dpl_mac_info_s mac_info;
	struct rnic_dpl_ip_info_s ip_info;
	struct rnic_dpl_priv_info_s priv_info;
};

struct rnic_vnet_dpl_tx_s {
	u8 ver;
	u8 rsv;
	u16 pkt_num;
	struct rnic_dpl_pkt_info_s pkt_infos[RNIC_VNET_DPL_TX_MAX];
};

struct rnic_vnet_dpl_rx_s {
	u8 ver;
	u8 pfa_ver;
	u16 pkt_num;
	struct rnic_dpl_pkt_info_s pkt_infos[RNIC_VNET_DPL_RX_MAX];
};

struct rnic_vnet_dpl_s {
	struct device *dev;
	u32 rd_ts;
	struct rnic_vnet_dpl_tx_s tx;
	struct rnic_vnet_dpl_rx_s rx;
};

#ifdef __cplusplus
}
#endif

#endif /* RNIC_VNET_DPL_H */
