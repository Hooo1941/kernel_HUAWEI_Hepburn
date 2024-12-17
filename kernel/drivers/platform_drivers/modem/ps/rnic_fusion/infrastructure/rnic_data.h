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

#ifndef RNIC_DATA_H
#define RNIC_DATA_H

#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include "product_config_ps_ap.h"
#include "ps_iface_global_def.h"
#include "mdrv_pfa.h"
#include "mdrv_dra.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RNIC_DEFAULT_MTU	ETH_DATA_LEN
#define RNIC_MIN_MTU		68
#define RNIC_MAX_MTU		2000
#define RNIC_RX_MAX_LEN 	1536
#define RNIC_VLAN_ID_MAX	4095

#define RNIC_LL_RES_SPACE	ETH_HLEN

#define RNIC_IPVER_4		0x40
#define RNIC_IPVER_6		0x60
#define RNIC_IPVER_MASK		0xF0

#define RNIC_INVALID_SESSID	0xFF
#define RNIC_INVALID_FCHEAD	0xF
#define RNIC_INVALID_MODEMID	0x3

#define RNIC_MAX_PIF_NUM	64
#define RNIC_PIFID_OFFSET	PS_IFACE_ID_RMNET_DATA_BEGIN

#define RNIC_MAP_VLAN_TCI_MASK	0x0000FFFF

#define RNIC_MAP_PKTTYPE_IP	0x00
#define RNIC_MAP_PKTTYPE_ETH	0x40
#define RNIC_MAP_PKTTYPE_VLAN	0x80
#define RNIC_MAP_PKTTYPE_MASK	0x000000C0
#define RNIC_MAP_PIFID_MASK	0x0000003F
#define RNIC_MAP_MULTIPIF_MASK	0x08000000
#define RNIC_MAP_NETID_MASK	0x000000FF
#define RNIC_MAP_FIELD1_MASK	0x40000000
#define RNIC_MAP_FIELD2_MASK	0x80000000

#define RNIC_WAN_PKTTYPE_IP	0x00
#define RNIC_WAN_PKTTYPE_ETH	0x40
#define RNIC_WAN_PKTTYPE_VLAN	0x80
#define RNIC_WAN_PKTTYPE_MASK	0x000000C0
#define RNIC_WAN_PIFID_MASK	0x0000003F
#define RNIC_WAN_FIELD1_MASK	0x40000000
#define RNIC_WAN_FIELD2_MASK	0x80000000
#define RNIC_WAN_DROP_MASK	0x0000FF00
#define RNIC_WAN_DROP_SHIFT	8
#define RNIC_WAN_PRI_MASK	0x00030000
#define RNIC_WAN_PRI_SHIFT	16

#define RNIC_MEMF_CB_POS	44
#define RNIC_MEMF_LAN		(WAN_DRA_OWN | WAN_PFA_L2ADDR_FROM_CB)

#define RNIC_VNET_DATA_PIFMASK	GENMASK_ULL(PS_IFACE_ID_RMNET_DATA_END, \
					    PS_IFACE_ID_RMNET_DATA_BEGIN)
#ifdef CONFIG_RNIC_IMS
#define RNIC_VNET_IMS_PIFMASK	GENMASK_ULL(PS_IFACE_ID_RMNET_IMS_END, \
					    PS_IFACE_ID_RMNET_IMS_BEGIN)
#define RNIC_VNET_IWLAN_PIFMASK	GENMASK_ULL(PS_IFACE_ID_RMNET_R_IMS_END, \
					    PS_IFACE_ID_RMNET_R_IMS_BEGIN)
#else
#define RNIC_VNET_IMS_PIFMASK	0
#define RNIC_VNET_IWLAN_PIFMASK	0
#endif

#define RNIC_VNET_D2D_PIFMASK	BIT_ULL(PS_IFACE_ID_RMNET_D2D)

#define RNIC_VNET_MBS_PIFMASK	BIT_ULL(PS_IFACE_ID_RMNET_MBS)

#define RNIC_VNET_PIF_MASK	(RNIC_VNET_DATA_PIFMASK | RNIC_VNET_IMS_PIFMASK | \
				 RNIC_VNET_IWLAN_PIFMASK | RNIC_VNET_D2D_PIFMASK | \
				 RNIC_VNET_MBS_PIFMASK)
#define RNIC_VNET_FRAG_MASK	(RNIC_VNET_IWLAN_PIFMASK | RNIC_VNET_D2D_PIFMASK)

#define RNIC_VNET_PTS_PKT_NO_OFFSET	4
#define RNIC_VNET_PTS_MOD_ID		5

enum rnic_addr_type_e {
	RNIC_ADDR_IPV4 = 0,
	RNIC_ADDR_IPV6,
	RNIC_ADDR_ETH,
	RNIC_ADDR_MBS,
	RNIC_ADDR_MAX
};

#define RNIC_ADDRF_IPV4		(1UL << RNIC_ADDR_IPV4)
#define RNIC_ADDRF_IPV6		(1UL << RNIC_ADDR_IPV6)
#define RNIC_ADDRF_ETH		(1UL << RNIC_ADDR_ETH)
#define RNIC_ADDRF_MBS		(1UL << RNIC_ADDR_MBS)
#define RNIC_ADDRF_ALL		(RNIC_ADDRF_IPV4 | RNIC_ADDRF_IPV6 | \
				 RNIC_ADDRF_ETH | RNIC_ADDRF_MBS)

#define RNIC_MS_TO_TIME_STAMP(a, b) (((a) * (b)) / 1000)
#define RNIC_TIME_STAMP_MAX 0xFFFFFFFF
#define RNIC_MODEM_ID_MAX 3
#define RNIC_PDUSESSION_ID_MAX 16

#define RNIC_IPF_ERR_MASK_V0 0x00017800
#define RNIC_IPF_ERR_MASK_V100 0x0002F000

#define RNIC_SPE_L3_PORTOCOL_ERR 0x06
#define RNIC_SPE_L3_IPHDR_CHECK_ERR 0x07
#define RNIC_SPE_IP_IPHDR_LEN_ERR 0x09

enum rnic_rx_result_e {
	RNIC_RX_CONSUMED,
	RNIC_RX_PASS
};

/*
 * [last, first]:[0, 1] First fragment; [1, 0] Last Fragment;
 *               [0, 0] Middle Fragment; [1, 1] No Fragment.
 */
#pragma pack(push, 1)
struct rnic_frag_info_s {
	union {
		struct {
			u8 last     : 1;
			u8 first    : 1;
			u8 seq      : 3;
			u8 reserved : 3;
		};
		u8 value;
	};
};
#pragma pack(pop)

struct rnic_field0_desc_s {
	union {
		struct {
			u32 pif_id    : 6;
			u32 pkt_type  : 2;
			u32 drop_code : 8;
			u32 priority  : 2;
			u32 reserved  : 9;
			u32 multi_pif : 1;
			u32 field_res : 2;
			u32 field1_en : 1;
			u32 field2_en : 1;
		};
		u32 field_value;
	};
};

struct rnic_field1_desc_s {
	union {
		struct {
			u8 frag_value;
			u8 rsv[3];
		};
		u32 field_value;
	};
};

struct rnic_field2_desc_s {
	u32 field_value;
};

#define rnic_map_skb_cb(skb) \
	((struct rx_cb_map_s *)&((skb)->cb[0]))

#define rnic_map_pifid(skb) \
	(rnic_map_skb_cb(skb)->userfield0 & RNIC_MAP_PIFID_MASK)

#define rnic_map_pkttype(skb) \
	(rnic_map_skb_cb(skb)->userfield0 & RNIC_MAP_PKTTYPE_MASK)

#define rnic_map_multi_pif(skb) \
	(rnic_map_skb_cb(skb)->userfield0 & RNIC_MAP_MULTIPIF_MASK)

#define rnic_map_netid(skb) \
	(rnic_map_skb_cb(skb)->userfield2 & RNIC_MAP_NETID_MASK)

#define rnic_map_pktid(skb) \
	(rnic_map_skb_cb(skb)->pkt_id)

#define rnic_map_field0_cb(skb) \
	((struct rnic_field0_desc_s *)&(rnic_map_skb_cb(skb)->userfield0))

#define rnic_map_field1_cb(skb) \
	((struct rnic_field1_desc_s *)&(rnic_map_skb_cb(skb)->userfield1))

#define rnic_map_field2_cb(skb) \
	((struct rnic_field2_desc_s *)&(rnic_map_skb_cb(skb)->userfield2))

#define rnic_map_frag_cb(skb) \
	((struct rnic_frag_info_s *)&(rnic_map_field1_cb(skb)->frag_value))

#define rnic_wan_skb_cb(skb) \
	((struct wan_info_s *)&((skb)->cb[0]))

#define rnic_wan_pifid(skb) \
	(rnic_wan_skb_cb(skb)->userfield0 & RNIC_WAN_PIFID_MASK)

#define rnic_wan_pkttype(skb) \
	(rnic_wan_skb_cb(skb)->userfield0 & RNIC_WAN_PKTTYPE_MASK)

#define rnic_wan_field0_cb(skb) \
	((struct rnic_field0_desc_s *)&(rnic_wan_skb_cb(skb)->userfield0))

#define rnic_wan_field1_cb(skb) \
	((struct rnic_field1_desc_s *)&(rnic_wan_skb_cb(skb)->userfield1))

#define rnic_wan_field2_cb(skb) \
	((struct rnic_field2_desc_s *)&(rnic_wan_skb_cb(skb)->userfield2))

#define rnic_wan_frag_cb(skb) \
	((struct rnic_frag_info_s *)&(rnic_wan_field1_cb(skb)->frag_value))

static inline u8 rnic_mem_flag(const struct sk_buff *skb)
{
	return (u8)skb->cb[RNIC_MEMF_CB_POS];
}

static inline bool rnic_mem_flag_is_set(const struct sk_buff *skb)
{
	return skb->cb[RNIC_MEMF_CB_POS] != 0;
}

static inline void rnic_set_mem_flag(struct sk_buff *skb, u8 flag)
{
	skb->cb[RNIC_MEMF_CB_POS] = flag;
}

static inline void rnic_kfree_skb(struct sk_buff *skb)
{
	if (rnic_mem_flag_is_set(skb))
		mdrv_dra_skb_free(skb);
	else
		dev_kfree_skb_any(skb);
}

static inline unsigned int rnic_caculate_gap_time(unsigned int now_time,
						  unsigned int pre_time)
{
	return (now_time >= pre_time) ? now_time - pre_time :
		RNIC_TIME_STAMP_MAX - pre_time + now_time + 1;
}

#ifdef __cplusplus
}
#endif

#endif /* RNIC_DATA_H */
